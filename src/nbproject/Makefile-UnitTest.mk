#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=clang-16
CCC=clang++-16
CXX=clang++-16
FC=gfortran
AS=lld-16

# Macros
CND_PLATFORM=CLang-Linux
CND_DLIB_EXT=so
CND_CONF=UnitTest
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/b0ad39d/format.o \
	${OBJECTDIR}/_ext/1e501df/gtest-all.o \
	${OBJECTDIR}/analysis.o \
	${OBJECTDIR}/builtin.o \
	${OBJECTDIR}/compiler.o \
	${OBJECTDIR}/context.o \
	${OBJECTDIR}/diag.o \
	${OBJECTDIR}/lexer.o \
	${OBJECTDIR}/lexer.yy.o \
	${OBJECTDIR}/logger.o \
	${OBJECTDIR}/macro.o \
	${OBJECTDIR}/module.o \
	${OBJECTDIR}/nlc.o \
	${OBJECTDIR}/object.o \
	${OBJECTDIR}/parser.o \
	${OBJECTDIR}/parser.yy.o \
	${OBJECTDIR}/runtime.o \
	${OBJECTDIR}/system.o \
	${OBJECTDIR}/term.o \
	${OBJECTDIR}/test/alg_test.o \
	${OBJECTDIR}/test/ast_test.o \
	${OBJECTDIR}/test/compiler_test.o \
	${OBJECTDIR}/test/diag_test.o \
	${OBJECTDIR}/test/example_test.o \
	${OBJECTDIR}/test/lexer_test.o \
	${OBJECTDIR}/test/logger_test.o \
	${OBJECTDIR}/test/macro_test.o \
	${OBJECTDIR}/test/module_test.o \
	${OBJECTDIR}/test/nlc_test.o \
	${OBJECTDIR}/test/object_test.o \
	${OBJECTDIR}/test/parser_analysis_test.o \
	${OBJECTDIR}/test/parser_test.o \
	${OBJECTDIR}/test/rational_test.o \
	${OBJECTDIR}/test/run_test.o \
	${OBJECTDIR}/test/system_test.o \
	${OBJECTDIR}/types.o \
	${OBJECTDIR}/version.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=`llvm-config --cxxflags` -std=c++20 -fexceptions -fcxx-exceptions -Wall -Wextra -Werror -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual -Wno-trigraphs -Wno-invalid-source-encoding -stdlib=libstdc++ -Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=switch -fsanitize=undefined -fsanitize-trap=undefined   -gdwarf-4               -Wno-undefined-var-template -Wno-switch  -fvisibility=default  -ggdb -O0 
CXXFLAGS=`llvm-config --cxxflags` -std=c++20 -fexceptions -fcxx-exceptions -Wall -Wextra -Werror -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual -Wno-trigraphs -Wno-invalid-source-encoding -stdlib=libstdc++ -Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=switch -fsanitize=undefined -fsanitize-trap=undefined   -gdwarf-4               -Wno-undefined-var-template -Wno-switch  -fvisibility=default  -ggdb -O0 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../contrib/libtorch/lib -L/usr/lib/x86_64-linux-gnu -L/usr/local/lib/x86_64-linux-gnu -L/usr/lib/llvm-16/lib -Wl,-rpath,'.' -Wl,-rpath,'../contrib/libtorch/lib' -Wl,-rpath,'/usr/lib/x86_64-linux-gnu' -lpthread -lc10 -ltorch -ltorch_cpu -lcrypto -ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../output/nlc_test

../output/nlc_test: ${OBJECTFILES}
	${MKDIR} -p ../output
	clang++-16 -o ../output/nlc_test ${OBJECTFILES} ${LDLIBSOPTIONS} `llvm-config --libs --ldflags` -fuse-ld=lld -g -lclangTooling -lclangFrontendTool -lclangFrontend -lclangDriver -lclangSerialization -lclangCodeGen -lclangParse -lclangSema -lclangStaticAnalyzerFrontend -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore -lclangAnalysis -lclangARCMigrate -lclangRewrite -lclangRewriteFrontend -lclangEdit -lclangAST -lclangLex -lclangBasic  -lclangASTMatchers -lclangSupport -fvisibility=default -Wl,--export-dynamic

${OBJECTDIR}/_ext/b0ad39d/format.o: ../contrib/fmt/src/format.cc
	${MKDIR} -p ${OBJECTDIR}/_ext/b0ad39d
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b0ad39d/format.o ../contrib/fmt/src/format.cc

${OBJECTDIR}/_ext/1e501df/gtest-all.o: ../contrib/googletest/googletest/src/gtest-all.cc
	${MKDIR} -p ${OBJECTDIR}/_ext/1e501df
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -std=c++14 -Wno-undef -Wno-sign-compare -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1e501df/gtest-all.o ../contrib/googletest/googletest/src/gtest-all.cc

${OBJECTDIR}/analysis.o: analysis.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/analysis.o analysis.cpp

${OBJECTDIR}/builtin.o: builtin.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/builtin.o builtin.cpp

${OBJECTDIR}/compiler.o: compiler.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/compiler.o compiler.cpp

${OBJECTDIR}/context.o: context.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/context.o context.cpp

${OBJECTDIR}/diag.o: diag.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/diag.o diag.cpp

${OBJECTDIR}/lexer.o: lexer.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lexer.o lexer.cpp

: lexer.h parser.yy.h parser.yy.cpp location.hh
	@echo Выполнение шага пользовательского сборки
	

.NO_PARALLEL:lexer.yy.cpp lexer.yy.h
lexer.yy.cpp lexer.yy.h: lexer.l parser.y parser.yy.h parser.yy.cpp location.hh term.h
	@echo 
	flex  --outfile=lexer.yy.cpp --header-file=lexer.yy.h --noline  lexer.l

${OBJECTDIR}/lexer.yy.o: lexer.yy.cpp parser.y parser.yy.h parser.yy.cpp location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lexer.yy.o lexer.yy.cpp

${OBJECTDIR}/logger.o: logger.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/logger.o logger.cpp

${OBJECTDIR}/macro.o: macro.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/macro.o macro.cpp

${OBJECTDIR}/module.o: module.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/module.o module.cpp

${OBJECTDIR}/nlc.o: nlc.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nlc.o nlc.cpp

${OBJECTDIR}/object.o: object.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/object.o object.cpp

${OBJECTDIR}/parser.o: parser.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/parser.o parser.cpp

.NO_PARALLEL:parser.yy.h parser.yy.cpp location.hh
parser.yy.h parser.yy.cpp location.hh: parser.y
	@echo ************* Bison compile ************* 
	bison --output-file=parser.yy.cpp --defines=parser.yy.h --warnings=all parser.y

${OBJECTDIR}/parser.yy.o: parser.yy.cpp parser.y
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/parser.yy.o parser.yy.cpp

: parser.yy.h parser.y
	@echo Выполнение шага пользовательского сборки
	

pch.h.pch: pch.h
	@echo Выполнение шага пользовательского сборки
	clang++ `llvm-config --cxxflags` -std=c++20 -fexceptions -fcxx-exceptions -Wall -Wextra -Werror -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual -Wno-trigraphs -Wno-invalid-source-encoding -stdlib=libstdc++ -Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=switch -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error                                         -Wno-undefined-var-template -Wno-switch   -fvisibility=default              -g -DBUILD_DEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DBUILD_UNITTEST -I. -I.. -I/usr/include/llvm-c-16 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include                 -x c++-header -o pch.h.pch pch.h -verify-pch -fpch-validate-input-files-content

${OBJECTDIR}/runtime.o: runtime.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/runtime.o runtime.cpp

${OBJECTDIR}/system.o: system.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/system.o system.cpp

${OBJECTDIR}/term.o: term.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/term.o term.cpp

: term.h parser.yy.cpp location.hh
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/test/alg_test.o: test/alg_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/alg_test.o test/alg_test.cpp

${OBJECTDIR}/test/ast_test.o: test/ast_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/ast_test.o test/ast_test.cpp

${OBJECTDIR}/test/compiler_test.o: test/compiler_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/compiler_test.o test/compiler_test.cpp

${OBJECTDIR}/test/diag_test.o: test/diag_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/diag_test.o test/diag_test.cpp

${OBJECTDIR}/test/example_test.o: test/example_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/example_test.o test/example_test.cpp

${OBJECTDIR}/test/lexer_test.o: test/lexer_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/lexer_test.o test/lexer_test.cpp

${OBJECTDIR}/test/logger_test.o: test/logger_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/logger_test.o test/logger_test.cpp

${OBJECTDIR}/test/macro_test.o: test/macro_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/macro_test.o test/macro_test.cpp

${OBJECTDIR}/test/module_test.o: test/module_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/module_test.o test/module_test.cpp

${OBJECTDIR}/test/nlc_test.o: test/nlc_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/nlc_test.o test/nlc_test.cpp

${OBJECTDIR}/test/object_test.o: test/object_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/object_test.o test/object_test.cpp

${OBJECTDIR}/test/parser_analysis_test.o: test/parser_analysis_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/parser_analysis_test.o test/parser_analysis_test.cpp

${OBJECTDIR}/test/parser_test.o: test/parser_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/parser_test.o test/parser_test.cpp

${OBJECTDIR}/test/rational_test.o: test/rational_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/rational_test.o test/rational_test.cpp

${OBJECTDIR}/test/run_test.o: test/run_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/run_test.o test/run_test.cpp

${OBJECTDIR}/test/system_test.o: test/system_test.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/system_test.o test/system_test.cpp

${OBJECTDIR}/types.o: types.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/types.o types.cpp

${OBJECTDIR}/version.o: version.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/16/include -I/usr/include/llvm -I/usr/include/llvm-c -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/fmt/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/version.o version.cpp

: warning_pop.h parser.yy.cpp location.hh
	@echo Выполнение шага пользовательского сборки
	

: warning_push.h parser.yy.cpp location.hh
	@echo Выполнение шага пользовательского сборки
	

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} 
	${RM} lexer.yy.cpp lexer.yy.h
	${RM} parser.yy.h parser.yy.cpp location.hh
	${RM} 
	${RM} pch.h.pch
	${RM} 
	${RM} 
	${RM} 

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
