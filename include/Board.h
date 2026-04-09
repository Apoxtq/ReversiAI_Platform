#pragma once

#include "core/BitBoard.h"
#include <vector>
#include <optional>

namespace Reversi {

class Board {
public:
    Board();
    bool makeMove(const Move& move);
    bool undoMove();
    std::vector<Move> getValidMoves() const;
    bool isGameOver() const;
    std::optional<PlayerColor> getWinner() const;
    const BitBoard& getBitBoard() const { return bitboard_; }
    PlayerColor getCurrentTurn() const { return current_turn_; }
    int getMoveCount() const { return move_count_; }
    int at(int row, int col) const;
    void syncFrom(const std::vector<std::vector<int>>& state,
                  PlayerColor nextPlayer,
                  int moveCount);
    void setCell(int row, int col, int value);
    void swapColors();

private:
    friend class GameController;

private:
    BitBoard bitboard_;
    PlayerColor current_turn_ = PlayerColor::Black;
    int move_count_ = 0;
    std::vector<BitBoard> history_;
};

} // namespace Reversi