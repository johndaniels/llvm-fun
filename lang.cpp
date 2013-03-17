#include "llvm/ADT/ArrayRef.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Support/IRBuilder.h"
#include "parser.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <memory>

using namespace std;
using namespace llvm;
using namespace lang;

int main()
{
	llvm::LLVMContext & context = llvm::getGlobalContext();
	llvm::Module *module = new llvm::Module("main", context);
	llvm::IRBuilder<> builder(context);

	llvm::FunctionType *funcType = llvm::FunctionType::get(builder.getVoidTy(), false);
	llvm::Function *mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);
	llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entrypoint", mainFunc);
	builder.SetInsertPoint(entry);

	shared_ptr<Ast> ast = lang::parse();
	return 0;
	vector<Value*> values;
	for (size_t i = 0; i < ast->statements->size(); i++) {
		values.push_back(ConstantInt::get(builder.getInt32Ty(), (*ast->statements)[i]->value));
	}
	Value* finalValue = values[0];
	cout << values.size() << endl;
	for (size_t i = 1; i < values.size(); i++) {
		finalValue = builder.CreateAdd(finalValue, values[i]);
		builder.CreateAlloca(builder.getInt32Ty());
	}
	builder.CreateRet(finalValue);


	llvm::Value *helloWorld = builder.CreateGlobalStringPtr("hello world!\n");

	std::vector<llvm::Type *> putsArgs;
	putsArgs.push_back(builder.getInt8Ty()->getPointerTo());
	llvm::ArrayRef<llvm::Type*>  argsRef(putsArgs);

	llvm::FunctionType *putsType = 
	llvm::FunctionType::get(builder.getInt32Ty(), argsRef, false);
	llvm::Constant *putsFunc = module->getOrInsertFunction("puts", putsType);

	builder.CreateCall(putsFunc, helloWorld);
	builder.CreateRetVoid();
	module->dump();
}