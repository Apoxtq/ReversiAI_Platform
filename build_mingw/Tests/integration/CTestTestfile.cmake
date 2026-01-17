# CMake generated Testfile for 
# Source directory: D:/Project/Reversi/ReversiAI_Platform/Tests/integration
# Build directory: D:/Project/Reversi/ReversiAI_Platform/build_mingw/Tests/integration
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(IntegrationTests_Game "D:/Project/Reversi/ReversiAI_Platform/build_mingw/Tests/integration/integration_tests_game.exe")
set_tests_properties(IntegrationTests_Game PROPERTIES  LABELS "integration;game" TIMEOUT "300" _BACKTRACE_TRIPLES "D:/Project/Reversi/ReversiAI_Platform/Tests/integration/CMakeLists.txt;55;add_test;D:/Project/Reversi/ReversiAI_Platform/Tests/integration/CMakeLists.txt;0;")
