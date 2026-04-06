include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

FetchContent_MakeAvailable(googletest)

# Создаем исполняемый файл для тестов
add_executable(bash_tests
    test_cat.cpp
    test_echo.cpp
    test_wc.cpp
    test_pwd.cpp
    test_grep.cpp
    test_assignment.cpp
    test_pipeline.cpp
    test_external.cpp
    test_environment.cpp
    test_exit.cpp
)

# Линкуем библиотеки
target_link_libraries(bash_tests
    bash_lib
    gtest_main
    gtest
    pthread
)

# Добавляем директории для include
target_include_directories(bash_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}
)

# Компиляторные флаги для тестов
target_compile_options(bash_tests PRIVATE
    -Wall -Wextra
    -Wno-sign-compare -Wno-unused-parameter
)

# Регистрируем тесты в CTest
include(GoogleTest)

# Тесты для каждой команды
gtest_discover_tests(bash_tests
    TEST_PREFIX "bash."
    DISCOVERY_MODE PRE_TEST
)

# Альтернативный способ - ручное добавление тестов
add_test(NAME bash.CatCommand COMMAND bash_tests --gtest_filter=CatCommandTest.*)
add_test(NAME bash.EchoCommand COMMAND bash_tests --gtest_filter=EchoCommandTest.*)
add_test(NAME bash.WcCommand COMMAND bash_tests --gtest_filter=WcCommandTest.*)
add_test(NAME bash.PwdCommand COMMAND bash_tests --gtest_filter=PwdCommandTest.*)
add_test(NAME bash.GrepCommand COMMAND bash_tests --gtest_filter=GrepCommandTest.*)
add_test(NAME bash.Assignment COMMAND bash_tests --gtest_filter=AssignmentTest.*)
add_test(NAME bash.Pipeline COMMAND bash_tests --gtest_filter=PipelineTest.*)
add_test(NAME bash.ExternalCommand COMMAND bash_tests --gtest_filter=ExternalCommandTest.*)
add_test(NAME bash.Environment COMMAND bash_tests --gtest_filter=EnvironmentTest.*)
add_test(NAME bash.ExitCommand COMMAND bash_tests --gtest_filter=ExitCommandTest.*)
