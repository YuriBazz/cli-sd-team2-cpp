include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

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
    test_ls.cpp
    test_cd.cpp
)

target_link_libraries(bash_tests
    bash_lib
    gtest_main
    gmock
    pthread
)

target_include_directories(bash_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}
)

target_compile_options(bash_tests PRIVATE
    -Wall -Wextra
    -Wno-sign-compare -Wno-unused-parameter
)

include(GoogleTest)
gtest_discover_tests(bash_tests)
