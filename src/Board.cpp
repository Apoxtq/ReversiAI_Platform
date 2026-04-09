#include "Board.h"
#include <algorithm>
#include <stdexcept>
#include <vector>

// Cross-platform bit operation support
#if defined(_MSC_VER)
    #include <intrin.h>
    inline int CTZ64(uint64_t x) {
        unsigned long index;
        _BitScanForward64(&index, x);
        return static_cast<int>(index);
    }
#else
    #define CTZ64(x) __builtin_ctzll(x)
#endif

namespace Reversi {

Board::Board() {
    bitboard_.resetToStandardOpening();
    current_turn_ = PlayerColor::Black;
    move_count_ = 0;
}

bool Board::makeMove(const Move& move) {
    if (!move.isValid()) {
        return false;
    }

    if (move.is_pass) {
        // Only allow pass when no valid moves exist
        uint64_t valid = bitboard_.getValidMoves(current_turn_);
        if (valid != 0) return false;
        current_turn_ = (current_turn_ == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
        move_count_++;
        return true;
    }

    uint64_t move_bit = 1ULL << (move.row * 8 + move.col);
    uint64_t valid_moves = bitboard_.getValidMoves(current_turn_);
    if (!(valid_moves & move_bit)) {
        return false;
    }

    // Save history for undo
    history_.push_back(bitboard_);

    bool ok = bitboard_.makeMove(move.row, move.col, current_turn_);
    if (ok) {
        current_turn_ = (current_turn_ == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
        move_count_++;
    } else {
        // Should not happen: if move is in valid_moves, makeMove should succeed
        // Restore history
        bitboard_ = history_.back();
        history_.pop_back();
    }
    return ok;
}

bool Board::undoMove() {
    if (history_.empty()) return false;
    bitboard_ = history_.back();
    history_.pop_back();
    move_count_ = std::max(0, move_count_ - 1);
    current_turn_ = (current_turn_ == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
    return true;
}

std::vector<Move> Board::getValidMoves() const {
    std::vector<Move> result;
    uint64_t moves = bitboard_.getValidMoves(current_turn_);
    while (moves) {
        uint64_t pos_bit = moves & -moves;
        int pos = CTZ64(pos_bit);
        int row = pos / 8;
        int col = pos % 8;
        result.emplace_back(row, col);
        moves &= moves - 1;
    }
    return result;
}

bool Board::isGameOver() const {
    return bitboard_.isGameOver();
}

std::optional<PlayerColor> Board::getWinner() const {
    return bitboard_.getWinner();
}

int Board::at(int row, int col) const {
    // Convert row/col to bit index (0-63)
    int pos = row * 8 + col;
    uint64_t blackMask = bitboard_.getPlayerBits();
    uint64_t whiteMask = bitboard_.getOpponentBits();

    uint64_t posMask = (uint64_t)1 << pos;

    if (blackMask & posMask) {
        return 2;  // Black
    } else if (whiteMask & posMask) {
        return 1;  // White
    }
    return 0;  // Empty
}

void Board::syncFrom(const std::vector<std::vector<int>>& state,
                     PlayerColor nextPlayer,
                     int moveCount) {
    bitboard_.reset();
    history_.clear();
    move_count_ = moveCount;
    current_turn_ = nextPlayer;

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            int val = state[row][col];
            if (val != 0) {
                int pos = row * 8 + col;
                bitboard_.setBit(pos, val == 2);
            }
        }
    }
}

void Board::setCell(int row, int col, int value) {
    if (value != 0) {
        int pos = row * 8 + col;
        bitboard_.setBit(pos, value == 2);
    }
}

void Board::swapColors() {
    // Swap player_pieces_ and opponent_pieces_
    uint64_t player_bits = bitboard_.getPlayerBits();
    uint64_t opponent_bits = bitboard_.getOpponentBits();
    // Swap internal bitmaps directly
    bitboard_.setPlayerBits(opponent_bits);
    bitboard_.setOpponentBits(player_bits);
    // Flip turn
    current_turn_ = (current_turn_ == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
}

} // namespace Reversi
