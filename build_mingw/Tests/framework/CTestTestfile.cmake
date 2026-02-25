# CMake generated Testfile for 
# Source directory: D:/Project/Reversi/ReversiAI_Platform/Tests/framework
# Build directory: D:/Project/Reversi/ReversiAI_Platform/build_mingw/Tests/framework
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(UnitTests_Board "D:/Project/Reversi/ReversiAI_Platform/build_mingw/Tests/framework/unit_tests_board.exe")
set_tests_properties(UnitTests_Board PROPERTIES  _BACKTRACE_TRIPLES "D:/Project/Reversi/ReversiAI_Platform/Tests/framework/CMakeLists.txt;116;add_test;D:/Project/Reversi/ReversiAI_Platform/Tests/framework/CMakeLists.txt;0;")
subdirs("../../_deps/googletest-build")
