#include "gtest/gtest.h"
#include "core/BitBoard.h"

using namespace Reversi;

TEST(BitBoardTest, InitialPosition) {
    BitBoard board;
    board.resetToStandardOpening();
    EXPECT_EQ(board.getScore(PlayerColor::Black), 2);
    EXPECT_EQ(board.getScore(PlayerColor::White), 2);

    uint64_t black_moves = board.getValidMoves(PlayerColor::Black);
    uint64_t white_moves = board.getValidMoves(PlayerColor::White);
    EXPECT_EQ(__builtin_popcountll(black_moves), 4);
    EXPECT_EQ(__builtin_popcountll(white_moves), 4);
}

TEST(BitBoardTest, MakeMoveFlips) {
    BitBoard board;
    board.resetToStandardOpening();

    uint64_t black_moves = board.getValidMoves(PlayerColor::Black);
    ASSERT_NE(black_moves, 0u);
    int first_pos = __builtin_ctzll(black_moves);
    int row = first_pos / 8;
    int col = first_pos % 8;

    bool ok = board.makeMove(row, col, PlayerColor::Black);
    EXPECT_TRUE(ok);
    EXPECT_EQ(board.getScore(PlayerColor::Black), 4);
    EXPECT_EQ(board.getScore(PlayerColor::White), 1);
}

TEST(BitBoardTest, CopyEquality) {
    BitBoard board;
    board.resetToStandardOpening();
    BitBoard copy = board.copy();
    EXPECT_TRUE(board == copy);
}


