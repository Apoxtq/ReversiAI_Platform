/**
 * @file MoveOrdererTest.cpp
 * @brief Move Orderer 单元测试
 * v1.0.0: 添加MoveOrderer单元测试提升覆盖率
 */

#include <gtest/gtest.h>
#include "ai/MoveOrderer.h"
#include "ai/Evaluator.h"
#include "Board.h"

namespace Reversi {

class MoveOrdererTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_.useKillerMoves = true;
        config_.useHistoryHeuristic = true;
        config_.useMobilityOrdering = true;
        config_.killerWeight = 8;
        config_.historyWeight = 6;
        config_.mobilityWeight = 35;
        config_.decayFactor = 0.99;
        
        orderer_ = std::make_unique<MoveOrderer>(config_);
        board_ = std::make_unique<Board>();
    }
    
    void TearDown() override {
        orderer_.reset();
        board_.reset();
    }
    
    MoveOrdererConfig config_;
    std::unique_ptr<MoveOrderer> orderer_;
    std::unique_ptr<Board> board_;
};

// 基础测试 - 初始化
TEST_F(MoveOrdererTest, Initialization) {
    auto stats = orderer_->getStatistics();
    EXPECT_EQ(stats.totalMoves, 0);
    EXPECT_EQ(stats.killerHits, 0);
    EXPECT_EQ(stats.historyHits, 0);
}

// 测试空列表排序
TEST_F(MoveOrdererTest, OrderEmptyMoves) {
    std::vector<Move> moves;
    auto result = orderer_->orderMoves(*board_, 5, moves);
    EXPECT_EQ(result.size(), 0);
}

// 测试单走法排序
TEST_F(MoveOrdererTest, OrderSingleMove) {
    // 获取当前合法走法
    auto validMoves = board_->getValidMoves();
    ASSERT_GT(validMoves.size(), 0);
    
    auto result = orderer_->orderMoves(*board_, 5, validMoves);
    EXPECT_EQ(result.size(), validMoves.size());
}

// 测试PV走法优先级
TEST_F(MoveOrdererTest, PvMovePriority) {
    auto validMoves = board_->getValidMoves();
    if (validMoves.size() >= 2) {
        // 使用第一个合法走法作为PV走法
        Move pvMove = validMoves[0];
        auto result = orderer_->orderMoves(*board_, 5, validMoves, pvMove);
        
        // PV走法应该排在最前面
        EXPECT_EQ(result[0], pvMove);
    }
}

// 测试Killer走法记录
TEST_F(MoveOrdererTest, RecordKiller) {
    orderer_->addKiller(5, 27);
    auto& killerTable = orderer_->getKillerTable();
    EXPECT_TRUE(killerTable.isKiller(5, 27));
}

// 测试History走法记录
TEST_F(MoveOrdererTest, RecordHistory) {
    orderer_->addHistory(27, 35, 5);
    auto& historyTable = orderer_->getHistoryTable();
    EXPECT_GT(historyTable.getHistoryScore(27, 35), 0);
}

// 测试记录Cutoff
TEST_F(MoveOrdererTest, RecordCutoff) {
    orderer_->recordCutoff(27, 35, 5, true);
    
    auto stats = orderer_->getStatistics();
    EXPECT_EQ(stats.killerHits, 1);
    EXPECT_EQ(stats.historyHits, 1);
}

// 测试无Cutoff不记录
TEST_F(MoveOrdererTest, NoCutoffNoRecord) {
    orderer_->recordCutoff(27, 35, 5, false);
    
    auto stats = orderer_->getStatistics();
    EXPECT_EQ(stats.killerHits, 0);
    EXPECT_EQ(stats.historyHits, 0);
}

// 测试Clear功能
TEST_F(MoveOrdererTest, Clear) {
    orderer_->addKiller(5, 27);
    orderer_->addHistory(27, 35, 5);
    orderer_->clear();
    
    auto& killerTable = orderer_->getKillerTable();
    auto killers = killerTable.getKillers(5);
    EXPECT_EQ(killers.size(), 0);
    
    auto& historyTable = orderer_->getHistoryTable();
    EXPECT_EQ(historyTable.getHistoryScore(27, 35), 0);
}

// 测试Decay功能
TEST_F(MoveOrdererTest, Decay) {
    orderer_->addKiller(5, 27);
    orderer_->decay();
    // 不应该崩溃，测试通过
}

// 测试统计功能
TEST_F(MoveOrdererTest, Statistics) {
    auto validMoves = board_->getValidMoves();
    if (validMoves.size() >= 1) {
        // 设置PV走法来触发PV命中统计
        orderer_->orderMoves(*board_, 5, validMoves, validMoves[0]);
        
        auto stats = orderer_->getStatistics();
        EXPECT_GE(stats.totalMoves, validMoves.size());
    }
}

// 测试重置统计
TEST_F(MoveOrdererTest, ResetStatistics) {
    auto validMoves = board_->getValidMoves();
    if (validMoves.size() >= 1) {
        orderer_->orderMoves(*board_, 5, validMoves, validMoves[0]);
        orderer_->resetStatistics();
        
        auto stats = orderer_->getStatistics();
        EXPECT_EQ(stats.totalMoves, 0);
        EXPECT_EQ(stats.killerHits, 0);
        EXPECT_EQ(stats.historyHits, 0);
    }
}

// 测试配置获取和设置
TEST_F(MoveOrdererTest, ConfigAccess) {
    auto config = orderer_->getConfig();
    EXPECT_EQ(config.useKillerMoves, true);
    EXPECT_EQ(config.useHistoryHeuristic, true);
    
    MoveOrdererConfig newConfig;
    newConfig.useKillerMoves = false;
    orderer_->setConfig(newConfig);
    
    config = orderer_->getConfig();
    EXPECT_EQ(config.useKillerMoves, false);
}

// 测试静态排序
TEST_F(MoveOrdererTest, OrderMovesStatic) {
    auto validMoves = board_->getValidMoves();
    if (validMoves.size() >= 2) {
        Move pvMove = validMoves[0];
        auto result = orderer_->orderMovesStatic(5, validMoves, pvMove);
        EXPECT_EQ(result.size(), validMoves.size());
    }
}

// 测试多深度走法排序
TEST_F(MoveOrdererTest, MultiDepthOrdering) {
    auto validMoves = board_->getValidMoves();
    if (validMoves.size() >= 1) {
        // 在不同深度添加killer
        orderer_->addKiller(3, validMoves[0].row * 8 + validMoves[0].col);
        orderer_->addKiller(5, validMoves[0].row * 8 + validMoves[0].col);
        
        auto result = orderer_->orderMoves(*board_, 5, validMoves);
        EXPECT_EQ(result.size(), validMoves.size());
    }
}

// 测试Killer得分获取
TEST_F(MoveOrdererTest, GetKillerScore) {
    orderer_->addKiller(5, 27);
    auto& killerTable = orderer_->getKillerTable();
    int score = killerTable.getKillerScore(5, 27);
    EXPECT_GT(score, 0);
}

// 测试History表获取
TEST_F(MoveOrdererTest, GetHistoryTable) {
    orderer_->addHistory(27, 35, 5);
    auto& historyTable = orderer_->getHistoryTable();
    int score = historyTable.getHistoryScore(27, 35);
    EXPECT_GT(score, 0);
}

// 测试灵活度排序启用
TEST_F(MoveOrdererTest, MobilityOrderingEnabled) {
    MoveOrdererConfig config;
    config.useMobilityOrdering = true;
    auto orderer = std::make_unique<MoveOrderer>(config);
    
    auto validMoves = board_->getValidMoves();
    if (validMoves.size() >= 1) {
        auto result = orderer->orderMoves(*board_, 5, validMoves);
        EXPECT_EQ(result.size(), validMoves.size());
    }
}

// 测试灵活度排序禁用
TEST_F(MoveOrdererTest, MobilityOrderingDisabled) {
    MoveOrdererConfig config;
    config.useMobilityOrdering = false;
    auto orderer = std::make_unique<MoveOrderer>(config);
    
    auto validMoves = board_->getValidMoves();
    if (validMoves.size() >= 1) {
        auto result = orderer->orderMoves(*board_, 5, validMoves);
        EXPECT_EQ(result.size(), validMoves.size());
    }
}

// 测试杀手法禁用
TEST_F(MoveOrdererTest, KillerDisabled) {
    MoveOrdererConfig config;
    config.useKillerMoves = false;
    auto orderer = std::make_unique<MoveOrderer>(config);
    
    auto validMoves = board_->getValidMoves();
    if (validMoves.size() >= 1) {
        orderer->addKiller(5, 27);  // 添加killer但禁用
        auto result = orderer->orderMoves(*board_, 5, validMoves);
        EXPECT_EQ(result.size(), validMoves.size());
    }
}

// 测试历史启发禁用
TEST_F(MoveOrdererTest, HistoryDisabled) {
    MoveOrdererConfig config;
    config.useHistoryHeuristic = false;
    auto orderer = std::make_unique<MoveOrderer>(config);
    
    auto validMoves = board_->getValidMoves();
    if (validMoves.size() >= 1) {
        orderer->addHistory(27, 35, 5);  // 添加history但禁用
        auto result = orderer->orderMoves(*board_, 5, validMoves);
        EXPECT_EQ(result.size(), validMoves.size());
    }
}

}  // namespace Reversi
