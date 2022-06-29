#include "pch.h"

#include <nlc.h>

using namespace std;
using namespace newlang;

#ifdef _MSC_VER

#pragma comment(lib, "torch.lib")
#pragma comment(lib, "torch_cpu.lib")
#pragma comment(lib, "c10.lib")
#pragma comment(lib, "libffi.dll.a")

//#pragma comment(lib, "pthreadpool.lib")
//#pragma comment(lib, "libprotoc.lib")
//#pragma comment(lib, "XNNPACK.lib")
//#pragma comment(lib, "dnnl.lib")
//#pragma comment(lib, "asmjit.lib")
//#pragma comment(lib, "cpuinfo.lib")
//
//#pragma comment(lib, "clog.lib")
//#pragma comment(lib, "fbgemm.lib")
//#pragma comment(lib, "kineto.lib")

#endif

#ifndef UNITTEST

int main(int argc, char** argv) {
    NLC nlc(argc, (const char **)argv);
    return nlc.Run();
}

#endif
