# CMake generated Testfile for 
# Source directory: D:/Project/Reversi/ReversiAI_Platform/Tests/unit
# Build directory: D:/Project/Reversi/ReversiAI_Platform/build_mingw/Tests/unit
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(UnitTests_Core "D:/Project/Reversi/ReversiAI_Platform/build_mingw/Tests/unit/unit_tests_core.exe")
set_tests_properties(UnitTests_Core PROPERTIES  LABELS "unit;core" TIMEOUT "60" _BACKTRACE_TRIPLES "D:/Project/Reversi/ReversiAI_Platform/Tests/unit/CMakeLists.txt;93;add_test;D:/Project/Reversi/ReversiAI_Platform/Tests/unit/CMakeLists.txt;0;")
add_test(UnitTests_AI "D:/Project/Reversi/ReversiAI_Platform/build_mingw/Tests/unit/unit_tests_ai.exe")
set_tests_properties(UnitTests_AI PROPERTIES  LABELS "unit;ai" TIMEOUT "120" _BACKTRACE_TRIPLES "D:/Project/Reversi/ReversiAI_Platform/Tests/unit/CMakeLists.txt;101;add_test;D:/Project/Reversi/ReversiAI_Platform/Tests/unit/CMakeLists.txt;0;")
add_test(UnitTests_UI "D:/Project/Reversi/ReversiAI_Platform/build_mingw/Tests/unit/unit_tests_ui.exe")
set_tests_properties(UnitTests_UI PROPERTIES  LABELS "unit;ui" TIMEOUT "60" _BACKTRACE_TRIPLES "D:/Project/Reversi/ReversiAI_Platform/Tests/unit/CMakeLists.txt;103;add_test;D:/Project/Reversi/ReversiAI_Platform/Tests/unit/CMakeLists.txt;0;")
add_test(UnitTests_Network "D:/Project/Reversi/ReversiAI_Platform/build_mingw/Tests/unit/unit_tests_network.exe")
set_tests_properties(UnitTests_Network PROPERTIES  LABELS "unit;network" TIMEOUT "60" _BACKTRACE_TRIPLES "D:/Project/Reversi/ReversiAI_Platform/Tests/unit/CMakeLists.txt;113;add_test;D:/Project/Reversi/ReversiAI_Platform/Tests/unit/CMakeLists.txt;0;")
add_test(UnitTests_Research "D:/Project/Reversi/ReversiAI_Platform/build_mingw/Tests/unit/unit_tests_research.exe")
set_tests_properties(UnitTests_Research PROPERTIES  LABELS "unit;research" TIMEOUT "300" _BACKTRACE_TRIPLES "D:/Project/Reversi/ReversiAI_Platform/Tests/unit/CMakeLists.txt;154;add_test;D:/Project/Reversi/ReversiAI_Platform/Tests/unit/CMakeLists.txt;0;")
