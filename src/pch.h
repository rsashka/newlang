#ifndef NEWLANG_PCH_H_
#define NEWLANG_PCH_H_


#include <map>
#include <set>
#include <iosfwd>
#include <memory>
#include <vector>
#include <deque>
#include <iterator>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <map>
#include <string>
#include <string_view>
#include <cmath>
#include <random>
#include <typeindex>
#include <locale>
#include <codecvt>
#include <functional>
#include <regex>
#include <filesystem>
#include <utility>
#include <cstdlib>
#include <ctime>

#include <sstream>
#include <iostream>
#include <fstream>
#include <fcntl.h>

#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _MSC_VER

#include <windows.h>
#include <wchar.h>

#else

#include <wait.h>
#include <dlfcn.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include <unistd.h>

#endif

#include "warning_push.h"

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Support.h>

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Builtins.h"
#include "clang/AST/Mangle.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/FrontendTool/Utils.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "llvm/IR/LLVMContext.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"


#include <torch/torch.h>
#include <ATen/ATen.h>

#include "warning_pop.h"

#include <logger.h>

#endif // NEWLANG_PCH_H_

