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
 * @brief Head-to-Head Battle Engine
 *
 * Batch AI battle testing, generates win rate statistics.
 *
 * Reference: Egaroucid bin/battle.py, bin/egaroucid_vs_edax.py
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @brief Battle Configuration
 */
struct BattleConfig {
    std::string player1_name;
    std::string player2_name;
    std::unique_ptr<AIStrategy> player1;
    std::unique_ptr<AIStrategy> player2;

    // Search limits
    SearchLimits limits1;
    SearchLimits limits2;

    // Battle settings
    int num_games = 50;                    ///< Number of games
    bool alternate_first = true;            ///< Alternate first player
    bool parallel = false;                  ///< Parallel battle
    int max_threads = 4;                   ///< Maximum parallelism

    // Output settings
    bool verbose = false;                   ///< Verbose output
    bool save_games = false;               ///< Save game records
    std::string log_path;                  ///< Log path

    // Random seed
    uint64_t random_seed = 0;              ///< Random seed (0=use time)

    BattleConfig() {
        limits1 = SearchLimits::createDefault();
        limits2 = SearchLimits::createDefault();
    }
};

/**
 * @brief Single Game Result
 */
struct SingleGameResult {
    int game_number;           ///< Game number
    PlayerColor winner;        ///< Winner
    int black_score;           ///< Black final piece count
    int white_score;           ///< White final piece count
    int moves_count;           ///< Move count
    double duration_ms;        ///< Battle duration (milliseconds)
    std::vector<Move> game_moves; ///< Game move records

    /**
     * @brief Check if draw
     */
    bool isDraw() const {
        return black_score == white_score;
    }

    /**
     * @brief Get winner score
     */
    int getWinnerScore() const {
        return winner == PlayerColor::Black ? black_score : white_score;
    }

    /**
     * @brief Get loser score
     */
    int getLoserScore() const {
        return winner == PlayerColor::Black ? white_score : black_score;
    }

    /**
     * @brief Get margin
     */
    int getMargin() const {
        return std::abs(black_score - white_score);
    }
};

/**
 * @brief Battle Statistics Result
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

    // Statistics by first/second player
    int player1_first_wins = 0;
    int player1_first_games = 0;
    int player2_first_wins = 0;
    int player2_first_games = 0;

    // Score statistics
    double avg_score1 = 0.0;
    double avg_score2 = 0.0;
    double avg_margin = 0.0;
    int max_margin = 0;

    // Statistical significance
    double p_value = 1.0;
    bool significant = false;

    // Game details
    std::vector<SingleGameResult> games;

    /**
     * @brief Calculate statistics
     */
    void calculate();

    /**
     * @brief Get win rate 95% confidence interval
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
 * @brief Battle progress callback
 */
using BattleProgressCallback = std::function<void(int current, int total, const SingleGameResult& result)>;

/**
 * @brief Head-to-Head Battle Engine
 *
 * Supports:
 * - Batch battle testing
 * - Parallel battles
 * - Statistical significance analysis
 * - Result export (CSV, JSON)
 */
class BattleEngine {
public:
    /**
     * @brief Run batch battles
     *
     * @param config Battle configuration
     * @param progress_callback Progress callback
     * @return Battle statistics result
     */
    static BattleStats runBattle(const BattleConfig& config,
                                 BattleProgressCallback progress_callback = nullptr);

    /**
     * @brief Run single game battle
     *
     * @param player1 AI strategy 1
     * @param player2 AI strategy 2
     * @param first_player First player
     * @param limits1 Search limits 1
     * @param limits2 Search limits 2
     * @return Game result
     */
    static SingleGameResult playSingleGame(
        AIStrategy& player1,
        AIStrategy& player2,
        PlayerColor first_player,
        const SearchLimits& limits1,
        const SearchLimits& limits2
    );

    /**
     * @brief Validate battle configuration
     *
     * @param config Battle configuration
     * @return true if configuration is valid
     */
    static bool validateConfig(const BattleConfig& config);

    /**
     * @brief Export battle results to CSV
     *
     * @param stats Statistics result
     * @param filepath File path
     */
    static void exportToCSV(const BattleStats& stats, const std::string& filepath);

    /**
     * @brief Export detailed game log
     *
     * @param stats Statistics result
     * @param filepath File path
     */
    static void exportGameLog(const BattleStats& stats, const std::string& filepath);

    /**
     * @brief Export to JSON format
     *
     * @param stats Statistics result
     * @param filepath File path
     */
    static void exportToJSON(const BattleStats& stats, const std::string& filepath);

    /**
     * @brief Run standard benchmark
     *
     * Uses standard 64 position test suite
     *
     * @param ai AI strategy
     * @param depth Search depth
     * @param num_games Number of games
     * @return Benchmark result
     */
    static BattleStats runStandardBenchmark(
        std::unique_ptr<AIStrategy> ai,
        int depth = 6,
        int num_games = 50
    );

    /**
     * @brief Compare two AIs
     *
     * @param ai1 AI strategy 1
     * @param ai2 AI strategy 2
     * @param num_games Number of games
     * @return Comparison result
     */
    static BattleStats compareAI(
        std::unique_ptr<AIStrategy> ai1,
        std::unique_ptr<AIStrategy> ai2,
        int num_games = 50
    );

    /**
     * @brief Set global random seed
     */
    static void setRandomSeed(uint64_t seed);

    /**
     * @brief Get global random seed
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
     * @brief Execute actual game (internal)
     */
    static SingleGameResult playGameInternal(
        AIStrategy& p1,
        AIStrategy& p2,
        PlayerColor first,
        const SearchLimits& l1,
        const SearchLimits& l2
    );

    /**
     * @brief Update statistics
     */
    static void updateStats(BattleStats& stats, const SingleGameResult& result,
                            bool player1_first);

    /**
     * @brief Parallel battle worker function
     */
    static std::vector<SingleGameResult> runParallelBattle(const BattleConfig& config);
};

/**
 * @brief Quick battle test
 *
 * Simplified API for quick testing
 */
struct QuickBattle {
    /**
     * @brief Run 10 quick test games
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
     * @brief Test AI vs Random
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

