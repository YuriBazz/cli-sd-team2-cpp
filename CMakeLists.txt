cmake_minimum_required(VERSION 3.16)
project(Bash VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Опция для сборки тестов
option(BUILD_TESTS "Build tests" ON)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# Bison parser
BISON_TARGET(parser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h
)

# Flex lexer
FLEX_TARGET(lexer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.cpp
)

add_flex_bison_dependency(lexer parser)

# Основная библиотека с командами
add_library(bash_lib STATIC
    src/ast.cpp
    src/environment.cpp
    src/commands.cpp
    src/executor.cpp
)

target_include_directories(bash_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(bash_lib PRIVATE
    -Wall -Wextra
    -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function
)

# Основной исполняемый файл
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${BISON_parser_OUTPUTS}
    ${FLEX_lexer_OUTPUTS}
)

target_link_libraries(${PROJECT_NAME} bash_lib)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra
    -Wno-sign-compare -Wno-unused-parameter -Wno-unused-function
)

# Тесты
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
