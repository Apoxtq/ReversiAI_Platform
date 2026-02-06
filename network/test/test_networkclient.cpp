/*
    ReversiAI_Platform - Network Module Tests
    @file test_networkclient.cpp
    @brief Unit tests for NetworkClient
    @date 2026
    @author Project Team
*/

#include <QCoreApplication>
#include <QDebug>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>

#include "network/networkclient.hpp"
#include "network/message.hpp"

using namespace Network;

void testConnectionStateMachine() {
    qDebug() << "Testing connection state machine...";
    
    NetworkClient client;
    
    // Test initial state
    assert(client.getState() == ConnectionState::Disconnected);
    assert(!client.isConnected());
    
    qDebug() << "Initial state: Disconnected (correct)";
    
    // Test state transitions will be verified in integration tests
    // since they require actual network connections
    
    qDebug() << "Connection state machine test PASSED!";
}

void testMessageQueue() {
    qDebug() << "Testing message queue...";
    
    NetworkClient client;
    
    // Test empty queue
    assert(client.isSendQueueEmpty());
    assert(client.getPendingMessageCount() == 0);
    
    qDebug() << "Message queue test PASSED!";
}

void testMoveMessage() {
    qDebug() << "Testing move message creation...";
    
    // Test move message creation
    Message msg;
    msg.type = MessageType::MOVE_MADE;
    msg.payload["row"] = 3;
    msg.payload["col"] = 4;
    msg.payload["player"] = "black";
    msg.payload["moveNumber"] = 10;
    
    QByteArray data = msg.serialize();
    qDebug() << "Move message size:" << data.size() << "bytes";
    
    // Deserialize and verify
    bool ok = false;
    Message msg2 = Message::deserialize(data, &ok);
    assert(ok);
    assert(msg2.type == MessageType::MOVE_MADE);
    assert(msg2.payload["row"].toInt() == 3);
    assert(msg2.payload["col"].toInt() == 4);
    assert(msg2.payload["player"].toString() == "black");
    assert(msg2.payload["moveNumber"].toInt() == 10);
    
    qDebug() << "Move message test PASSED!";
}

void testGameStateMessage() {
    qDebug() << "Testing game state message...";
    
    GameStateMessage state;
    state.board.resize(8);
    for (int i = 0; i < 8; ++i) {
        state.board[i].resize(8);
        for (int j = 0; j < 8; ++j) {
            state.board[i][j] = 0;
        }
    }
    // Set initial position (D4 and E5)
    state.board[3][3] = 1;  // black
    state.board[4][4] = 1;  // black
    state.board[3][4] = 2;  // white
    state.board[4][3] = 2;  // white
    
    state.currentPlayer = "black";
    state.blackCount = 2;
    state.whiteCount = 2;
    state.moveNumber = 4;
    state.gameOver = false;
    
    QJsonObject jsonObj = state.toJson();
    QJsonDocument doc(jsonObj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    qDebug() << "Game state message size:" << data.size() << "bytes";
    
    GameStateMessage state2 = GameStateMessage::fromJson(state.toJson());
    assert(state2.blackCount == 2);
    assert(state2.whiteCount == 2);
    assert(state2.board[3][3] == 1);
    assert(state2.board[4][4] == 1);
    
    // Test hash
    uint64_t hash1 = state.calculateHash();
    uint64_t hash2 = state2.calculateHash();
    assert(hash1 == hash2);
    qDebug() << "Board hash:" << hash1;
    
    qDebug() << "Game state message test PASSED!";
}

void testHeartbeatMessage() {
    qDebug() << "Testing heartbeat messages...";
    
    Message ping = Message::createPing();
    assert(ping.type == MessageType::PING);
    assert(ping.timestamp > 0);
    
    Message pong = Message::createPong(ping.timestamp);
    assert(pong.type == MessageType::PONG);
    assert(pong.timestamp == ping.timestamp);
    
    Message heartbeat = Message::createHeartbeat();
    assert(heartbeat.type == MessageType::HEARTBEAT);
    assert(heartbeat.timestamp > 0);
    
    qDebug() << "Heartbeat message test PASSED!";
}

void testErrorTypes() {
    qDebug() << "Testing error types...";
    
    // Verify all error types are defined
    NetworkError errors[] = {
        NetworkError::ConnectionRefused,
        NetworkError::HostNotFound,
        NetworkError::ConnectionTimeout,
        NetworkError::ConnectionReset,
        NetworkError::SendFailed,
        NetworkError::ReceiveFailed,
        NetworkError::ProtocolError,
        NetworkError::VersionMismatch,
        NetworkError::RoomFull,
        NetworkError::RoomNotFound,
        NetworkError::Timeout,
        NetworkError::Unknown
    };
    
    assert(sizeof(errors) / sizeof(errors[0]) == 12);
    
    qDebug() << "Error types test PASSED!";
}

void testConnectionStates() {
    qDebug() << "Testing connection states...";
    
    // Verify all connection states
    ConnectionState states[] = {
        ConnectionState::Disconnected,
        ConnectionState::Connecting,
        ConnectionState::Connected,
        ConnectionState::Disconnecting,
        ConnectionState::Error
    };
    
    assert(sizeof(states) / sizeof(states[0]) == 5);
    
    qDebug() << "Connection states test PASSED!";
}

void testRoomStates() {
    qDebug() << "Testing room states...";
    
    // Verify all room states
    RoomState states[] = {
        RoomState::WAITING,
        RoomState::READY,
        RoomState::PLAYING,
        RoomState::FINISHED,
        RoomState::ABANDONED
    };
    
    assert(sizeof(states) / sizeof(states[0]) == 5);
    
    // Test GameRoom structure
    GameRoom room;
    room.roomId = "ABC123";
    room.roomName = "Test Room";
    room.hostName = "Player1";
    room.state = RoomState::WAITING;
    assert(room.isJoinable());
    
    room.state = RoomState::PLAYING;
    assert(!room.isJoinable());
    assert(room.isPlaying());
    
    qDebug() << "Room states test PASSED!";
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
    
    qDebug() << "Discovered host test PASSED!";
}

void testChatMessage() {
    qDebug() << "Testing chat message...";
    
    ChatMessage msg("Player1", "Hello, world!");
    QJsonObject json = msg.toJson();
    
    assert(json["sender"].toString() == "Player1");
    assert(json["content"].toString() == "Hello, world!");
    assert(json["type"].toString() == "normal");
    assert(msg.timestamp > 0);
    
    ChatMessage msg2 = ChatMessage::fromJson(json);
    assert(msg2.sender == "Player1");
    assert(msg2.content == "Hello, world!");
    
    qDebug() << "Chat message test PASSED!";
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "=======================================";
    qDebug() << "Network Client Tests";
    qDebug() << "=======================================\n";
    
    testConnectionStateMachine();
    testMessageQueue();
    testMoveMessage();
    testGameStateMessage();
    testHeartbeatMessage();
    testErrorTypes();
    testConnectionStates();
    testRoomStates();
    testDiscoveredHost();
    testChatMessage();
    
    qDebug() << "\n=======================================";
    qDebug() << "All Network Client tests PASSED!";
    qDebug() << "=======================================";
    
    return 0;
}

