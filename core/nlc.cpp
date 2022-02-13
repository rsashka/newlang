#include "pch.h"

#include <core/nlc.h>

using namespace std;
using namespace newlang;

#ifndef UNITTEST

int main(int argc, char** argv) {
    NLC nlc(argc, (const char **)argv);
    return nlc.Run();
}

#endif
