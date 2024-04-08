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
CC=clang-18
CCC=clang++-18
CXX=clang++-18
FC=gfortran
AS=lld-18

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
	${OBJECTDIR}/_ext/511e2d96/unittest.o \
	${OBJECTDIR}/analysis.o \
	${OBJECTDIR}/builtin.o \
	${OBJECTDIR}/context.o \
	${OBJECTDIR}/diag.o \
	${OBJECTDIR}/jit.o \
	${OBJECTDIR}/lexer.o \
	${OBJECTDIR}/lexer.yy.o \
	${OBJECTDIR}/logger.o \
	${OBJECTDIR}/macro.o \
	${OBJECTDIR}/module.o \
	${OBJECTDIR}/object.o \
	${OBJECTDIR}/parser.o \
	${OBJECTDIR}/parser.yy.o \
	${OBJECTDIR}/rational.o \
	${OBJECTDIR}/runtime.o \
	${OBJECTDIR}/system.o \
	${OBJECTDIR}/term.o \
	${OBJECTDIR}/test/alg_test.o \
	${OBJECTDIR}/test/ast_test.o \
	${OBJECTDIR}/test/class_test.o \
	${OBJECTDIR}/test/diag_test.o \
	${OBJECTDIR}/test/example_test.o \
	${OBJECTDIR}/test/include_test.o \
	${OBJECTDIR}/test/jit_test.o \
	${OBJECTDIR}/test/lexer_test.o \
	${OBJECTDIR}/test/llvm_test.o \
	${OBJECTDIR}/test/logger_test.o \
	${OBJECTDIR}/test/macro_test.o \
	${OBJECTDIR}/test/module_test.o \
	${OBJECTDIR}/test/object_test.o \
	${OBJECTDIR}/test/parser_analysis_test.o \
	${OBJECTDIR}/test/parser_test.o \
	${OBJECTDIR}/test/rational_test.o \
	${OBJECTDIR}/test/run_test.o \
	${OBJECTDIR}/test/system_test.o \
	${OBJECTDIR}/version.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=`llvm-config-18 --cxxflags` `cat ../build_options.txt | tr -d '\r\n'`  -fsanitize=undefined -fsanitize-trap=undefined -Werror   -gdwarf-4  -ggdb 
CXXFLAGS=`llvm-config-18 --cxxflags` `cat ../build_options.txt | tr -d '\r\n'`  -fsanitize=undefined -fsanitize-trap=undefined -Werror   -gdwarf-4  -ggdb 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../contrib/libtorch/lib -Wl,-rpath,'.' -lpthread -lc10 -ltorch -ltorch_cpu -lzip -ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../output/nlc-unittest

../output/nlc-unittest: ${OBJECTFILES}
	${MKDIR} -p ../output
	${LINK.cc} -o ../output/nlc-unittest ${OBJECTFILES} ${LDLIBSOPTIONS} `llvm-config-18 --link-static --system-libs --libs all` -g -fvisibility=default -Wl,--export-dynamic -Wl,--exclude-libs,ALL -lclang-cpp -lcrypto -lssl

${OBJECTDIR}/_ext/b0ad39d/format.o: ../contrib/fmt/src/format.cc
	${MKDIR} -p ${OBJECTDIR}/_ext/b0ad39d
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b0ad39d/format.o ../contrib/fmt/src/format.cc

${OBJECTDIR}/_ext/1e501df/gtest-all.o: ../contrib/googletest/googletest/src/gtest-all.cc
	${MKDIR} -p ${OBJECTDIR}/_ext/1e501df
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1e501df/gtest-all.o ../contrib/googletest/googletest/src/gtest-all.cc

${OBJECTDIR}/_ext/511e2d96/unittest.o: ../nlc/unittest.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e2d96
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e2d96/unittest.o ../nlc/unittest.cpp

${OBJECTDIR}/analysis.o: analysis.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/analysis.o analysis.cpp

${OBJECTDIR}/builtin.o: builtin.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/builtin.o builtin.cpp

${OBJECTDIR}/context.o: context.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/context.o context.cpp

${OBJECTDIR}/diag.o: diag.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/diag.o diag.cpp

${OBJECTDIR}/jit.o: jit.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/jit.o jit.cpp

${OBJECTDIR}/lexer.o: lexer.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lexer.o lexer.cpp

: lexer.h lexer.l parser.yy.h parser.yy.cpp location.hh
	@echo Выполнение шага пользовательского сборки
	

.NO_PARALLEL:lexer.yy.cpp lexer.yy.h
lexer.yy.cpp lexer.yy.h: lexer.l parser.y parser.yy.h parser.yy.cpp location.hh term.h
	@echo Выполнение шага пользовательского сборки
	flex  --outfile=lexer.yy.cpp --header-file=lexer.yy.h --noline  lexer.l

${OBJECTDIR}/lexer.yy.o: lexer.yy.cpp lexer.l parser.y parser.yy.h parser.yy.cpp location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lexer.yy.o lexer.yy.cpp

${OBJECTDIR}/logger.o: logger.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/logger.o logger.cpp

${OBJECTDIR}/macro.o: macro.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/macro.o macro.cpp

${OBJECTDIR}/module.o: module.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/module.o module.cpp

${OBJECTDIR}/object.o: object.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/object.o object.cpp

${OBJECTDIR}/parser.o: parser.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/parser.o parser.cpp

: parser.h parser.y
	@echo Выполнение шага пользовательского сборки
	

.NO_PARALLEL:parser.yy.h parser.yy.cpp location.hh
parser.yy.h parser.yy.cpp location.hh: parser.y
	@echo Выполнение шага пользовательского сборки
	bison --output-file=parser.yy.cpp --defines=parser.yy.h --warnings=all parser.y

${OBJECTDIR}/parser.yy.o: parser.yy.cpp parser.y
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/parser.yy.o parser.yy.cpp

${OBJECTDIR}/rational.o: rational.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/rational.o rational.cpp

${OBJECTDIR}/runtime.o: runtime.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/runtime.o runtime.cpp

${OBJECTDIR}/system.o: system.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/system.o system.cpp

${OBJECTDIR}/term.o: term.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/term.o term.cpp

: term.h parser.yy.cpp location.hh
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/test/alg_test.o: test/alg_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/alg_test.o test/alg_test.cpp

${OBJECTDIR}/test/ast_test.o: test/ast_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/ast_test.o test/ast_test.cpp

${OBJECTDIR}/test/class_test.o: test/class_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/class_test.o test/class_test.cpp

${OBJECTDIR}/test/diag_test.o: test/diag_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/diag_test.o test/diag_test.cpp

${OBJECTDIR}/test/example_test.o: test/example_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/example_test.o test/example_test.cpp

${OBJECTDIR}/test/include_test.o: test/include_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/include_test.o test/include_test.cpp

${OBJECTDIR}/test/jit_test.o: test/jit_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/jit_test.o test/jit_test.cpp

${OBJECTDIR}/test/lexer_test.o: test/lexer_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/lexer_test.o test/lexer_test.cpp

${OBJECTDIR}/test/llvm_test.o: test/llvm_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/llvm_test.o test/llvm_test.cpp

${OBJECTDIR}/test/logger_test.o: test/logger_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/logger_test.o test/logger_test.cpp

${OBJECTDIR}/test/macro_test.o: test/macro_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/macro_test.o test/macro_test.cpp

${OBJECTDIR}/test/module_test.o: test/module_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/module_test.o test/module_test.cpp

${OBJECTDIR}/test/object_test.o: test/object_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/object_test.o test/object_test.cpp

${OBJECTDIR}/test/parser_analysis_test.o: test/parser_analysis_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/parser_analysis_test.o test/parser_analysis_test.cpp

${OBJECTDIR}/test/parser_test.o: test/parser_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/parser_test.o test/parser_test.cpp

${OBJECTDIR}/test/rational_test.o: test/rational_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/rational_test.o test/rational_test.cpp

${OBJECTDIR}/test/run_test.o: test/run_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/run_test.o test/run_test.cpp

${OBJECTDIR}/test/system_test.o: test/system_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/system_test.o test/system_test.cpp

${OBJECTDIR}/version.o: version.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../lib -I../../lib -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/version.o version.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} 
	${RM} lexer.yy.cpp lexer.yy.h
	${RM} 
	${RM} parser.yy.h parser.yy.cpp location.hh
	${RM} 

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
