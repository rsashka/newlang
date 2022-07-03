#include "pch.h"

#include <nlc.h>

using namespace std;
using namespace newlang;

#ifdef _MSC_VER

#pragma comment(lib, "torch.lib")
#pragma comment(lib, "torch_cpu.lib")
#pragma comment(lib, "c10.lib")

#endif

#ifndef UNITTEST

int main(int argc, char** argv) {
    NLC nlc(argc, (const char **)argv);
    return nlc.Run();
}

#endif
