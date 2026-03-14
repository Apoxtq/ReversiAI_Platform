cmake_minimum_required(VERSION 3.16)

project(performance_tests)

add_executable(perft_performance
    perft.cpp
)

target_include_directories(perft_performance PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

set_target_properties(perft_performance PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Link BitBoard implementation into perft target so symbols resolve
target_sources(perft_performance PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core/BitBoard.cpp
)
target_include_directories(perft_performance PRIVATE
    ${CMAKE_SOURCE_DIR}/include/core
)

# Simple MCTS test
add_executable(simple_mcts_test
    simple_mcts_test.cpp
)

target_include_directories(simple_mcts_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_sources(simple_mcts_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core/BitBoard.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/Evaluator.cpp
)

target_link_libraries(simple_mcts_test PRIVATE
    core_lib
    ai_lib
)

# MCTS comprehensive test
add_executable(mcts_comprehensive_test
    mcts_comprehensive_test.cpp
)

target_include_directories(mcts_comprehensive_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_sources(mcts_comprehensive_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core/BitBoard.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/Evaluator.cpp
)

target_link_libraries(mcts_comprehensive_test PRIVATE
    core_lib
    ai_lib
)

# MCTS step test
add_executable(mcts_step_test
    mcts_step_test.cpp
)

target_include_directories(mcts_step_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_sources(mcts_step_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core/BitBoard.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/Evaluator.cpp
)

target_link_libraries(mcts_step_test PRIVATE
    core_lib
    ai_lib
)

# MCTS vs Minimax test
add_executable(mcts_vs_minimax_test
    mcts_vs_minimax_test.cpp
)

target_include_directories(mcts_vs_minimax_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_sources(mcts_vs_minimax_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core/BitBoard.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/Evaluator.cpp
)

target_link_libraries(mcts_vs_minimax_test PRIVATE
    core_lib
    ai_lib
)

# Test MCTS alone
add_executable(test_mcts_alone
    test_mcts_alone.cpp
)

target_include_directories(test_mcts_alone PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_sources(test_mcts_alone PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core/BitBoard.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/Evaluator.cpp
)

target_link_libraries(test_mcts_alone PRIVATE
    core_lib
    ai_lib
)

# Debug creation test
add_executable(debug_creation
    debug_creation.cpp
)

target_include_directories(debug_creation PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_sources(debug_creation PRIVATE
    ${CMAKE_SOURCE_DIR}/src/core/BitBoard.cpp
    ${CMAKE_SOURCE_DIR}/src/ai/Evaluator.cpp
)

target_link_libraries(debug_creation PRIVATE
    core_lib
    ai_lib
)