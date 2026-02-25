# CMake generated Testfile for 
# Source directory: D:/Project/Reversi/ReversiAI_Platform/Tests/framework
# Build directory: D:/Project/Reversi/ReversiAI_Platform/build_test/Tests/framework
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(UnitTests_Board "D:/Project/Reversi/ReversiAI_Platform/build_test/Tests/framework/Debug/unit_tests_board.exe")
  set_tests_properties(UnitTests_Board PROPERTIES  _BACKTRACE_TRIPLES "D:/Project/Reversi/ReversiAI_Platform/Tests/framework/CMakeLists.txt;116;add_test;D:/Project/Reversi/ReversiAI_Platform/Tests/framework/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(UnitTests_Board "D:/Project/Reversi/ReversiAI_Platform/build_test/Tests/framework/Release/unit_tests_board.exe")
  set_tests_properties(UnitTests_Board PROPERTIES  _BACKTRACE_TRIPLES "D:/Project/Reversi/ReversiAI_Platform/Tests/framework/CMakeLists.txt;116;add_test;D:/Project/Reversi/ReversiAI_Platform/Tests/framework/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(UnitTests_Board "D:/Project/Reversi/ReversiAI_Platform/build_test/Tests/framework/MinSizeRel/unit_tests_board.exe")
  set_tests_properties(UnitTests_Board PROPERTIES  _BACKTRACE_TRIPLES "D:/Project/Reversi/ReversiAI_Platform/Tests/framework/CMakeLists.txt;116;add_test;D:/Project/Reversi/ReversiAI_Platform/Tests/framework/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(UnitTests_Board "D:/Project/Reversi/ReversiAI_Platform/build_test/Tests/framework/RelWithDebInfo/unit_tests_board.exe")
  set_tests_properties(UnitTests_Board PROPERTIES  _BACKTRACE_TRIPLES "D:/Project/Reversi/ReversiAI_Platform/Tests/framework/CMakeLists.txt;116;add_test;D:/Project/Reversi/ReversiAI_Platform/Tests/framework/CMakeLists.txt;0;")
else()
  add_test(UnitTests_Board NOT_AVAILABLE)
endif()
subdirs("../../_deps/googletest-build")
