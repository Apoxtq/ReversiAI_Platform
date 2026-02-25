/**
 * @file KillerTableTest.cpp
 * @brief Killer Table 单元测试
 */

#include <gtest/gtest.h>
#include "ai/KillerTable.h"

namespace Reversi {

class KillerTableTest : public ::testing::Test {
protected:
    void SetUp() override {
        kt_ = std::make_unique<KillerTable>();
    }
    void TearDown() override {
        kt_.reset();
    }
    std::unique_ptr<KillerTable> kt_;
};

// 基础测试
TEST_F(KillerTableTest, Initialization) {
    auto killers = kt_->getKillers(5);
    EXPECT_EQ(killers.size(), 0);
}

TEST_F(KillerTableTest, AddKiller) {
    kt_->addKiller(5, 27, 1);
    auto killers = kt_->getKillers(5);
    EXPECT_EQ(killers.size(), 1);
    EXPECT_EQ(killers[0], 27);
}

TEST_F(KillerTableTest, AddMultipleKillers) {
    kt_->addKiller(5, 27, 1);
    kt_->addKiller(5, 35, 1);
    kt_->addKiller(5, 28, 1);
    auto killers = kt_->getKillers(5);
    EXPECT_EQ(killers.size(), 2);  // 最多2个
}

TEST_F(KillerTableTest, KillerAtDifferentDepths) {
    kt_->addKiller(3, 27, 1);
    kt_->addKiller(5, 35, 1);
    auto killers3 = kt_->getKillers(3);
    auto killers5 = kt_->getKillers(5);
    EXPECT_EQ(killers3.size(), 1);
    EXPECT_EQ(killers3[0], 27);
    EXPECT_EQ(killers5.size(), 1);
    EXPECT_EQ(killers5[0], 35);
}

TEST_F(KillerTableTest, ScoreAccumulation) {
    kt_->addKiller(5, 27, 1);
    kt_->addKiller(5, 27, 1);
    int score = kt_->getKillerScore(5, 27);
    EXPECT_GE(score, 2);
}

TEST_F(KillerTableTest, IsKiller) {
    kt_->addKiller(5, 27, 1);
    EXPECT_TRUE(kt_->isKiller(5, 27));
    EXPECT_FALSE(kt_->isKiller(5, 35));
    EXPECT_FALSE(kt_->isKiller(3, 27));
}

TEST_F(KillerTableTest, Clear) {
    kt_->addKiller(5, 27, 1);
    kt_->clear();
    auto killers = kt_->getKillers(5);
    EXPECT_EQ(killers.size(), 0);
}

TEST_F(KillerTableTest, Decay) {
    kt_->addKiller(5, 27, 100);
    kt_->decay(0.5);
    int score = kt_->getKillerScore(5, 27);
    EXPECT_LE(score, 50);
}

TEST_F(KillerTableTest, OutOfBounds) {
    kt_->addKiller(-1, 27, 1);  // 无效深度
    kt_->addKiller(5, -1, 1);   // 无效走法
    kt_->addKiller(5, 100, 1);  // 无效走法
    auto killers = kt_->getKillers(5);
    EXPECT_EQ(killers.size(), 0);
}

}  // namespace Reversi
