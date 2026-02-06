# Network Module CMakeLists.txt
# v0.5.0 - LAN Multiplayer Functionality

# Qt Network and Test modules are required
find_package(Qt6 REQUIRED COMPONENTS Core Network Test)

# Generate MOC files for Qt meta-object system
qt6_wrap_cpp(MOC_SOURCES
    include/network/message.hpp
    include/network/networkclient.hpp
    include/network/networkdiscovery.hpp
    include/network/gamesynchronizer.hpp
    include/network/roommanager.hpp
    include/network/reconnectionmanager.hpp
)

# Network library: compile network components
add_library(network_lib STATIC
    src/message.cpp
    src/networkclient.cpp
    src/networkdiscovery.cpp
    src/gamesynchronizer.cpp
    src/roommanager.cpp
    src/reconnectionmanager.cpp
    ${MOC_SOURCES}
)

target_include_directories(network_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(network_lib PRIVATE 
    Qt6::Core 
    Qt6::Network
    core_lib  # Link against core library for Board types
)
target_compile_features(network_lib PUBLIC cxx_std_17)

# Network tests
if(BUILD_TESTS)
    # NetworkClient tests
    add_executable(network_test_networkclient
        test/test_networkclient.cpp
    )
    target_link_libraries(network_test_networkclient PRIVATE
        network_lib
        Qt6::Core
        Qt6::Network
        Qt6::Test
    )
    add_test(network_test_networkclient network_test_networkclient)
    
    # Discovery & Synchronization tests
    add_executable(network_test_discovery_sync
        test/test_discovery_sync.cpp
    )
    target_link_libraries(network_test_discovery_sync PRIVATE
        network_lib
        Qt6::Core
        Qt6::Network
        Qt6::Test
    )
    add_test(network_test_discovery_sync network_test_discovery_sync)
    
    # Room & Reconnection tests
    add_executable(network_test_room_reconnect
        test/test_room_reconnect.cpp
    )
    target_link_libraries(network_test_room_reconnect PRIVATE
        network_lib
        Qt6::Core
        Qt6::Network
        Qt6::Test
    )
    add_test(network_test_room_reconnect network_test_room_reconnect)
    
    # Integration tests
    add_executable(network_test_integration
        test/test_network_integration.cpp
    )
    target_link_libraries(network_test_integration PRIVATE
        network_lib
        Qt6::Core
        Qt6::Network
        Qt6::Test
    )
    add_test(network_test_integration network_test_integration)

    # Full integration tests
    add_executable(network_test_full
        test/test_network_full.cpp
    )
    target_link_libraries(network_test_full PRIVATE
        network_lib
        Qt6::Core
        Qt6::Network
        Qt6::Test
    )
    add_test(network_test_full network_test_full)
endif()
