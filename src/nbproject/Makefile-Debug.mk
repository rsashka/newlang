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
CND_CONF=Debug
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
	${OBJECTDIR}/compiler.o \
	${OBJECTDIR}/context.o \
	${OBJECTDIR}/diag.o \
	${OBJECTDIR}/dsl.o \
	${OBJECTDIR}/lexer.o \
	${OBJECTDIR}/lexer.yy.o \
	${OBJECTDIR}/macro.o \
	${OBJECTDIR}/module.o \
	${OBJECTDIR}/nlc.o \
	${OBJECTDIR}/object.o \
	${OBJECTDIR}/parser.o \
	${OBJECTDIR}/parser.yy.o \
	${OBJECTDIR}/runtime.o \
	${OBJECTDIR}/syntax_help_en.o \
	${OBJECTDIR}/syntax_help_ru.o \
	${OBJECTDIR}/system.o \
	${OBJECTDIR}/term.o \
	${OBJECTDIR}/types.o \
	${OBJECTDIR}/version.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=`llvm-config-15 --cxxflags` -std=c++17 -fexceptions -fcxx-exceptions -Wall -Wextra -Werror -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual -Wno-trigraphs -Wno-invalid-source-encoding -stdlib=libstdc++ -Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=switch -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error                                         -Wno-undefined-var-template -Wno-switch  -fvisibility=default 
CXXFLAGS=`llvm-config-15 --cxxflags` -std=c++17 -fexceptions -fcxx-exceptions -Wall -Wextra -Werror -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual -Wno-trigraphs -Wno-invalid-source-encoding -stdlib=libstdc++ -Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=switch -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error                                         -Wno-undefined-var-template -Wno-switch  -fvisibility=default 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../contrib/libtorch/lib -L/usr/lib/x86_64-linux-gnu -L/usr/lib/llvm-15/lib -Wl,-rpath,'.' -Wl,-rpath,'../contrib/libtorch/lib' -Wl,-rpath,'/usr/lib/x86_64-linux-gnu' -lpthread -lc10 -ltorch -ltorch_cpu -lcrypto -ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../output/nlc

../output/nlc: ${OBJECTFILES}
	${MKDIR} -p ../output
	${LINK.cc} -o ../output/nlc ${OBJECTFILES} ${LDLIBSOPTIONS} `llvm-config-15 --libs --ldflags` -fuse-ld=lld-15 -g -lclangTooling -lclangFrontendTool -lclangFrontend -lclangDriver -lclangSerialization -lclangCodeGen -lclangParse -lclangSema -lclangStaticAnalyzerFrontend -lclangStaticAnalyzerCheckers -lclangStaticAnalyzerCore -lclangAnalysis -lclangARCMigrate -lclangRewrite -lclangRewriteFrontend -lclangEdit -lclangAST -lclangLex -lclangBasic  -lclangASTMatchers -lclangSupport -fvisibility=default -Wl,--export-dynamic

${OBJECTDIR}/_ext/e16507f5/logger.o: ../contrib/logger/logger.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e16507f5
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e16507f5/logger.o ../contrib/logger/logger.cpp

${OBJECTDIR}/builtin.o: builtin.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/builtin.o builtin.cpp

${OBJECTDIR}/compiler.o: compiler.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/compiler.o compiler.cpp

${OBJECTDIR}/context.o: context.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/context.o context.cpp

${OBJECTDIR}/diag.o: diag.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/diag.o diag.cpp

${OBJECTDIR}/dsl.o: dsl.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/dsl.o dsl.cpp

${OBJECTDIR}/lexer.o: lexer.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lexer.o lexer.cpp

: lexer.h parser.yy.h parser.yy.cpp location.hh
	@echo Выполнение шага пользовательского сборки
	

.NO_PARALLEL:lexer.yy.cpp lexer.yy.h
lexer.yy.cpp lexer.yy.h: lexer.l parser.y parser.yy.h parser.yy.cpp location.hh term.h
	@echo 
	flex  --outfile=lexer.yy.cpp --header-file=lexer.yy.h --noline  lexer.l

${OBJECTDIR}/lexer.yy.o: lexer.yy.cpp parser.y parser.yy.h parser.yy.cpp location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lexer.yy.o lexer.yy.cpp

${OBJECTDIR}/macro.o: macro.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/macro.o macro.cpp

${OBJECTDIR}/module.o: module.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/module.o module.cpp

${OBJECTDIR}/nlc.o: nlc.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nlc.o nlc.cpp

${OBJECTDIR}/object.o: object.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/object.o object.cpp

${OBJECTDIR}/parser.o: parser.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/parser.o parser.cpp

.NO_PARALLEL:parser.yy.h parser.yy.cpp location.hh
parser.yy.h parser.yy.cpp location.hh: parser.y
	@echo ************* Bison compile ************* 
	bison --output-file=parser.yy.cpp --defines=parser.yy.h --warnings=all parser.y

${OBJECTDIR}/parser.yy.o: parser.yy.cpp parser.y
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/parser.yy.o parser.yy.cpp

: parser.yy.h parser.y
	@echo Выполнение шага пользовательского сборки
	

pch.h.pch: pch.h
	@echo Выполнение шага пользовательского сборки
	clang++ `llvm-config-15 --cxxflags` -std=c++17 -fexceptions -fcxx-exceptions -Wall -Wextra -Werror -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wmissing-declarations -Wredundant-decls -Wshadow -Woverloaded-virtual -Wno-trigraphs -Wno-invalid-source-encoding -stdlib=libstdc++ -Wno-error=unused-variable -Wno-error=unused-parameter -Wno-error=switch -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error                                         -Wno-undefined-var-template -Wno-switch   -fvisibility=default              -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include                 -x c++-header -o pch.h.pch pch.h -verify-pch -fpch-validate-input-files-content

${OBJECTDIR}/runtime.o: runtime.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/runtime.o runtime.cpp

${OBJECTDIR}/syntax_help_en.o: syntax_help_en.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/syntax_help_en.o syntax_help_en.cpp

${OBJECTDIR}/syntax_help_ru.o: syntax_help_ru.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/syntax_help_ru.o syntax_help_ru.cpp

${OBJECTDIR}/system.o: system.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/system.o system.cpp

${OBJECTDIR}/term.o: term.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/term.o term.cpp

: term.h parser.yy.cpp location.hh
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/types.o: types.cpp pch.h.pch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -include-pch pch.h.pch -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/types.o types.cpp

${OBJECTDIR}/version.o: version.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -I. -I.. -I/usr/lib/clang/15/include -I/usr/include/llvm-c-15 -I/usr/include/x86_64-linux-gnu/c++/10 -I/usr/include/c++/10 -I/usr/local/include -I/usr/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/version.o version.cpp

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
