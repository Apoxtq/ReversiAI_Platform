# Regression Tests CMake Configuration

# Bug regression tests
add_executable(regression_tests_bugs
    bug_regression_test.cpp
)

target_link_libraries(regression_tests_bugs
    PRIVATE
        test_framework
        GTest::gtest_main
)

target_include_directories(regression_tests_bugs
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework
)

# Performance regression tests
add_executable(regression_tests_performance
    performance_regression_test.cpp
)

target_link_libraries(regression_tests_performance
    PRIVATE
        test_framework
        GTest::gtest_main
)

target_include_directories(regression_tests_performance
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework
)

# Register regression tests
add_test(NAME RegressionTests_Bugs COMMAND regression_tests_bugs)
add_test(NAME RegressionTests_Performance COMMAND regression_tests_performance)

# Set test properties
set_tests_properties(RegressionTests_Bugs PROPERTIES
    TIMEOUT 300
    LABELS "regression;bugs"
)

set_tests_properties(RegressionTests_Performance PROPERTIES
    TIMEOUT 600
    LABELS "regression;performance"
)
