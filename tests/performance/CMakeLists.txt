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


