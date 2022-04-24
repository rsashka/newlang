cmake_minimum_required(VERSION 3.16.0)
project(NewLang VERSION 0.1.0 LANGUAGES CXX)

get_directory_property(IS_SUBPROJECT PARENT_DIRECTORY)

option(MYLIB_TESTING "Включить модульное тестирование" ON)
option(MYLIB_COVERAGE "Включить измерение покрытия кода тестами" OFF)

add_compile_options(
#[[   -Werror

    -Wall
    -Wextra
    -Wpedantic

    -Wcast-align
    -Wcast-qual
    -Wconversion
    -Wctor-dtor-privacy
    -Wenum-compare
    -Wfloat-equal
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    -Wredundant-decls
    -Wsign-conversion
    -Wsign-promo
]]

    --no-gnu-unique
    -Wno-trigraphs
    -Winvalid-pch
    -Werror=return-type
    -Wformat
    -Wmaybe-uninitialized
    -Wuninitialized
)


#if(NOT CMAKE_CXX_EXTENSIONS)
#    set(CMAKE_CXX_EXTENSIONS OFF)
#endif()

# add_executable(GCC core/nlc.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#file(GLOB protos "contrib/tensorboard_logger/proto/*.proto")

#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${protos})



file(GLOB CORE_SRC
    core/builtin.cpp
    core/context.cpp
    core/lexer.cpp
    core/lexer.yy.cpp
    core/newlang.cpp
    core/object.cpp
    core/parser.cpp
    core/parser.yy.cpp
    core/term.cpp
    core/variable.cpp
    
    contrib/logger/logger.cpp

    #contrib/tensorboard_logger/src/crc.cc
    #contrib/tensorboard_logger/src/tensorboard_logger.cc
    )


file(GLOB TEST_SRC
    core/test/*.cpp)

link_directories(contrib/libtorch/lib)
link_directories(contrib/libffi/output/lib)


add_executable(newlang-unit-tests ${CORE_SRC})
target_compile_features(newlang-unit-tests PUBLIC cxx_std_17)


ADD_DEFINITIONS(-DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG)
ADD_DEFINITIONS(-DPDC_WIDE)
ADD_DEFINITIONS(-DDEBUG)
ADD_DEFINITIONS(-DUNITTEST)

#target_compile_options(newlang-unit-tests PRIVATE -Wl,--export-dynamic)

target_precompile_headers(newlang-unit-tests PRIVATE core/pch.h)
SET(PCH_DST core/pch.h)


target_link_libraries(newlang-unit-tests PRIVATE 
    Threads::Threads
    libffi.a)

#cmake --build path/to/build/directory
#cmake --build path/to/build/directory --target all

#link_directories("contrib/libtorch/lib")

add_library(bar SHARED IMPORTED) # or STATIC instead of SHARED
set_target_properties(bar PROPERTIES  
    #IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libbar.so"  
    INTERFACE_INCLUDE_DIRECTORIES "contrib/libtorch/lib"
    )



target_link_libraries(newlang-unit-tests PRIVATE libffi.a ${CMAKE_DL_LIBS})
target_link_libraries(newlang-unit-tests PRIVATE 
    c10 
    torch 
    torch_cpu 
    crypto
    LLVM-13)
#target_link_libraries(newlang-unit-tests bar)


#link_directories(./lib)
#target_compile_options(newlang-unit-tests PRIVATE -Lcontrib/libtorch/lib)
#target_compile_options(newlang-unit-tests PRIVATE -Wl,-rpath,../contrib/libtorch/lib:contrib/libtorch/lib)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

# target_compile_options(newlang-unit-tests PRIVATE -g)

target_sources(newlang-unit-tests PRIVATE contrib/googletest/googletest/src/gtest_main.cc)
target_sources(newlang-unit-tests PRIVATE contrib/googletest/googletest/src/gtest-all.cc)
target_include_directories(newlang-unit-tests PUBLIC contrib/googletest/googletest)
target_include_directories(newlang-unit-tests PUBLIC contrib/googletest/googletest/include)

target_include_directories(newlang-unit-tests PUBLIC contrib/libtorch/include/torch/csrc/api/include)
target_include_directories(newlang-unit-tests PUBLIC contrib/Lyra/include)
target_include_directories(newlang-unit-tests PUBLIC contrib/libtorch/include)
target_include_directories(newlang-unit-tests PUBLIC contrib/tensorboard_logger/include)
target_include_directories(newlang-unit-tests PUBLIC /usr/lib/llvm-13/include)

target_include_directories(newlang-unit-tests PRIVATE .)
target_sources(newlang-unit-tests PRIVATE ${TEST_SRC})

if(NOT MYLIB_TESTING)
    message(STATUS "Тестирование проекта Mylib выключено")
elseif(IS_SUBPROJECT)
    message(STATUS "Mylib не тестируется в режиме подмодуля")
else()
    target_sources(newlang-unit-tests PRIVATE core/test/)
endif()



include(CTest)
enable_testing()



set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
