
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
		ID_EXPRESSION,
		CLASS_DEFINITION,
		ASSIGNMENT
	};

	class AstNode {
	public:
		virtual ~AstNode() {};
		virtual AstType type() = 0;
	};

	class Expression : public AstNode {

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

	class Statement : public AstNode {

	};

	class ClassDefinition : public Statement {
	public:
		std::string id;

		ClassDefinition(const char *id) : id(id) {

		}

		virtual AstType type() {
			return CLASS_DEFINITION;
		}
	};

	class Assignment : public Statement {
	public:
		Assignment(const char *id, Expression* value) : id(id), value(value) {
		}
		std::string id;
		Expression* value;

		virtual AstType type() {
			return ASSIGNMENT;
		}

		virtual ~Assignment() {
			delete value;
		}
	};

	class CompilationUnit {
	public:
		std::vector<Statement*> statements;

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