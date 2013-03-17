lang: lang.cpp parser.cpp parser.hpp
	g++ lang.cpp parser.cpp -o lang -std=c++11 -I/usr/lib/llvm-3.1/include  -DNDEBUG -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS \
	-g -O2 -fomit-frame-pointer -fvisibility-inlines-hidden -fno-exceptions -fPIC -Woverloaded-virtual -Wcast-qual \
	-L/usr/lib/llvm-3.1/lib  -lpthread -lffi -ldl -lm \
	-lLLVMAsmParser -lLLVMInstrumentation -lLLVMLinker -lLLVMArchive -lLLVMBitReader -lLLVMDebugInfo -lLLVMJIT -lLLVMipo -lLLVMVectorize -lLLVMBitWriter \
	-lLLVMTableGen -lLLVMHexagonCodeGen -lLLVMHexagonAsmPrinter -lLLVMHexagonDesc -lLLVMHexagonInfo -lLLVMPTXCodeGen -lLLVMPTXDesc \
	-lLLVMPTXInfo -lLLVMPTXAsmPrinter -lLLVMMBlazeAsmParser -lLLVMMBlazeCodeGen -lLLVMMBlazeDisassembler -lLLVMMBlazeDesc \
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
