#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <atomic>
#include "../ai/AIStrategy.h"
#include "../Board.h"
#include "Statistics.h"
#include "BenchmarkTargets.h"
#include "PositionSuite.h"

/**
 * @file BattleEngine.h
 * @brief Head-to-Head对战引擎
 *
 * 批量AI对战测试，生成胜率统计。
 *
 * 参考: Egaroucid bin/battle.py, bin/egaroucid_vs_edax.py
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @brief 对战配置
 */
struct BattleConfig {
    std::string player1_name;
    std::string player2_name;
    std::unique_ptr<AIStrategy> player1;
    std::unique_ptr<AIStrategy> player2;

    // 搜索限制
    SearchLimits limits1;
    SearchLimits limits2;

    // 对战设置
    int num_games = 50;                    ///< 对战局数
    bool alternate_first = true;            ///< 交替先手
    bool parallel = false;                  ///< 并行对战
    int max_threads = 4;                    ///< 最大并行数

    // 输出设置
    bool verbose = false;                   ///< 详细输出
    bool save_games = false;                ///< 保存对局记录
    std::string log_path;                   ///< 日志路径

    // 随机种子
    uint64_t random_seed = 0;               ///< 随机种子 (0=使用时间)

    BattleConfig() {
        limits1 = SearchLimits::createDefault();
        limits2 = SearchLimits::createDefault();
    }
};

/**
 * @brief 单场对战结果
 */
struct SingleGameResult {
    int game_number;           ///< 对局编号
    PlayerColor winner;        ///< 胜者
    int black_score;           ///< 黑棋最终棋子数
    int white_score;           ///< 白棋最终棋子数
    int moves_count;           ///< 回合数
    double duration_ms;        ///< 对战耗时 (毫秒)
    std::vector<Move> game_moves; ///< 对局着法记录

    /**
     * @brief 判断是否平局
     */
    bool isDraw() const {
        return black_score == white_score;
    }

    /**
     * @brief 获取胜者分数
     */
    int getWinnerScore() const {
        return winner == PlayerColor::Black ? black_score : white_score;
    }

    /**
     * @brief 获取输者分数
     */
    int getLoserScore() const {
        return winner == PlayerColor::Black ? white_score : black_score;
    }

    /**
     * @brief 获取净胜分
     */
    int getMargin() const {
        return std::abs(black_score - white_score);
    }
};

/**
 * @brief 对战统计结果
 */
struct BattleStats {
    std::string player1_name;
    std::string player2_name;

    int total_games = 0;
    int player1_wins = 0;
    int player2_wins = 0;
    int draws = 0;

    double win_rate1 = 0.0;
    double win_rate2 = 0.0;
    double avg_moves = 0.0;
    double avg_duration_ms = 0.0;

    // 按先手/后手统计
    int player1_first_wins = 0;
    int player1_first_games = 0;
    int player2_first_wins = 0;
    int player2_first_games = 0;

    // 分数统计
    double avg_score1 = 0.0;
    double avg_score2 = 0.0;
    double avg_margin = 0.0;
    int max_margin = 0;

    // 统计显著性
    double p_value = 1.0;
    bool significant = false;

    // 对局详情
    std::vector<SingleGameResult> games;

    /**
     * @brief 计算统计信息
     */
    void calculate();

    /**
     * @brief 获取胜率置信区间 (95%)
     */
    std::pair<double, double> getWinRateCI1() const;

    /**
     * @brief Convert to summary string
     */
    std::string toString() const;

    // ===== Pass/Fail Checking Methods =====

    /**
     * @brief Check if win rate meets target
     * @param target_winrate Target win rate (0.0 to 1.0)
     * @return true if win_rate1 >= target
     */
    bool checkVsTarget(double target_winrate) const {
        return win_rate1 >= target_winrate;
    }

    /**
     * @brief Get Pass/Fail status for win rate target
     * @param target Target win rate (0.0 to 1.0)
     * @return "PASS" or "FAIL"
     */
    std::string getPassFailStatus(double target) const {
        return checkVsTarget(target) ? "PASS" : "FAIL";
    }

    /**
     * @brief Check if result is statistically significant
     * @return true if p_value < 0.05
     */
    bool isStatisticallySignificant() const {
        return p_value < BenchmarkTargets::P_VALUE_THRESHOLD;
    }

    /**
     * @brief Get Pass/Fail status for statistical significance
     * @return "PASS" or "FAIL"
     */
    std::string getSignificanceStatus() const {
        return isStatisticallySignificant() ? "PASS" : "FAIL";
    }
};

/**
 * @brief 对战进度回调
 */
using BattleProgressCallback = std::function<void(int current, int total, const SingleGameResult& result)>;

/**
 * @brief Head-to-Head对战引擎
 *
 * 支持:
 * - 批量对战测试
 * - 并行对战
 * - 统计显著性分析
 * - 结果导出 (CSV, JSON)
 */
class BattleEngine {
public:
    /**
     * @brief 运行批量对战
     *
     * @param config 对战配置
     * @param progress_callback 进度回调
     * @return 对战统计结果
     */
    static BattleStats runBattle(const BattleConfig& config,
                                   BattleProgressCallback progress_callback = nullptr);

    /**
     * @brief 运行单场对战
     *
     * @param player1 AI策略1
     * @param player2 AI策略2
     * @param first_player 先手玩家
     * @param limits1 搜索限制1
     * @param limits2 搜索限制2
     * @return 对局结果
     */
    static SingleGameResult playSingleGame(
        AIStrategy& player1,
        AIStrategy& player2,
        PlayerColor first_player,
        const SearchLimits& limits1,
        const SearchLimits& limits2
    );

    /**
     * @brief 验证对战配置
     *
     * @param config 对战配置
     * @return true 如果配置有效
     */
    static bool validateConfig(const BattleConfig& config);

    /**
     * @brief 导出对战结果到CSV
     *
     * @param stats 统计结果
     * @param filepath 文件路径
     */
    static void exportToCSV(const BattleStats& stats, const std::string& filepath);

    /**
     * @brief 导出详细对战记录
     *
     * @param stats 统计结果
     * @param filepath 文件路径
     */
    static void exportGameLog(const BattleStats& stats, const std::string& filepath);

    /**
     * @brief 导出为JSON格式
     *
     * @param stats 统计结果
     * @param filepath 文件路径
     */
    static void exportToJSON(const BattleStats& stats, const std::string& filepath);

    /**
     * @brief 运行标准基准测试
     *
     * 使用标准64位置测试套件
     *
     * @param ai AI策略
     * @param depth 搜索深度
     * @param num_games 测试局数
     * @return 基准测试结果
     */
    static BattleStats runStandardBenchmark(
        std::unique_ptr<AIStrategy> ai,
        int depth = 6,
        int num_games = 50
    );

    /**
     * @brief 比较两个AI
     *
     * @param ai1 AI策略1
     * @param ai2 AI策略2
     * @param num_games 测试局数
     * @return 对比结果
     */
    static BattleStats compareAI(
        std::unique_ptr<AIStrategy> ai1,
        std::unique_ptr<AIStrategy> ai2,
        int num_games = 50
    );

    /**
     * @brief 设置全局随机种子
     */
    static void setRandomSeed(uint64_t seed);

    /**
     * @brief 获取全局随机种子
     */
    static uint64_t getRandomSeed();

    // ===== Position Suite Integration =====

    /**
     * @brief Play a single game from a specific position
     *
     * @param position Test position to start from
     * @param player1 AI strategy 1
     * @param player2 AI strategy 2
     * @param limits1 Search limits for player 1
     * @param limits2 Search limits for player 2
     * @return Game result
     */
    static SingleGameResult playFromPosition(
        const TestPosition& position,
        AIStrategy& player1,
        AIStrategy& player2,
        const SearchLimits& limits1,
        const SearchLimits& limits2
    );

    /**
     * @brief Run battle on position suite
     *
     * @param positions Position suite to test
     * @param config Battle configuration
     * @param progress_callback Progress callback
     * @return Battle statistics
     */
    static BattleStats runSuiteBattle(
        const std::vector<TestPosition>& positions,
        const BattleConfig& config,
        BattleProgressCallback progress_callback = nullptr
    );

    /**
     * @brief Get position suite by type
     *
     * @param type Suite type (0=Standard64, 1=Opening, 2=Midgame, 3=Endgame)
     * @return Position suite
     */
    static std::vector<TestPosition> getSuiteByType(int type);

private:
    static uint64_t global_seed_;
    static std::mt19937_64 rng_;

    /**
     * @brief 执行实际的对局 (内部)
     */
    static SingleGameResult playGameInternal(
        AIStrategy& p1,
        AIStrategy& p2,
        PlayerColor first,
        const SearchLimits& l1,
        const SearchLimits& l2
    );

    /**
     * @brief 更新统计信息
     */
    static void updateStats(BattleStats& stats, const SingleGameResult& result,
                            bool player1_first);

    /**
     * @brief 并行对战工作函数
     */
    static std::vector<SingleGameResult> runParallelBattle(const BattleConfig& config);
};

/**
 * @brief 快速对战测试
 *
 * 简化API，用于快速测试
 */
struct QuickBattle {
    /**
     * @brief 运行10局快速测试
     */
    static BattleStats quickTest(
        std::unique_ptr<AIStrategy> ai1,
        std::unique_ptr<AIStrategy> ai2
    ) {
        BattleConfig config;
        config.player1 = std::move(ai1);
        config.player2 = std::move(ai2);
        config.player1_name = config.player1->getName();
        config.player2_name = config.player2->getName();
        config.num_games = 10;
        config.verbose = false;

        return BattleEngine::runBattle(config);
    }

    /**
     * @brief 测试AI vs Random
     */
    static BattleStats testVsRandom(
        std::unique_ptr<AIStrategy> ai,
        int num_games = 50
    ) {
        BattleConfig config;
        config.player1 = std::move(ai);
        config.player2 = AIStrategyFactory::createRandomAI();
        config.player1_name = config.player1->getName();
        config.player2_name = "RandomAI";
        config.num_games = num_games;
        config.verbose = false;

        return BattleEngine::runBattle(config);
    }
};

} // namespace Reversi

