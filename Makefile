lang: lang.cpp parsersupport.cpp AstCompiler.cpp parser.o lexer.o ast.h
	clang++ lang.cpp AstCompiler.cpp parsersupport.cpp parser.o lexer.o -o lang -Wall -Werror -std=c++11 -I/usr/lib/llvm-3.2/include  -DNDEBUG -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS \
	-g -fomit-frame-pointer -fvisibility-inlines-hidden -fno-exceptions -fPIC -Woverloaded-virtual -Wcast-qual \
	-L/usr/lib/llvm-3.2/lib  -lpthread -lffi -ldl -lm \
	-lLLVMAsmParser -lLLVMInstrumentation -lLLVMLinker -lLLVMArchive -lLLVMBitReader -lLLVMDebugInfo -lLLVMJIT -lLLVMipo -lLLVMVectorize -lLLVMBitWriter \
	-lLLVMTableGen -lLLVMHexagonCodeGen -lLLVMHexagonAsmPrinter -lLLVMHexagonDesc -lLLVMHexagonInfo -lLLVMMBlazeAsmParser \
	-lLLVMMBlazeCodeGen -lLLVMMBlazeDisassembler -lLLVMMBlazeDesc \
	-lLLVMMBlazeInfo -lLLVMMBlazeAsmPrinter -lLLVMCppBackendCodeGen -lLLVMCppBackendInfo -lLLVMMSP430CodeGen -lLLVMMSP430Desc \
	-lLLVMMSP430AsmPrinter -lLLVMMSP430Info -lLLVMXCoreCodeGen -lLLVMXCoreDesc -lLLVMXCoreInfo -lLLVMCellSPUCodeGen \
	-lLLVMCellSPUDesc -lLLVMCellSPUInfo -lLLVMMipsAsmParser -lLLVMMipsCodeGen -lLLVMMipsDesc -lLLVMMipsAsmPrinter -lLLVMMipsDisassembler \
	-lLLVMMipsInfo -lLLVMARMDisassembler -lLLVMARMAsmParser -lLLVMARMCodeGen -lLLVMARMDesc -lLLVMARMAsmPrinter -lLLVMARMInfo \
	-lLLVMPowerPCCodeGen -lLLVMPowerPCDesc -lLLVMPowerPCInfo -lLLVMPowerPCAsmPrinter -lLLVMSparcCodeGen -lLLVMSparcDesc \
	-lLLVMSparcInfo -lLLVMX86Disassembler -lLLVMX86CodeGen -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMX86AsmParser \
	-lLLVMX86Desc -lLLVMX86Info -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMMCDisassembler -lLLVMMCParser -lLLVMInterpreter \
	-lLLVMCodeGen -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMMCJIT \
	-lLLVMRuntimeDyld -lLLVMExecutionEngine \
	-lLLVMTarget -lLLVMMC -lLLVMObject -lLLVMCore -lLLVMSupport -lpthread -ldl

lexer.o: lexer.cpp ast.h
	clang -c lexer.cpp

parser.o: parser.cpp ast.h
	clang -c parser.cpp

lexer.cpp: lexer.l
	flex -olexer.cpp lexer.l

parser.cpp: parser.y
	yacc parser.y