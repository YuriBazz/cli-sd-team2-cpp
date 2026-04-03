cmake_minimum_required(VERSION 3.16)
project(Bash VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

BISON_TARGET(parser src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cpp
             DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h)
FLEX_TARGET(lexer src/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cpp)

add_flex_bison_dependency(lexer parser)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/ast.cpp
    src/environment.cpp
    src/commands.cpp
    src/executor.cpp
    ${BISON_parser_OUTPUTS}
    ${FLEX_lexer_OUTPUTS}
)

set(LIB_SOURCES
    src/ast.cpp
    src/environment.cpp
    src/commands.cpp
    src/executor.cpp
    ${BISON_parser_OUTPUTS}
    ${FLEX_lexer_OUTPUTS}
)

add_library(bash_lib STATIC ${LIB_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} ${FLEX_LIBRARIES})

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic -Werror
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function
)

enable_testing()
add_subdirectory(tests)