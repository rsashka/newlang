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
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=GCOV
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1e501df/gtest-all.o \
	${OBJECTDIR}/_ext/e16507f5/logger.o \
	${OBJECTDIR}/analysis.o \
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
	${OBJECTDIR}/test/alg_test.o \
	${OBJECTDIR}/test/ast_test.o \
	${OBJECTDIR}/test/compiler_test.o \
	${OBJECTDIR}/test/diag_test.o \
	${OBJECTDIR}/test/example_test.o \
	${OBJECTDIR}/test/lexer_test.o \
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
CCFLAGS=-std=c++17 --no-gnu-unique --coverage
CXXFLAGS=-std=c++17 --no-gnu-unique --coverage

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lpthread -ldl ../contrib/PDCurses/x11/libXCurses.a -lXaw -lXmu -lXt -lX11 -lXpm -lgcov

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk dist/gcov.run

dist/gcov.run: ../contrib/PDCurses/x11/libXCurses.a

dist/gcov.run: ${OBJECTFILES}
	${MKDIR} -p dist
	${LINK.cc} -o dist/gcov.run ${OBJECTFILES} ${LDLIBSOPTIONS} -Wl,--export-dynamic

${OBJECTDIR}/_ext/1e501df/gtest-all.o: ../contrib/googletest/googletest/src/gtest-all.cc
	${MKDIR} -p ${OBJECTDIR}/_ext/1e501df
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1e501df/gtest-all.o ../contrib/googletest/googletest/src/gtest-all.cc

${OBJECTDIR}/_ext/e16507f5/logger.o: ../contrib/logger/logger.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e16507f5
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e16507f5/logger.o ../contrib/logger/logger.cpp

${OBJECTDIR}/analysis.o: analysis.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/analysis.o analysis.cpp

${OBJECTDIR}/builtin.o: builtin.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/builtin.o builtin.cpp

${OBJECTDIR}/compiler.o: compiler.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/compiler.o compiler.cpp

${OBJECTDIR}/context.o: context.cpp parser.h parser.yy.h
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/context.o context.cpp

${OBJECTDIR}/diag.o: diag.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/diag.o diag.cpp

${OBJECTDIR}/dsl.o: dsl.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/dsl.o dsl.cpp

${OBJECTDIR}/lexer.o: lexer.cpp parser.yy.h parser.yy.cpp location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lexer.o lexer.cpp

: lexer.h parser.yy.h parser.yy.cpp location.hh
	@echo Выполнение шага пользовательского сборки
	

.NO_PARALLEL:lexer.yy.cpp lexer.yy.h
lexer.yy.cpp lexer.yy.h: lexer.l parser.y parser.yy.h parser.yy.cpp location.hh types.h
	@echo 
	flex  --outfile=lexer.yy.cpp --header-file=lexer.yy.h --noline  lexer.l

${OBJECTDIR}/lexer.yy.o: lexer.yy.cpp parser.y parser.yy.h parser.yy.cpp location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lexer.yy.o lexer.yy.cpp

${OBJECTDIR}/macro.o: macro.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/macro.o macro.cpp

${OBJECTDIR}/module.o: module.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/module.o module.cpp

${OBJECTDIR}/nlc.o: nlc.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nlc.o nlc.cpp

${OBJECTDIR}/object.o: object.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/object.o object.cpp

${OBJECTDIR}/parser.o: parser.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/parser.o parser.cpp

.NO_PARALLEL:parser.yy.h parser.yy.cpp location.hh
parser.yy.h parser.yy.cpp location.hh: parser.y
	@echo ************* Bison compile ************* 
	bison --output-file=parser.yy.cpp --defines=parser.yy.h --warnings=all parser.y

${OBJECTDIR}/parser.yy.o: parser.yy.cpp parser.y
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/parser.yy.o parser.yy.cpp

: parser.yy.h parser.y
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/runtime.o: runtime.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/runtime.o runtime.cpp

${OBJECTDIR}/syntax_help_en.o: syntax_help_en.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/syntax_help_en.o syntax_help_en.cpp

${OBJECTDIR}/syntax_help_ru.o: syntax_help_ru.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/syntax_help_ru.o syntax_help_ru.cpp

${OBJECTDIR}/system.o: system.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/system.o system.cpp

${OBJECTDIR}/term.o: term.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/term.o term.cpp

: term.h parser.yy.cpp location.hh
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/test/alg_test.o: test/alg_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/alg_test.o test/alg_test.cpp

${OBJECTDIR}/test/ast_test.o: test/ast_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/ast_test.o test/ast_test.cpp

${OBJECTDIR}/test/compiler_test.o: test/compiler_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/compiler_test.o test/compiler_test.cpp

${OBJECTDIR}/test/diag_test.o: test/diag_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/diag_test.o test/diag_test.cpp

${OBJECTDIR}/test/example_test.o: test/example_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/example_test.o test/example_test.cpp

${OBJECTDIR}/test/lexer_test.o: test/lexer_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/lexer_test.o test/lexer_test.cpp

${OBJECTDIR}/test/macro_test.o: test/macro_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/macro_test.o test/macro_test.cpp

${OBJECTDIR}/test/module_test.o: test/module_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/module_test.o test/module_test.cpp

${OBJECTDIR}/test/nlc_test.o: test/nlc_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/nlc_test.o test/nlc_test.cpp

${OBJECTDIR}/test/object_test.o: test/object_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/object_test.o test/object_test.cpp

${OBJECTDIR}/test/parser_analysis_test.o: test/parser_analysis_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/parser_analysis_test.o test/parser_analysis_test.cpp

${OBJECTDIR}/test/parser_test.o: test/parser_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/parser_test.o test/parser_test.cpp

${OBJECTDIR}/test/rational_test.o: test/rational_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/rational_test.o test/rational_test.cpp

${OBJECTDIR}/test/run_test.o: test/run_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/run_test.o test/run_test.cpp

${OBJECTDIR}/test/system_test.o: test/system_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/system_test.o test/system_test.cpp

${OBJECTDIR}/types.o: types.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/types.o types.cpp

${OBJECTDIR}/version.o: version.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Tensorflow/bazel-bin/tensorflow/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/version.o version.cpp

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
	${RM} 
	${RM} 
	${RM} 

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
