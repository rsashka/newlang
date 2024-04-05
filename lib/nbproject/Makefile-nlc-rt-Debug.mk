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
CND_CONF=nlc-rt-Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/b0ad39d/format.o \
	${OBJECTDIR}/logger.o \
	${OBJECTDIR}/object.o \
	${OBJECTDIR}/rational.o \
	${OBJECTDIR}/version.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=`llvm-config-18 --cxxflags` `cat build_options.txt | tr -d '\r\n'`  -fsanitize=undefined -fsanitize-trap=undefined    -gdwarf-4  -ggdb 
CXXFLAGS=`llvm-config-18 --cxxflags` `cat build_options.txt | tr -d '\r\n'`  -fsanitize=undefined -fsanitize-trap=undefined    -gdwarf-4  -ggdb 

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../output/libnlc-rt.${CND_DLIB_EXT}

../output/libnlc-rt.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ../output
	${LINK.cc} -o ../output/libnlc-rt.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/_ext/b0ad39d/format.o: ../contrib/fmt/src/format.cc
	${MKDIR} -p ${OBJECTDIR}/_ext/b0ad39d
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/b0ad39d/format.o ../contrib/fmt/src/format.cc

${OBJECTDIR}/logger.o: logger.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/logger.o logger.cpp

${OBJECTDIR}/object.o: object.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/object.o object.cpp

${OBJECTDIR}/rational.o: rational.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/rational.o rational.cpp

${OBJECTDIR}/version.o: version.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/version.o version.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
