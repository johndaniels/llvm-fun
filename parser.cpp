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
		shared_ptr<string> id;
		IdToken(shared_ptr<string> value) : id(value) {}
		virtual TokenType getType() {
			return ID;
		}
	};

	template <class T> struct Iterator {
		virtual T value() = 0;
		virtual bool hasValue() = 0;
		virtual shared_ptr<Iterator<T> > next() = 0;
		virtual size_t getPosition() = 0;
	};

	struct StringIterator : Iterator<char> {
		StringIterator(const string& text) {
			this->position = 0;
			this->text = shared_ptr<string>(new string(text));
		}

		virtual char value() {
			if (!hasValue()) {
				return 0;
			}
			return (*text)[position];
		}

		virtual bool hasValue() {
			return position < text->size();
		}

		virtual shared_ptr<Iterator<char> > next() {
			return shared_ptr<Iterator<char> > (new StringIterator(position + 1, text));
		}

		virtual size_t getPosition() {
			return this->position;
		}

	private:
		size_t position;
		shared_ptr<string> text;

		StringIterator(size_t pos, shared_ptr<string> text) {
			this->position = pos;
			this->text = text;
		}
	};

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
				return ParseResult<char, char>(it, '\0');
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
			this->parsers = shared_ptr<vector<shared_ptr<Parser<InT, OutT> > > >(new vector<shared_ptr<Parser<InT, OutT> > >);
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
				ParseResult<InT, OutT> result = (*parsers)[i].parse(totalResult.iterator);
				if (!result.success) {
					return ParseResult<InT, shared_ptr<vector<OutT> > >(start);
				}
				totalResult.iterator = result.iterator;
				totalResult.value->push_back(result.value);
			}
			return totalResult;
		}
	};

	template <class InT, class OutT>
	struct StarParser : Parser<InT, shared_ptr<vector<OutT> > > {
		shared_ptr<Parser<InT, OutT> > parser;
		StarParser(shared_ptr<Parser<InT, OutT> > parser) : parser(parser) {
		}

		virtual ParseResult<InT, shared_ptr<vector<OutT> > > parse(shared_ptr<Iterator<InT> > it) {
			ParseResult<InT, shared_ptr<vector<OutT> > > totalResult(it, shared_ptr<vector<OutT> >(new vector<OutT>()));
			while (true) {
				ParseResult<InT, OutT> result = parser->parse(totalResult.iterator);
				if (!result.success) {
					return totalResult;
				}
				totalResult.iterator = result.iterator;
				totalResult.value->push_back(result.value);
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
			ParseResult<InT, OutT> best(it);
			for (size_t i=0; i<parsers->size(); i++) {
				ParseResult<InT, InT> result = (*parsers)[i]->parse(it);
				if (result.success && result.iterator->getPosition() > best.iterator->getPosition()) {
					best = result;
				}
			}
			return best;
		}
	};

	template <class OutT>
	struct TokenParser : Parser<char, shared_ptr<Token> > {
		shared_ptr<Parser<char, shared_ptr<vector<char> > > > parser;
		TokenParser(shared_ptr<Parser<char, shared_ptr<vector<char> > > > parser) : parser(parser) {}
		virtual ParseResult<char, shared_ptr<Token> > parse(shared_ptr<Iterator<char> > it) {
			ParseResult<char, shared_ptr<vector<char> > > result = parser->parse(it);
			if (result.success) {
				shared_ptr<string> text(new string(result.value->begin(), result.value->end()));
				return ParseResult<char, shared_ptr<Token> >(it, shared_ptr<Token>(new OutT(text)));
			}
			return ParseResult<char, shared_ptr<Token> >(it);
		}
	};

	template <class T> shared_ptr<T> smartify(T* ptr) {
		return shared_ptr<T>(ptr);
	}

	namespace tokenizer {
		template <class OutT> shared_ptr<Parser<char, shared_ptr<Token> > > token_parser(shared_ptr<Parser<char, shared_ptr<vector<char> > > > input) {
			return shared_ptr<Parser<char, shared_ptr<Token> > >(new TokenParser< OutT >(input));
		}

		template <class OutT> shared_ptr<Parser<char, shared_ptr< vector<OutT> > > > star(shared_ptr<Parser<char, OutT> > parser) {
			return shared_ptr<Parser<char, shared_ptr< vector<OutT> > > >(new StarParser<char, OutT>(parser));
		}

		shared_ptr<vector<shared_ptr<Token> > > > tokenize() {
			shared_ptr<CharClassParser> id_char_parser(new CharClassParser("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"));
			auto id_parser = token_parser<IdToken>(star<char>(id_char_parser));
			auto multi_whitespace_parser = tokenParser<IdToken>(star<char>(smartify(new CharClassParser(" \t\n\r"))));
		}
	}

	typedef StarParser<StringIterator, char> StarCharClassParser;
	void do_parse() {
		
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

	shared_ptr<Ast> parse() {
		do_parse();
		shared_ptr<Ast> ret(new Ast());
		return ret;
	}
}

