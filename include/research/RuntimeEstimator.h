#pragma once

#include <string>
#include <sstream>

namespace Reversi {

/**
 * @brief Runtime estimator for battle configurations
 *
 * Estimates how long a battle will take based on:
 * - Number of games
 * - AI types (Minimax, MCTS, Random)
 * - Search depth
 * - MCTS simulation count
 */
class RuntimeEstimator {
public:
    /**
     * @brief Configuration for estimation
     */
    struct EstimateConfig {
        int num_games = 10;
        std::string ai1_type = "Minimax";    // "Minimax", "MCTS", "Random"
        std::string ai2_type = "Random";
        int depth = 4;
        int mcts_simulations = 10000;
        int threads = 1;
    };

    /**
     * @brief Estimation result
     */
    struct Estimate {
        int estimated_seconds = 0;
        int estimated_minutes = 0;

        std::string getDisplayString() const {
            std::ostringstream oss;
            if (estimated_minutes >= 60) {
                int hours = estimated_minutes / 60;
                int mins = estimated_minutes % 60;
                oss << "~" << hours << "h " << mins << "m";
            } else if (estimated_minutes >= 1) {
                oss << "~" << estimated_minutes << " minutes";
            } else {
                oss << "~" << estimated_seconds << " seconds";
            }
            return oss.str();
        }
    };

    /**
     * @brief Calculate runtime estimate
     */
    static Estimate calculate(const EstimateConfig& config) {
        Estimate result;

        double seconds_per_game = getSecondsPerGame(config);
        double total_seconds = seconds_per_game * config.num_games;

        // Account for parallel processing
        if (config.threads > 1) {
            total_seconds /= config.threads;
            total_seconds *= 0.85;  // 15% overhead for thread management
        }

        result.estimated_seconds = static_cast<int>(total_seconds);
        result.estimated_minutes = static_cast<int>(total_seconds / 60.0);

        return result;
    }

private:
    /**
     * @brief Get base time per game in seconds
     */
    static double getSecondsPerGame(const EstimateConfig& config) {
        double base_time = 0.5;  // Default base time

        std::string type1 = config.ai1_type;
        std::string type2 = config.ai2_type;
        int depth = config.depth;

        if (type1 == "Random" && type2 == "Random") {
            // Both random - very fast
            base_time = 0.05;
        } else if (type1 == "Random" || type2 == "Random") {
            // One random - fast
            if (type1 == "Minimax" || type2 == "Minimax") {
                base_time = 0.5 * std::pow(2.0, depth - 4);
            } else {
                base_time = 0.3;
            }
        } else if (type1 == "Minimax" && type2 == "Minimax") {
            // Minimax vs Minimax - exponential growth with depth
            base_time = 1.0 * std::pow(2.0, depth - 4);
        } else if (type1 == "MCTS" || type2 == "MCTS") {
            // MCTS - depends on simulation count
            double sim_factor = config.mcts_simulations / 10000.0;
            base_time = 2.0 * sim_factor;
        }

        return base_time;
    }
};

/**
 * @brief Simple runtime estimator for quick estimates
 */
class SimpleRuntimeEstimator {
public:
    /**
     * @brief Quick estimate without full configuration
     */
    static int estimateSeconds(
        int num_games,
        const std::string& ai1_type,
        const std::string& ai2_type,
        int depth = 4,
        int threads = 1
    ) {
        RuntimeEstimator::EstimateConfig config;
        config.num_games = num_games;
        config.ai1_type = ai1_type;
        config.ai2_type = ai2_type;
        config.depth = depth;
        config.threads = threads;

        return RuntimeEstimator::calculate(config).estimated_seconds;
    }

    /**
     * @brief Get human-readable estimate
     */
    static std::string getEstimateString(
        int num_games,
        const std::string& ai1_type,
        const std::string& ai2_type,
        int depth = 4,
        int threads = 1
    ) {
        RuntimeEstimator::EstimateConfig config;
        config.num_games = num_games;
        config.ai1_type = ai1_type;
        config.ai2_type = ai2_type;
        config.depth = depth;
        config.threads = threads;

        return RuntimeEstimator::calculate(config).getDisplayString();
    }
};

} // namespace Reversi
