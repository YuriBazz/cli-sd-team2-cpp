add_executable(bash_tests
    test_example.cpp
#    test_module1.cpp
)

target_link_libraries(bash_tests bash_lib gtest_main)

target_include_directories(bash_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

add_test(NAME Module1Test COMMAND bash_tests --gtest_filter=Module1.*)
add_test(NAME Module2Test COMMAND bash_tests --gtest_filter=Module2.*)
add_test(NAME AllTests COMMAND bash_tests)
