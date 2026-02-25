/**
 * @file TranspositionTableTest.cpp
 * @brief 转置表单元测试
 *
 * 测试TranspositionTable类的功能:
 * - 存储和查找
 * - 命中率统计
 * - 深度验证
 * - 类型验证
 */

#include <gtest/gtest.h>
#include "ai/TranspositionTable.h"
#include "ai/ZobristHash.h"
#include "core/BitBoard.h"

namespace Reversi {

class TranspositionTableTest : public ::testing::Test {
protected:
    void SetUp() override {
        ZobristHash::init(25);
        tt_ = std::make_unique<TranspositionTable>(64);  // 64MB
    }

    void TearDown() override {
        tt_.reset();
        ZobristHash::shutdown();
    }

    std::unique_ptr<TranspositionTable> tt_;
};

// ============================================================================
// 基础测试
// ============================================================================

TEST_F(TranspositionTableTest, Initialization) {
    EXPECT_TRUE(tt_->getSize() > 0);
    EXPECT_EQ(tt_->getHits(), 0u);
    EXPECT_EQ(tt_->getLookups(), 0u);
    EXPECT_EQ(tt_->getHitRate(), 0.0);
}

TEST_F(TranspositionTableTest, MemoryUsage) {
    size_t memory = tt_->getMemoryUsage();
    // 64MB转置表
    EXPECT_GE(memory, 64 * 1024 * 1024);
}

// ============================================================================
// 存储和查找测试
// ============================================================================

TEST_F(TranspositionTableTest, StoreAndProbe) {
    BitBoard board;
    board.resetToStandardOpening();

    uint32_t hash = ZobristHash::computeHash(
        board.getPlayerBits(),
        board.getOpponentBits()
    );

    Move best_move(3, 3);
    tt_->store(hash, 6, 10, -20, 20, best_move);

    // 查找
    int alpha = -20, beta = 20, score = 0;
    Move found_move;

    bool found = tt_->probe(hash, 6, alpha, beta, score, found_move);

    EXPECT_TRUE(found);
    EXPECT_EQ(score, 10);
    EXPECT_EQ(found_move.row, best_move.row);
    EXPECT_EQ(found_move.col, best_move.col);
}

TEST_F(TranspositionTableTest, DepthMismatch) {
    BitBoard board;
    board.resetToStandardOpening();

    uint32_t hash = ZobristHash::computeHash(
        board.getPlayerBits(),
        board.getOpponentBits()
    );

    // 存储深度6的结果
    tt_->store(hash, 6, 10, -20, 20, Move(3, 3));

    // 查找深度10（应该找不到，因为存储的深度不足）
    int alpha = -20, beta = 20, score = 0;
    Move found_move;

    bool found = tt_->probe(hash, 10, alpha, beta, score, found_move);

    EXPECT_FALSE(found);
}

TEST_F(TranspositionTableTest, DifferentHashNoMatch) {
    BitBoard board1;
    board1.resetToStandardOpening();

    // 空棋盘 - 使用默认构造函数
    BitBoard board2;

    uint32_t hash1 = ZobristHash::computeHash(
        board1.getPlayerBits(),
        board1.getOpponentBits()
    );

    uint32_t hash2 = ZobristHash::computeHash(
        board2.getPlayerBits(),
        board2.getOpponentBits()
    );

    // 存储hash1
    tt_->store(hash1, 6, 10, -20, 20, Move(3, 3));

    // 查找hash2（应该找不到）
    int alpha = -20, beta = 20, score = 0;
    Move found_move;

    bool found = tt_->probe(hash2, 6, alpha, beta, score, found_move);

    EXPECT_FALSE(found);
}

// ============================================================================
// 命中率测试
// ============================================================================

TEST_F(TranspositionTableTest, HitRate) {
    BitBoard board;
    board.resetToStandardOpening();

    uint32_t hash = ZobristHash::computeHash(
        board.getPlayerBits(),
        board.getOpponentBits()
    );

    // 多次存储和查找
    for (int i = 0; i < 10; ++i) {
        tt_->store(hash, 6, i * 10, -20, 20, Move(3, 3));
        int alpha = -20, beta = 20, score = 0;
        Move found_move;
        tt_->probe(hash, 6, alpha, beta, score, found_move);
    }

    // 命中率应该很高
    EXPECT_GE(tt_->getHitRate(), 0.9);
}

// ============================================================================
// 清空测试
// ============================================================================

TEST_F(TranspositionTableTest, Clear) {
    BitBoard board;
    board.resetToStandardOpening();

    uint32_t hash = ZobristHash::computeHash(
        board.getPlayerBits(),
        board.getOpponentBits()
    );

    tt_->store(hash, 6, 10, -20, 20, Move(3, 3));

    // 清空
    tt_->clear();

    // 查找应该失败
    int alpha = -20, beta = 20, score = 0;
    Move found_move;
    bool found = tt_->probe(hash, 6, alpha, beta, score, found_move);

    EXPECT_FALSE(found);
    // 注意: probe会增加lookups_计数，所以即使clear后，第一次probe也会使lookups变为1
    // 这是一个设计选择：统计probe调用次数而不是成功查找次数
    EXPECT_EQ(tt_->getHits(), 0u);
}

// ============================================================================
// Alpha/Beta边界测试
// ============================================================================

TEST_F(TranspositionTableTest, AlphaBetaBounds) {
    BitBoard board;
    board.resetToStandardOpening();

    uint32_t hash = ZobristHash::computeHash(
        board.getPlayerBits(),
        board.getOpponentBits()
    );

    // 测试EXACT类型
    tt_->store(hash, 6, 10, -20, 20, Move(3, 3));

    int alpha = -20, beta = 20, score = 0;
    Move found_move;
    bool found = tt_->probe(hash, 6, alpha, beta, score, found_move);

    EXPECT_TRUE(found);
    EXPECT_EQ(score, 10);
}

}  // namespace Reversi

