# Integration Tests CMake Configuration

# Game flow integration tests
add_executable(integration_tests_game
    game_flow_test.cpp
)

target_link_libraries(integration_tests_game
    PRIVATE
        test_framework
        GTest::gtest_main
)

target_include_directories(integration_tests_game
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework
)

# AI integration tests
add_executable(integration_tests_ai
    ai_integration_test.cpp
)

target_link_libraries(integration_tests_ai
    PRIVATE
        test_framework
        GTest::gtest_main
)

target_include_directories(integration_tests_ai
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework
)

# Network integration tests
add_executable(integration_tests_network
    network_integration_test.cpp
)

target_link_libraries(integration_tests_network
    PRIVATE
        test_framework
        GTest::gtest_main
)

target_include_directories(integration_tests_network
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../framework
)

# Register integration tests
add_test(NAME IntegrationTests_Game COMMAND integration_tests_game)
add_test(NAME IntegrationTests_AI COMMAND integration_tests_ai)
add_test(NAME IntegrationTests_Network COMMAND integration_tests_network)

# Set test properties
set_tests_properties(IntegrationTests_Game PROPERTIES
    TIMEOUT 300
    LABELS "integration;game"
)

set_tests_properties(IntegrationTests_AI PROPERTIES
    TIMEOUT 600
    LABELS "integration;ai"
)

set_tests_properties(IntegrationTests_Network PROPERTIES
    TIMEOUT 120
    LABELS "integration;network"
)
