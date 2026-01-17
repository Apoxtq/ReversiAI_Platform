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

# Set Qt installation path (modify this to your Qt installation directory)
# Example: set(QT_DIR "C:/Qt/6.5.0/msvc2019_64")
# set(QT_DIR "")

# Testing options
option(BUILD_TESTS "Build test executables" ON)
option(COVERAGE "Enable code coverage analysis" OFF)
option(PERFORMANCE_TESTING "Enable performance testing features" OFF)

# Console application (no Qt dependency)
add_executable(ReversiAI_Platform_Console
    main_console.cpp
    ai/MCTS.cpp
)

# Include directories
target_include_directories(ReversiAI_Platform_Console PRIVATE include ai)

# Core library: compile central sources once and link where needed
add_library(core_lib STATIC
    src/core/BitBoard.cpp
    src/Board.cpp
)
target_include_directories(core_lib PUBLIC include include/core)
target_compile_features(core_lib PUBLIC cxx_std_17)

# Link core_lib into console executable
target_link_libraries(ReversiAI_Platform_Console PRIVATE core_lib)

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
    # Enable CTest
    enable_testing()

    # Add test subdirectories only if they actually exist (explicit checks).
    if(EXISTS "${CMAKE_SOURCE_DIR}/Tests/framework")
        add_subdirectory(Tests/framework)
        if(EXISTS "${CMAKE_SOURCE_DIR}/Tests/unit")
            add_subdirectory(Tests/unit)
        endif()
        if(EXISTS "${CMAKE_SOURCE_DIR}/Tests/integration")
            add_subdirectory(Tests/integration)
        endif()
        if(EXISTS "${CMAKE_SOURCE_DIR}/Tests/performance")
            add_subdirectory(Tests/performance)
        endif()
        if(EXISTS "${CMAKE_SOURCE_DIR}/Tests/system")
            add_subdirectory(Tests/system)
        endif()
        if(EXISTS "${CMAKE_SOURCE_DIR}/Tests/regression")
            add_subdirectory(Tests/regression)
        endif()
    elseif(EXISTS "${CMAKE_SOURCE_DIR}/tests/framework")
        add_subdirectory(tests/framework)
        if(EXISTS "${CMAKE_SOURCE_DIR}/tests/unit")
            add_subdirectory(tests/unit)
        endif()
        if(EXISTS "${CMAKE_SOURCE_DIR}/tests/integration")
            add_subdirectory(tests/integration)
        endif()
        if(EXISTS "${CMAKE_SOURCE_DIR}/tests/performance")
            add_subdirectory(tests/performance)
        endif()
        if(EXISTS "${CMAKE_SOURCE_DIR}/tests/system")
            add_subdirectory(tests/system)
        endif()
        if(EXISTS "${CMAKE_SOURCE_DIR}/tests/regression")
            add_subdirectory(tests/regression)
        endif()
    else()
        message(WARNING "No tests/framework directory found (checked 'Tests/framework' and 'tests/framework'). Skipping test configuration.")
    endif()

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
