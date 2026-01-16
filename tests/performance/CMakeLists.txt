# Performance Tests CMake Configuration

# Find Google Benchmark
find_package(benchmark QUIET)
if(NOT benchmark_FOUND)
    message(STATUS "Google Benchmark not found. Performance tests will use basic timing.")
endif()

# Core performance benchmarks
add_executable(performance_tests_core
    benchmark_core.cpp
)

target_link_libraries(performance_tests_core
    PRIVATE
        test_framework
)

if(benchmark_FOUND)
    target_link_libraries(performance_tests_core PRIVATE benchmark::benchmark)
endif()

target_include_directories(performance_tests_core
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework
)

# AI performance benchmarks
add_executable(performance_tests_ai
    # benchmark_ai.cpp  # To be implemented
)

target_link_libraries(performance_tests_ai
    PRIVATE
        test_framework
)

if(benchmark_FOUND)
    target_link_libraries(performance_tests_ai PRIVATE benchmark::benchmark)
endif()

target_include_directories(performance_tests_ai
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework
)

# UI performance benchmarks (only if Qt is available)
if(BUILD_QT_GUI)
    add_executable(performance_tests_ui
        # benchmark_ui.cpp  # To be implemented
    )

    target_link_libraries(performance_tests_ui
        PRIVATE
            test_framework
    )

    if(benchmark_FOUND)
        target_link_libraries(performance_tests_ui PRIVATE benchmark::benchmark)
    endif()

    target_include_directories(performance_tests_ui
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/../framework
    )
endif()

# Register performance tests
add_test(NAME PerformanceTests_Core COMMAND performance_tests_core)
add_test(NAME PerformanceTests_AI COMMAND performance_tests_ai)

if(BUILD_QT_GUI)
    add_test(NAME PerformanceTests_UI COMMAND performance_tests_ui)
endif()

# Set test properties (performance tests can take longer)
set_tests_properties(PerformanceTests_Core PROPERTIES
    TIMEOUT 1800  # 30 minutes
    LABELS "performance;core"
)

set_tests_properties(PerformanceTests_AI PROPERTIES
    TIMEOUT 3600  # 1 hour
    LABELS "performance;ai"
)

if(BUILD_QT_GUI)
    set_tests_properties(PerformanceTests_UI PROPERTIES
        TIMEOUT 1800
        LABELS "performance;ui"
    )
endif()

# Add performance test target
add_custom_target(run_performance_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -L performance --output-on-failure
    COMMENT "Running all performance tests"
)
