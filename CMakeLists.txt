cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)
project(NewLang VERSION 0.1.0 LANGUAGES CXX)

get_directory_property(IS_SUBPROJECT PARENT_DIRECTORY)

# option(BUILD_TEST "Build test" OFF)
# if (BUILD_TEST)
#     add_executable(tensorboard_logger_test tests/test_tensorboard_logger.cc)
#     target_include_directories(tensorboard_logger_test
#     PRIVATE
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
#     )
#     target_link_libraries(tensorboard_logger_test tensorboard_logger)
# endif()

# cmake -DBUILD_TEST=O


option(MYLIB_TESTING "Включить модульное тестирование" ON)
option(MYLIB_COVERAGE "Включить измерение покрытия кода тестами" OFF)

add_compile_options(
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

# add_executable(GCC src/nlc.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


add_custom_command(
    OUTPUT
        # ${ATXT}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.yy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.yy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.yy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.yy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/location.hh
    COMMAND ./compile_syntax.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMMENT "Compile syntax from files parser.y and lexer.l"
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/compile_syntax.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.y
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.l
    VERBATIM)
 

file(GLOB src_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/builtin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.yy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/newlang.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/object.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.yy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/term.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/variable.cpp
    

    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/logger/logger.cpp

    # src/builtin.cpp
    # src/context.cpp
    # src/lexer.cpp
    # src/lexer.yy.cpp
    # src/newlang.cpp
    # src/object.cpp
    # src/parser.cpp
    # src/parser.yy.cpp
    # src/term.cpp
    # src/variable.cpp
    
    # contrib/logger/logger.cpp

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
    LLVM-13
    )

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})

add_executable(nlc ${src_SRC})
target_sources(nlc PRIVATE ${NLC_SRC})

add_executable(newlang-unit-tests ${src_SRC})
target_sources(newlang-unit-tests PRIVATE ${TEST_SRC})
# и RUNTIME_OUTPUT_NAME 



ADD_DEFINITIONS(-DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG)
ADD_DEFINITIONS(-DPDC_WIDE)
ADD_DEFINITIONS(-DDEBUG)

target_compile_options(newlang-unit-tests PRIVATE -DUNITTEST)


target_precompile_headers(newlang-unit-tests PRIVATE src/pch.h)
target_precompile_headers(nlc PRIVATE src/pch.h)
SET(PCH_DST src/pch.h)




if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

# if(WIN32)
#   target_sources(hello PRIVATE system_win.cxx)
# elseif(UNIX)
#   target_sources(hello PRIVATE system_posix.cxx)
# else()
#   target_sources(hello PRIVATE system_generic.cxx)
# endif()



target_sources(newlang-unit-tests PRIVATE contrib/googletest/googletest/src/gtest_main.cc)
target_sources(newlang-unit-tests PRIVATE contrib/googletest/googletest/src/gtest-all.cc)

target_include_directories(newlang-unit-tests PUBLIC contrib/libffi/win64/include)

target_include_directories(newlang-unit-tests PUBLIC contrib/googletest/googletest)
target_include_directories(newlang-unit-tests PUBLIC contrib/googletest/googletest/include)

include_directories(
    .
    contrib/Lyra/include
    contrib/libtorch/include/torch/csrc/api/include
    contrib/libtorch/include
    contrib/tensorboard_logger/include
    # /usr/lib/llvm-13/include
    )


if(NOT MYLIB_TESTING)
    message(STATUS "Тестирование проекта Mylib выключено")
elseif(IS_SUBPROJECT)
    message(STATUS "Mylib не тестируется в режиме подмодуля")
else()
    target_sources(newlang-unit-tests PRIVATE src/test/)
endif()

enable_testing()


# Для установки целей необходимо первым аргументом описанной функции передать ключевое слово TARGETS, за которым должен следовать список устанавливаемых целей, а затем ключевое слово DESTINATION с расположением каталога, в который установятся указанные цели. Данный пример демонстрирует типичную установку целей:
# # Установить цели "TimePrinter" и "DataScanner" в директорию "bin":
# install(TARGETS TimePrinter DataScanner DESTINATION bin)
# Процесс описания установки файлов аналогичен, за тем исключением, что вместо ключевого слова TARGETS следует указать FILES. Пример, демонстрирующий установку файлов:
# # Установить файлы "DataCache.txt" и "MessageLog.txt" в директорию "~/":
# install(FILES DataCache.txt MessageLog.txt DESTINATION ~/)
# Процесс описания установки папок аналогичен, за тем исключением, что вместо ключевого слова FILES следует указать DIRECTORY. Важно подметить, что при установке будет копироваться всё содержимое папки, а не только её название. Пример установки папок выглядит следующим образом:
# # Установить каталоги "MessageCollection" и "srcFiles" в директорию "~/":
# install(DIRECTORY MessageCollection srcFiles DESTINATION ~/)