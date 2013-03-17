#include "parser.hpp"
#include <memory>
#include <iostream>

using namespace std;
namespace lang {
	enum TokenType {
		ID,
		INTEGER
	};

	struct Token {
		virtual TokenType getType() = 0;
	};

	struct IntegerToken : Token {
		virtual TokenType getType() {
			return INTEGER;
		}
	};

	struct IdToken : Token {
		string id;
		IdToken(shared_ptr<vector<char> > value) : id(value->begin(), value->end()) {}
		virtual TokenType getType() {
			return ID;
		}
	};

	struct StringIterator {
		StringIterator(const string& text) {
			this->position = 0;
			this->text = shared_ptr<string>(new string(text));
		}

		size_t position;
		shared_ptr<string> text;

		char value() {
			if (!hasValue()) {
				return 0;
			}
			return (*text)[position];
		}

		bool hasValue() {
			return position < text->size();
		}

		StringIterator next() {
			return StringIterator(position + 1, text);
		}

		bool operator> (const StringIterator& other) {
			return position > other.position;
		}

	private:
		StringIterator(size_t pos, shared_ptr<string> text) {
			this->position = pos;
			this->text = text;
		}
	};

	template <class Iterator, class T> struct ParseResult {
		ParseResult(Iterator iterator, T value) : iterator(iterator) {
			this->success = true;
			this->value = value;
		}
		ParseResult(Iterator iterator) : iterator(iterator) {
			this->success = false;
		}

		ParseResult operator||(const ParseResult& other) {
			if (this->success) {
				return (*this);
			} else {
				return other;
			}
		}

		Iterator iterator;
		bool success;
		T value;
	};

	template <class Iterator, class T>
	struct Parser {
		virtual ParseResult<Iterator, T> parse(Iterator it) = 0;
	};

	template <class Iterator>
	struct CharParser : Parser<Iterator, char> {
		char character;
		CharParser(char character) {
			this->character = character;
		}

		virtual ParseResult<Iterator, char> parse(Iterator it) {
			if (!it.hasValue() || it.value() != character) {
				return get_failure(it, '\0');
			}
			return get_success(it.next(), this->character);
		}
	};

	template <class Iterator, class T>
	struct OrParser : Parser<Iterator, T> {
		shared_ptr<vector<shared_ptr<Parser<Iterator, T> > > > parsers;
		OrParser(shared_ptr<vector<shared_ptr<Parser<Iterator, T> > > > parsers) {
			this->parsers = parsers;
		}

		OrParser(Parser<Iterator, T> a, Parser<Iterator, T> b) : parsers(new vector<shared_ptr<Parser<Iterator, T> > >) {
			this->parsers.push_back(a);
			this->parsers.push_back(b);
		}

		virtual ParseResult<Iterator, T> parse(Iterator it) {
			for (size_t i=0; i<parsers->size(); i++) {
				ParseResult<Iterator, T> result = (*parsers)[i]->parse(it);
				if (result.success) {
					return result;
				}
			}
			return ParseResult<Iterator, T>(it);
		}

	protected:
		OrParser() {
			this->parsers = shared_ptr<vector<shared_ptr<Parser<Iterator, T> > > >(new vector<shared_ptr<Parser<Iterator, T> > >);
		}
	};

	template <class Iterator>
	struct CharClassParser : OrParser<Iterator, char>{
		CharClassParser(const string& text) {
			for (size_t i=0; i<text.size(); i++) {
				shared_ptr<CharParser<Iterator> > charParser(new CharParser<Iterator>(text[i]));
				this->parsers->push_back(charParser);
			}
		}
	};

	template <class Iterator, class T, class ParserT, class SeqT>
	struct SequenceParser : Parser<Iterator, shared_ptr<SeqT> > {
		shared_ptr<vector<shared_ptr<ParserT> > > parsers;
		SequenceParser(shared_ptr<vector<shared_ptr<ParserT> > > parsers) {
			this->parsers = parsers;
		}

		virtual ParseResult<Iterator, shared_ptr<SeqT> > parse(Iterator it) {
			Iterator start = it;
			ParseResult<Iterator, shared_ptr<SeqT> > totalResult(it, shared_ptr<SeqT>(new SeqT()));
			for (size_t i=0; i<parsers->size(); i++) {
				ParseResult<Iterator, T> result = (*parsers)[i].parse(totalResult.iterator);
				if (!result.success) {
					return ParseResult<Iterator, shared_ptr<SeqT> >(start);
				}
				totalResult.iterator = result.iterator;
				totalResult.value->push_back(result.value);
			}
			return totalResult;
		}
	};

	template <class Iterator, class T>
	struct StarParser : Parser<Iterator, shared_ptr<vector<T> > > {
		shared_ptr<Parser<Iterator, T> > parser;
		StarParser(shared_ptr<Parser<Iterator, T> > parser) : parser(parser) {
		}

		virtual ParseResult<Iterator, shared_ptr<vector<T> > > parse(Iterator it) {
			ParseResult<Iterator, shared_ptr<vector<T> > > totalResult(it, shared_ptr<vector<T> >(new vector<T>()));
			while (true) {
				ParseResult<Iterator, T> result = parser->parse(totalResult.iterator);
				if (!result.success) {
					return totalResult;
				}
				totalResult.iterator = result.iterator;
				totalResult.value->push_back(result.value);
			}
		}
	};

	template <class Iterator, class T>
	struct LongestOrParser : Parser<Iterator, T> {
		shared_ptr<vector<shared_ptr<Parser<Iterator, T>> > > parsers;
		LongestOrParser(shared_ptr<vector<shared_ptr<Parser<Iterator, T> > > > parsers) {
			this->parsers = parsers;
		}

		virtual ParseResult<Iterator, T> parse(Iterator it) {
			ParseResult<Iterator, T> best = ParseResult<Iterator, T>(it);
			for (size_t i=0; i<parsers->size(); i++) {
				ParseResult<Iterator, T> result = (*parsers)[i]->parse(it);
				if (result.success && result.iterator > best.iterator) {
					best = result;
				}
			}
			return best;
		}
	};

	template <class Iterator, class InT, class OutT>
	struct TokenParser : Parser<Iterator, shared_ptr<Token> > {
		shared_ptr<Parser<Iterator, InT> > parser;
		TokenParser(shared_ptr<Parser<Iterator, InT> > parser) : parser(parser) {}
		virtual ParseResult<Iterator, shared_ptr<Token> > parse(Iterator it) {
			ParseResult<Iterator, InT> result = parser->parse(it);
			if (result.success) {
				return ParseResult<Iterator, shared_ptr<Token> >(it, shared_ptr<Token>(new OutT(result.value)));
			}
			return ParseResult<Iterator, shared_ptr<Token> >(it);
		}
	};

	template <class Iterator, class OutT> shared_ptr<Parser<Iterator, shared_ptr<Token> > > tokenParser(shared_ptr<Parser<Iterator, shared_ptr<vector<char> > > > input) {
		return shared_ptr<Parser<Iterator, shared_ptr<Token> > >(new TokenParser<Iterator, shared_ptr<vector<char> >, OutT >(input));
	}

	template <class Iterator, class T> ParseResult<Iterator, T> get_success(Iterator iterator, T value) {
		return ParseResult<Iterator, T>(iterator, value);
	}

	template <class Iterator, class T> ParseResult<Iterator, T> get_failure(Iterator iterator, T value) {
		return ParseResult<Iterator, T>(iterator);
	}

	template <class Iterator, class T> shared_ptr<Parser<Iterator, T> > orParser(shared_ptr<Parser<Iterator, T> > a, shared_ptr<Parser<Iterator, T> > b) {
		return shared_ptr<Parser<Iterator, T> >(new OrParser<Iterator, T>(a, b));
	}

	template <class Iterator, class T> shared_ptr<Parser<Iterator, shared_ptr< vector<T> > > > star(shared_ptr<Parser<Iterator, T> > parser) {
		return shared_ptr<Parser<Iterator, shared_ptr< vector<T> > > >(new StarParser<Iterator, T>(parser));
	}

	template <class T> shared_ptr<T> smartify(T* ptr) {
		return shared_ptr<T>(ptr);
	}		

	typedef StarParser<StringIterator, char> StarCharClassParser;
	void do_parse() {


		shared_ptr<Parser<StringIterator, char>> parser(new CharClassParser<StringIterator>("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"));
		auto id_parser = tokenParser<StringIterator, IdToken>(star<StringIterator, char>(parser));
		auto multi_whitespace_parser = tokenParser<StringIterator, IdToken>(star<StringIterator, char>(smartify(new CharClassParser<StringIterator>(" \t\n\r"))));

		shared_ptr<vector<shared_ptr<Parser<StringIterator, shared_ptr<Token> > > > > tokenParserList(new vector<shared_ptr<Parser<StringIterator, shared_ptr<Token> > > >);
		tokenParserList->push_back(id_parser);
		tokenParserList->push_back(multi_whitespace_parser);

		shared_ptr<LongestOrParser<StringIterator, shared_ptr<Token> > > tokenParser(new LongestOrParser<StringIterator, shared_ptr<Token> >(tokenParserList));

		StringIterator it("  basdf sdfsd");
		
		auto result = tokenParser->parse(it);
		//cout << *result.value << endl;
	}

	shared_ptr<Ast> parse() {
		do_parse();
		shared_ptr<Ast> ret(new Ast());
		return ret;
	}
}

