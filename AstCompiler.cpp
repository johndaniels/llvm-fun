#include "llvm/ADT/ArrayRef.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Support/IRBuilder.h"
#include "ast.h"
#include "AstCompiler.h"
#include <vector>
#include <string>
#include <iostream>

using namespace std;

namespace lang {
	
	AstCompiler::AstCompiler(llvm::IRBuilder<>* builder) : builder(builder) {}

	llvm::Value* AstCompiler::expression(Expression *expr) {
		if (expr->type() == LITERAL_EXPRESSION) {
			return llvm::ConstantInt::get(builder->getInt32Ty(), ((LiteralExpression*)expr)->number);
		} else if (expr->type() == BINARY_EXPRESSION) {
			BinaryExpression * binExpr = (BinaryExpression*)expr;
			return builder->CreateAdd(expression(binExpr->left), expression(binExpr->right));
		} else if (expr->type() == ID_EXPRESSION) {
			return ids[((IdExpression*)expr)->id];
		}
		return NULL;
	}

	void AstCompiler::compile(CompilationUnit *compilation_unit) {
		for (size_t i=0; i<compilation_unit->statements.size(); i++) {
			statement(compilation_unit->statements[i]);
		}
	}

	void AstCompiler::statement(Statement *statement) {
		if (statement->type() == CLASS_DEFINITION) {
			cout << "CLASS" << ((Assignment*)statement)->id << endl;
		} else if (statement->type() == ASSIGNMENT) {
			assignment((Assignment*)statement);
		}
	}

	void AstCompiler::assignment(Assignment *assignment) {
		ids[assignment->id] = expression(assignment->value);
		builder->CreateCall(printFunc, ids[assignment->id]);
	}
}