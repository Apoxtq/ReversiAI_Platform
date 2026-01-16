# System Tests CMake Configuration

# End-to-end system tests
add_executable(system_tests_e2e
    end_to_end_test.cpp
)

target_link_libraries(system_tests_e2e
    PRIVATE
        test_framework
        GTest::gtest_main
)

target_include_directories(system_tests_e2e
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework
)

# Stress tests
add_executable(system_tests_stress
    stress_test.cpp
)

target_link_libraries(system_tests_stress
    PRIVATE
        test_framework
        GTest::gtest_main
)

target_include_directories(system_tests_stress
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework
)

# Compatibility tests
add_executable(system_tests_compatibility
    compatibility_test.cpp
)

target_link_libraries(system_tests_compatibility
    PRIVATE
        test_framework
        GTest::gtest_main
)

target_include_directories(system_tests_compatibility
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework
)

# Register system tests
add_test(NAME SystemTests_E2E COMMAND system_tests_e2e)
add_test(NAME SystemTests_Stress COMMAND system_tests_stress)
add_test(NAME SystemTests_Compatibility COMMAND system_tests_compatibility)

# Set test properties (system tests can be time-consuming)
set_tests_properties(SystemTests_E2E PROPERTIES
    TIMEOUT 600   # 10 minutes
    LABELS "system;e2e"
)

set_tests_properties(SystemTests_Stress PROPERTIES
    TIMEOUT 1800  # 30 minutes
    LABELS "system;stress"
)

set_tests_properties(SystemTests_Compatibility PROPERTIES
    TIMEOUT 300   # 5 minutes
    LABELS "system;compatibility"
)
