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

    /**
     * @brief 获取指定位置的棋子值
     * @param row 行 (0-7)
     * @param col 列 (0-7)
     * @return 0=空, 1=白棋, 2=黑棋
     */
    int at(int row, int col) const;

    /**
     * @brief 从外部状态同步棋盘（用于网络对战的客户端同步）
     * @param state 8x8 棋盘状态 (0=空, 1=白棋, 2=黑棋)
     * @param nextPlayer 下一个落子方
     * @param moveCount 已下步数
     */
    void syncFrom(const std::vector<std::vector<int>>& state,
                  PlayerColor nextPlayer,
                  int moveCount);

    /**
     * @brief 设置指定位置的棋子（用于外部状态同步）
     * @param row 行 (0-7)
     * @param col 列 (0-7)
     * @param value 0=空, 1=白棋, 2=黑棋
     */
    void setCell(int row, int col, int value);

private:
    friend class GameController;

private:
    BitBoard bitboard_;
    PlayerColor current_turn_ = PlayerColor::Black;
    int move_count_ = 0;
    std::vector<BitBoard> history_;  // 用于悔棋
};

} // namespace Reversi


