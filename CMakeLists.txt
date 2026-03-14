cmake_minimum_required(VERSION 3.16)
project(ReversiAI_Platform VERSION 1.0.2 LANGUAGES CXX)

# Detect compiler
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(COMPILER_TYPE "MSVC")
    message(STATUS "Detected compiler: MSVC")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMPILER_TYPE "MinGW")
    message(STATUS "Detected compiler: MinGW (GCC)")
else()
    set(COMPILER_TYPE "Unknown")
    message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Use default generator on Windows

# Performance optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        # MSVC Release optimizations
        add_compile_options(/O2 /Oi /Ot /GL /fp:fast)
        add_link_options(/LTCG)
        message(STATUS "Using MSVC Release optimizations: /O2 /Oi /Ot /GL /fp:fast /LTCG")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # GCC/MinGW Release optimizations
        add_compile_options(-O3 -march=native -funroll-loops -ffast-math)
        message(STATUS "Using GCC Release optimizations: -O3 -march=native -funroll-loops -ffast-math")
    endif()
endif()

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC specific: Disable warnings about non-UTF8 source files and enable proper C++ standard
if(MSVC)
    add_compile_options(/wd4819)
    # Qt requires /Zc:__cplusplus to properly detect C++ standard
    add_compile_options(/Zc:__cplusplus)
    # Qt6.10+ requires /permissive- for MOC compatibility (Qt official workaround)
    add_compile_options(/permissive-)
endif()

# Qt settings - AUTOMOC handles Qt's MOC automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Option to build with Qt GUI (default OFF until Qt is configured)
option(BUILD_QT_GUI "Build Qt GUI application" OFF)

# Set Qt installation path based on compiler
# MSVC: Try system Qt6 first (user installed), then vcpkg
# MinGW: Use system installed Qt6
if(COMPILER_TYPE STREQUAL "MSVC")
    # Check for system MSVC Qt6 (user installed) - highest priority
    if(EXISTS "D:/Dev/SDKs/Qt/6.10.2/msvc2022_64")
        set(DEFAULT_QT_PATH "D:/Dev/SDKs/Qt/6.10.2/msvc2022_64")
        message(STATUS "MSVC detected, using system Qt6 at ${DEFAULT_QT_PATH}")
    elseif(EXISTS "D:/Dev/SDKs/Qt/6.10.1/msvc2022_64")
        set(DEFAULT_QT_PATH "D:/Dev/SDKs/Qt/6.10.1/msvc2022_64")
        message(STATUS "MSVC detected, using system Qt6 at ${DEFAULT_QT_PATH}")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/installed/x64-windows/lib/cmake/Qt6")
        set(DEFAULT_QT_PATH "C:/vcpkg/installed/x64-windows")
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake")
        message(STATUS "MSVC detected, using vcpkg Qt6")
    else()
        message(WARNING "MSVC Qt6 not found. Please install Qt MSVC version via Qt MaintenanceTool")
        set(DEFAULT_QT_PATH "")
    endif()
else()
    # MinGW uses system Qt
    set(DEFAULT_QT_PATH "D:/Dev/SDKs/Qt/6.10.1/mingw_64")
    if(NOT EXISTS "${DEFAULT_QT_PATH}")
        set(DEFAULT_QT_PATH "C:/Qt/6.10.1/mingw_64")
    endif()
    if(EXISTS "${DEFAULT_QT_PATH}")
        message(STATUS "MinGW detected, using system Qt6 at ${DEFAULT_QT_PATH}")
    else()
        message(WARNING "MinGW Qt6 not found at ${DEFAULT_QT_PATH}")
        set(DEFAULT_QT_PATH "")
    endif()
endif()

# Allow user to override Qt path
set(QT_PATH "" CACHE PATH "Path to Qt6 installation")
if(QT_PATH)
    set(DEFAULT_QT_PATH ${QT_PATH})
endif()

# Set Qt6_ROOT for CMake
if(EXISTS "${DEFAULT_QT_PATH}")
    set(Qt6_ROOT "${DEFAULT_QT_PATH}" CACHE PATH "Qt6 Root Directory")
endif()

# Testing options
option(BUILD_TESTS "Build test executables" ON)
option(COVERAGE "Enable code coverage analysis" OFF)
option(PERFORMANCE_TESTING "Enable performance testing features" OFF)

# Console application (no Qt dependency)
add_executable(ReversiAI_Platform_Console
    main_console.cpp
    # ai/MCTS.cpp  # 旧的MCTS实现，暂时移除
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

# AI library: compile AI algorithms
add_library(ai_lib STATIC
    src/ai/Evaluator.cpp
    src/ai/AIStrategy.cpp
    src/ai/MinimaxAI.cpp
    src/ai/MCTSAI.cpp
    src/ai/RandomAI.cpp
    src/ai/AIBattle.cpp
    src/ai/ZobristHash.cpp              # v0.6.0: Zobrist哈希
    src/ai/TranspositionTable.cpp       # v0.6.0: 转置表
    # v0.7.0: Killer/History优化
    src/ai/KillerTable.cpp
    src/ai/HistoryTable.cpp
    src/ai/MoveOrderer.cpp
)
target_include_directories(ai_lib PUBLIC include include/ai)
target_compile_features(ai_lib PUBLIC cxx_std_17)
target_link_libraries(ai_lib PUBLIC core_lib)

# Research library: benchmark and statistics tools (v0.6.0)
# v0.9.0: GameRecord uses Qt, so we need to find Qt first
find_package(Qt6 QUIET COMPONENTS Core)
if(Qt6_FOUND)
    add_library(research_lib STATIC
        src/research/PositionSuite.cpp
        src/research/BattleEngine.cpp
        src/research/Statistics.cpp
        src/research/GameRecord.cpp         # v0.9.0: 对局记录系统
        src/research/BitboardBenchmark.cpp  # v0.8.0: Bitboard性能测试
        src/research/AIBenchmark.cpp       # v0.8.0: AI性能测试
        src/research/DataExporter.cpp      # v0.8.0: 数据导出
    )
    target_include_directories(research_lib PUBLIC include include/research)
    target_compile_features(research_lib PUBLIC cxx_std_17)
    target_link_libraries(research_lib PUBLIC ai_lib Qt6::Core)
else()
    add_library(research_lib STATIC
        src/research/PositionSuite.cpp
        src/research/BattleEngine.cpp
        src/research/Statistics.cpp
        src/research/BitboardBenchmark.cpp
        src/research/AIBenchmark.cpp
        src/research/DataExporter.cpp
    )
    target_include_directories(research_lib PUBLIC include include/research)
    target_compile_features(research_lib PUBLIC cxx_std_17)
    target_link_libraries(research_lib PUBLIC ai_lib)
endif()

# Network library: compile network components for LAN multiplayer (v0.5.0)
# Reference: Egaroucid ggs.hpp protocol implementation
# 检查 Qt6 Network 模块是否存在
find_package(Qt6 QUIET COMPONENTS Core Network)
if(Qt6_FOUND AND Qt6Network_FOUND)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/network/CMakeLists.txt)
        add_subdirectory(network)
        target_link_libraries(ai_lib PRIVATE network_lib)
        target_link_libraries(ReversiAI_Platform_Console PRIVATE network_lib)
    endif()
else()
    message(STATUS "Qt6 Network not found, using stub network library")
    # Create stub network_lib for linking
    file(WRITE "${CMAKE_BINARY_DIR}/network_stub.cpp" "namespace Reversi { namespace Network { } } // Empty stub\n")
    add_library(network_lib STATIC "${CMAKE_BINARY_DIR}/network_stub.cpp")
    target_include_directories(network_lib PUBLIC include)
endif()

# UI library: compile UI components (GameController, StatisticsManager)
# Requires Qt for signals/slots and QObject base class

# Print Qt6 search paths for debugging
message(STATUS "Looking for Qt6...")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

# Set Qt6 path explicitly from environment or default
if(NOT Qt6_ROOT)
    set(Qt6_ROOT "$ENV{Qt6_ROOT}")
endif()
if(NOT Qt6_ROOT)
    set(Qt6_ROOT "D:/Dev/SDKs/Qt/6.10.1/mingw_64")
endif()
message(STATUS "Using Qt6_ROOT: ${Qt6_ROOT}")

# Try to find Qt6 with explicit path
find_package(Qt6 6.10.1 REQUIRED COMPONENTS Core Widgets Network Concurrent PATHS "${Qt6_ROOT}")

message(STATUS "Qt6_FOUND: ${Qt6_FOUND}")
if(Qt6_FOUND)
    add_library(ui_lib STATIC
        src/ui/GameController.cpp
        src/ui/StatisticsManager.cpp
        src/ui/NetworkLobbyWindow.cpp
        src/ui/NetworkGameWindow.cpp
        src/ui/AIvsAIWindow.cpp            # v0.6.0: AI对战窗口
        # v0.9.0: 可视化增强
        src/ui/SearchTreeWidget.cpp
        src/ui/SearchStatisticsPanel.cpp
        src/ui/BoardHeatmap.cpp
        src/ui/ReplayAnalysisWindow.cpp
    )
    target_include_directories(ui_lib PUBLIC include include/ui network/include)
    target_compile_features(ui_lib PUBLIC cxx_std_17)
    target_link_libraries(ui_lib PRIVATE Qt6::Core Qt6::Widgets Qt6::Network Qt6::Concurrent network_lib)
else()
    message(STATUS "Qt6 not found, UI library will be disabled")
    # Create empty stub library with empty source files
    file(WRITE "${CMAKE_BINARY_DIR}/ui_stub.cpp" "namespace Reversi { namespace UI { } } // Empty stub\n")
    add_library(ui_lib STATIC "${CMAKE_BINARY_DIR}/ui_stub.cpp")
    target_include_directories(ui_lib PUBLIC include)
    target_compile_features(ui_lib PUBLIC cxx_std_17)
endif()

# Link libraries into console executable
target_link_libraries(ReversiAI_Platform_Console PRIVATE core_lib ai_lib research_lib)

# Console app for testing core functionality
if(BUILD_QT_GUI)
    # Qt6 support with fallback to Qt5
    find_package(Qt6 QUIET COMPONENTS Core Widgets Network Concurrent)
    if(NOT Qt6_FOUND)
        find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network Concurrent)
    endif()

    # Add Qt executable
    if(Qt6_FOUND)
        qt6_wrap_cpp(MOC_SOURCES
            include/mainwindow.h
            include/ui/GameController.h
            include/ui/MenuWindow.h
            include/ui/PvEWindow.h
            include/ui/PvPWindow.h
            include/ui/NetworkLobbyWindow.h
            include/ui/NetworkGameWindow.h
            include/ui/StatisticsManager.h
            include/ui/AIvsAIWindow.h         # v0.6.0: AI对战窗口
            # v0.9.0: 可视化增强
            include/ui/SearchTreeWidget.h
            include/ui/SearchStatisticsPanel.h
            include/ui/BoardHeatmap.h
            include/ui/ReplayAnalysisWindow.h
        )
        qt6_add_resources(QRC_SOURCES src/rsc.qrc)
    else()
        qt5_wrap_cpp(MOC_SOURCES
            include/mainwindow.h
            include/ui/GameController.h
            include/ui/MenuWindow.h
            include/ui/PvEWindow.h
            include/ui/PvPWindow.h
            include/ui/NetworkLobbyWindow.h
            include/ui/NetworkGameWindow.h
            include/ui/StatisticsManager.h
        )
        qt5_add_resources(QRC_SOURCES src/rsc.qrc)
    endif()

    add_executable(ReversiAI_Platform
        main.cpp
        ui/mainwindow.cpp
        ui/menuwindow.cpp
        ui/MenuWindowAIResearch.cpp
        ui/MenuWindowReplayAnalysis.cpp   # v0.9.0: 复盘分析按钮
        ui/pvewindow.cpp
        ui/pvpwindow.cpp
        src/ui/networklobbywindow.cpp
        src/ui/networkgamewindow.cpp
        src/ui/AIvsAIWindow.cpp          # v0.6.0: AI对战窗口
        # v0.9.0: 可视化增强
        src/ui/SearchTreeWidget.cpp
        src/ui/SearchStatisticsPanel.cpp
        src/ui/BoardHeatmap.cpp
        src/ui/ReplayAnalysisWindow.cpp
        ${MOC_SOURCES}
        ${QRC_SOURCES}
    )

    # Link Qt libraries
    if(Qt6_FOUND)
        target_link_libraries(ReversiAI_Platform PRIVATE Qt6::Core Qt6::Widgets Qt6::Network)
    else()
        target_link_libraries(ReversiAI_Platform PRIVATE Qt5::Core Qt5::Widgets Qt5::Network)
    endif()
    
    # Link internal libraries
    target_link_libraries(ReversiAI_Platform PRIVATE network_lib ui_lib ai_lib research_lib)

    # Include directories for Qt app
    target_include_directories(ReversiAI_Platform PRIVATE include ui ai src network/include)

    # Link all required libraries
    target_link_libraries(ReversiAI_Platform PRIVATE ui_lib core_lib ai_lib network_lib)
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
