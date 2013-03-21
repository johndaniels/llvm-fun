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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;
using namespace lang;

extern FILE* yyin;
extern int yyparse();
namespace lang {
	CompilationUnit* parsed_compilation_unit;
}

static void ff_pipe(int fd[2], const char* fail_message) {
	if (pipe(fd) < 0) {
		perror(fail_message);
		exit(1);
	}
}

static pid_t ff_fork(const char* fail_message) {
	pid_t ret = fork();
	if (ret < 0) {
		perror(fail_message);
		exit(1);
	}
	return ret;
}

static void ff_dup2(int first, int second, const char* fail_message) {
	if (dup2(first, second) < 0) {
		perror(fail_message);
		exit(1);
	}
}

static void ff_close(int fd, const char* fail_message) {
	if (close(fd) < 0) {
		perror(fail_message);
		exit(1);
	}
}

static pid_t ff_waitpid(pid_t pid, const char* fail_message) {
	int status;
	pid_t ret_pid = waitpid(pid, &status, 0);
	if (ret_pid < 0 && WIFEXITED(status) && WEXITSTATUS(status)) {
		perror(fail_message);
		exit(1);
	}
	return ret_pid;
}

void compile_object_file(llvm::Module *module) {
	int aspipe[2];
	ff_pipe(aspipe, "open aspipe");

	int llcpipe[2];
	ff_pipe(llcpipe, "open llcpipe");

	pid_t llc = ff_fork("llc fork");

	if (llc == 0) {
		ff_close(llcpipe[1], "close llcpipe[1] in llc");
		ff_close(aspipe[0], "close aspipe[1] in llc");

		ff_dup2(llcpipe[0], STDIN_FILENO, "dup llcpipe[0] to STDIN in llc");
		ff_dup2(aspipe[1], STDOUT_FILENO, "dup aspipe[1] to STDOUT in llc");

		execlp("llc", "llc", NULL);
		perror("Exec LLC");
		exit(1);
	}
	ff_close(llcpipe[0], "close llcpipe[0]");

	pid_t as = ff_fork("as fork");
	ff_close(aspipe[1], "close aspipe[1]");

	if (as == 0) {
		ff_close(llcpipe[1], "close llcpipe[1] in as process");
		
		ff_dup2(aspipe[0], STDIN_FILENO, "dup aspipe[0] to STDIN in as");
		execlp("as", "as", "-o", "test.o", NULL);
		perror("exec as");
		exit(1);
	}

	ff_close(aspipe[0], "close aspipe[0]");

	raw_fd_ostream outfile(llcpipe[1], true);
	outfile << *module;
	outfile.close();

	ff_waitpid(llc, "wait for llc");
	ff_waitpid(as, "wait for as");
}

void compile_exe_file() {
	pid_t gcc = ff_fork("fork compiling exe");
	if (gcc == 0) {
		execlp("gcc", "gcc", "-o", "output", "test.o", "runtime.c", NULL);
		perror("failed exec gcc");
		exit(1);
	}
	ff_waitpid(gcc, "wait for gcc");
}

int main()
{
	CompilationUnit *ast;

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

	compile_object_file(module);
	compile_exe_file();

	delete ast;
	return 0;
}