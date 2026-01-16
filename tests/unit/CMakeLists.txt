# Unit Tests CMake Configuration

# Find Google Test
find_package(GTest REQUIRED)
if(NOT GTest_FOUND)
    message(FATAL_ERROR "Google Test not found. Please install Google Test.")
endif()

# Core unit tests
add_executable(unit_tests_core
    core/BitBoardTest.cpp
    # Add more core test files here as they are implemented
)

target_link_libraries(unit_tests_core
    PRIVATE
        test_framework
        GTest::gtest_main
)

target_include_directories(unit_tests_core
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework
)

# AI unit tests
add_executable(unit_tests_ai
    # ai/MinimaxTest.cpp  # To be implemented
    # ai/MCTSTest.cpp     # To be implemented
)

target_link_libraries(unit_tests_ai
    PRIVATE
        test_framework
        GTest::gtest_main
)

target_include_directories(unit_tests_ai
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework
)

# UI unit tests (only if Qt is available)
if(BUILD_QT_GUI)
    add_executable(unit_tests_ui
        # ui/MainWindowTest.cpp  # To be implemented
    )

    target_link_libraries(unit_tests_ui
        PRIVATE
            test_framework
            GTest::gtest_main
    )

    target_include_directories(unit_tests_ui
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/../framework
    )
endif()

# Network unit tests
add_executable(unit_tests_network
    # network/NetworkTest.cpp  # To be implemented
)

target_link_libraries(unit_tests_network
    PRIVATE
        test_framework
        GTest::gtest_main
)

target_include_directories(unit_tests_network
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework
)

# Register tests
add_test(NAME UnitTests_Core COMMAND unit_tests_core)
add_test(NAME UnitTests_AI COMMAND unit_tests_ai)
if(BUILD_QT_GUI)
    add_test(NAME UnitTests_UI COMMAND unit_tests_ui)
endif()
add_test(NAME UnitTests_Network COMMAND unit_tests_network)

# Set test properties
set_tests_properties(UnitTests_Core PROPERTIES
    TIMEOUT 60
    LABELS "unit;core"
)

set_tests_properties(UnitTests_AI PROPERTIES
    TIMEOUT 120
    LABELS "unit;ai"
)

if(BUILD_QT_GUI)
    set_tests_properties(UnitTests_UI PROPERTIES
        TIMEOUT 60
        LABELS "unit;ui"
    )
endif()

set_tests_properties(UnitTests_Network PROPERTIES
    TIMEOUT 60
    LABELS "unit;network"
)
