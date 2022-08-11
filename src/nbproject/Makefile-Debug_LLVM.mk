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
CC=clang
CCC=clang++
CXX=clang++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=CLang-Linux
CND_DLIB_EXT=so
CND_CONF=Debug_LLVM
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/e16507f5/logger.o \
	${OBJECTDIR}/builtin.o \
	${OBJECTDIR}/context.o \
	${OBJECTDIR}/lexer.o \
	${OBJECTDIR}/lexer.yy.o \
	${OBJECTDIR}/newlang.o \
	${OBJECTDIR}/nlc.o \
	${OBJECTDIR}/object.o \
	${OBJECTDIR}/parser.o \
	${OBJECTDIR}/parser.yy.o \
	${OBJECTDIR}/syntax_help.o \
	${OBJECTDIR}/term.o \
	${OBJECTDIR}/test/alg_test.o \
	${OBJECTDIR}/test/compiler_test.o \
	${OBJECTDIR}/test/eval_test.o \
	${OBJECTDIR}/test/example_test.o \
	${OBJECTDIR}/test/lexer_test.o \
	${OBJECTDIR}/test/nlc_test.o \
	${OBJECTDIR}/test/object_test.o \
	${OBJECTDIR}/test/parser_test.o \
	${OBJECTDIR}/test/rational_test.o \
	${OBJECTDIR}/variable.o \
	${OBJECTDIR}/version.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=`llvm-config-13 --cxxflags` -std=c++17 -fexceptions -fcxx-exceptions -Wall -Wextra -Werror -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual -Wno-trigraphs -Wno-invalid-source-encoding -stdlib=libstdc++ -Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=switch -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error                                         -Wno-undefined-var-template -Wno-switch                         
CXXFLAGS=`llvm-config-13 --cxxflags` -std=c++17 -fexceptions -fcxx-exceptions -Wall -Wextra -Werror -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual -Wno-trigraphs -Wno-invalid-source-encoding -stdlib=libstdc++ -Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=switch -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error                                         -Wno-undefined-var-template -Wno-switch                         

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../contrib/libtorch/lib -Wl,-rpath,'.' -Wl,-rpath,'../contrib/libtorch/lib' -lc10 -ltorch_cpu -lcrypto

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../output/nlc

../output/nlc: ${OBJECTFILES}
	${MKDIR} -p ../output
	${LINK.cc} -o ../output/nlc ${OBJECTFILES} ${LDLIBSOPTIONS} `llvm-config-13 --libs --ldflags` -fuse-ld=lld-13 -g

${OBJECTDIR}/_ext/e16507f5/logger.o: ../contrib/logger/logger.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e16507f5
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e16507f5/logger.o ../contrib/logger/logger.cpp

${OBJECTDIR}/builtin.o: builtin.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/builtin.o builtin.cpp

${OBJECTDIR}/context.o: context.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/context.o context.cpp

${OBJECTDIR}/lexer.o: lexer.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lexer.o lexer.cpp

: lexer.h parser.yy.h parser.yy.cpp location.hh
	@echo Выполнение шага пользовательского сборки
	

.NO_PARALLEL:lexer.yy.cpp lexer.yy.h
lexer.yy.cpp lexer.yy.h: lexer.l parser.y parser.yy.h parser.yy.cpp location.hh term.h
	@echo 
	flex  --outfile=lexer.yy.cpp --header-file=lexer.yy.h --noline  lexer.l

${OBJECTDIR}/lexer.yy.o: lexer.yy.cpp parser.y parser.yy.h parser.yy.cpp location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lexer.yy.o lexer.yy.cpp

${OBJECTDIR}/newlang.o: newlang.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/newlang.o newlang.cpp

${OBJECTDIR}/nlc.o: nlc.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nlc.o nlc.cpp

${OBJECTDIR}/object.o: object.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/object.o object.cpp

${OBJECTDIR}/parser.o: parser.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/parser.o parser.cpp

.NO_PARALLEL:parser.yy.h parser.yy.cpp location.hh
parser.yy.h parser.yy.cpp location.hh: parser.y
	@echo ************* Bison compile ************* 
	bison --output-file=parser.yy.cpp --defines=parser.yy.h --warnings=all parser.y

${OBJECTDIR}/parser.yy.o: parser.yy.cpp parser.y
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/parser.yy.o parser.yy.cpp

: parser.yy.h parser.y
	@echo Выполнение шага пользовательского сборки
	

pch.h.pch: pch.h
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/syntax_help.o: syntax_help.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/syntax_help.o syntax_help.cpp

${OBJECTDIR}/term.o: term.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/term.o term.cpp

: term.h parser.yy.cpp location.hh
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/test/alg_test.o: test/alg_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/alg_test.o test/alg_test.cpp

${OBJECTDIR}/test/compiler_test.o: test/compiler_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/compiler_test.o test/compiler_test.cpp

${OBJECTDIR}/test/eval_test.o: test/eval_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/eval_test.o test/eval_test.cpp

${OBJECTDIR}/test/example_test.o: test/example_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/example_test.o test/example_test.cpp

${OBJECTDIR}/test/lexer_test.o: test/lexer_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/lexer_test.o test/lexer_test.cpp

${OBJECTDIR}/test/nlc_test.o: test/nlc_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/nlc_test.o test/nlc_test.cpp

${OBJECTDIR}/test/object_test.o: test/object_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/object_test.o test/object_test.cpp

${OBJECTDIR}/test/parser_test.o: test/parser_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/parser_test.o test/parser_test.cpp

${OBJECTDIR}/test/rational_test.o: test/rational_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/rational_test.o test/rational_test.cpp

${OBJECTDIR}/variable.o: variable.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/variable.o variable.cpp

${OBJECTDIR}/version.o: version.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -O2 -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/llvm-13/lib/clang/13.0.1/include -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/version.o version.cpp

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
