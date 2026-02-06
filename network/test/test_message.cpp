/*
    ReversiAI_Platform - Network Module Tests
    @file test_message.cpp
    @brief Unit tests for message serialization
    @date 2026
    @author Project Team
*/

#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <cassert>

#include "network/message.hpp"

using namespace Network;

void testMessageSerialization() {
    qDebug() << "Testing Message serialization...";
    
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
    qDebug() << "Data:" << QString(data);
    
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

void testMoveMessage() {
    qDebug() << "Testing MoveMessage...";
    
    MoveMessage move(3, 4, "black", 10);
    QJsonObject json = move.toJson();
    
    MoveMessage move2 = MoveMessage::fromJson(json);
    
    assert(move2.row == 3);
    assert(move2.col == 4);
    assert(move2.player == "black");
    assert(move2.moveNumber == 10);
    
    qDebug() << "MoveMessage test PASSED!";
}

void testGameStateMessage() {
    qDebug() << "Testing GameStateMessage...";
    
    GameStateMessage state;
    state.board.resize(8);
    for (int i = 0; i < 8; ++i) {
        state.board[i].resize(8);
        for (int j = 0; j < 8; ++j) {
            state.board[i][j] = (i == j) ? 1 : 0;
        }
    }
    state.currentPlayer = "black";
    state.blackCount = 2;
    state.whiteCount = 2;
    state.moveNumber = 4;
    state.gameOver = false;
    
    QJsonObject json = state.toJson();
    GameStateMessage state2 = GameStateMessage::fromJson(json);
    
    assert(state2.blackCount == 2);
    assert(state2.currentPlayer == "black");
    assert(state2.board[0][0] == 1);
    assert(state2.board[1][1] == 1);
    
    // Test hash calculation
    uint64_t hash1 = state.calculateHash();
    uint64_t hash2 = state2.calculateHash();
    assert(hash1 == hash2);
    
    qDebug() << "GameStateMessage test PASSED!";
    qDebug() << "Board hash:" << hash1;
}

void testRoomMessage() {
    qDebug() << "Testing GameRoom...";
    
    GameRoom room;
    room.roomId = "ABC123";
    room.roomName = "Test Room";
    room.hostName = "Player1";
    room.state = RoomState::WAITING;
    room.timeLimit = 300;
    
    QJsonObject json = room.toJson();
    GameRoom room2 = GameRoom::fromJson(json);
    
    assert(room2.roomId == "ABC123");
    assert(room2.roomName == "Test Room");
    assert(room2.state == RoomState::WAITING);
    assert(room2.isJoinable());
    
    qDebug() << "GameRoom test PASSED!";
}

void testCoordConversion() {
    qDebug() << "Testing coordinate conversion...";
    
    // Test GTP to coord
    int row, col;
    assert(gtpToCoord("D5", row, col));
    assert(row == 4 && col == 3);
    
    assert(gtpToCoord("A1", row, col));
    assert(row == 0 && col == 0);
    
    assert(gtpToCoord("H8", row, col));
    assert(row == 7 && col == 7);
    
    assert(gtpToCoord("PASS", row, col));
    assert(row == -1 && col == -1);
    
    // Test coord to GTP
    assert(coordToGTP(4, 3) == "D5");
    assert(coordToGTP(0, 0) == "A1");
    assert(coordToGTP(7, 7) == "H8");
    
    qDebug() << "Coordinate conversion test PASSED!";
}

void testHeartbeatMessage() {
    qDebug() << "Testing heartbeat message...";
    
    Message ping = Message::createPing();
    assert(ping.type == MessageType::PING);
    
    Message pong = Message::createPong(ping.timestamp);
    assert(pong.type == MessageType::PONG);
    assert(pong.timestamp == ping.timestamp);
    
    Message heartbeat = Message::createHeartbeat();
    assert(heartbeat.type == MessageType::HEARTBEAT);
    
    qDebug() << "Heartbeat message test PASSED!";
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "======================================";
    qDebug() << "Network Message Serialization Tests";
    qDebug() << "======================================\n";
    
    testMessageSerialization();
    testMoveMessage();
    testGameStateMessage();
    testRoomMessage();
    testCoordConversion();
    testHeartbeatMessage();
    
    qDebug() << "\n======================================";
    qDebug() << "All tests PASSED!";
    qDebug() << "======================================";
    
    return 0;
}

