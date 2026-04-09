#pragma once

#include "ai/AIStrategy.h"
#include <random>

/**
 * @file RandomAI.h
 * @brief Random AI implementation
 *
 * Simple random-move AI for testing and baseline comparison
 * Reference: alpha-zero-general random policy
 */

namespace Reversi {

/**
 * @class RandomAI
 * @brief Random AI strategy
 *
 * Randomly selects from valid moves.
 * Used for benchmarking and as a weak AI opponent.
 */
class RandomAI : public AIStrategy {
public:
    /**
     * @brief Constructor
     * @param seed Random seed (for reproducibility)
     */
    explicit RandomAI(unsigned int seed = 42);

    // AIStrategy interface
    Move findBestMove(const Board& board, const SearchLimits& limits) override;
    std::string getName() const override { return "RandomAI"; }
    std::string getDescription() const override;
    Difficulty getDifficulty() const override { return Difficulty::EASY; }
    AIStats getStats() const override;
    std::string getConfigDescription() const override;
    void reset() override;
    bool supportsFeature(const std::string& feature) const override;
    void setColor(PlayerColor color) override;
    PlayerColor getColor() const override;

    /**
     * @brief Set random seed
     * @param seed Random seed
     */
    void setSeed(unsigned int seed);

private:
    std::mt19937 rng_;                    ///< Random number generator
    mutable AIStats stats_;               ///< Statistics
    unsigned int seed_;                   ///< Current seed
};

} // namespace Reversi
