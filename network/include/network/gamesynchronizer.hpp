/*
    ReversiAI_Platform - Network Module

    @file gamesynchronizer.hpp
    @brief Game state synchronization with hash verification
    @date 2026
    @author Project Team
    @license GPL-3.0

    Reference: Egaroucid gtp_command.hpp (GTP protocol)
*/

#ifndef GAME_SYNCHRONIZER_HPP
#define GAME_SYNCHRONIZER_HPP

#include <QObject>
#include <QJsonObject>
#include <QVector>
#include <QHash>
#include <cstdint>

#include "network/message.hpp"

namespace Network {

/**
 * @brief Game State Synchronizer
 * 
 * Handles game state synchronization between peers with hash verification.
 * 
 * Reference: Egaroucid gtp_command.hpp GTP coordinate handling
 * Uses Zobrist hashing for board state verification.
 */
class GameSynchronizer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a game synchronizer
     * @param parent Parent QObject
     */
    explicit GameSynchronizer(QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~GameSynchronizer() override;

    // ==================== State Management ====================

    /**
     * @brief Set local game state
     * @param state Current game state
     */
    void setLocalState(const GameStateMessage& state);

    /**
     * @brief Get local game state
     * @return Current local state
     */
    GameStateMessage getLocalState() const { return localState_; }

    /**
     * @brief Check if local and remote states match
     * @param remoteState Remote state to compare
     * @return true if states are consistent
     */
    bool verifyState(const GameStateMessage& remoteState);

    /**
     * @brief Request state synchronization
     * @param target Target player to sync with
     */
    void requestSync(const QString& target);

    /**
     * @brief Handle sync request
     * @param requester Requesting player
     */
    void handleSyncRequest(const QString& requester);

    // ==================== Move Processing ====================

    /**
     * @brief Apply a move to local state
     * @param row Row (0-7)
     * @param col Column (0-7)
     * @param player Player who made the move
     * @return true if move was valid and applied
     */
    bool applyMove(int row, int col, const QString& player);

    /**
     * @brief Apply a move message
     * @param move Move message
     * @return true if move was valid
     */
    bool applyMove(const MoveMessage& move);

    /**
     * @brief Get available moves for current player
     * @return List of valid moves (row, col pairs)
     */
    QVector<QPair<int, int>> getAvailableMoves();

    /**
     * @brief Check if a move is valid
     * @param row Row
     * @param col Column
     * @param player Player
     * @return true if move is valid
     */
    bool isValidMove(int row, int col, const QString& player);

    // ==================== Hash Verification ====================

    /**
     * @brief Calculate board hash
     * @param board Board to hash
     * @return Hash value
     * 
     * Uses Zobrist hashing for efficient comparison.
     */
    uint64_t calculateBoardHash(const QVector<QVector<int>>& board);

    /**
     * @brief Calculate local state hash
     * @return Hash value
     */
    uint64_t calculateStateHash();

    // ==================== Coordinate Conversion ====================

    /**
     * @brief Convert GTP coordinate to board position
     * @param coord GTP coordinate (e.g., "D5", "A1", "H8")
     * @param row Output row (0-7)
     * @param col Output col (0-7)
     * @return true if conversion successful
     * 
     * Reference: Egaroucid gtp_command.hpp gtp_idx_to_coord() (line 78-83)
     */
    static bool gtpToCoord(const QString& coord, int& row, int& col);

    /**
     * @brief Convert board position to GTP coordinate
     * @param row Row (0-7)
     * @param col Column (0-7)
     * @return GTP coordinate string
     * 
     * Reference: Egaroucid gtp_command.hpp inverse of gtp_idx_to_coord()
     */
    static QString coordToGTP(int row, int col);

    /**
     * @brief Parse GTP move string
     * @param gtpMove GTP move string (e.g., "play black D5")
     * @param player Output player color
     * @param row Output row
     * @param col Output col
     * @return true if parsing successful
     */
    static bool parseGTPMove(const QString& gtpMove, QString& player, int& row, int& col);

    /**
     * @brief Generate GTP move string
     * @param player Player color
     * @param row Row
     * @param col Column
     * @return GTP move string
     */
    static QString generateGTPMove(const QString& player, int row, int col);

    // ==================== State Serialization ====================

    /**
     * @brief Serialize game state for network
     * @param state State to serialize
     * @return JSON representation
     */
    QJsonObject serializeState(const GameStateMessage& state);

    /**
     * @brief Deserialize game state from network
     * @param json JSON representation
     * @return Deserialized state
     */
    GameStateMessage deserializeState(const QJsonObject& json);

signals:
    /**
     * @brief Emitted when state is synchronized
     * @param state Synchronized state
     */
    void stateSynchronized(const GameStateMessage& state);

    /**
     * @brief Emitted when state mismatch detected
     * @param localHash Local state hash
     * @param remoteHash Remote state hash
     */
    void stateMismatch(uint64_t localHash, uint64_t remoteHash);

    /**
     * @brief Emitted when move is applied
     * @param row Row
     * @param col Column
     * @param player Player
     */
    void moveApplied(int row, int col, const QString& player);

    /**
     * @brief Emitted when invalid move received
     * @param row Row
     * @param col Column
     * @param player Player
     * @param reason Reason for rejection
     */
    void invalidMove(int row, int col, const QString& player, const QString& reason);

    /**
     * @brief Emitted when synchronization requested
     * @param requester Requesting player
     */
    void syncRequested(const QString& requester);

    /**
     * @brief Emitted when available moves change
     * @param moves List of available moves
     */
    void availableMovesChanged(const QVector<QPair<int, int>>& moves);

private:
    // ==================== State ====================
    GameStateMessage localState_;      ///< Local game state
    uint64_t localStateHash_;          ///< Local state hash for comparison

    // ==================== Zobrist Hashing ====================
    // Precomputed Zobrist random numbers for board positions
    static uint64_t zobristBlack_[8][8];
    static uint64_t zobristWhite_[8][8];
    static uint64_t zobristPlayer_[2];
    static bool zobristInitialized_;

    // ==================== Private Methods ====================

    /**
     * @brief Initialize Zobrist hash table
     */
    void initializeZobrist();

    /**
     * @brief Get piece representation from string
     * @param player Player string
     * @return Piece value (0=empty, 1=black, 2=white)
     */
    static int playerToPiece(const QString& player);

    /**
     * @brief Get player string from piece
     * @param piece Piece value
     * @return Player string
     */
    static QString pieceToPlayer(int piece);

    /**
     * @brief Check if coordinate is valid
     * @param row Row
     * @param col Column
     * @return true if valid
     */
    static bool isValidCoord(int row, int col);

    /**
     * @brief Count pieces in a direction
     * @param board Board to check
     * @param row Start row
     * @param col Start column
     * @param dr Delta row
     * @param dc Delta column
     * @param player Player to check for
     * @return Number of pieces that would be flipped
     */
    int countDirection(const QVector<QVector<int>>& board, int row, int col, int dr, int dc, int player);

    /**
     * @brief Flip pieces in a direction
     * @param board Board to modify
     * @param row Start row
     * @param col Start column
     * @param dr Delta row
     * @param dc Delta column
     * @param player Player to set
     */
    void flipDirection(QVector<QVector<int>>& board, int row, int col, int dr, int dc, int player);

    /**
     * @brief Update black and white piece counts
     */
    void updatePieceCounts();
};

/**
 * @brief Sync Protocol Messages
 * 
 * Reference: Egaroucid gtp_command.hpp command format
 */
struct SyncProtocol {
    static constexpr const char* REQUEST_SYNC = "sync_request";
    static constexpr const char* RESPONSE_SYNC = "sync_response";
    static constexpr const char* STATE_UPDATE = "state_update";
    static constexpr const char* HASH_MISMATCH = "hash_mismatch";
    static constexpr const char* MOVE_VALID = "move_valid";
    static constexpr const char* MOVE_INVALID = "move_invalid";
};

} // namespace Network

#endif // GAME_SYNCHRONIZER_HPP

