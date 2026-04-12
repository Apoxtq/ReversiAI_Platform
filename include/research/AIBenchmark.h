#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <memory>
#include <sstream>
#include <iomanip>
#include "../ai/AIStrategy.h"
#include "../core/BitBoard.h"
#include "BitboardBenchmark.h"

/**
 * @file AIBenchmark.h
 * @brief AI Search Performance Benchmark
 *
 * Measures AI algorithm search throughput:
 * - Minimax throughput
 * - MCTS simulation rate
 * - Search depth performance
 *
 * Reference: Egaroucid src/engine/search.hpp
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @brief AI performance test result
 */
struct AISearchBenchmarkResult {
    std::string ai_name;         ///< AI name
    std::string test_name;       ///< Test name
    int depth;                   ///< Search depth
    int64_t nodes_searched;       ///< Searched nodes
    double time_ms;               ///< Time elapsed (milliseconds)
    double nps;                   ///< Nodes per second
    double throughput;            ///< Throughput (M nodes/sec)
    std::string unit;             ///< Unit
    bool passed;                  ///< Whether passed acceptance criteria
    std::string message;          ///< Result description

    /**
     * @brief Convert to string
     */
    std::string toString() const;
};

/**
 * @brief AI search benchmark class
 *
 * Provides systematic AI performance testing
 */
class AISearchBenchmark {
public:
    // Acceptance criteria (based on Reversi_Proposal.md 8.3.2)
    static constexpr double TARGET_MINIMAX6_NPS = 2.0;   // M nodes/sec @ depth 6
    static constexpr double TARGET_MINIMAX8_NPS = 1.0;   // M nodes/sec @ depth 8
    static constexpr double TARGET_MCTS_SIMS = 200.0;    // K sims/sec

    // Default test parameters
    static constexpr int DEFAULT_TIME_LIMIT_MS = 5000;
    static constexpr int DEFAULT_POSITION_COUNT = 10;

    /**
     * @brief Constructor
     */
    AISearchBenchmark();

    /**
     * @brief Run full AI benchmark
     * @return Test result vector
     */
    std::vector<AISearchBenchmarkResult> runFullBenchmark();

    // =========================================================================
    // Minimax Performance Tests
    // =========================================================================

    /**
     * @brief Minimax performance test
     *
     * Measures Minimax AI search throughput
     *
     * @param ai AI strategy
     * @param depth Search depth
     * @param time_limit_ms Time limit (milliseconds)
     * @return Test result
     */
    AISearchBenchmarkResult benchmarkMinimax(
        AIStrategy& ai,
        int depth,
        int time_limit_ms = DEFAULT_TIME_LIMIT_MS
    );

    /**
     * @brief Measure Minimax throughput
     *
     * @param depth Search depth
     * @param time_limit_ms Time limit
     * @return Throughput (M nodes/sec)
     */
    double measureMinimaxThroughput(int depth, int time_limit_ms = DEFAULT_TIME_LIMIT_MS);

    // =========================================================================
    // MCTS Performance Tests
    // =========================================================================

    /**
     * @brief MCTS performance test
     *
     * Measures MCTS AI simulation rate
     *
     * @param ai AI strategy
     * @param simulations Number of simulations
     * @param time_limit_ms Time limit
     * @return Test result
     */
    AISearchBenchmarkResult benchmarkMCTS(
        AIStrategy& ai,
        int simulations = 1000,
        int time_limit_ms = DEFAULT_TIME_LIMIT_MS
    );

    /**
     * @brief Measure MCTS simulation rate
     *
     * @param time_limit_ms Time limit
     * @return Simulation rate (K sims/sec)
     */
    double measureMCTSSimulationRate(int time_limit_ms = DEFAULT_TIME_LIMIT_MS);

    // =========================================================================
    // Combined Tests
    // =========================================================================

    /**
     * @brief Full AI comparison test
     *
     * Compare multiple AI performances
     *
     * @param ai1 AI 1
     * @param ai2 AI 2
     * @param num_games Number of test games
     * @return Comparison results
     */
    std::vector<AISearchBenchmarkResult> compareAI(
        AIStrategy& ai1,
        AIStrategy& ai2,
        int num_games = 10
    );

    /**
     * @brief Benchmark configuration
     */
    struct Config {
        int time_limit_ms = DEFAULT_TIME_LIMIT_MS;
        int position_count = DEFAULT_POSITION_COUNT;
        bool verbose = true;
        bool warmup = true;
    };

    /**
     * @brief Set test configuration
     */
    void setConfig(const Config& config);

    /**
     * @brief Get test configuration
     */
    const Config& getConfig() const;

    /**
     * @brief Test callback function type
     */
    using ProgressCallback = std::function<void(int current, int total, const std::string& name)>;

    /**
     * @brief Set progress callback
     */
    void setProgressCallback(ProgressCallback callback);

    /**
     * @brief Get test positions
     */
    static std::vector<BitBoard> getTestPositions();

    /**
     * @brief Generate report
     */
    static std::string generateReport(const std::vector<AISearchBenchmarkResult>& results);

    /**
     * @brief Export as JSON
     */
    static std::string toJson(const std::vector<AISearchBenchmarkResult>& results);

private:
    Config config_;
    ProgressCallback progress_callback_;

    /**
     * @brief Warmup
     */
    void warmUp();
};

} // namespace Reversi
