cmake_minimum_required(VERSION 3.17.0 FATAL_ERROR)

# set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_MESSAGE_LOG_LEVEL DEBUG)


set(VERSION_MAJOR 0) # Set manually
set(VERSION_MINOR 3) # Set manually
set(VERSION_PATCH 0) # Set manually

set(VERSION_HEADER      "version.h")
set(VERSION_FILE        "version.cpp")

if(WIN32)
    set(CMD cmd "/c")
    message(FATAL_ERROR "Support windows not tested!")
elseif(UNIX)
    set(CMD bash "-c")
else()
    message(FATAL_ERROR "Unsupport platform!")
endif()

execute_process(COMMAND ${CMD} "git describe --abbrev=0 --tags" OUTPUT_VARIABLE  GIT_TAG_VERSION)
string(REPLACE "\n" "" GIT_TAG_VERSION ${GIT_TAG_VERSION})
execute_process(COMMAND ${CMD} "git rev-parse --short HEAD"     OUTPUT_VARIABLE  GIT_SHORT_HASH)
string(REPLACE "\n" "" GIT_SHORT_HASH ${GIT_SHORT_HASH})

string(TIMESTAMP DATE_BUILD "%y.%m.%d %H:%M:%S")
set(GIT_SOURCE_ID "${GIT_TAG_VERSION}-${GIT_SHORT_HASH}")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# message(DEBUG ${GIT_TAG_VERSION})
# message(DEBUG ${GIT_SHORT_HASH})
# message(DEBUG ${DATE_BUILD})
# message(DEBUG ${GIT_SOURCE_ID})
# message(DEBUG ${VERSION})

if("${GIT_SHORT_HASH}" STREQUAL "")
    message(FATAL_ERROR "Fail read git hash on repo!")
endif()

if(NOT "${GIT_TAG_VERSION}" STREQUAL "v${VERSION}")
    message(FATAL_ERROR "Git TAG ${GIT_TAG_VERSION} differ version v${VERSION}")
endif()


project(NewLang VERSION ${VERSION} HOMEPAGE_URL "https://newlang.net" LANGUAGES CXX)
message(STATUS "Used git source ${GIT_SOURCE_ID}")

execute_process(COMMAND ${CMD} "echo -n \"\" > ${CMAKE_SOURCE_DIR}/docs/newlang_doc.md")
foreach(DOC_FILE 
        syntax.md
        types.md
        type_nor.md
        type_str.md
        type_func.md
        type_oop.md
        type_native.md
        type_iter.md
        type_other.md
        ops.md
        syntax_dsl.md
    )
    execute_process(COMMAND ${CMD} "cat ${CMAKE_SOURCE_DIR}/docs/${DOC_FILE} >> ${CMAKE_SOURCE_DIR}/docs/newlang_doc.md")
    execute_process(COMMAND ${CMD} "echo \"-------\n${DOC_FILE}\n\n\" >> ../docs/newlang_doc.md")
endforeach()


function(CHECK_NEW_FILE  name  skip)
    if(NOT EXISTS "${name}.temp")
        message(FATAL_ERROR "File name '${name}.temp' not found!")
    endif()
    if(NOT EXISTS "${name}")
      file(RENAME "${name}.temp" "${name}")
    else()
        file(READ "${name}.temp" NEW_FILE_STR)
        file(READ "${name}" OLD_FILE_STR)

        string(SUBSTRING "${NEW_FILE_STR}" ${skip} 100000  NEW_FILE_STR)
        string(SUBSTRING "${OLD_FILE_STR}" ${skip} 100000  OLD_FILE_STR)

        if("${NEW_FILE_STR}" STREQUAL "${OLD_FILE_STR}")
            file(REMOVE "${name}.temp")
            message(STATUS "Use file ${name} for current build")
        else()
            file(REMOVE "${name}")
            file(RENAME "${name}.temp" "${name}")
            message(STATUS "Make file ${name} for version ${GIT_SOURCE_ID} at date ${DATE_BUILD}")
        endif()
    endif()
endfunction()


execute_process(COMMAND ${CMD} "pandoc -t plain ${CMAKE_SOURCE_DIR}/docs/newlang_doc.md > ${CMAKE_SOURCE_DIR}/docs/newlang_doc.txt")

execute_process(COMMAND ${CMD} "${CMAKE_SOURCE_DIR}/contrib/text2cpp/output/bin/text2cpp ${CMAKE_SOURCE_DIR}/docs/newlang_doc.md  ${CMAKE_SOURCE_DIR}/src/syntax_help.cpp.temp  newlang_syntax_help  c")
cmake_language(CALL CHECK_NEW_FILE  "${CMAKE_SOURCE_DIR}/src/syntax_help.cpp" 0)

execute_process(COMMAND ${CMD} "${CMAKE_SOURCE_DIR}/contrib/text2cpp/output/bin/text2cpp  ${CMAKE_SOURCE_DIR}/examples/dsl.nlp  ${CMAKE_SOURCE_DIR}/src/dsl.cpp.temp  newlang_dsl  c")
cmake_language(CALL CHECK_NEW_FILE  "${CMAKE_SOURCE_DIR}/src/dsl.cpp" 0)


execute_process(COMMAND ${CMD} "echo -n \"\" > ${CMAKE_SOURCE_DIR}/src/${VERSION_HEADER}.temp")
foreach(VERSION_HEADER_STR
        "/** @file ${VERSION_HEADER}"
        "* Auto generate file for identification current build"
        "* Date build ${DATE_BUILD}"
        "*/"
        ""
        "#define VERSION_DATE_BUILD_STR \"${DATE_BUILD}\""
        "#define VERSION_SOURCE_FULL_ID \"${GIT_SOURCE_ID} ${DATE_BUILD}\""
        ""
        "#include <stdint.h>"
        ""
        "extern const uint8_t VERSION_MAJOR;"
        "extern const uint8_t VERSION_MINOR;"
        "extern const uint8_t VERSION_PATCH;"
        "extern const uint16_t VERSION_BUILD;"
        ""
        "#define VERSION (${VERSION_MAJOR} << 4 | ${VERSION_MINOR})"
        "#define VERSION_GIT_SOURCE \"${GIT_SOURCE_ID}\""
        ""
        "extern const char * GIT_SOURCE;"
        "extern const char * DATE_BUILD_STR;"
        "extern const char * SOURCE_FULL_ID;"
        ""
    )
    execute_process(COMMAND ${CMD} "echo '${VERSION_HEADER_STR}' >> ${CMAKE_SOURCE_DIR}/src/${VERSION_HEADER}.temp")
endforeach()
cmake_language(CALL CHECK_NEW_FILE  "${CMAKE_SOURCE_DIR}/src/${VERSION_HEADER}" 250)


execute_process(COMMAND ${CMD} "echo -n \"\" > ${CMAKE_SOURCE_DIR}/src/${VERSION_FILE}.temp")
foreach(VERSION_FILE_STR
        "/** @file ${VERSION_FILE}"
        "* Auto generate file for identification current build"
        "* Date build ${DATE_BUILD}"
        "*/"
        ""
        "#include \"version.h\""
        ""
        "const uint8_t VERSION_MAJOR=${VERSION_MAJOR};"
        "const uint8_t VERSION_MINOR=${VERSION_MINOR};"
        "const uint8_t VERSION_PATCH=${VERSION_PATCH};"
        ""
        "const char * GIT_SOURCE = VERSION_GIT_SOURCE;"
        "const char * DATE_BUILD_STR = VERSION_DATE_BUILD_STR;"
        "const char * SOURCE_FULL_ID = VERSION_SOURCE_FULL_ID;"
        ""
    )
    execute_process(COMMAND ${CMD} "echo '${VERSION_FILE_STR}' >> ${CMAKE_SOURCE_DIR}/src/${VERSION_FILE}.temp")
endforeach()
cmake_language(CALL CHECK_NEW_FILE  "${CMAKE_SOURCE_DIR}/src/${VERSION_FILE}" 110)





get_directory_property(IS_SUBPROJECT PARENT_DIRECTORY)



# option(MYLIB_TESTING "Включить модульное тестирование" ON)
# option(MYLIB_COVERAGE "Включить измерение покрытия кода тестами" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(COMMAND ${CMD} "llvm-config-15 --cxxflags" OUTPUT_VARIABLE  LLVM_CONFIG)
separate_arguments(LLVM_CONFIG UNIX_COMMAND ${LLVM_CONFIG})

add_compile_options(
    ${LLVM_CONFIG}
    -Wno-trigraphs
    -Winvalid-pch
    -Werror=return-type
    -Wformat
    -Wuninitialized

    -fexceptions
    -fcxx-exceptions
    -Wall
    -Wextra
    -Werror
    -Wfloat-equal
    -Wundef
    -Wcast-align
    -Wwrite-strings
    -Wmissing-declarations
    -Wredundant-decls
    -Wshadow
    -Woverloaded-virtual
    -Wno-trigraphs
    -Wno-invalid-source-encoding
    
    -stdlib=libstdc++
    -fvisibility=default
    
    -Wno-error=unused-variable
    -Wno-error=unused-parameter
    -fsanitize=undefined-trap
    -fsanitize-undefined-trap-on-error

    # remove after correct source
    -Wno-error=switch
    -Wno-error=trigraphs
    -Wno-undefined-var-template
    -Wno-switch
)



set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


execute_process(COMMAND ${CMD} "${CMAKE_SOURCE_DIR}/src/compile_syntax.sh" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/src")

# add_custom_command(
#     OUTPUT
#         # ${ATXT}
#         ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.yy.h
#         ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.yy.cpp
#         ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.yy.h
#         ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.yy.cpp
#         ${CMAKE_CURRENT_SOURCE_DIR}/src/location.hh
#     COMMAND ./compile_syntax.sh
#     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
#     COMMENT "Compile syntax from files parser.y and lexer.l"
#     DEPENDS
#         ${CMAKE_CURRENT_SOURCE_DIR}/src/compile_syntax.sh
#         ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.y
#         ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.l
#     VERBATIM)
 

file(GLOB src_SRC
    # Generated files
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.yy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.yy.cpp

    # Source files
    ${CMAKE_CURRENT_SOURCE_DIR}/src/builtin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/newlang.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/object.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/term.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/variable.cpp
    

    # Contrib files
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/logger/logger.cpp

    #contrib/tensorboard_logger/src/crc.cc
    #contrib/tensorboard_logger/src/tensorboard_logger.cc
    )


file(GLOB TEST_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test/*.cpp)
file(GLOB NLC_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nlc.cpp)

link_directories(contrib/libtorch/lib)
link_directories(contrib/libffi/output/lib)

link_libraries(
    ${CMAKE_DL_LIBS}
    Threads::Threads
    libffi.a
    c10 
    torch 
    torch_cpu 
    crypto
    LLVM-15
    )

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})

add_executable(nlc ${src_SRC})
target_sources(nlc PRIVATE ${NLC_SRC})

add_executable(newlang-unit-tests ${src_SRC})
target_sources(newlang-unit-tests PRIVATE ${TEST_SRC})



ADD_DEFINITIONS(-DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG)
# ADD_DEFINITIONS(-DPDC_WIDE)
ADD_DEFINITIONS(-DDEBUG)

target_compile_options(newlang-unit-tests PRIVATE -DUNITTEST)


target_precompile_headers(newlang-unit-tests PRIVATE src/pch.h)
target_precompile_headers(nlc PRIVATE src/pch.h)
SET(PCH_DST src/pch.h)





target_sources(newlang-unit-tests PRIVATE contrib/googletest/googletest/src/gtest_main.cc)
target_sources(newlang-unit-tests PRIVATE contrib/googletest/googletest/src/gtest-all.cc)

target_include_directories(newlang-unit-tests PUBLIC contrib/googletest/googletest)
target_include_directories(newlang-unit-tests PUBLIC contrib/googletest/googletest/include)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src

    contrib/Lyra/include
    contrib/libtorch/include/torch/csrc/api/include
    contrib/libtorch/include
    contrib/tensorboard_logger/include

    )


# if(NOT MYLIB_TESTING)
#     message(STATUS "Тестирование проекта Mylib выключено")
# elseif(IS_SUBPROJECT)
#     message(STATUS "Mylib не тестируется в режиме подмодуля")
# else()
#     target_sources(newlang-unit-tests PRIVATE src/test/)
# endif()

# enable_testing()
