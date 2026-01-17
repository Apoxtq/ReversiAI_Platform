#pragma once

#include "core/BitBoard.h"
#include <vector>
#include <optional>

namespace Reversi {

class Board {
public:
    Board();

    // 执行移动，返回是否成功
    bool makeMove(const Move& move);

    // 悔棋：恢复到上一个状态，返回是否成功
    bool undoMove();

    // 获取当前玩家的有效移动列表
    std::vector<Move> getValidMoves() const;

    // 游戏是否结束
    bool isGameOver() const;

    // 获取胜者
    std::optional<PlayerColor> getWinner() const;

    // 获取底层 BitBoard（只读）
    const BitBoard& getBitBoard() const { return bitboard_; }

    // 获取当前回合
    PlayerColor getCurrentTurn() const { return current_turn_; }

    int getMoveCount() const { return move_count_; }

private:
    BitBoard bitboard_;
    PlayerColor current_turn_ = PlayerColor::Black;
    int move_count_ = 0;
    std::vector<BitBoard> history_;  // 用于悔棋
};

} // namespace Reversi


