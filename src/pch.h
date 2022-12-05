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

#endif

#include "warning_push.h"

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Support.h>

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

#include <torch/torch.h>
#include <ATen/ATen.h>

#include <contrib/logger/logger.h>
#include "warning_pop.h"


#undef LOG_RUNTIME
#define LOG_RUNTIME(format, ...)  LOG_EXCEPT(newlang::Return, format, ##__VA_ARGS__)

#include "types.h"


#endif // NEWLANG_PCH_H_

