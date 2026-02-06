/*
    ReversiAI_Platform - Network Module Tests
    @file test_discovery_sync.cpp
    @brief Tests for NetworkDiscovery and GameSynchronizer
    @date 2026
    @author Project Team
*/

#include <QCoreApplication>
#include <QDebug>
#include <QSignalSpy>
#include <QUdpSocket>

#include "network/networkdiscovery.hpp"
#include "network/gamesynchronizer.hpp"
#include "network/message.hpp"

using namespace Network;

void testGTPCoordinates() {
    qDebug() << "Testing GTP coordinate conversion...";
    
    int row, col;
    
    // Test valid coordinates
    assert(gtpToCoord("D5", row, col));
    assert(row == 4 && col == 3);
    
    assert(gtpToCoord("A1", row, col));
    assert(row == 0 && col == 0);
    
    assert(gtpToCoord("H8", row, col));
    assert(row == 7 && col == 7);
    
    assert(gtpToCoord("PASS", row, col));
    assert(row == -1 && col == -1);
    
    // Test lowercase
    assert(gtpToCoord("d5", row, col));
    assert(row == 4 && col == 3);
    
    // Test invalid coordinates
    assert(!gtpToCoord("I1", row, col));  // Invalid column
    assert(!gtpToCoord("A9", row, col));  // Invalid row
    assert(!gtpToCoord("", row, col));     // Empty
    
    // Test coordToGTP
    assert(coordToGTP(4, 3) == "D5");
    assert(coordToGTP(0, 0) == "A1");
    assert(coordToGTP(7, 7) == "H8");
    
    qDebug() << "GTP coordinate conversion test PASSED!";
}

void testGTPParseMove() {
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
    
    qDebug() << "GTP move parsing test PASSED!";
}

void testGameSynchronizer() {
    qDebug() << "Testing GameSynchronizer...";
    
    GameSynchronizer sync;
    
    // Test initial state
    GameStateMessage state = sync.getLocalState();
    assert(state.moveNumber == 4);
    assert(state.blackCount == 2);
    assert(state.whiteCount == 2);
    assert(state.currentPlayer == "black");
    
    // Test valid move
    assert(sync.isValidMove(2, 3, "black"));  // D3 should be valid
    assert(sync.applyMove(2, 3, "black"));
    
    // After black plays D3
    state = sync.getLocalState();
    assert(state.moveNumber == 5);
    assert(state.currentPlayer == "white");
    
    qDebug() << "GameSynchronizer test PASSED!";
}

void testStateHash() {
    qDebug() << "Testing state hash verification...";
    
    GameSynchronizer sync;
    
    // Get initial hash
    uint64_t hash1 = sync.calculateStateHash();
    
    // Make a move
    sync.applyMove(2, 3, "black");
    
    // Hash should change
    uint64_t hash2 = sync.calculateStateHash();
    assert(hash1 != hash2);
    
    // Verify state
    GameStateMessage remoteState = sync.getLocalState();
    assert(sync.verifyState(remoteState));
    
    qDebug() << "State hash test PASSED!";
    qDebug() << "Initial hash:" << hash1;
    qDebug() << "After move hash:" << hash2;
}

void testInvalidMove() {
    qDebug() << "Testing invalid move rejection...";
    
    GameSynchronizer sync;
    
    // D4 should be occupied (black starts there)
    assert(!sync.isValidMove(3, 3, "black"));
    assert(!sync.applyMove(3, 3, "black"));
    
    // A1 should be invalid for black
    assert(!sync.isValidMove(0, 0, "black"));
    
    qDebug() << "Invalid move test PASSED!";
}

void testAvailableMoves() {
    qDebug() << "Testing available moves calculation...";
    
    GameSynchronizer sync;
    
    // Initial position: black has 4 valid moves
    QVector<QPair<int, int>> moves = sync.getAvailableMoves();
    qDebug() << "Initial black moves:" << moves.size();
    
    // Should have D3, C4, E4, F5 (or similar)
    assert(moves.size() == 4);
    
    // Make a move
    sync.applyMove(2, 3, "black");
    
    // White's turn - should have different moves
    moves = sync.getAvailableMoves();
    qDebug() << "White moves after black D3:" << moves.size();
    assert(moves.size() > 0);
    
    qDebug() << "Available moves test PASSED!";
}

void testBoardHash() {
    qDebug() << "Testing board hash calculation...";
    
    QVector<QVector<int>> board(8, QVector<int>(8, 0));
    
    // Set initial position
    board[3][3] = 1; board[3][4] = 2;
    board[4][3] = 2; board[4][4] = 1;
    
    GameSynchronizer sync(nullptr);
    uint64_t hash1 = sync.calculateBoardHash(board);
    
    // Modify board
    board[2][3] = 1;
    uint64_t hash2 = sync.calculateBoardHash(board);
    
    assert(hash1 != hash2);
    
    qDebug() << "Board hash test PASSED!";
    qDebug() << "Initial board hash:" << hash1;
    qDebug() << "After move hash:" << hash2;
}

void testDiscoveredHost() {
    qDebug() << "Testing DiscoveredHost...";
    
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
    
    qDebug() << "DiscoveredHost test PASSED!";
}

void testGameStateMessage() {
    qDebug() << "Testing GameStateMessage...";
    
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
    
    // Test serialization
    QJsonObject json = state.toJson();
    GameStateMessage state2 = GameStateMessage::fromJson(json);
    
    assert(state2.blackCount == 2);
    assert(state2.whiteCount == 2);
    assert(state2.board[3][3] == 1);
    assert(state2.board[4][4] == 1);
    
    // Test hash
    uint64_t hash1 = state.calculateHash();
    uint64_t hash2 = state2.calculateHash();
    assert(hash1 == hash2);
    
    qDebug() << "GameStateMessage test PASSED!";
    qDebug() << "State hash:" << hash1;
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "========================================";
    qDebug() << "Network Discovery & Sync Tests";
    qDebug() << "========================================\n";
    
    testGTPCoordinates();
    testGTPParseMove();
    testGameSynchronizer();
    testStateHash();
    testInvalidMove();
    testAvailableMoves();
    testBoardHash();
    testDiscoveredHost();
    testGameStateMessage();
    
    qDebug() << "\n========================================";
    qDebug() << "All Discovery & Sync tests PASSED!";
    qDebug() << "========================================";
    
    return 0;
}

