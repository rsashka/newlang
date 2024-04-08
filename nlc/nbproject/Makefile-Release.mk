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
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1e501df/gtest-all.o \
	${OBJECTDIR}/nlc.o \
	${OBJECTDIR}/test/diag_test.o \
	${OBJECTDIR}/test/logger_test.o \
	${OBJECTDIR}/test/object_test.o \
	${OBJECTDIR}/test/rational_test.o \
	${OBJECTDIR}/unittest.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libnlc.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libnlc.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libnlc.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/_ext/1e501df/gtest-all.o: ../contrib/googletest/googletest/src/gtest-all.cc
	${MKDIR} -p ${OBJECTDIR}/_ext/1e501df
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1e501df/gtest-all.o ../contrib/googletest/googletest/src/gtest-all.cc

${OBJECTDIR}/nlc.o: nlc.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/nlc.o nlc.cpp

${OBJECTDIR}/test/diag_test.o: test/diag_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/diag_test.o test/diag_test.cpp

${OBJECTDIR}/test/logger_test.o: test/logger_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/logger_test.o test/logger_test.cpp

${OBJECTDIR}/test/object_test.o: test/object_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/object_test.o test/object_test.cpp

${OBJECTDIR}/test/rational_test.o: test/rational_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/rational_test.o test/rational_test.cpp

${OBJECTDIR}/unittest.o: unittest.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/unittest.o unittest.cpp

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
