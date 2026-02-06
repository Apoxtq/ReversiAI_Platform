/*
    ReversiAI_Platform - Network Module Tests
    @file test_network_integration.cpp
    @brief Integration tests for the entire network module
    @date 2026
    @author Project Team
*/

#include <QCoreApplication>
#include <QDebug>
#include <QSignalSpy>
#include <QThread>

#include "network/networkclient.hpp"
#include "network/networkdiscovery.hpp"
#include "network/gamesynchronizer.hpp"
#include "network/roommanager.hpp"
#include "network/reconnectionmanager.hpp"
#include "network/message.hpp"

using namespace Network;

void testMessageTypes() {
    qDebug() << "Testing message types...";
    
    // Verify all message types are defined
    assert(static_cast<int>(MessageType::HANDSHAKE) == 0);
    assert(static_cast<int>(MessageType::MOVE_MADE) == 4);
    assert(static_cast<int>(MessageType::HEARTBEAT) == 8);
    assert(static_cast<int>(MessageType::PING) == 9);
    assert(static_cast<int>(MessageType::CREATE_ROOM) == 14);
    
    // Test message creation
    Message ping = Message::createPing();
    assert(ping.type == MessageType::PING);
    
    Message pong = Message::createPong(ping.timestamp);
    assert(pong.type == MessageType::PONG);
    
    Message heartbeat = Message::createHeartbeat();
    assert(heartbeat.type == MessageType::HEARTBEAT);
    
    qDebug() << "Message types test PASSED!";
}

void testConnectionStates() {
    qDebug() << "Testing connection states...";
    
    assert(static_cast<int>(ConnectionState::Disconnected) == 0);
    assert(static_cast<int>(ConnectionState::Connecting) == 1);
    assert(static_cast<int>(ConnectionState::Connected) == 2);
    assert(static_cast<int>(ConnectionState::Disconnecting) == 3);
    assert(static_cast<int>(ConnectionState::Error) == 4);
    
    qDebug() << "Connection states test PASSED!";
}

void testRoomStates() {
    qDebug() << "Testing room states...";
    
    assert(static_cast<int>(RoomState::WAITING) == 0);
    assert(static_cast<int>(RoomState::READY) == 1);
    assert(static_cast<int>(RoomState::PLAYING) == 2);
    assert(static_cast<int>(RoomState::FINISHED) == 3);
    assert(static_cast<int>(RoomState::ABANDONED) == 4);
    
    qDebug() << "Room states test PASSED!";
}

void testNetworkErrors() {
    qDebug() << "Testing network errors...";
    
    assert(static_cast<int>(NetworkError::ConnectionRefused) == 0);
    assert(static_cast<int>(NetworkError::HostNotFound) == 1);
    assert(static_cast<int>(NetworkError::ConnectionTimeout) == 2);
    assert(static_cast<int>(NetworkError::ConnectionReset) == 3);
    assert(static_cast<int>(NetworkError::Unknown) == 11);
    
    qDebug() << "Network errors test PASSED!";
}

void testGameRoomJoinable() {
    qDebug() << "Testing GameRoom joinable states...";
    
    GameRoom room;
    
    // WAITING room is joinable
    room.state = RoomState::WAITING;
    assert(room.isJoinable());
    assert(!room.isPlaying());
    
    // READY room is joinable
    room.state = RoomState::READY;
    assert(room.isJoinable());
    assert(!room.isPlaying());
    
    // PLAYING room is not joinable
    room.state = RoomState::PLAYING;
    assert(!room.isJoinable());
    assert(room.isPlaying());
    
    // FINISHED room is not joinable
    room.state = RoomState::FINISHED;
    assert(!room.isJoinable());
    assert(!room.isPlaying());
    
    qDebug() << "GameRoom joinable test PASSED!";
}

void testMoveMessage() {
    qDebug() << "Testing move message...";
    
    MoveMessage move(3, 4, "black", 10);
    
    assert(move.row == 3);
    assert(move.col == 4);
    assert(move.player == "black");
    assert(move.moveNumber == 10);
    
    QJsonObject json = move.toJson();
    assert(json["row"].toInt() == 3);
    assert(json["col"].toInt() == 4);
    
    MoveMessage move2 = MoveMessage::fromJson(json);
    assert(move2.row == 3);
    assert(move2.col == 4);
    
    qDebug() << "Move message test PASSED!";
}

void testGameStateMessage() {
    qDebug() << "Testing game state message...";
    
    GameStateMessage state;
    state.board.resize(8);
    for (int i = 0; i < 8; ++i) {
        state.board[i].resize(8);
    }
    
    // Set initial position
    state.board[3][3] = 1;
    state.board[4][4] = 1;
    state.board[3][4] = 2;
    state.board[4][3] = 2;
    
    state.currentPlayer = "black";
    state.blackCount = 2;
    state.whiteCount = 2;
    state.moveNumber = 4;
    state.gameOver = false;
    
    // Test hash
    uint64_t hash = state.calculateHash();
    qDebug() << "Initial state hash:" << hash;
    assert(hash != 0);
    
    // Serialize and deserialize
    QJsonObject json = state.toJson();
    GameStateMessage state2 = GameStateMessage::fromJson(json);
    
    assert(state2.blackCount == 2);
    assert(state2.whiteCount == 2);
    assert(state2.calculateHash() == hash);
    
    qDebug() << "Game state message test PASSED!";
}

void testChatMessage() {
    qDebug() << "Testing chat message...";
    
    ChatMessage msg("Player1", "Hello, world!");
    
    assert(msg.sender == "Player1");
    assert(msg.content == "Hello, world!");
    assert(msg.type == "normal");
    assert(msg.timestamp > 0);
    
    QJsonObject json = msg.toJson();
    assert(json["sender"].toString() == "Player1");
    assert(json["content"].toString() == "Hello, world!");
    
    ChatMessage msg2 = ChatMessage::fromJson(json);
    assert(msg2.sender == "Player1");
    assert(msg2.content == "Hello, world!");
    
    qDebug() << "Chat message test PASSED!";
}

void testDiscoveredHost() {
    qDebug() << "Testing discovered host...";
    
    DiscoveredHost host;
    host.playerName = "TestPlayer";
    host.roomName = "TestRoom";
    host.address = QHostAddress("192.168.1.100");
    host.port = 12345;
    host.gameVersion = "0.5.0";
    
    QJsonObject json = host.toJson();
    assert(json["playerName"].toString() == "TestPlayer");
    assert(json["roomName"].toString() == "TestRoom");
    assert(json["port"].toInt() == 12345);
    
    DiscoveredHost host2 = DiscoveredHost::fromJson(json, QHostAddress("192.168.1.100"));
    assert(host2.playerName == "TestPlayer");
    assert(host2.port == 12345);
    assert(host2.address == QHostAddress("192.168.1.100"));
    
    qDebug() << "Discovered host test PASSED!";
}

void testGTPCoordinates() {
    qDebug() << "Testing GTP coordinate conversion...";
    
    int row, col;
    
    // Valid coordinates
    assert(gtpToCoord("D5", row, col));
    assert(row == 4 && col == 3);
    
    assert(gtpToCoord("A1", row, col));
    assert(row == 0 && col == 0);
    
    assert(gtpToCoord("H8", row, col));
    assert(row == 7 && col == 7);
    
    assert(gtpToCoord("PASS", row, col));
    assert(row == -1 && col == -1);
    
    // Invalid coordinates
    assert(!gtpToCoord("I1", row, col));
    assert(!gtpToCoord("A9", row, col));
    assert(!gtpToCoord("", row, col));
    
    // Reverse conversion
    assert(coordToGTP(4, 3) == "D5");
    assert(coordToGTP(0, 0) == "A1");
    assert(coordToGTP(7, 7) == "H8");
    
    qDebug() << "GTP coordinate conversion test PASSED!";
}

void testGTPMoveParsing() {
    qDebug() << "Testing GTP move parsing...";
    
    QString player;
    int row, col;
    
    // Format: "play black D5"
    assert(parseGTPMove("play black D5", player, row, col));
    assert(player == "black");
    assert(row == 4 && col == 3);
    
    // Format: "white E6"
    assert(parseGTPMove("white E6", player, row, col));
    assert(player == "white");
    assert(row == 5 && col == 4);
    
    // Format: "play black PASS"
    assert(parseGTPMove("play black PASS", player, row, col));
    assert(row == -1 && col == -1);
    
    qDebug() << "GTP move parsing test PASSED!";
}

void testRoomSettings() {
    qDebug() << "Testing room settings...";
    
    RoomSettings settings;
    settings.timeLimit = 300;
    settings.isRanked = true;
    settings.allowSpectators = true;
    settings.maxPlayers = 2;
    settings.gameType = "pvp";
    settings.aiDifficulty = 5;
    
    QJsonObject json = settings.toJson();
    assert(json["timeLimit"].toInt() == 300);
    assert(json["isRanked"].toBool() == true);
    assert(json["maxPlayers"].toInt() == 2);
    
    RoomSettings settings2 = RoomSettings::fromJson(json);
    assert(settings2.timeLimit == 300);
    assert(settings2.isRanked == true);
    assert(settings2.gameType == "pvp");
    
    qDebug() << "Room settings test PASSED!";
}

void testMessageSerialization() {
    qDebug() << "Testing message serialization...";
    
    // Create a message
    Message msg;
    msg.type = MessageType::MOVE_MADE;
    msg.sequence = 42;
    msg.timestamp = 1234567890;
    msg.sender = "Player1";
    msg.receiver = "Player2";
    msg.payload["row"] = 3;
    msg.payload["col"] = 4;
    
    // Serialize
    QByteArray data = msg.serialize();
    qDebug() << "Serialized size:" << data.size() << "bytes";
    assert(data.size() > 0);
    
    // Deserialize
    bool ok = false;
    Message msg2 = Message::deserialize(data, &ok);
    assert(ok);
    assert(msg2.type == MessageType::MOVE_MADE);
    assert(msg2.sequence == 42);
    assert(msg2.sender == "Player1");
    assert(msg2.payload["row"].toInt() == 3);
    
    qDebug() << "Message serialization test PASSED!";
}

void testStateHash() {
    qDebug() << "Testing state hash consistency...";
    
    GameStateMessage state1;
    state1.board.resize(8);
    for (int i = 0; i < 8; ++i) {
        state1.board[i].resize(8);
    }
    state1.board[3][3] = 1;
    state1.board[4][4] = 1;
    state1.board[3][4] = 2;
    state1.board[4][3] = 2;
    state1.currentPlayer = "black";
    state1.moveNumber = 4;
    
    // Hash should be same for same state
    uint64_t hash1 = state1.calculateHash();
    uint64_t hash2 = state1.calculateHash();
    assert(hash1 == hash2);
    
    // Create identical state
    GameStateMessage state2 = GameStateMessage::fromJson(state1.toJson());
    uint64_t hash3 = state2.calculateHash();
    assert(hash1 == hash3);
    
    // Modify state
    state1.board[2][3] = 1;
    uint64_t hash4 = state1.calculateHash();
    assert(hash1 != hash4);
    
    qDebug() << "State hash test PASSED!";
    qDebug() << "Initial hash:" << hash1;
    qDebug() << "After move hash:" << hash4;
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "==========================================";
    qDebug() << "Network Module Integration Tests";
    qDebug() << "==========================================\n";
    
    testMessageTypes();
    testConnectionStates();
    testRoomStates();
    testNetworkErrors();
    testGameRoomJoinable();
    testMoveMessage();
    testGameStateMessage();
    testChatMessage();
    testDiscoveredHost();
    testGTPCoordinates();
    testGTPMoveParsing();
    testRoomSettings();
    testMessageSerialization();
    testStateHash();
    
    qDebug() << "\n==========================================";
    qDebug() << "All integration tests PASSED!";
    qDebug() << "==========================================";
    qDebug() << "";
    qDebug() << "v0.5.0 Network Module Summary:";
    qDebug() << "- Message types: Core, Game, Room, Chat";
    qDebug() << "- Connection states: Disconnected to Error";
    qDebug() << "- Room management: Create, Join, Play, End";
    qDebug() << "- GTP coordinate support: A1-H8";
    qDebug() << "- State hash: Zobrist hashing for sync";
    qDebug() << "==========================================";
    
    return 0;
}

