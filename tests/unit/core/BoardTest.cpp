#include "gtest/gtest.h"
#include "Board.h"

using namespace Reversi;

TEST(BoardTest, InitialAndValidMoves) {
    Board board;
    auto moves = board.getValidMoves();
    // 标准开局应至少有4个有效移动（黑先）
    EXPECT_GE(moves.size(), 4);
    EXPECT_FALSE(board.isGameOver());
}

TEST(BoardTest, MakeMoveAndUndo) {
    Board board;
    auto moves = board.getValidMoves();
    ASSERT_FALSE(moves.empty());
    Move m = moves[0];
    bool ok = board.makeMove(m);
    EXPECT_TRUE(ok);
    // 进行悔棋
    bool undone = board.undoMove();
    EXPECT_TRUE(undone);
}


