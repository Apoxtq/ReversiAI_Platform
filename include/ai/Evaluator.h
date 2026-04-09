#pragma once

#include <memory>
#include <string>
#include "core/BitBoard.h"

/**
 * @file Evaluator.h
 * @brief Evaluation function interface and implementation
 *
 * Based on Reversi(Java) evaluator design, converted to C++ implementation.
 * Reference: Reversi(Java)/src/player/ai/Evaluator.java
 *           Reversi(Java)/src/player/ai/StaticEvaluator.java
 */

namespace Reversi {

class Evaluator;

/**
 * @brief Evaluation function interface
 *
 * Based on Reversi(Java) Evaluator interface design.
 */
class Evaluator {
public:
    virtual ~Evaluator() = default;

    /**
     * @brief Evaluate board state
     *
     * @param board Current board state
     * @param player Player to evaluate (1 or 2, corresponding to PlayerColor)
     * @return Evaluation score, higher is better for the player
     *
     * Reference: Evaluator.eval(int[][] board, int player)
     */
    virtual int evaluate(const BitBoard& board, PlayerColor player) const = 0;

    /**
     * @brief Get evaluator name
     */
    virtual std::string getName() const = 0;
};

/**
 * @brief Static weight evaluator
 *
 * Based on Reversi(Java) StaticEvaluator implementation.
 * Combines: mobility (2x) + disc difference + corner control (1000x).
 */
class StaticEvaluator : public Evaluator {
public:
    int evaluate(const BitBoard& board, PlayerColor player) const override;

    std::string getName() const override { return "StaticEvaluator"; }

private:
    /**
     * @brief Evaluate disc count difference
     *
     * Reference: StaticEvaluator.evalDiscDiff()
     */
    int evaluateDiscDifference(const BitBoard& board, PlayerColor player) const;

    /**
     * @brief Evaluate mobility
     *
     * Reference: StaticEvaluator.evalMobility()
     */
    int evaluateMobility(const BitBoard& board, PlayerColor player) const;

    /**
     * @brief Evaluate corner control
     *
     * Reference: StaticEvaluator.evalCorner()
     */
    int evaluateCorners(const BitBoard& board, PlayerColor player) const;

    /**
     * @brief Evaluate position weights
     *
     * Reference: StaticEvaluator.evalBoardMap()
     */
    int evaluatePositionWeights(const BitBoard& board, PlayerColor player) const;

    // Position weight table - based on Reversi(Java) classic weights
    static constexpr int POSITION_WEIGHTS[8][8] = {
        {200 , -100, 100,  50,  50, 100, -100,  200},
        {-100, -200, -50, -50, -50, -50, -200, -100},
        {100 ,  -50, 100,   0,   0, 100,  -50,  100},
        {50  ,  -50,   0,   0,   0,   0,  -50,   50},
        {50  ,  -50,   0,   0,   0,   0,  -50,   50},
        {100 ,  -50, 100,   0,   0, 100,  -50,  100},
        {-100, -200, -50, -50, -50, -50, -200, -100},
        {200 , -100, 100,  50,  50, 100, -100,  200}
    };

    // Weight factors - based on Reversi(Java) weight allocation
    static constexpr int MOBILITY_WEIGHT = 2;
    static constexpr int DISC_DIFF_WEIGHT = 1;
    static constexpr int CORNER_WEIGHT = 1000;
    static constexpr int POSITION_WEIGHT = 1;
};

/**
 * @brief Dynamic evaluator
 *
 * Based on Reversi(Java) RealtimeEvaluator concept.
 * Dynamically adjusts weights based on game phase.
 */
class DynamicEvaluator : public Evaluator {
public:
    /**
     * @brief Set game phase
     * @param phase 0.0=opening, 1.0=mid-game, 2.0=end-game
     */
    void setGamePhase(double phase);

    int evaluate(const BitBoard& board, PlayerColor player) const override;
    std::string getName() const override { return "DynamicEvaluator"; }

private:
    double game_phase_ = 0.0;
    mutable StaticEvaluator static_evaluator_;  // Base evaluator
};

/**
 * @brief Evaluator factory
 */
class EvaluatorFactory {
public:
    static std::unique_ptr<Evaluator> createStaticEvaluator();
    static std::unique_ptr<Evaluator> createDynamicEvaluator();
};

} // namespace Reversi
