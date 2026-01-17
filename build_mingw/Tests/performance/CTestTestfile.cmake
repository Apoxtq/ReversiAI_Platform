# CMake generated Testfile for 
# Source directory: D:/Project/Reversi/ReversiAI_Platform/Tests/performance
# Build directory: D:/Project/Reversi/ReversiAI_Platform/build_mingw/Tests/performance
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(PerformanceTests_Core "D:/Project/Reversi/ReversiAI_Platform/build_mingw/Tests/performance/performance_tests_core.exe")
set_tests_properties(PerformanceTests_Core PROPERTIES  LABELS "performance;core" TIMEOUT "1800" _BACKTRACE_TRIPLES "D:/Project/Reversi/ReversiAI_Platform/Tests/performance/CMakeLists.txt;77;add_test;D:/Project/Reversi/ReversiAI_Platform/Tests/performance/CMakeLists.txt;0;")
