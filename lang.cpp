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
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;
using namespace lang;

extern FILE* yyin;
extern int yyparse();
namespace lang {
	CompilationUnit* parsed_compilation_unit;
}

int main()
{
	CompilationUnit *ast;
	/*vector<Assignment*> statements;
	for (int j=0; j < 10; j++) {
		statements.push_back(new Assignment("test", 4));
	}
	for (size_t j=0; j < statements.size(); j++) {
		delete statements[j];
	}*/
	

	yyin = fopen("test.lang", "r");
	if (!yyin) {
		return 1;
	}

	int ret = yyparse();

	if (ret) {
		return ret;
	}

	ast = lang::parsed_compilation_unit;

	lang::parsed_compilation_unit = NULL;
	fclose(yyin);
	

	llvm::LLVMContext & context = llvm::getGlobalContext();
	llvm::Module *module = new llvm::Module("main", context);
	llvm::IRBuilder<> builder(context);

	llvm::FunctionType *funcType = llvm::FunctionType::get(builder.getVoidTy(), false);
	llvm::Function *mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);
	llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entrypoint", mainFunc);
	builder.SetInsertPoint(entry);
	

	llvm::Value *helloWorld = builder.CreateGlobalStringPtr("hello world!\n");

	std::vector<llvm::Type *> printArgs;
	printArgs.push_back(builder.getInt32Ty());
	llvm::ArrayRef<llvm::Type*> printArgsRef(printArgs);

	llvm::FunctionType *printType = llvm::FunctionType::get(builder.getInt32Ty(), printArgsRef, false);
	llvm::Constant *printFunc = module->getOrInsertFunction("print", printType);

	AstCompiler compiler(&builder);
	compiler.printFunc = printFunc;
	compiler.compile(ast);

	std::vector<llvm::Type *> putsArgs;
	putsArgs.push_back(builder.getInt8Ty()->getPointerTo());
	llvm::ArrayRef<llvm::Type*>  argsRef(putsArgs);

	llvm::FunctionType *putsType = llvm::FunctionType::get(builder.getInt32Ty(), argsRef, false);
	llvm::Constant *putsFunc = module->getOrInsertFunction("puts", putsType);

	builder.CreateCall(putsFunc, helloWorld);
	builder.CreateRetVoid();

	int aspipe[2];
	pipe(aspipe);

	int llcpipe[2];
	pipe(llcpipe);
	pid_t llc = fork();
	if (llc == 0) {
		close(llcpipe[1]);

		dup2(llcpipe[0], STDIN_FILENO);
		dup2(aspipe[1], STDOUT_FILENO);

		//close(aspipe[0]);
		//write(STDOUT_FILENO, "asdf", 4);

		execlp("llc", "llc", NULL);
		perror(NULL);
		exit(1);
	}
	close(llcpipe[0]);
	

	pid_t as = fork();
	if (as == 0) {
		close(llcpipe[0]);
		close(llcpipe[1]);
		close(aspipe[1]);
		dup2(aspipe[0], STDIN_FILENO);
		execlp("as", "as", "-o", "test.o", NULL);
		perror(NULL);
		exit(0);
	}

	close(aspipe[1]);
	close(aspipe[0]);

	raw_fd_ostream outfile(llcpipe[1], true);
	//raw_fd_ostream outfile(STDOUT_FILENO, false);
	outfile << *module;
	outfile.close();

	int status;
	close(llcpipe[1]);
	waitpid(llc, &status, 0);

	/*char temp[1000];
	while (1) {
		memset(temp, 0, sizeof(temp));
		read(aspipe[0], temp, 100);
		cout << temp;
	}*/

	delete ast;
}