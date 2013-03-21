
#ifndef AST_COMPILER_H_
#define AST_COMPILER_H_

#include "llvm/Support/IRBuilder.h"
#include "ast.h"

#include <map>

namespace lang {
	class AstCompiler {
		llvm::IRBuilder<>* builder;
		std::map<std::string, llvm::Value*> ids;
	public:
		llvm::Value* printFunc;
		AstCompiler(llvm::IRBuilder<>* builder);
		llvm::Value* expression(Expression *expr);
		void compile(CompilationUnit *compilation_unit); 
		void assignment(Assignment *assignment);
		void statement(Statement *statement);
	};
}

#endif