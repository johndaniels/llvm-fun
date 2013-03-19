
#ifndef __AST_H__
#define __AST_H__

#include <string>
#include <vector>
#include <deque>
#include <stdint.h>

namespace lang {
	enum AstType {
		LITERAL_EXPRESSION,
		BINARY_EXPRESSION,
		ID_EXPRESSION
	};

	class Expression {
	public:
		virtual ~Expression() {};
		virtual AstType type() = 0;
	};

	class LiteralExpression: public Expression {
	public:
		int32_t number;
		LiteralExpression(int32_t number) : number(number) {}

		virtual AstType type() {
			return LITERAL_EXPRESSION;
		}

		virtual ~LiteralExpression() {}
	};

	class BinaryExpression: public Expression {
	public:
		Expression* left;
		Expression* right;
		BinaryExpression(Expression* left, Expression* right) : left(left), right(right) {}

		virtual AstType type() {
			return BINARY_EXPRESSION;
		}

		virtual ~BinaryExpression() {
			delete left;
			delete right;
		}
	};

	class IdExpression: public Expression {
	public:
		std::string id;
		IdExpression(const char *id) : id(id) {}

		virtual AstType type() {
			return ID_EXPRESSION;
		}
	};

	class Assignment {
	public:
		Assignment(const char *id, Expression* value) : id(id), value(value) {
		}
		std::string id;
		Expression* value;

		virtual ~Assignment() {
			delete value;
		}
	};

	class CompilationUnit {
	public:
		std::deque<Assignment*> statements;

		CompilationUnit() {
		}

		~CompilationUnit() {
			for (size_t i = 0; i < statements.size(); i++) {
				delete statements[i];
			}
		}
	};
	
	extern CompilationUnit* parsed_compilation_unit;
}

#endif