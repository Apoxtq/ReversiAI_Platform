#-------------------------------------------------
#
# ReversiAI_Platform Qt GUI Application
# Created for Qt Creator (D:\Dev\SDKs\Qt\Tools\QtCreator\bin\qtcreator.exe)
#
#-------------------------------------------------

# Qt installation path (adjust if your Qt installation is different)
QT_DIR = D:/Dev/SDKs/Qt/6.10.1/mingw_64
# Uncomment and modify the line below if using a different Qt version
# QT_DIR = D:/Dev/SDKs/Qt/5.15.x/mingw_64

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ReversiAI_Platform_GUI
TEMPLATE = app

# Define for deprecated Qt features
DEFINES += QT_DEPRECATED_WARNINGS

# Include directories
INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/ai

# Source files
SOURCES += \
    main.cpp \
    ui/mainwindow.cpp \
    ai/MCTS.cpp \
    src/core/BitBoard.cpp \
    src/Board.cpp \
    src/ai/Evaluator.cpp \
    src/ai/AIStrategy.cpp \
    src/ai/MinimaxAI.cpp \
    src/ai/RandomAI.cpp \
    src/ai/AIBattle.cpp \
    src/ai/ZobristHash.cpp \
    src/ai/TranspositionTable.cpp \
    src/ai/KillerTable.cpp \
    src/ai/HistoryTable.cpp \
    src/ai/MoveOrderer.cpp \
    src/research/PositionSuite.cpp \
    src/research/BattleEngine.cpp \
    src/research/Statistics.cpp

# Header files
HEADERS += \
    include/mainwindow.h \
    include/MCTS.h \
    include/core/BitBoard.h \
    include/Board.h \
    include/ai/AIStrategy.h \
    include/ai/Evaluator.h \
    include/ai/MinimaxAI.h \
    include/ai/RandomAI.h \
    include/ai/AIBattle.h \
    include/ai/ZobristHash.h \
    include/ai/TranspositionTable.h \
    include/ai/KillerTable.h \
    include/ai/HistoryTable.h \
    include/ai/MoveOrderer.h \
    include/research/PositionSuite.h \
    include/research/BattleEngine.h \
    include/research/Statistics.h

# UI forms
FORMS += \
    src/mainwindow.ui

# Resources
RESOURCES += \
    src/rsc.qrc

# Build configuration
CONFIG += c++17
# CONFIG += console  # 移除控制台，GUI应用不需要

# Compiler warnings
QMAKE_CXXFLAGS += -Wall -Wextra

# Debug/Release configurations
CONFIG(debug, debug|release) {
    DEFINES += DEBUG_MODE
} else {
    DEFINES += RELEASE_MODE
    QMAKE_CXXFLAGS_RELEASE += -O3
}
