cmake_minimum_required(VERSION 3.16)
project(ReversiAI_Platform VERSION 1.0.0 LANGUAGES CXX)

# Set MinGW as default generator on Windows for better compatibility
if(WIN32 AND NOT CMAKE_GENERATOR)
    set(CMAKE_GENERATOR "MinGW Makefiles" CACHE INTERNAL "" FORCE)
endif()

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to build with Qt GUI (default OFF until Qt is configured)
option(BUILD_QT_GUI "Build Qt GUI application" OFF)

# Testing options
option(BUILD_TESTS "Build test executables" ON)
option(COVERAGE "Enable code coverage analysis" OFF)
option(PERFORMANCE_TESTING "Enable performance testing features" OFF)

# Console application (no Qt dependency)
add_executable(ReversiAI_Platform_Console
    main_console.cpp
    ai/MCTS.cpp
    src/core/BitBoard.cpp
)

# Include directories
target_include_directories(ReversiAI_Platform_Console PRIVATE include ai)

# Console app for testing core functionality
if(BUILD_QT_GUI)
    # Qt6 support with fallback to Qt5
    find_package(Qt6 QUIET COMPONENTS Core Widgets)
    if(NOT Qt6_FOUND)
        find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
    endif()

    # Add Qt executable
    if(Qt6_FOUND)
        qt6_wrap_cpp(MOC_SOURCES src/mainwindow.h)
        qt6_add_resources(QRC_SOURCES src/rsc.qrc)
    else()
        qt5_wrap_cpp(MOC_SOURCES src/mainwindow.h)
        qt5_add_resources(QRC_SOURCES src/rsc.qrc)
    endif()

    add_executable(ReversiAI_Platform
        main.cpp
        ui/mainwindow.cpp
        ai/MCTS.cpp
        ${MOC_SOURCES}
        ${QRC_SOURCES}
    )

    # Link Qt libraries
    if(Qt6_FOUND)
        target_link_libraries(ReversiAI_Platform Qt6::Core Qt6::Widgets)
    else()
        target_link_libraries(ReversiAI_Platform Qt5::Core Qt5::Widgets)
    endif()

    # Include directories for Qt app
    target_include_directories(ReversiAI_Platform PRIVATE include ui ai)
endif()

# Testing configuration
if(BUILD_TESTS)
    # Enable testing
    enable_testing()

    # Add test framework
    add_subdirectory(Tests/framework)

    # Add unit tests
    add_subdirectory(Tests/unit)

    # Add integration tests
    add_subdirectory(Tests/integration)

    # Add performance tests
    add_subdirectory(Tests/performance)

    # Add system tests
    add_subdirectory(Tests/system)

    # Add regression tests
    add_subdirectory(Tests/regression)

    # Code coverage (GCC/Clang)
    if(COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(ReversiAI_Platform_Console PRIVATE --coverage)
        target_link_options(ReversiAI_Platform_Console PRIVATE --coverage)

        # Add coverage target
        add_custom_target(coverage
            COMMAND lcov --capture --directory . --output-file coverage.info
            COMMAND lcov --remove coverage.info '/usr/*' '*/tests/*' '*/build/*' --output-file coverage.info
            COMMAND genhtml coverage.info --output-directory coverage_report
            COMMENT "Generating code coverage report"
        )
    endif()
endif()
