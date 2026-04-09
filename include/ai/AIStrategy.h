#pragma once

#ifdef _WIN32
#define NOMINMAX
#endif

#include "Board.h"
#include <memory>
#include <string>
#include <chrono>
#include <optional>
#include <algorithm>

/**
 * @file AIStrategy.h
 * @brief AI Strategy unified interface
 *
 * Defines unified AI algorithm interface, implementing the Strategy pattern.
 * Supports unified management and switching of multiple AI algorithms.
 */

namespace Reversi {

/**
 * @enum Difficulty
 * @brief AI difficulty level
 */
enum class Difficulty {
    EASY,      ///< Easy difficulty
    MEDIUM,    ///< Medium difficulty
    HARD,      ///< Hard difficulty
    CUSTOM     ///< Custom configuration
};

/**
 * @struct SearchLimits
 * @brief Search constraint parameters
 */
struct SearchLimits {
    std::optional<int> maxDepth;                    ///< Maximum search depth
    std::optional<std::chrono::milliseconds> timeLimit;  ///< Time limit
    std::optional<long long> maxNodes;             ///< Maximum search nodes

    /**
     * @brief Create default search limits
     */
    static SearchLimits createDefault() {
        SearchLimits limits;
        limits.maxDepth = 4;
        limits.timeLimit = std::chrono::milliseconds(3000);
        limits.maxNodes = std::nullopt;
        return limits;
    }
};

/**
 * @struct AIStats
 * @brief AI algorithm performance statistics
 */
struct AIStats {
    long long nodesExplored = 0;      ///< Number of nodes explored
    std::chrono::milliseconds timeUsed = std::chrono::milliseconds(0);  ///< Total time used
    double avgBranching = 0.0;        ///< Average branching factor
    long long evaluationCount = 0;    ///< Evaluation function call count
    int cutoffs = 0;                  ///< Cutoff count
    int depthReached = 0;             ///< Maximum depth reached

    /**
     * @brief Reset statistics
     */
    void reset() {
        nodesExplored = 0;
        timeUsed = std::chrono::milliseconds(0);
        avgBranching = 0.0;
        evaluationCount = 0;
        cutoffs = 0;
        depthReached = 0;
    }

    /**
     * @brief Merge statistics
     */
    AIStats& operator+=(const AIStats& other) {
        nodesExplored += other.nodesExplored;
        timeUsed += other.timeUsed;
        avgBranching = (avgBranching + other.avgBranching) / 2.0;
        evaluationCount += other.evaluationCount;
        cutoffs += other.cutoffs;
        depthReached = (depthReached > other.depthReached) ? depthReached : other.depthReached;
        return *this;
    }
};

/**
 * @class AIStrategy
 * @brief AI Strategy abstract base class
 *
 * Defines unified AI algorithm interface, implementing the Strategy pattern.
 * All AI algorithms inherit from this class, providing a unified interface.
 */
class AIStrategy {
public:
    virtual ~AIStrategy() = default;

    /**
     * @brief Find best move
     *
     * @param board Current board state
     * @param limits Search constraint parameters
     * @return Best move decision
     */
    virtual Move findBestMove(const Board& board, const SearchLimits& limits) = 0;

    /**
     * @brief Get AI algorithm name
     *
     * @return Algorithm name string
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Get AI algorithm description
     *
     * @return Algorithm description string
     */
    virtual std::string getDescription() const = 0;

    /**
     * @brief Get difficulty level
     *
     * @return Difficulty level
     */
    virtual Difficulty getDifficulty() const = 0;

    /**
     * @brief Get performance statistics
     *
     * @return Latest performance statistics
     */
    virtual AIStats getStats() const = 0;

    /**
     * @brief Get current configuration description
     *
     * @return Configuration description string
     */
    virtual std::string getConfigDescription() const = 0;

    /**
     * @brief Reset internal state
     *
     * Used for clearing caches, statistics, etc.
     */
    virtual void reset() = 0;

    /**
     * @brief Set AI player color (for correct evaluation perspective)
     * @param color AI player color (black or white)
     */
    virtual void setColor(PlayerColor color) {}

    /**
     * @brief Get AI player color
     * @return AI color
     */
    virtual PlayerColor getColor() const { return PlayerColor::Black; }

    /**
     * @brief Check if algorithm supports a specific feature
     */
    virtual bool supportsFeature(const std::string& feature) const {
        return false;
    }
};

/**
 * @class AIStrategyFactory
 * @brief AI Strategy factory class
 *
 * Provides factory methods for creating various AI strategies.
 */
class AIStrategyFactory {
public:
    /**
     * @brief Create Minimax AI
     *
     * @param difficulty Difficulty level
     * @return Minimax AI instance
     */
    static std::unique_ptr<AIStrategy> createMinimaxAI(Difficulty difficulty = Difficulty::MEDIUM);

    /**
     * @brief Create MCTS AI
     *
     * @param difficulty Difficulty level
     * @return MCTS AI instance
     */
    static std::unique_ptr<AIStrategy> createMCTSAI(Difficulty difficulty = Difficulty::MEDIUM);

    /**
     * @brief Create Random AI (for testing)
     *
     * @return Random AI instance
     */
    static std::unique_ptr<AIStrategy> createRandomAI();

    /**
     * @brief Get list of supported AI algorithms
     *
     * @return AI algorithm name list
     */
    static std::vector<std::string> getAvailableAlgorithms();

    /**
     * @brief Create AI instance by name
     *
     * @param name AI algorithm name
     * @param difficulty Difficulty level
     * @return AI instance, nullptr if name is invalid
     */
    static std::unique_ptr<AIStrategy> createByName(const std::string& name,
                                                   Difficulty difficulty = Difficulty::MEDIUM);
};

/**
 * @brief Convert difficulty level to string
 */
std::string difficultyToString(Difficulty difficulty);

/**
 * @brief Convert string to difficulty level
 */
Difficulty stringToDifficulty(const std::string& str);

} // namespace Reversi
