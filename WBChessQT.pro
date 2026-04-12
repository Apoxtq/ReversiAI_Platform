#-------------------------------------------------
#
# ReversiAI_Platform - Qt GUI Application
# Auto-generated based on CMakeLists.txt structure
#
# Qt installation path (adjust if your Qt installation is different)
QT_DIR = D:/Dev/SDKs/Qt/6.10.1/mingw_64
#
#-------------------------------------------------

QT       += core gui widgets network concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ReversiAI_Platform
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

# Version
VERSION = 1.0.7
DEFINES += APP_VERSION=\"\\\"$$VERSION\\\"\"

# C++ Standard
CONFIG += c++17

# Compiler flags
QMAKE_CXXFLAGS += -Wall -Wextra
QMAKE_CXXFLAGS_RELEASE += -O3 -march=native -funroll-loops -ffast-math

# Build mode
CONFIG(debug, debug|release) {
    DEFINES += DEBUG_MODE
} else {
    DEFINES += RELEASE_MODE
}

# ========================
# INCLUDE PATHS
# ========================
INCLUDEPATH += $$PWD/include \
               $$PWD/include/ai \
               $$PWD/include/core \
               $$PWD/include/research \
               $$PWD/include/ui \
               $$PWD/src \
               $$PWD/src/ai \
               $$PWD/src/core \
               $$PWD/src/research \
               $$PWD/src/ui \
               $$PWD/ui \
               $$PWD/ai \
               $$PWD/network/include

# ========================
# HEADERS  (all .h files)
# ========================
HEADERS += \
    # Root-level headers
    include/mainwindow.h \
    include/Board.h \
    # AI
    include/ai/AIBattle.h \
    include/ai/AIStrategy.h \
    include/ai/Evaluator.h \
    include/ai/HistoryTable.h \
    include/ai/KillerTable.h \
    include/ai/MCTSAI.h \
    include/ai/MinimaxAI.h \
    include/ai/MoveOrderer.h \
    include/ai/RandomAI.h \
    include/ai/TranspositionTable.h \
    include/ai/ZobristHash.h \
    # Core
    include/core/BitBoard.h \
    include/core/GamePhase.h \
    include/core/PlatformUtils.h \
    # Research
    include/research/AIBenchmark.h \
    include/research/BattleEngine.h \
    include/research/BenchmarkTargets.h \
    include/research/BitboardBenchmark.h \
    include/research/DataExporter.h \
    include/research/GameRecord.h \
    include/research/PositionSuite.h \
    include/research/RuntimeEstimator.h \
    include/research/SearchStats.h \
    include/research/Statistics.h \
    include/research/ValidationSuite.h \
    # UI
    include/ui/AIvsAISelectWindow.h \
    include/ui/AIvsAIWindow.h \
    include/ui/AIWatchWindow.h \
    include/ui/BoardHeatmap.h \
    include/ui/GameController.h \
    include/ui/MenuWindow.h \
    include/ui/NetworkGameWindow.h \
    include/ui/NetworkLobbyWindow.h \
    include/ui/PvEWindow.h \
    include/ui/PvPWindow.h \
    include/ui/ReplayAnalysisWindow.h \
    include/ui/SearchStatisticsPanel.h \
    include/ui/SearchTreeWidget.h \
    include/ui/StatisticsManager.h

# ========================
# SOURCES  (all .cpp files)
# ========================
SOURCES += \
    # Root-level main entry
    main.cpp \
    # UI layer (Qt Designer forms + logic)
    ui/mainwindow.cpp \
    ui/menuwindow.cpp \
    ui/MenuWindowReplayAnalysis.cpp \
    ui/pvewindow.cpp \
    ui/pvpwindow.cpp \
    # UI layer (pure Qt/C++ logic)
    src/ui/AIvsAISelectWindow.cpp \
    src/ui/AIvsAIWindow.cpp \
    src/ui/AIWatchWindow.cpp \
    src/ui/BoardHeatmap.cpp \
    src/ui/GameController.cpp \
    src/ui/networkgamewindow.cpp \
    src/ui/networklobbywindow.cpp \
    src/ui/ReplayAnalysisWindow.cpp \
    src/ui/SearchStatisticsPanel.cpp \
    src/ui/SearchTreeWidget.cpp \
    src/ui/StatisticsManager.cpp \
    # AI algorithms
    src/ai/AIBattle.cpp \
    src/ai/AIStrategy.cpp \
    src/ai/Evaluator.cpp \
    src/ai/HistoryTable.cpp \
    src/ai/KillerTable.cpp \
    src/ai/MCTSAI.cpp \
    src/ai/MinimaxAI.cpp \
    src/ai/MoveOrderer.cpp \
    src/ai/RandomAI.cpp \
    src/ai/TranspositionTable.cpp \
    src/ai/ZobristHash.cpp \
    # Core
    src/Board.cpp \
    src/core/BitBoard.cpp \
    src/core/Board_clean.cpp \
    # Research
    src/research/AIBenchmark.cpp \
    src/research/BattleEngine.cpp \
    src/research/BitboardBenchmark.cpp \
    src/research/DataExporter.cpp \
    src/research/GameRecord.cpp \
    src/research/PositionSuite.cpp \
    src/research/RuntimeEstimator.cpp \
    src/research/Statistics.cpp \
    src/research/ValidationSuite.cpp \
    # Resource
    src/resource.cpp

# ========================
# UI FORMS  (.ui files)
# ========================
FORMS += \
    ui/mainwindow.ui \
    ui/menuwindow.ui \
    ui/pvewindow.ui \
    ui/pvpwindow.ui

# ========================
# RESOURCES  (.qrc files)
# ========================
RESOURCES += \
    src/rsc.qrc

# ========================
# NETWORK  (conditional)
# ========================
exists($$PWD/network/src/networkcore.cpp) {
    INCLUDEPATH += $$PWD/network/src
    SOURCES += \
        network/src/networkcore.cpp \
        network/src/protocol.cpp
    HEADERS += \
        network/include/networkcore.h \
        network/include/protocol.h \
        network/include/message.h
}

# ========================
# TESTS  (optional, not included in GUI build)
# ========================
# Tests are built via CMake (see CMakeLists.txt)
# Manual Qt build of ValidationTest:
#   qmake WBChessQT.pro "DEFINES+=BUILD_VALIDATION_TEST"
# Uncomment the following to include ValidationTest as a separate target:
# SOURCES += src/ValidationTest.cpp
