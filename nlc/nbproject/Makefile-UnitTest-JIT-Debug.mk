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
CND_CONF=UnitTest-JIT-Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1e501df/gtest-all.o \
	${OBJECTDIR}/test/diag_test.o \
	${OBJECTDIR}/test/logger_test.o \
	${OBJECTDIR}/test/nlc_test.o \
	${OBJECTDIR}/test/object_test.o \
	${OBJECTDIR}/test/rational_test.o \
	${OBJECTDIR}/test/unittest.o


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
LDLIBSOPTIONS=-L../contrib/libtorch/lib -L../output -Wl,-rpath,'.' -lpthread -lc10 -ltorch -ltorch_cpu -lnlc-rt -lnlc-jit -lcrypt -lzip -ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../output/nlc-jit-unttest

../output/nlc-jit-unttest: ${OBJECTFILES}
	${MKDIR} -p ../output
	${LINK.cc} -o ../output/nlc-jit-unttest ${OBJECTFILES} ${LDLIBSOPTIONS} `llvm-config-18 --link-static --system-libs --libs all` -g -fvisibility=default -Wl,--export-dynamic -Wl,--exclude-libs,ALL -lclang-cpp

${OBJECTDIR}/_ext/1e501df/gtest-all.o: ../contrib/googletest/googletest/src/gtest-all.cc
	${MKDIR} -p ${OBJECTDIR}/_ext/1e501df
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -I../lib -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1e501df/gtest-all.o ../contrib/googletest/googletest/src/gtest-all.cc

${OBJECTDIR}/test/diag_test.o: test/diag_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -I../lib -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/diag_test.o test/diag_test.cpp

${OBJECTDIR}/test/logger_test.o: test/logger_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -I../lib -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/logger_test.o test/logger_test.cpp

${OBJECTDIR}/test/nlc_test.o: test/nlc_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -I../lib -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/nlc_test.o test/nlc_test.cpp

${OBJECTDIR}/test/object_test.o: test/object_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -I../lib -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/object_test.o test/object_test.cpp

${OBJECTDIR}/test/rational_test.o: test/rational_test.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -I../lib -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/rational_test.o test/rational_test.cpp

${OBJECTDIR}/test/unittest.o: test/unittest.cpp
	${MKDIR} -p ${OBJECTDIR}/test
	${RM} "$@.d"
	$(COMPILE.cc) -g -DBUILD_DEBUG -DBUILD_UNITTEST -DGTEST_HAS_CXXABI_H_=0 -I../lib -I../contrib/libtorch/include -I../contrib/libtorch/include/torch/csrc/api/include -I../contrib/fmt/include -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/test/unittest.o test/unittest.cpp

# Subprojects
.build-subprojects:
	cd /home/rsashka/SOURCE/NewLang/newlang/lib && ${MAKE}  -f Makefile CONF=nlc-rt-Debug
	cd /home/rsashka/SOURCE/NewLang/newlang/lib && ${MAKE}  -f Makefile CONF=nlc-jit-Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd /home/rsashka/SOURCE/NewLang/newlang/lib && ${MAKE}  -f Makefile CONF=nlc-rt-Debug clean
	cd /home/rsashka/SOURCE/NewLang/newlang/lib && ${MAKE}  -f Makefile CONF=nlc-jit-Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
