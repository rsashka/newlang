#include "pch.h"

#include <nlc.h>

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

#ifdef UNITTEST

#include <cstdio>
#include "gtest/gtest.h"

int main(int argc, char **argv) {
    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);

    //#0  __GI___libc_free (mem=0x1) at malloc.c:3102
    //#1  0x00007fffe3d0c113 in llvm::cl::Option::~Option() () from ../contrib/libtorch/lib/libtorch_cpu.so
    //#2  0x00007fffd93eafde in __cxa_finalize (d=0x7ffff6c74000) at cxa_finalize.c:83
    //#3  0x00007fffe0b80723 in __do_global_dtors_aux () from ../contrib/libtorch/lib/libtorch_cpu.so
    //#4  0x00007fffffffdd80 in ?? ()
    //#5  0x00007ffff7fe0f6b in _dl_fini () at dl-fini.c:138

    //#0  0x00000000c0200000 in ?? ()
    //#1  0x00007fffda7b844f in ?? () from /lib/x86_64-linux-gnu/libLLVM-13.so.1
    //#2  0x00007fffd95ecfde in __cxa_finalize (d=0x7fffdf9a0ba0) at cxa_finalize.c:83
    //#3  0x00007fffda743cd7 in ?? () from /lib/x86_64-linux-gnu/libLLVM-13.so.1
    //#4  0x00007fffffffdd80 in ?? ()
    //#5  0x00007ffff7fe0f6b in _dl_fini () at dl-fini.c:138

    // При завершении приложения происходит Segmentation fault из-за двойного освобожнения памяти статической переменой
    // llvm::cl::Option::~Option() во время выгрузки динамически библиотек libLLVM или libtorch_cpu
    // Чтобы убрать этот coredump вместо нормального завершения main вызываю _exit, чтобы 
    // все остальные функции освобождения памяти не вызывались при завершении процесса.

    //    return RUN_ALL_TESTS();
    _exit(RUN_ALL_TESTS());
}

#else

// Для теста 
char convert(char c) {
    if(c == 'A') return 'C';
    if(c == 'C') return 'G';
    if(c == 'G') return 'T';
    if(c == 'T') return 'A';
    return ' ';
}

int main(int argc, char** argv) {

    LLVMAddSymbol("convert", (void *) &convert);

    newlang::NLC nlc(argc, (const char **) argv);
    //    return nlc.Run();
    _exit(nlc.Run());
}

#endif
