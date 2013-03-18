#include "parser.hpp"
#include <memory>
#include <iostream>

using namespace std;
namespace lang {
	enum TokenType {
		NONE,
		ID,
		INTEGER,
		EQUALS,
		WHITESPACE,
		SEMICOLON
	};

	template <class T> shared_ptr<T> smartify(T* ptr) {
		return shared_ptr<T>(ptr);
	}

	struct Token {
		TokenType type;
		shared_ptr<string> text;
		Token(shared_ptr<string> text, TokenType type) : type(type), text(text) {
		}
		Token() : type(NONE), text(new string()) {
		}
		~Token() {
		}
	};

	template <class T> struct Iterator {
		virtual T value() = 0;
		virtual bool hasValue() = 0;
		virtual shared_ptr<Iterator<T> > next() = 0;
		virtual size_t getPosition() = 0;
	};
	template <typename T>
	using IteratorP = shared_ptr<Iterator<T> >;

	template <typename T, typename SeqT>
	struct SeqIterator : Iterator<T> {
		SeqIterator(const SeqT& seq) {
			this->position = 0;
			this->seq = shared_ptr<SeqT>(new SeqT(seq));
		}

		SeqIterator(shared_ptr<SeqT> seq) {
			this->position = 0;
			this->seq = seq;
		}

		virtual T value() {
			if (!hasValue()) {
				return T();
			}
			return (*seq)[position];
		}

		virtual bool hasValue() {
			return position < seq->size();
		}

		virtual shared_ptr<Iterator<T> > next() {
			return shared_ptr<Iterator<T> > (new SeqIterator<T, SeqT>(position + 1, seq));
		}

		virtual size_t getPosition() {
			return this->position;
		}

	private:
		size_t position;
		shared_ptr<SeqT> seq;

		SeqIterator(size_t pos, shared_ptr<SeqT> seq) {
			this->position = pos;
			this->seq = seq;
		}
	};

	typedef SeqIterator<char, string> StringIterator;
	typedef SeqIterator<Token, vector<Token> > TokenIterator;

	template <class IteratorT, class T> struct ParseResult {
		ParseResult(shared_ptr<Iterator<IteratorT> > iterator, T value) : iterator(iterator) {
			this->success = true;
			this->value = value;
		}
		ParseResult(shared_ptr<Iterator<IteratorT> > iterator) : iterator(iterator) {
			this->success = false;
		}

		ParseResult operator||(const ParseResult& other) {
			if (this->success) {
				return (*this);
			} else {
				return other;
			}
		}

		shared_ptr<Iterator<IteratorT> > iterator;
		bool success;
		T value;
	};

	template <class InT, class OutT>
	struct Parser {
		virtual ParseResult<InT, OutT> parse(shared_ptr<Iterator<InT> > it) = 0;
	};

	struct CharParser : Parser<char, char> {
		char character;
		CharParser(char character) {
			this->character = character;
		}

		virtual ParseResult<char, char> parse(shared_ptr<Iterator<char> > it) {
			if (!it->hasValue() || it->value() != character) {
				return ParseResult<char, char>(it);
			}
			return ParseResult<char, char>(it->next(), this->character);
		}
	};

	template <class InT, class OutT>
	struct OrParser : Parser<InT, OutT> {
		shared_ptr<vector<shared_ptr<Parser<InT, OutT> > > > parsers;
		OrParser(shared_ptr<vector<shared_ptr<Parser<InT, OutT> > > > parsers) {
			this->parsers = parsers;
		}

		OrParser(shared_ptr<Parser<InT, OutT> > a, shared_ptr<Parser<InT, OutT> > b) : parsers(new vector<shared_ptr<Parser<InT, OutT> > >) {
			this->parsers.push_back(a);
			this->parsers.push_back(b);
		}

		virtual ParseResult<InT, OutT> parse(shared_ptr<Iterator<InT> > it) {
			for (size_t i=0; i<parsers->size(); i++) {
				ParseResult<InT, OutT> result = (*parsers)[i]->parse(it);
				if (result.success) {
					return result;
				}
			}
			return ParseResult<InT, OutT>(it);
		}

	protected:
		OrParser() {
			this->parsers = shared_ptr<vector<shared_ptr<Parser<InT, OutT> > > >(new vector<shared_ptr<Parser<InT, OutT> > >());
		}
	};

	struct CharClassParser : OrParser<char, char>{
		CharClassParser(const string& text) {
			for (size_t i=0; i<text.size(); i++) {
				shared_ptr<CharParser> charParser(new CharParser(text[i]));
				this->parsers->push_back(charParser);
			}
		}
	};

	template <class InT, class OutT>
	struct SequenceParser : Parser<InT, shared_ptr<vector<OutT> > > {
		shared_ptr<vector<shared_ptr<Parser<InT, OutT> > > > parsers;
		SequenceParser(shared_ptr<vector<shared_ptr<Parser<InT, OutT> > > > parsers) {
			this->parsers = parsers;
		}

		virtual ParseResult<InT, shared_ptr<vector<OutT> > > parse(shared_ptr<Iterator<InT> > it) {
			shared_ptr<Iterator<InT> > start = it;
			ParseResult<InT, shared_ptr<vector<OutT> > > totalResult(it, shared_ptr<vector<OutT> >(new vector<OutT>()));
			for (size_t i=0; i<parsers->size(); i++) {
				ParseResult<InT, OutT> result = (*parsers)[i]->parse(totalResult.iterator);
				if (!result.success) {
					return ParseResult<InT, shared_ptr<vector<OutT> > >(start);
				}
				totalResult.iterator = result.iterator;
				totalResult.value->push_back(result.value);
			}
			return totalResult;
		}
	protected:
		SequenceParser() {
			this->parsers = smartify(new vector<shared_ptr<Parser<InT, OutT> > >());
		}
	};

	struct LiteralParser : SequenceParser<char, char> {
		LiteralParser(const string& literal) {
			for (size_t i = 0; i < literal.size(); i++) {
				this->parsers->push_back(smartify(new CharParser(literal[i])));
			}
		}
	};

	template <class InT, class OutT>
	struct StarParser : Parser<InT, shared_ptr<vector<OutT> > > {
		shared_ptr<Parser<InT, OutT> > parser;
		size_t minimum;
		StarParser(shared_ptr<Parser<InT, OutT> > parser, size_t min) : parser(parser), minimum(min) {
		}

		virtual ParseResult<InT, shared_ptr<vector<OutT> > > parse(shared_ptr<Iterator<InT> > it) {
			auto start = it;
			ParseResult<InT, shared_ptr<vector<OutT> > > totalResult(it, shared_ptr<vector<OutT> >(new vector<OutT>()));
			size_t matched = 0;
			while (true) {
				ParseResult<InT, OutT> result = parser->parse(totalResult.iterator);
				if (!result.success) {
					if (matched >= minimum) {
						return totalResult;
					} else {
						return ParseResult<InT, shared_ptr<vector<OutT> > >(it);
					}
				}
				totalResult.iterator = result.iterator;
				totalResult.value->push_back(result.value);
				matched++;
			}
		}
	};

	template <class InT, class OutT>
	struct LongestOrParser : Parser<InT, OutT> {
		shared_ptr<vector<shared_ptr<Parser<InT, OutT> > > > parsers;
		LongestOrParser(shared_ptr<vector<shared_ptr<Parser<InT, OutT> > > > parsers) {
			this->parsers = parsers;
		}

		virtual ParseResult<InT, OutT> parse(shared_ptr<Iterator<InT> > it) {
			ParseResult<InT, OutT> best(it);//3
			for (size_t i=0; i<parsers->size(); i++) {
				ParseResult<InT, OutT> result = (*parsers)[i]->parse(it); //3
				if (result.success && result.iterator->getPosition() > best.iterator->getPosition()) {
					best = result;
				}
			}
			return best;
		}
	};

	struct TokenParser : Parser<char, Token > {
		shared_ptr<Parser<char, shared_ptr<vector<char> > > > parser;
		TokenType type;
		TokenParser(shared_ptr<Parser<char, shared_ptr<vector<char> > > > parser, TokenType type) : parser(parser), type(type) {}
		virtual ParseResult<char, Token> parse(shared_ptr<Iterator<char> > it) {
			ParseResult<char, shared_ptr<vector<char> > > result = parser->parse(it);
			if (result.success) {
				shared_ptr<string> text(new string(result.value->begin(), result.value->end()));
				return ParseResult<char, Token>(result.iterator, Token(text, type));
			}
			return ParseResult<char, Token>(it);
		}
	};

	namespace tokenizer {
		typedef shared_ptr<Parser<char, Token> > TokenGenPointer;

		TokenGenPointer token_parser(shared_ptr<Parser<char, shared_ptr<vector<char> > > > input, TokenType type) {
			return TokenGenPointer(new TokenParser(input, type));
		}

		template <class OutT> shared_ptr<Parser<char, shared_ptr< vector<OutT> > > > star(shared_ptr<Parser<char, OutT> > parser, size_t min = 1) {
			return shared_ptr<Parser<char, shared_ptr< vector<OutT> > > >(new StarParser<char, OutT>(parser, min));
		}

		TokenGenPointer literal_parser(const string& literal, TokenType type) {
			return TokenGenPointer(new TokenParser(smartify(new LiteralParser(literal)), type));
		}

		shared_ptr<vector<Token> > tokenize(const string& text) {
			auto integer_parser = token_parser(star<char>(smartify(new CharClassParser("0123456789"))), INTEGER);
			shared_ptr<CharClassParser> id_char_parser(new CharClassParser("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"));
			auto id_parser = token_parser(star<char>(id_char_parser), ID);
			auto multi_whitespace_parser = token_parser(star<char>(smartify(new CharClassParser(" \t\n\r"))), WHITESPACE);
			auto equals_token_parser = literal_parser("=", EQUALS);
			auto semicolon_token_parser = literal_parser(";", SEMICOLON);

			auto tokenParserList = smartify(new vector<TokenGenPointer>());
			tokenParserList->push_back(integer_parser);
			tokenParserList->push_back(id_parser);
			tokenParserList->push_back(multi_whitespace_parser);
			tokenParserList->push_back(equals_token_parser);
			tokenParserList->push_back(semicolon_token_parser);


			auto singleTokenParser = smartify(new LongestOrParser<char, Token>(tokenParserList));
			auto tokenizerParser = star<Token>(singleTokenParser);

			auto it = smartify(new StringIterator(text));
			auto result = tokenizerParser->parse(it);
			if (result.success) {
				return result.value;
			}
			return shared_ptr<vector<Token> >(NULL);
		}
	}

	namespace parser {
		struct TokenTypeParser : Parser<Token, Token> {
			TokenType type;
			TokenTypeParser(TokenType type) : type(type) {}
			virtual ParseResult<Token, Token> parse(IteratorP<Token> it) {
				if (it->hasValue() && it->value().type == type) {
					return ParseResult<Token, Token>(it->next(), it->value());
				}
				return ParseResult<Token, Token>(it);
			}
		};

		struct AssignmentParser : Parser<Token, shared_ptr<Assignment>> {
			shared_ptr<SequenceParser<Token, Token> > parser;
			AssignmentParser() {
				auto sequence = smartify(new vector<shared_ptr<Parser<Token, Token>>>());
				sequence->push_back(smartify(new TokenTypeParser(ID)));
				sequence->push_back(smartify(new TokenTypeParser(EQUALS)));
				sequence->push_back(smartify(new TokenTypeParser(INTEGER)));
				sequence->push_back(smartify(new TokenTypeParser(SEMICOLON)));
				parser = smartify(new SequenceParser<Token, Token>(sequence));

			}

			virtual ParseResult<Token, AssignmentP> parse(IteratorP<Token> it) {
				auto result = parser->parse(it);
				if (!result.success) {
					return ParseResult<Token, AssignmentP>(result.iterator);
				}
				auto vec = result.value;
				auto resultAssignment = smartify(new Assignment((*vec)[0].text, 5));
				return ParseResult<Token, AssignmentP>(result.iterator, resultAssignment);
			}
		};

		template <class OutT> shared_ptr<Parser<Token, shared_ptr< vector<OutT> > > > star(shared_ptr<Parser<Token, OutT> > parser, size_t min = 1) {
			return shared_ptr<Parser<Token, shared_ptr< vector<OutT> > > >(new StarParser<Token, OutT>(parser, min));
		}

		struct CompilationUnitParser : Parser<Token, CompilationUnitP> {
			shared_ptr<Parser<Token, shared_ptr<vector<AssignmentP> > > > parser;
			CompilationUnitParser() {
				parser = star<AssignmentP>(smartify(new AssignmentParser()));
			}

			virtual ParseResult<Token, CompilationUnitP> parse(IteratorP<Token> it) {
				auto result = parser->parse(it);
				if (!result.success) {
					return ParseResult<Token, CompilationUnitP>(it);
				}
				return ParseResult<Token, CompilationUnitP>(it, smartify(new CompilationUnit(result.value)));
			}
		};

		shared_ptr<CompilationUnit> parse(shared_ptr<vector<Token> > tokens) {
			auto it = smartify(new TokenIterator(tokens));
			auto parser = smartify(new CompilationUnitParser());
			auto result = parser->parse(it);
			if (result.success) {
				for (size_t i=0; i<result.value->statements->size(); i++) {
					cout << *(*result.value->statements)[i]->id << endl;
				}
			} else {
				cout << "FAILURE" << endl;
			}
			return shared_ptr<CompilationUnit>(NULL);

		}
	}

	typedef StarParser<StringIterator, char> StarCharClassParser;
	void do_parse() {
		
		auto tokens = tokenizer::tokenize("asdf = 5; b=5 ;");
		auto noWhitespaceTokens = smartify(new vector<Token>());

		for (size_t i=0; i < tokens->size(); i++) {
			auto token = (*tokens)[i];
			if (token.type != WHITESPACE) {
				noWhitespaceTokens->push_back(token);
			}
		}

		parser::parse(noWhitespaceTokens);

		/*

		shared_ptr<vector<shared_ptr<Parser<StringIterator, shared_ptr<Token> > > > > tokenParserList(new vector<shared_ptr<Parser<StringIterator, shared_ptr<Token> > > >);
		tokenParserList->push_back(id_parser);
		tokenParserList->push_back(multi_whitespace_parser);

		shared_ptr<LongestOrParser<StringIterator, shared_ptr<Token> > > tokenParser(new LongestOrParser<StringIterator, shared_ptr<Token> >(tokenParserList));

		StringIterator it("  basdf sdfsd");
		
		auto result = tokenParser->parse(it);
		//cout << *result.value << endl;
		*/
	}

	shared_ptr<CompilationUnit> parse() {
		do_parse();
		shared_ptr<CompilationUnit> ret(NULL);
		return ret;
	}
}

