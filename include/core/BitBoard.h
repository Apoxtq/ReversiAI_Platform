#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

/**
 * @file BitBoard.h
 * @brief BitBoard core data structure
 *
 * Uses uint64_t bitmaps to represent 8x8 Othello board, providing efficient
 * move generation and flip operations.
 *
 * Learning sources:
 * - Egaroucid: https://github.com/Nyanyan/Egaroucid
 *   * Board class design pattern
 *   * uint64_t bitmap representation
 *   * Bit operation optimization strategy
 *
 * - edax-reversi: https://github.com/abulmo/edax-reversi
 *   * Classic bit operation move generation
 *   * Flip logic implementation
 *   * Functional interface design
 *
 * - Reversi (Java): https://github.com/abulmo/Reversi
 *   * Clean OOP design
 *   * Complete game state management
 *
 * @academic
 * This implementation integrates the best features of multiple mature solutions,
 * verified through performance testing, with O(1) move generation and flip operations,
 * providing an efficient foundation for AI algorithm research.
 *
 * @author AI Assistant (based on open source projects)
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @enum PlayerColor
 * @brief Player color enum
 */
enum class PlayerColor {
    Black = 0,  ///< Black pieces
    White = 1   ///< White pieces
};

/**
 * @brief BitBoard core class
 *
 * Uses uint64_t bitmap to represent 8x8 Othello board, providing efficient game operations.
 * Black pieces bitmap is stored in player_pieces_, white pieces bitmap is in opponent_pieces_.
 *
 * Bitmap layout:
 * Board coordinate (row, col) maps to bit position: row * 8 + col
 * e.g.: (0,0) is bit 0, (7,7) is bit 63
 *
 * @invariant Black and white bitmaps do not overlap: (player_pieces_ & opponent_pieces_) == 0
 * @invariant All bits are within 64-bit range
 */
class BitBoard {
public:
    /**
     * @brief Default constructor, creates empty board
     *
     * @complexity O(1)
     * @academic Empty board initialization for testing and custom board setup
     */
    BitBoard();

    /**
     * @brief Create board with specified bitmaps
     *
     * @param player_bits Black player bitmap
     * @param opponent_bits White player bitmap
     *
     * @complexity O(1)
     * @academic Direct bitmap construction for algorithm testing and custom positions
     * @reference Egaroucid Board(uint64_t, uint64_t) constructor
     */
    BitBoard(uint64_t player_bits, uint64_t opponent_bits);

    /**
     * @brief Create board from string
     *
     * String format: 8x8 character matrix, 'B' = black, 'W' = white, '.' = empty
     * e.g.: "........B......WB..............................."
     *
     * @param board_str Board string representation
     *
     * @complexity O(1)
     * @academic Human-readable format for test cases and debugging
     */
    explicit BitBoard(const std::string& board_str);

    /**
     * @brief Destructor
     *
     * @complexity O(1)
     */
    ~BitBoard() = default;

    // Core game operations

    /**
     * @brief Get valid moves bitmap for specified color
     *
     * Computes all valid positions the specified color player can move.
     * Uses bit operations for efficient move generation.
     *
     * @param color Player color
     * @return uint64_t bitmap of valid moves
     *
     * @complexity O(1) - bit operation optimized
     * @academic Based on edax-reversi bit operation move generation algorithm
     * @reference edax-reversi/src/move.c get_moves function
     */
    uint64_t getValidMoves(PlayerColor color) const;

    /**
     * @brief Execute move
     *
     * Places a piece at the specified position and flips all captured pieces.
     *
     * @param row Row coordinate (0-7)
     * @param col Column coordinate (0-7)
     * @param color Player color
     * @return true if move is valid and has been executed
     *
     * @complexity O(1) - bit operation optimized
     * @academic Complete game rule validation and flip logic
     * @reference edax-reversi/src/flip.c flip function
     */
    bool makeMove(int row, int col, PlayerColor color);

    /**
     * @brief Check if game is over
     *
     * Game over condition: both players have no valid moves
     *
     * @return true if game is over
     *
     * @complexity O(1)
     * @academic Endgame detection based on bit operations
     */
    bool isGameOver() const;

    /**
     * @brief Get piece count for specified color
     *
     * @param color Player color
     * @return Number of pieces of that color
     *
     * @complexity O(1) - uses __builtin_popcountll
     * @academic GCC built-in function optimization, hardware-accelerated bit counting
     */
    int getScore(PlayerColor color) const;

    /**
     * @brief Get empty square count
     *
     * @return Number of empty squares on the board
     *
     * @complexity O(1)
     * @academic Fast remaining space calculation for game progress judgment
     */
    int getEmptyCount() const;

    /**
     * @brief Get game winner
     *
     * @return Winner color, or std::nullopt if draw
     *
     * @complexity O(1)
     * @academic Win/loss judgment based on piece count
     */
    std::optional<PlayerColor> getWinner() const;

    // Board operations

    /**
     * @brief Copy board state
     *
     * @return New BitBoard instance with same state
     *
     * @complexity O(1)
     * @academic Supports immutable operations and backtracking
     * @reference Egaroucid Board::copy() method
     */
    BitBoard copy() const;

    /**
     * @brief Reset to standard opening
     *
     * Standard opening: Black at (3,3) and (4,4), White at (3,4) and (4,3)
     *
     * @complexity O(1)
     * @academic Standard Othello opening positions
     */
    void resetToStandardOpening();

    /**
     * @brief Clear board
     *
     * @complexity O(1)
     * @academic For testing and initialization
     */
    void clear();

    /**
     * @brief Reset to empty board (alias for clear)
     */
    void reset() { clear(); }

    /**
     * @brief Set piece at position
     * @param pos Position index (0-63)
     * @param isBlack true=black, false=white
     */
    void setBit(int pos, bool isBlack);

    // Bitmap direct access (for Board::swapColors)
    void setPlayerBits(uint64_t bits) { player_pieces_ = bits; }
    void setOpponentBits(uint64_t bits) { opponent_pieces_ = bits; }

    // Bitmap direct access (advanced users)

    /**
     * @brief Get black player bitmap
     *
     * @return uint64_t bitmap of black pieces
     *
     * @complexity O(1)
     * @academic Direct bitmap access for advanced algorithm implementation
     */
    uint64_t getPlayerBits() const { return player_pieces_; }

    /**
     * @brief Get white player bitmap
     *
     * @return uint64_t bitmap of white pieces
     *
     * @complexity O(1)
     * @academic Direct bitmap access for advanced algorithm implementation
     */
    uint64_t getOpponentBits() const { return opponent_pieces_; }

    /**
     * @brief Get all occupied bits
     *
     * @return uint64_t bitmap of all pieces (black | white)
     *
     * @complexity O(1)
     * @academic Fast occupied position retrieval for move generation
     */
    uint64_t getOccupiedBits() const { return player_pieces_ | opponent_pieces_; }

    /**
     * @brief Get empty bits
     *
     * @return uint64_t bitmap of empty positions
     *
     * @complexity O(1)
     * @academic Empty position calculation for move validation
     */
    uint64_t getEmptyBits() const { return ~(player_pieces_ | opponent_pieces_); }

    // Debug and display

    /**
     * @brief Convert to string representation
     *
     * @param current_player Current player (for hint display)
     * @return String representation of board
     *
     * @complexity O(1)
     * @academic Human-readable debug output for testing and development
     */
    std::string toString(PlayerColor current_player = PlayerColor::Black) const;

    /**
     * @brief Print board to console
     *
     * @param current_player Current player
     *
     * @complexity O(1)
     * @academic Debugging and demonstration purposes
     */
    void print(PlayerColor current_player = PlayerColor::Black) const;

    // Equality comparison

    /**
     * @brief Compare two boards for equality
     *
     * @param other Another BitBoard instance
     * @return true if both board states are identical
     *
     * @complexity O(1)
     * @academic Testing and state validation
     */
    bool operator==(const BitBoard& other) const;

    /**
     * @brief Compare two boards for inequality
     *
     * @param other Another BitBoard instance
     * @return true if board states differ
     *
     * @complexity O(1)
     */
    bool operator!=(const BitBoard& other) const;

private:
    // Bitmap storage: 64-bit representation of black and white pieces
    uint64_t player_pieces_;     ///< Black pieces bitmap (1 = black piece present)
    uint64_t opponent_pieces_;   ///< White pieces bitmap (1 = white piece present)

    // Private helper functions

    /**
     * @brief Parse board from string
     *
     * @param board_str Board string
     *
     * @complexity O(1)
     * @academic String to bitmap conversion
     */
    void fromString(const std::string& board_str);

    /**
     * @brief Calculate flip bitmap for single direction
     *
     * Helper function to compute pieces flipped in a given direction.
     *
     * @param position Bitmap of move position (single bit)
     * @param mask Direction mask
     * @param player_bits Current player bitmap
     * @param opponent_bits Opponent bitmap
     * @return Bitmap of flipped pieces in this direction
     *
     * @complexity O(1)
     * @academic Core flip algorithm, efficient implementation based on bit operations
     * @reference edax-reversi/src/flip.c flip function bit operation logic
     */
    static uint64_t flipInDirection(uint64_t position, uint64_t mask,
                                   uint64_t player_bits, uint64_t opponent_bits);

    /**
     * @brief Calculate flip bitmap for all directions
     *
     * @param position Move position (0-63)
     * @param player_bits Current player bitmap
     * @param opponent_bits Opponent bitmap
     * @return Bitmap of all flipped pieces in 8 directions
     *
     * @complexity O(1)
     * @academic Complete flip calculation, 8-direction bit operations
     */
    static uint64_t calculateFlips(int position, uint64_t player_bits, uint64_t opponent_bits);

    // Bit operation direction masks (precomputed constants)
    static constexpr uint64_t DIRECTION_MASKS[8] = {
        0x7F7F7F7F7F7F7F7FULL,  // Right shift
        0xFEFEFEFEFEFEFEFEULL,  // Left shift
        0x00FFFFFFFFFFFFFFULL,  // Down shift
        0xFFFFFFFFFFFFFF00ULL,  // Up shift
        0x7F7F7F7F7F7F7F00ULL,  // Down-right
        0x007F7F7F7F7F7F7FULL,  // Down-left
        0xFEFEFEFEFEFEFE00ULL,  // Up-right
        0x00FEFEFEFEFEFEFEULL   // Up-left
    };
};

/**
 * @brief Move structure
 *
 * Represents a move operation in Othello.
 */
struct Move {
    int row;        ///< Row coordinate (0-7)
    int col;        ///< Column coordinate (0-7)
    bool is_pass;   ///< Whether this is a pass turn

    /**
     * @brief Default constructor
     */
    Move() : row(-1), col(-1), is_pass(false) {}

    /**
     * @brief Construct move
     */
    Move(int r, int c, bool pass = false) : row(r), col(c), is_pass(pass) {}

    /**
     * @brief Create pass move
     */
    static Move pass() { return Move(-1, -1, true); }

    /**
     * @brief Check if move is valid
     */
    bool isValid() const {
        return is_pass || (row >= 0 && row < 8 && col >= 0 && col < 8);
    }

    /**
     * @brief Convert to string
     */
    std::string toString() const;

    /**
     * @brief Equality comparison
     */
    bool operator==(const Move& other) const {
        return row == other.row && col == other.col && is_pass == other.is_pass;
    }
};

} // namespace Reversi

// Convenience type aliases
using ReversiBitBoard = Reversi::BitBoard;
using ReversiPlayerColor = Reversi::PlayerColor;
using ReversiMove = Reversi::Move;

// Provide hash function support for Move struct to use as unordered_map key
namespace std {

template<>
struct hash<Reversi::Move> {
    size_t operator()(const Reversi::Move& move) const {
        // Simple hash: combine row, col and is_pass
        size_t h = 0;
        h = h * 31 + static_cast<size_t>(move.row + 1);      // +1 to handle -1
        h = h * 31 + static_cast<size_t>(move.col + 1);      // +1 to handle -1
        h = h * 31 + static_cast<size_t>(move.is_pass ? 1 : 0);
        return h;
    }
};

} // namespace std
