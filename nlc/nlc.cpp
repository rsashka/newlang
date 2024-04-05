#include "nlc.h"

#ifdef _MSC_VER

#pragma comment(lib, "torch.lib")
#pragma comment(lib, "torch_cpu.lib")
#pragma comment(lib, "c10.lib")


/* #pragma comment(lib, "clangTooling.lib")
#pragma comment(lib, "clangFrontendTool.lib")
#pragma comment(lib, "clangFrontend.lib")
#pragma comment(lib, "clangDriver.lib")
#pragma comment(lib, "clangSerialization.lib")
#pragma comment(lib, "clangCodeGen.lib")
#pragma comment(lib, "clangParse.lib")
#pragma comment(lib, "clangSema.lib")
#pragma comment(lib, "clangStaticAnalyzerFrontend.lib")
#pragma comment(lib, "clangStaticAnalyzerCheckers.lib")
#pragma comment(lib, "clangStaticAnalyzerCore.lib")
#pragma comment(lib, "clangAnalysis.lib")
#pragma comment(lib, "clangARCMigrate.lib")
#pragma comment(lib, "clangRewrite.lib")
#pragma comment(lib, "clangASTMatchers.lib")
#pragma comment(lib, "clangSupport.lib")
#pragma comment(lib, "clangRewriteFrontend.lib")
#pragma comment(lib, "clangEdit.lib")
#pragma comment(lib, "clangAST.lib")
#pragma comment(lib, "clangLex.lib")
#pragma comment(lib, "clangBasic.lib")
 */


#pragma comment(lib, "LLVMSupport.lib")

#pragma comment(lib, "LLVM-C.lib")

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "legacy_stdio_definitions.lib")
#pragma comment(lib, "legacy_stdio_wide_specifiers.lib")
#pragma comment(lib, "libcrypto_static.lib")

#endif

#ifndef BUILD_UNITTEST

int main(int argc, char** argv) {
    newlang::NLC nlc;
    int res = nlc.Run(argc, argv);
    fflush(nullptr);
    _exit(res);
}

#else
#error Fail build type
#endif
