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
CC=i686-w64-mingw32-gcc-9.3-posix
CCC=i686-w64-mingw32-g++-posix
CXX=i686-w64-mingw32-g++-posix
FC=gfortran
AS=i686-w64-mingw32-as

# Macros
CND_PLATFORM=Win32-i686-Linux
CND_DLIB_EXT=so
CND_CONF=UnitTest-Win32
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
	${OBJECTDIR}/test/oop_test.o \
	${OBJECTDIR}/test/parser_test.o \
	${OBJECTDIR}/test/rational_test.o \
	${OBJECTDIR}/variable.o \
	${OBJECTDIR}/version.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-std=c++17 --no-gnu-unique -Wno-trigraphs -Winvalid-pch -Werror=return-type -Wmaybe-uninitialized -Wuninitialized -Wformat -Wmaybe-uninitialized
CXXFLAGS=-std=c++17 --no-gnu-unique -Wno-trigraphs -Winvalid-pch -Werror=return-type -Wmaybe-uninitialized -Wuninitialized -Wformat -Wmaybe-uninitialized

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L../contrib/libtorch/lib -Wl,-rpath,'../contrib/libtorch/lib' -lpthread -ldl ../contrib/libffi/output/lib/libffi.a -lcrypto -lLLVM-13 -ltorch -lc10 -ltorch_cpu

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk newlang_test

newlang_test: ../contrib/libffi/output/lib/libffi.a

newlang_test: ${OBJECTFILES}
	${LINK.cc} -o newlang_test ${OBJECTFILES} ${LDLIBSOPTIONS} -Wl,--export-dynamic

${OBJECTDIR}/_ext/1e501df/gtest-all.o: ../contrib/googletest/googletest/src/gtest-all.cc
	${MKDIR} -p ${OBJECTDIR}/_ext/1e501df
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -std=c++14 -Wno-error -Wno-all -Wno-extra -Wno-ctor-dtor-privacy -Wno-undef -Wno-sign-compare -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1e501df/gtest-all.o ../contrib/googletest/googletest/src/gtest-all.cc

${OBJECTDIR}/_ext/e16507f5/logger.o: ../contrib/logger/logger.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e16507f5
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e16507f5/logger.o ../contrib/logger/logger.cpp

${OBJECTDIR}/builtin.o: builtin.cpp parser.h parser.yy.h pch.h.gch location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/builtin.o builtin.cpp

: builtin.h pch.h.gch
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/context.o: context.cpp parser.h parser.yy.h pch.h.gch location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/context.o context.cpp

: context.h pch.h.gch
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/lexer.o: lexer.cpp location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lexer.o lexer.cpp

: lexer.h parser.yy.h parser.yy.cpp location.hh pch.h.gch
	@echo Выполнение шага пользовательского сборки
	

.NO_PARALLEL:lexer.yy.cpp lexer.yy.h
lexer.yy.cpp lexer.yy.h: lexer.l parser.y parser.yy.h parser.yy.cpp location.hh term.h
	@echo 
	./compile_syntax.sh

${OBJECTDIR}/lexer.yy.o: lexer.yy.cpp parser.y parser.yy.h parser.yy.cpp location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/lexer.yy.o lexer.yy.cpp

: lexer.yy.h pch.h.gch
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/newlang.o: newlang.cpp parser.h parser.yy.h pch.h.gch location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/newlang.o newlang.cpp

: newlang.h pch.h.gch
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/nlc.o: nlc.cpp parser.h parser.yy.h pch.h.gch location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nlc.o nlc.cpp

${OBJECTDIR}/object.o: object.cpp parser.h parser.yy.h pch.h.gch location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/object.o object.cpp

: object.h pch.h.gch
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/parser.o: parser.cpp parser.h parser.yy.h pch.h.gch location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/parser.o parser.cpp

: parser.h pch.h.gch
	@echo Выполнение шага пользовательского сборки
	

.NO_PARALLEL:parser.yy.h parser.yy.cpp location.hh
parser.yy.h parser.yy.cpp location.hh: parser.y
	@echo ************* Bison compile ************* 
	./compile_syntax.sh

${OBJECTDIR}/parser.yy.o: parser.yy.cpp parser.y
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/parser.yy.o parser.yy.cpp

: parser.yy.h parser.y pch.h.gch
	@echo Выполнение шага пользовательского сборки
	

pch.h.gch: pch.h
	@echo Выполнение шага пользовательского сборки
	g++ -o pch.h.gch -c pch.h -g -I.. -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include --no-gnu-unique -Wno-trigraphs -Winvalid-pch -Werror=return-type -Wmaybe-uninitialized -Wuninitialized -Wformat -Wmaybe-uninitialized -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I.. -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -std=c++17

${OBJECTDIR}/syntax_help.o: syntax_help.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/syntax_help.o syntax_help.cpp

${OBJECTDIR}/term.o: term.cpp parser.h parser.yy.h pch.h.gch location.hh
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/term.o term.cpp

: term.h parser.yy.cpp location.hh pch.h.gch
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/test/alg_test.o: test/alg_test.cpp pch.h.gch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/alg_test.o test/alg_test.cpp

${OBJECTDIR}/test/compiler_test.o: test/compiler_test.cpp pch.h.gch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/compiler_test.o test/compiler_test.cpp

${OBJECTDIR}/test/eval_test.o: test/eval_test.cpp pch.h.gch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/eval_test.o test/eval_test.cpp

${OBJECTDIR}/test/example_test.o: test/example_test.cpp pch.h.gch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/example_test.o test/example_test.cpp

${OBJECTDIR}/test/lexer_test.o: test/lexer_test.cpp pch.h.gch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/lexer_test.o test/lexer_test.cpp

${OBJECTDIR}/test/nlc_test.o: test/nlc_test.cpp pch.h.gch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/nlc_test.o test/nlc_test.cpp

${OBJECTDIR}/test/object_test.o: test/object_test.cpp pch.h.gch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/object_test.o test/object_test.cpp

${OBJECTDIR}/test/oop_test.o: test/oop_test.cpp pch.h.gch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/oop_test.o test/oop_test.cpp

${OBJECTDIR}/test/parser_test.o: test/parser_test.cpp pch.h.gch
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/parser_test.o test/parser_test.cpp

${OBJECTDIR}/test/rational_test.o: test/rational_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/rational_test.o test/rational_test.cpp

: types.h pch.h.gch
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/variable.o: variable.cpp pch.h.gch
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/variable.o variable.cpp

: variable.h pch.h.gch
	@echo Выполнение шага пользовательского сборки
	

${OBJECTDIR}/version.o: version.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DPDC_WIDE -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/Lyra/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/libtorch/include -I../contrib/tensorboard_logger/include -I/usr/lib/llvm-13/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/version.o version.cpp

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
	${RM} 
	${RM} 
	${RM} lexer.yy.cpp lexer.yy.h
	${RM} 
	${RM} 
	${RM} 
	${RM} 
	${RM} parser.yy.h parser.yy.cpp location.hh
	${RM} 
	${RM} pch.h.gch
	${RM} 
	${RM} 
	${RM} 
	${RM} 
	${RM} 

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
