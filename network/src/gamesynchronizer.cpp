/*
    ReversiAI_Platform - Network Module

    @file gamesynchronizer.cpp
    @brief Game state synchronization implementation
    @date 2026
    @author Project Team
    @license GPL-3.0

    Reference: Egaroucid gtp_command.hpp (GTP protocol)
*/

#include "network/gamesynchronizer.hpp"
#include <QDebug>
#include <QRandomGenerator>
#include <cstring>

namespace Network {

// ==================== Zobrist Hash Initialization ====================

uint64_t GameSynchronizer::zobristBlack_[8][8];
uint64_t GameSynchronizer::zobristWhite_[8][8];
uint64_t GameSynchronizer::zobristPlayer_[2];
bool GameSynchronizer::zobristInitialized_ = false;

void GameSynchronizer::initializeZobrist()
{
    if (zobristInitialized_) {
        return;
    }
    
    // Reference: Zobrist hashing for board games
    QRandomGenerator* rng = QRandomGenerator::global();
    
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            zobristBlack_[i][j] = rng->generate64();
            zobristWhite_[i][j] = rng->generate64();
        }
    }
    
    zobristPlayer_[0] = rng->generate64();  // Black
    zobristPlayer_[1] = rng->generate64();  // White
    
    zobristInitialized_ = true;
    qDebug() << "Zobrist hash table initialized";
}

// ==================== Constructor/Destructor ====================

GameSynchronizer::GameSynchronizer(QObject* parent)
    : QObject(parent)
    , localStateHash_(0)
{
    initializeZobrist();
    
    // Initialize local state with starting position
    localState_.board.resize(8);
    for (int i = 0; i < 8; ++i) {
        localState_.board[i].resize(8);
        for (int j = 0; j < 8; ++j) {
            localState_.board[i][j] = 0;
        }
    }
    
    // Set initial position (D4=3,3 and E5=4,4)
    localState_.board[3][3] = 1;  // black
    localState_.board[4][4] = 1;  // black
    localState_.board[3][4] = 2;  // white
    localState_.board[4][3] = 2;  // white
    
    localState_.currentPlayer = "black";
    localState_.blackCount = 2;
    localState_.whiteCount = 2;
    localState_.moveNumber = 4;
    localState_.gameOver = false;
    
    // Calculate initial hash
    localStateHash_ = calculateStateHash();
}

GameSynchronizer::~GameSynchronizer()
{
}

// ==================== State Management ====================

void GameSynchronizer::setLocalState(const GameStateMessage& state)
{
    localState_ = state;
    localStateHash_ = calculateStateHash();
    emit stateSynchronized(state);
}

bool GameSynchronizer::verifyState(const GameStateMessage& remoteState)
{
    uint64_t remoteHash = remoteState.calculateHash();
    
    if (localStateHash_ == remoteHash) {
        // States match
        return true;
    }
    
    // Mismatch detected
    emit stateMismatch(localStateHash_, remoteHash);
    return false;
}

void GameSynchronizer::requestSync(const QString& target)
{
    Q_UNUSED(target)
    // Create sync request message
    Message msg;
    msg.type = MessageType::SYNC_REQUEST;
    msg.payload["type"] = SyncProtocol::REQUEST_SYNC;
    msg.payload["hash"] = static_cast<qint64>(localStateHash_);
    
    emit syncRequested(target);
    // Note: Message should be sent by NetworkClient
}

void GameSynchronizer::handleSyncRequest(const QString& requester)
{
    Q_UNUSED(requester)
    // Respond with our current state
    Message msg;
    msg.type = MessageType::SYNC_RESPONSE;
    msg.payload["type"] = SyncProtocol::RESPONSE_SYNC;
    msg.payload["state"] = serializeState(localState_);
    msg.payload["hash"] = static_cast<qint64>(localStateHash_);
    
    // Note: Message should be sent by NetworkClient
}

// ==================== Move Processing ====================

bool GameSynchronizer::applyMove(int row, int col, const QString& player)
{
    if (!isValidMove(row, col, player)) {
        emit invalidMove(row, col, player, "Invalid move");
        return false;
    }
    
    int piece = playerToPiece(player);
    localState_.board[row][col] = piece;
    
    // Flip opponent pieces in all 8 directions
    const int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    const int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    
    for (int i = 0; i < 8; ++i) {
        flipDirection(localState_.board, row, col, dr[i], dc[i], piece);
    }
    
    // Update counts
    updatePieceCounts();
    
    // Switch player
    localState_.currentPlayer = (player == "black") ? "white" : "black";
    localState_.moveNumber++;
    
    // Recalculate hash
    localStateHash_ = calculateStateHash();
    
    emit moveApplied(row, col, player);
    emit availableMovesChanged(getAvailableMoves());
    
    return true;
}

bool GameSynchronizer::applyMove(const MoveMessage& move)
{
    return applyMove(move.row, move.col, move.player);
}

QVector<QPair<int, int>> GameSynchronizer::getAvailableMoves()
{
    QVector<QPair<int, int>> moves;
    QString player = localState_.currentPlayer;
    
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (isValidMove(i, j, player)) {
                moves.append(qMakePair(i, j));
            }
        }
    }
    
    return moves;
}

bool GameSynchronizer::isValidMove(int row, int col, const QString& player)
{
    // Must be empty square
    if (localState_.board[row][col] != 0) {
        return false;
    }
    
    // Must flip at least one piece
    int piece = playerToPiece(player);
    int opponent = (piece == 1) ? 2 : 1;
    
    const int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    const int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    
    for (int i = 0; i < 8; ++i) {
        int r = row + dr[i];
        int c = col + dc[i];
        bool foundOpponent = false;
        
        while (isValidCoord(r, c) && localState_.board[r][c] == opponent) {
            r += dr[i];
            c += dc[i];
            foundOpponent = true;
        }
        
        if (foundOpponent && isValidCoord(r, c) && localState_.board[r][c] == piece) {
            return true;  // Valid move - flips pieces
        }
    }
    
    return false;
}

// ==================== Hash Verification ====================

uint64_t GameSynchronizer::calculateBoardHash(const QVector<QVector<int>>& board)
{
    uint64_t hash = 0;
    
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board[i][j] == 1) {
                hash ^= zobristBlack_[i][j];
            } else if (board[i][j] == 2) {
                hash ^= zobristWhite_[i][j];
            }
        }
    }
    
    return hash;
}

uint64_t GameSynchronizer::calculateStateHash()
{
    uint64_t hash = calculateBoardHash(localState_.board);
    
    // Include player turn in hash
    if (localState_.currentPlayer == "black") {
        hash ^= zobristPlayer_[0];
    } else {
        hash ^= zobristPlayer_[1];
    }
    
    return hash;
}

// ==================== Coordinate Conversion ====================

// Reference: Egaroucid gtp_command.hpp gtp_idx_to_coord() (line 78-83)
bool GameSynchronizer::gtpToCoord(const QString& coord, int& row, int& col)
{
    QString c = coord.trimmed().toUpper();
    
    // Handle PASS
    if (c == "PASS") {
        row = -1;
        col = -1;
        return true;
    }
    
    // Validate format: Letter followed by number (A-H, 1-8)
    if (c.length() < 2 || c.length() > 3) {
        return false;
    }
    
    // Column: A-H (file)
    QChar colChar = c[0];
    if (colChar < 'A' || colChar > 'H') {
        return false;
    }
    col = colChar.unicode() - 'A';
    
    // Row: 1-8 (rank) - GTP uses 1-based
    QString rowStr = c.mid(1);
    bool ok = false;
    int rowNum = rowStr.toInt(&ok);
    if (!ok || rowNum < 1 || rowNum > 8) {
        return false;
    }
    row = rowNum - 1;  // Convert to 0-based
    
    return true;
}

// Reference: Inverse of Egaroucid gtp_idx_to_coord()
QString GameSynchronizer::coordToGTP(int row, int col)
{
    if (!isValidCoord(row, col)) {
        return "INVALID";
    }
    
    const char* xCoord = "ABCDEFGH";
    return QString("%1%2").arg(xCoord[col]).arg(row + 1);
}

// Reference: Egaroucid gtp_command.hpp parsing pattern
bool GameSynchronizer::parseGTPMove(const QString& gtpMove, QString& player, int& row, int& col)
{
    QStringList parts = gtpMove.trimmed().split(" ", Qt::SkipEmptyParts);
    
    if (parts.size() < 3) {
        return false;
    }
    
    // Format: "play black D5" or just "black D5"
    QString cmd = parts[0].toLower();
    if (cmd == "play") {
        player = parts[1].toLower();
        return gtpToCoord(parts[2], row, col);
    } else {
        player = parts[0].toLower();
        return gtpToCoord(parts[1], row, col);
    }
}

QString GameSynchronizer::generateGTPMove(const QString& player, int row, int col)
{
    return QString("%1 %2").arg(player).arg(coordToGTP(row, col));
}

// ==================== State Serialization ====================

QJsonObject GameSynchronizer::serializeState(const GameStateMessage& state)
{
    return state.toJson();
}

GameStateMessage GameSynchronizer::deserializeState(const QJsonObject& json)
{
    return GameStateMessage::fromJson(json);
}

// ==================== Private Methods ====================

int GameSynchronizer::playerToPiece(const QString& player)
{
    if (player == "black") {
        return 1;
    } else if (player == "white") {
        return 2;
    }
    return 0;
}

QString GameSynchronizer::pieceToPlayer(int piece)
{
    if (piece == 1) {
        return "black";
    } else if (piece == 2) {
        return "white";
    }
    return "empty";
}

bool GameSynchronizer::isValidCoord(int row, int col)
{
    return row >= 0 && row < 8 && col >= 0 && col < 8;
}

int GameSynchronizer::countDirection(const QVector<QVector<int>>& board, int row, int col, int dr, int dc, int player)
{
    int r = row + dr;
    int c = col + dc;
    int count = 0;
    int opponent = (player == 1) ? 2 : 1;
    
    while (isValidCoord(r, c) && board[r][c] == opponent) {
        r += dr;
        c += dc;
        count++;
    }
    
    if (isValidCoord(r, c) && board[r][c] == player && count > 0) {
        return count;
    }
    
    return 0;
}

void GameSynchronizer::flipDirection(QVector<QVector<int>>& board, int row, int col, int dr, int dc, int player)
{
    int count = countDirection(board, row, col, dr, dc, player);
    
    if (count > 0) {
        int r = row + dr;
        int c = col + dc;
        int opponent = (player == 1) ? 2 : 1;
        
        for (int i = 0; i < count; ++i) {
            board[r][c] = player;
            r += dr;
            c += dc;
        }
    }
}

void GameSynchronizer::updatePieceCounts()
{
    int black = 0, white = 0;
    
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (localState_.board[i][j] == 1) {
                black++;
            } else if (localState_.board[i][j] == 2) {
                white++;
            }
        }
    }
    
    localState_.blackCount = black;
    localState_.whiteCount = white;
}

} // namespace Network

