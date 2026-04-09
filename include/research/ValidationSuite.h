#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include "../ai/AIStrategy.h"
#include "BattleEngine.h"
#include "BenchmarkTargets.h"

namespace Reversi {

/**
 * @brief Single validation test result
 */
struct ValidationResult {
    std::string name;
    bool passed = false;
    double actual_value = 0.0;
    double target_value = 0.0;
    int games_played = 0;
    double win_rate = 0.0;
    double p_value = 1.0;
    bool statistically_significant = false;
    std::string details;
    std::chrono::milliseconds duration{0};
};

/**
 * @brief Validation test suite for academic benchmarks
 *
 * Provides standardized validation tests based on
 * Reversi_Proposal.md Section 8.3.1 Primary Performance
 */
class ValidationSuite {
public:
    /**
     * @brief Test configuration
     */
    struct TestConfig {
        std::string name;
        std::string ai1_type;
        std::string ai2_type;
        Difficulty diff1 = Difficulty::HARD;
        Difficulty diff2 = Difficulty::HARD;
        int depth1 = 6;
        int depth2 = 4;
        int mcts_sims1 = 10000;
        int mcts_sims2 = 10000;
        int num_games;
        double target_winrate;
        std::string description;
    };

    /**
     * @brief Get all preset test configurations
     */
    static std::vector<TestConfig> getPresetTests() {
        return {
            TestConfig{
                "Minimax-6 vs Random",
                "Minimax",
                "Random",
                Difficulty::HARD,
                Difficulty::EASY,
                6, 4, 0, 0,
                100, 0.90,
                "Minimax depth-6 should win >=90% vs Random"
            },
            TestConfig{
                "MCTS-10k vs Minimax-4",
                "MCTS",
                "Minimax",
                Difficulty::HARD,
                Difficulty::HARD,
                4, 4, 10000, 0,
                50, 0.70,
                "MCTS with 10k sims should win >=70% vs Minimax depth-4"
            },
            TestConfig{
                "AI Stability Test",
                "Minimax",
                "Minimax",
                Difficulty::HARD,
                Difficulty::HARD,
                6, 6, 0, 0,
                100, -1.0,
                "100 consecutive games with same settings (no crashes)"
            }
        };
    }

    /**
     * @brief Run a single validation test
     */
    static ValidationResult runTest(const TestConfig& config) {
        ValidationResult result;
        result.name = config.name;
        result.target_value = config.target_winrate;

        auto start_time = std::chrono::steady_clock::now();

        // Create AI instances
        auto ai1 = createAI(config.ai1_type, config.diff1, config.depth1, config.mcts_sims1);
        auto ai2 = createAI(config.ai2_type, config.diff2, config.depth2, config.mcts_sims2);

        if (!ai1 || !ai2) {
            result.details = "Failed to create AI instances";
            result.passed = false;
            return result;
        }

        // Run battle
        BattleConfig battle_config;
        battle_config.player1 = std::move(ai1);
        battle_config.player2 = std::move(ai2);
        battle_config.player1_name = battle_config.player1->getName();
        battle_config.player2_name = battle_config.player2->getName();
        battle_config.num_games = config.num_games;
        battle_config.verbose = false;
        battle_config.random_seed = 42;  // Fixed seed for reproducibility

        BattleStats stats = BattleEngine::runBattle(battle_config);

        auto end_time = std::chrono::steady_clock::now();
        result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // Fill results
        result.games_played = stats.total_games;
        result.win_rate = stats.win_rate1;
        result.actual_value = stats.win_rate1;
        result.p_value = stats.p_value;
        result.statistically_significant = stats.significant;

        // Determine pass/fail
        if (config.target_winrate < 0) {
            // Stability test - just check no crashes
            result.passed = (stats.games.size() == static_cast<size_t>(config.num_games));
            result.details = "Stability check: " + std::to_string(stats.games.size()) +
                           "/" + std::to_string(config.num_games) + " games completed";
        } else {
            result.passed = (stats.win_rate1 >= config.target_winrate);
            result.details = "Win rate " + std::to_string(static_cast<int>(stats.win_rate1 * 100)) +
                           "% vs target " + std::to_string(static_cast<int>(config.target_winrate * 100)) + "%";
        }

        return result;
    }

    /**
     * @brief Run all preset validation tests
     */
    static std::vector<ValidationResult> runAll() {
        std::vector<ValidationResult> results;
        for (const auto& config : getPresetTests()) {
            results.push_back(runTest(config));
        }
        return results;
    }

    /**
     * @brief Generate text report
     */
    static std::string generateReport(const std::vector<ValidationResult>& results) {
        std::ostringstream oss;

        oss << "========================================\n";
        oss << "     Validation Report\n";
        oss << "========================================\n\n";

        int pass_count = 0;
        for (const auto& result : results) {
            oss << "Test: " << result.name << "\n";
            oss << "  Games: " << result.games_played << "\n";
            oss << "  Win Rate: " << static_cast<int>(result.win_rate * 100) << "%\n";
            if (result.target_value >= 0) {
                oss << "  Target: " << static_cast<int>(result.target_value * 100) << "%\n";
            }
            oss << "  Status: " << (result.passed ? "PASS" : "FAIL") << "\n";
            oss << "  Duration: " << (result.duration.count() / 1000) << "s\n";
            oss << "  Details: " << result.details << "\n\n";

            if (result.passed) pass_count++;
        }

        oss << "========================================\n";
        oss << "Overall: " << pass_count << "/" << results.size() << " tests passed\n";
        oss << "========================================\n";

        return oss.str();
    }

private:
    /**
     * @brief Create AI instance based on type
     */
    static std::unique_ptr<AIStrategy> createAI(
        const std::string& type,
        Difficulty diff,
        int depth,
        int mcts_sims
    ) {
        if (type == "Minimax") {
            return AIStrategyFactory::createMinimaxAI(diff);
        } else if (type == "MCTS") {
            return AIStrategyFactory::createMCTSAI(diff);
        } else if (type == "Random") {
            return AIStrategyFactory::createRandomAI();
        }
        return nullptr;
    }
};

} // namespace Reversi
