/**
 * @file BattleEngineTest.cpp
 * @brief 对战引擎单元测试
 *
 * 测试BattleEngine类的功能:
 * - 单局对战
 * - 批量对战
 * - 统计计算
 * - 结果导出
 */

#include <gtest/gtest.h>
#include "research/BattleEngine.h"
#include "ai/AIStrategy.h"
#include "ai/RandomAI.h"

namespace Reversi {

class BattleEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建AI实例
        ai1_ = AIStrategyFactory::createRandomAI();
        ai2_ = AIStrategyFactory::createRandomAI();
    }

    std::unique_ptr<AIStrategy> ai1_;
    std::unique_ptr<AIStrategy> ai2_;
};

// ============================================================================
// 单局对战测试
// ============================================================================

TEST_F(BattleEngineTest, PlaySingleGame) {
    SearchLimits limits;
    limits.maxDepth = 2;

    GameResult result = BattleEngine::playSingleGame(
        *ai1_, *ai2_,
        PlayerColor::Black,
        limits, limits
    );

    // 检查结果
    EXPECT_GE(result.black_score, 0);
    EXPECT_GE(result.white_score, 0);
    EXPECT_GE(result.moves_count, 0);
    EXPECT_GE(result.duration_ms, 0.0);
    // 游戏应该有结果（黑棋赢、白棋赢或平局）
    EXPECT_TRUE(result.winner == PlayerColor::Black ||
                result.winner == PlayerColor::White ||
                result.isDraw());
}

TEST_F(BattleEngineTest, PlayMultipleGames) {
    SearchLimits limits;
    limits.maxDepth = 2;

    int black_wins = 0, white_wins = 0, draws = 0;

    for (int i = 0; i < 10; ++i) {
        GameResult result = BattleEngine::playSingleGame(
            *ai1_, *ai2_,
            PlayerColor::Black,
            limits, limits
        );

        if (result.isDraw()) draws++;
        else if (result.winner == PlayerColor::Black) black_wins++;
        else white_wins++;

        // 重置AI状态
        ai1_->reset();
        ai2_->reset();
    }

    // 10局游戏应该有一定分布
    EXPECT_EQ(black_wins + white_wins + draws, 10);
}

// ============================================================================
// 批量对战测试
// ============================================================================

TEST_F(BattleEngineTest, RunBattle) {
    BattleConfig config;
    config.player1 = AIStrategyFactory::createRandomAI();
    config.player2 = AIStrategyFactory::createRandomAI();
    config.player1_name = "Random1";
    config.player2_name = "Random2";
    config.num_games = 20;
    config.limits1.maxDepth = 2;
    config.limits2.maxDepth = 2;
    config.verbose = false;

    BattleStats stats = BattleEngine::runBattle(config);

    // 验证统计
    EXPECT_EQ(stats.total_games, 20);
    EXPECT_EQ(stats.player1_wins + stats.player2_wins + stats.draws, 20);
    EXPECT_GE(stats.win_rate1, 0.0);
    EXPECT_LE(stats.win_rate1, 1.0);
    EXPECT_GE(stats.avg_moves, 0.0);
}

TEST_F(BattleEngineTest, BattleStatsCalculation) {
    BattleStats stats;
    stats.total_games = 100;
    stats.player1_wins = 60;
    stats.player2_wins = 30;
    stats.draws = 10;

    // 手动添加一些游戏结果
    for (int i = 0; i < 60; ++i) {
        GameResult game;
        game.game_number = i + 1;
        game.winner = PlayerColor::Black;
        game.black_score = 40 + (i % 10);
        game.white_score = 24 + (i % 10);
        game.moves_count = 40 + (i % 20);
        game.duration_ms = 100.0;
        stats.games.push_back(game);
    }

    stats.calculate();

    // 验证计算
    EXPECT_DOUBLE_EQ(stats.win_rate1, 0.6);
    EXPECT_DOUBLE_EQ(stats.win_rate2, 0.3);
    EXPECT_DOUBLE_EQ(stats.avg_margin, 16.0);
}

// ============================================================================
// 配置验证测试
// ============================================================================

TEST_F(BattleEngineTest, ValidateConfig) {
    // 有效配置
    BattleConfig valid_config;
    valid_config.player1 = AIStrategyFactory::createRandomAI();
    valid_config.player2 = AIStrategyFactory::createRandomAI();
    valid_config.num_games = 10;

    EXPECT_TRUE(BattleEngine::validateConfig(valid_config));

    // 无效配置：没有AI
    BattleConfig invalid_config;
    invalid_config.num_games = 10;

    EXPECT_FALSE(BattleEngine::validateConfig(invalid_config));

    // 无效配置：游戏数为0
    BattleConfig invalid_config2;
    invalid_config2.player1 = AIStrategyFactory::createRandomAI();
    invalid_config2.player2 = AIStrategyFactory::createRandomAI();
    invalid_config2.num_games = 0;

    EXPECT_FALSE(BattleEngine::validateConfig(invalid_config2));
}

// ============================================================================
// Random AI基准测试
// ============================================================================

TEST_F(BattleEngineTest, RandomAIvsRandomAI) {
    // 随机AI之间的对战应该接近50%胜率
    auto ai1 = AIStrategyFactory::createRandomAI();
    auto ai2 = AIStrategyFactory::createRandomAI();

    BattleConfig config;
    config.player1 = std::move(ai1);
    config.player2 = std::move(ai2);
    config.player1_name = "Random1";
    config.player2_name = "Random2";
    config.num_games = 50;
    config.limits1.maxDepth = 1;  // 快速测试
    config.limits2.maxDepth = 1;
    config.verbose = false;

    BattleStats stats = BattleEngine::runBattle(config);

    // 50局游戏，胜率应该在20%-80%之间（考虑随机性）
    EXPECT_GE(stats.win_rate1, 0.2);
    EXPECT_LE(stats.win_rate1, 0.8);
    // 平均回合数应该合理
    EXPECT_GE(stats.avg_moves, 20.0);
    EXPECT_LE(stats.avg_moves, 64.0);
}

// ============================================================================
// 统计显著性测试
// ============================================================================

TEST_F(BattleEngineTest, StatisticalSignificance) {
    // 创建两个确定性的AI
    auto ai1 = AIStrategyFactory::createRandomAI();
    auto ai2 = AIStrategyFactory::createRandomAI();

    BattleConfig config;
    config.player1 = std::move(ai1);
    config.player2 = std::move(ai2);
    config.player1_name = "AI1";
    config.player2_name = "AI2";
    config.num_games = 100;
    config.limits1.maxDepth = 2;
    config.limits2.maxDepth = 2;
    config.verbose = false;

    BattleStats stats = BattleEngine::runBattle(config);

    // 应该有统计信息
    EXPECT_GE(stats.total_games, 10);
    // p值应该被计算
    EXPECT_GE(stats.p_value, 0.0);
    EXPECT_LE(stats.p_value, 1.0);
}

}  // namespace Reversi

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

