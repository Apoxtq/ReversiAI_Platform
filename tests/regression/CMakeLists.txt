# Regression Tests CMake Configuration

# Bug regression tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/bug_regression_test.cpp")
    add_executable(regression_tests_bugs
        bug_regression_test.cpp
    )
target_link_libraries(regression_tests_bugs
    PRIVATE
        test_framework
)
    target_include_directories(regression_tests_bugs
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/../framework
    )
endif()

# Performance regression tests
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/performance_regression_test.cpp")
    add_executable(regression_tests_performance
        performance_regression_test.cpp
    )
target_link_libraries(regression_tests_performance
    PRIVATE
        test_framework
)
    target_include_directories(regression_tests_performance
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/../framework
    )
endif()

# Register regression tests
if(TARGET regression_tests_bugs)
    add_test(NAME RegressionTests_Bugs COMMAND regression_tests_bugs)
endif()
if(TARGET regression_tests_performance)
    add_test(NAME RegressionTests_Performance COMMAND regression_tests_performance)
endif()

# Set test properties
if(TARGET regression_tests_bugs)
    set_tests_properties(RegressionTests_Bugs PROPERTIES
        TIMEOUT 300
        LABELS "regression;bugs"
    )
endif()

if(TARGET regression_tests_performance)
    set_tests_properties(RegressionTests_Performance PROPERTIES
        TIMEOUT 600
        LABELS "regression;performance"
    )
endif()
