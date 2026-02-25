/**
 * @file HistoryTableTest.cpp
 * @brief History Table 单元测试
 */

#include <gtest/gtest.h>
#include "ai/HistoryTable.h"

namespace Reversi {

class HistoryTableTest : public ::testing::Test {
protected:
    void SetUp() override {
        ht_ = std::make_unique<HistoryTable>();
    }
    void TearDown() override {
        ht_.reset();
    }
    std::unique_ptr<HistoryTable> ht_;
};

// 基础测试
TEST_F(HistoryTableTest, Initialization) {
    int score = ht_->getHistoryScore(27, 35);
    EXPECT_EQ(score, 0);
}

TEST_F(HistoryTableTest, AddHistory) {
    ht_->addHistory(27, 35, 4, true);
    int score = ht_->getHistoryScore(27, 35);
    EXPECT_GT(score, 0);
}

TEST_F(HistoryTableTest, DepthWeighting) {
    ht_->addHistory(27, 35, 2, true);
    ht_->addHistory(27, 35, 4, true);
    int score = ht_->getHistoryScore(27, 35);
    // 深度4应该比深度2权重更高
    EXPECT_GT(score, 2 * 2 * 2);  // 至少大于2^2=4
}

TEST_F(HistoryTableTest, MultipleMoves) {
    ht_->addHistory(27, 35, 4, true);
    ht_->addHistory(28, 36, 4, true);
    EXPECT_GT(ht_->getHistoryScore(27, 35), 0);
    EXPECT_GT(ht_->getHistoryScore(28, 36), 0);
}

TEST_F(HistoryTableTest, NoCutoffNoRecord) {
    ht_->addHistory(27, 35, 4, false);  // 不导致剪枝
    int score = ht_->getHistoryScore(27, 35);
    EXPECT_EQ(score, 0);
}

TEST_F(HistoryTableTest, Clear) {
    ht_->addHistory(27, 35, 4, true);
    ht_->clear();
    int score = ht_->getHistoryScore(27, 35);
    EXPECT_EQ(score, 0);
}

TEST_F(HistoryTableTest, Decay) {
    ht_->addHistory(27, 35, 4, true);
    ht_->decay(0.5);
    int score = ht_->getHistoryScore(27, 35);
    EXPECT_LE(score, 8);  // 原来约16，衰减后约8
}

TEST_F(HistoryTableTest, SortedMoves) {
    ht_->addHistory(27, 35, 4, true);  // 高分
    ht_->addHistory(28, 36, 2, true);  // 低分
    std::vector<int> moves = {28 * 64 + 36, 27 * 64 + 35};
    auto sorted = ht_->getSortedMoves(moves);
    EXPECT_EQ(sorted[0], 27 * 64 + 35);  // 高分在前
}

TEST_F(HistoryTableTest, OutOfBounds) {
    ht_->addHistory(-1, 35, 4, true);
    ht_->addHistory(27, 100, 4, true);
    // 应该不崩溃
    EXPECT_EQ(ht_->getHistoryScore(-1, 35), 0);
    EXPECT_EQ(ht_->getHistoryScore(27, 100), 0);
}

TEST_F(HistoryTableTest, Statistics) {
    ht_->addHistory(27, 35, 4, true);
    ht_->addHistory(28, 36, 4, true);
    auto stats = ht_->getStatistics();
    EXPECT_EQ(stats.totalEntries, 2);
    EXPECT_GT(stats.maxScore, 0);
}

}  // namespace Reversi
