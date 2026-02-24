#include "gtest/gtest.h"
#include "core/BitBoard.h"

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <bits/stdc++.h>
#endif

namespace Reversi {

// Helper function for popcount (cross-platform)
inline int popcount64(uint64_t x) {
#ifdef _MSC_VER
    return __popcnt64(x);
#else
    return __builtin_popcountll(x);
#endif
}

// Helper function for count trailing zeros (cross-platform)
inline int ctz64(uint64_t x) {
#ifdef _MSC_VER
    unsigned long index;
    _BitScanForward64(&index, x);
    return static_cast<int>(index);
#else
    return __builtin_ctzll(x);
#endif
}

TEST(BitBoardTest, InitialPosition) {
    BitBoard board;
    board.resetToStandardOpening();
    EXPECT_EQ(board.getScore(PlayerColor::Black), 2);
    EXPECT_EQ(board.getScore(PlayerColor::White), 2);

    uint64_t black_moves = board.getValidMoves(PlayerColor::Black);
    uint64_t white_moves = board.getValidMoves(PlayerColor::White);
    EXPECT_EQ(popcount64(black_moves), 4);
    EXPECT_EQ(popcount64(white_moves), 4);
}

TEST(BitBoardTest, MakeMoveFlips) {
    BitBoard board;
    board.resetToStandardOpening();

    uint64_t black_moves = board.getValidMoves(PlayerColor::Black);
    ASSERT_NE(black_moves, 0u);
    int first_pos = ctz64(black_moves);
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

}  // namespace Reversi


