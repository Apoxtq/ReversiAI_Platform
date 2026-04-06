#include "Board.h"
#include <stdexcept>
#include <vector>

// 跨平台位操作支持
#if defined(_MSC_VER)
    #include <intrin.h>
    // MSVC内联函数实现CTZ (Count Trailing Zeros)
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
        // 只有在没有合法移动时才允许跳过
        uint64_t moves = bitboard_.getValidMoves(current_turn_);
        if (moves != 0) return false;
        current_turn_ = (current_turn_ == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
        move_count_++;
        return true;
    }

    // coordinateToBit helper removed; compute bit directly
    uint64_t move_bit = 1ULL << (move.row * 8 + move.col);
    uint64_t valid_moves = bitboard_.getValidMoves(current_turn_);
    if (!(valid_moves & move_bit)) {
        return false;
    }

    // 保存历史以便悔棋
    history_.push_back(bitboard_);

    bool ok = bitboard_.makeMove(move.row, move.col, current_turn_);
    if (ok) {
        current_turn_ = (current_turn_ == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
        move_count_++;
    } else {
        // 不应该出现：如果 move 在 valid_moves 中，则 makeMove 应成功
        // 还原历史
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
    // 将行列转换为位索引 (0-63)
    int pos = row * 8 + col;
    uint64_t blackMask = bitboard_.getPlayerBits();
    uint64_t whiteMask = bitboard_.getOpponentBits();

    uint64_t posMask = (uint64_t)1 << pos;

    if (blackMask & posMask) {
        return 2;  // 黑棋
    } else if (whiteMask & posMask) {
        return 1;  // 白棋
    }
    return 0;  // 空位
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

} // namespace Reversi


