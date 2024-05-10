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
CND_CONF=nlc-jit-Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/b0ad39d/format.o \
	${OBJECTDIR}/jit.o \
	${OBJECTDIR}/macro.o \
	${OBJECTDIR}/module.o \
	${OBJECTDIR}/types.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=`llvm-config-18 --cxxflags` `cat ../build_options.txt | tr -d '\r\n'`  -fsanitize=undefined -fsanitize-trap=undefined    -gdwarf-4  -ggdb 
CXXFLAGS=`llvm-config-18 --cxxflags` `cat ../build_options.txt | tr -d '\r\n'`  -fsanitize=undefined -fsanitize-trap=undefined    -gdwarf-4  -ggdb 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-Wl,-rpath,'../contrib/libtorch/lib'

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../output/libnlc-jit.${CND_DLIB_EXT}

../output/libnlc-jit.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ../output
	${LINK.cc} -o ../output/libnlc-jit.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/_ext/b0ad39d/format.o: ../contrib/fmt/src/format.cc nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}/_ext/b0ad39d
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b0ad39d/format.o ../contrib/fmt/src/format.cc

${OBJECTDIR}/jit.o: jit.cpp nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/jit.o jit.cpp

: lexer.h lexer.l parser.yy.h parser.yy.cpp location.hh nbproject/Makefile-${CND_CONF}.mk
	@echo Выполнение шага пользовательского сборки
	

.NO_PARALLEL:lexer.yy.cpp lexer.yy.h
lexer.yy.cpp lexer.yy.h: lexer.l parser.y parser.yy.h parser.yy.cpp location.hh term.h nbproject/Makefile-${CND_CONF}.mk
	@echo Выполнение шага пользовательского сборки
	flex  --outfile=lexer.yy.cpp --header-file=lexer.yy.h --noline  lexer.l

${OBJECTDIR}/macro.o: macro.cpp nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/macro.o macro.cpp

${OBJECTDIR}/module.o: module.cpp nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/module.o module.cpp

.NO_PARALLEL:parser.yy.h parser.yy.cpp location.hh
parser.yy.h parser.yy.cpp location.hh: parser.y nbproject/Makefile-${CND_CONF}.mk
	@echo Выполнение шага пользовательского сборки
	bison --output-file=parser.yy.cpp --defines=parser.yy.h --warnings=all parser.y

${OBJECTDIR}/types.o: types.cpp nbproject/Makefile-${CND_CONF}.mk
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/types.o types.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} 
	${RM} lexer.yy.cpp lexer.yy.h
	${RM} parser.yy.h parser.yy.cpp location.hh

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
