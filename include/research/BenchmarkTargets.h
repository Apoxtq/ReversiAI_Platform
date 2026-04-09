#pragma once

#include <string>

namespace Reversi {

/**
 * @brief Benchmark target values for Pass/Fail determination
 *
 * Reference: Reversi_Proposal.md Section 8.3
 */
struct BenchmarkTargets {
    // ===== Primary Performance (AI Strength) =====
    static constexpr double MINIMAX6_VS_RANDOM_WINRATE = 0.90;   // >= 90%
    static constexpr double MCTS_VS_MINIMAX4_WINRATE = 0.70;     // >= 70%
    static constexpr int MINIMAX6_VS_RANDOM_GAMES = 100;        // 100 games
    static constexpr int MCTS_VS_MINIMAX4_GAMES = 50;           // 50 games

    // ===== Secondary Efficiency (Bitboard Operations) =====
    static constexpr double BITBOARD_FLIP_SPEED = 100.0;         // >= 100 M/s
    static constexpr double BITBOARD_MOVE_GEN = 50.0;            // >= 50 M/s
    static constexpr double BITBOARD_LEGAL_CHECK = 100.0;        // >= 100 M/s
    static constexpr double BITBOARD_COPY = 20.0;                // >= 20 M/s

    // ===== Secondary Efficiency (AI Search) =====
    static constexpr double MINIMAX_THROUGHPUT = 2.0;            // >= 2.0 M nodes/s
    static constexpr double MCTS_SIMULATION_RATE = 200.0;        // >= 200 K sims/s

    // ===== Stability Tests =====
    static constexpr int LAN_STABILITY_TOTAL = 10;               // 10/10 games
    static constexpr int LAN_STABILITY_DESYNCS = 0;             // 0 desyncs
    static constexpr int AI_STABILITY_TOTAL = 100;              // 100 games
    static constexpr int AI_STABILITY_CRASHES = 0;              // 0 crashes

    // ===== Statistical Significance =====
    static constexpr double P_VALUE_THRESHOLD = 0.05;            // p < 0.05

    /**
     * @brief Check if win rate passes the target
     */
    static bool checkWinRate(double actual_winrate, double target_winrate) {
        return actual_winrate >= target_winrate;
    }

    /**
     * @brief Check if performance metric passes the target
     */
    static bool checkPerformance(double actual, double target) {
        return actual >= target;
    }

    /**
     * @brief Check if p-value indicates statistical significance
     */
    static bool checkSignificance(double p_value) {
        return p_value < P_VALUE_THRESHOLD;
    }

    /**
     * @brief Get Pass/Fail status string
     */
    static std::string getPassFailStatus(bool passed) {
        return passed ? "PASS" : "FAIL";
    }
};

} // namespace Reversi
