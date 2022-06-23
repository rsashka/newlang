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

# add_executable(GCC core/nlc.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)


add_custom_command(
    OUTPUT
        # ${ATXT}
        ${CMAKE_CURRENT_SOURCE_DIR}/core/parser.yy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/core/parser.yy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/lexer.yy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/core/lexer.yy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/core/location.hh
    COMMAND ./compile_syntax.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/core
    COMMENT "Compile syntax from files parser.y and lexer.l"
    DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/core/compile_syntax.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/core/parser.y
        ${CMAKE_CURRENT_SOURCE_DIR}/core/lexer.l
    VERBATIM)
 

file(GLOB CORE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/core/builtin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/context.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/lexer.yy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/newlang.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/object.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/parser.yy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/term.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/variable.cpp
    

    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/logger/logger.cpp

    # core/builtin.cpp
    # core/context.cpp
    # core/lexer.cpp
    # core/lexer.yy.cpp
    # core/newlang.cpp
    # core/object.cpp
    # core/parser.cpp
    # core/parser.yy.cpp
    # core/term.cpp
    # core/variable.cpp
    
    # contrib/logger/logger.cpp

    #contrib/tensorboard_logger/src/crc.cc
    #contrib/tensorboard_logger/src/tensorboard_logger.cc
    )


file(GLOB TEST_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/core/test/*.cpp)
file(GLOB NLC_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/core/nlc.cpp)

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

add_executable(nlc ${CORE_SRC})
target_sources(nlc PRIVATE ${NLC_SRC})

add_executable(newlang-unit-tests ${CORE_SRC})
target_sources(newlang-unit-tests PRIVATE ${TEST_SRC})
# и RUNTIME_OUTPUT_NAME 



ADD_DEFINITIONS(-DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG)
ADD_DEFINITIONS(-DPDC_WIDE)
ADD_DEFINITIONS(-DDEBUG)

target_compile_options(newlang-unit-tests PRIVATE -DUNITTEST)


target_precompile_headers(newlang-unit-tests PRIVATE core/pch.h)
target_precompile_headers(nlc PRIVATE core/pch.h)
SET(PCH_DST core/pch.h)




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

target_include_directories(newlang-unit-tests PUBLIC contrib/googletest/googletest)
target_include_directories(newlang-unit-tests PUBLIC contrib/googletest/googletest/include)

include_directories(
    .
    contrib/Lyra/include
    contrib/libtorch/include/torch/csrc/api/include
    contrib/libtorch/include
    contrib/tensorboard_logger/include
    /usr/lib/llvm-13/include
    )


if(NOT MYLIB_TESTING)
    message(STATUS "Тестирование проекта Mylib выключено")
elseif(IS_SUBPROJECT)
    message(STATUS "Mylib не тестируется в режиме подмодуля")
else()
    target_sources(newlang-unit-tests PRIVATE core/test/)
endif()

enable_testing()


# Для установки целей необходимо первым аргументом описанной функции передать ключевое слово TARGETS, за которым должен следовать список устанавливаемых целей, а затем ключевое слово DESTINATION с расположением каталога, в который установятся указанные цели. Данный пример демонстрирует типичную установку целей:
# # Установить цели "TimePrinter" и "DataScanner" в директорию "bin":
# install(TARGETS TimePrinter DataScanner DESTINATION bin)
# Процесс описания установки файлов аналогичен, за тем исключением, что вместо ключевого слова TARGETS следует указать FILES. Пример, демонстрирующий установку файлов:
# # Установить файлы "DataCache.txt" и "MessageLog.txt" в директорию "~/":
# install(FILES DataCache.txt MessageLog.txt DESTINATION ~/)
# Процесс описания установки папок аналогичен, за тем исключением, что вместо ключевого слова FILES следует указать DIRECTORY. Важно подметить, что при установке будет копироваться всё содержимое папки, а не только её название. Пример установки папок выглядит следующим образом:
# # Установить каталоги "MessageCollection" и "CoreFiles" в директорию "~/":
# install(DIRECTORY MessageCollection CoreFiles DESTINATION ~/)