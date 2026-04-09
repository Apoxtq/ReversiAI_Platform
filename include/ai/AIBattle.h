#pragma once

#include "ai/AIStrategy.h"
#include "Board.h"
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <functional>

/**
 * @file AIBattle.h
 * @brief AI battle system
 *
 * Automatic AI vs AI battles for performance comparison and benchmarking.
 * Supports async battles with progress callbacks in GUI environment.
 */

namespace Reversi {

/**
 * @struct BattleResult
 * @brief Single game result
 */
struct BattleResult {
    PlayerColor winner = PlayerColor::Black;  ///< Winner
    bool isDraw = false;                       ///< Whether draw
    int totalMoves = 0;                        ///< Total moves
    std::chrono::milliseconds duration;        ///< Game duration

    // AI statistics
    AIStats blackAIStats;                      ///< Black AI statistics
    AIStats whiteAIStats;                      ///< White AI statistics

    // Move history
    std::vector<Move> moveHistory;             ///< Move history record
};

/**
 * @struct TournamentResult
 * @brief Tournament result
 */
struct TournamentResult {
    std::string blackAIName;                   ///< Black AI name
    std::string whiteAIName;                   ///< White AI name

    int totalGames = 0;                        ///< Total games
    int blackWins = 0;                         ///< Black wins
    int whiteWins = 0;                         ///< White wins
    int draws = 0;                             ///< Draws

    double blackWinRate = 0.0;                 ///< Black win rate
    double whiteWinRate = 0.0;                 ///< White win rate
    double drawRate = 0.0;                     ///< Draw rate

    std::chrono::milliseconds totalDuration;   ///< Total duration

    // Average statistics
    AIStats avgBlackStats;                     ///< Black average stats
    AIStats avgWhiteStats;                     ///< White average stats

    // All individual game results
    std::vector<BattleResult> gameResults;     ///< All game results
};

/**
 * @class AIBattle
 * @brief AI battle manager
 *
 * Manages automatic AI vs AI battles:
 * - Single game battles
 * - Batch tournaments
 * - Progress callbacks (for GUI updates)
 * - Thread safety
 */
class AIBattle {
public:
    /**
     * @brief Progress callback function type
     * @param current Current completed games
     * @param total Total games
     * @param result Current game result
     */
    using ProgressCallback = std::function<void(int current, int total, const BattleResult& result)>;

    /**
     * @brief Constructor
     * @param blackAI Black player AI
     * @param whiteAI White player AI
     */
    AIBattle(std::unique_ptr<AIStrategy> blackAI, std::unique_ptr<AIStrategy> whiteAI);

    /**
     * @brief Execute single game
     * @param searchLimits Search limits
     * @return Game result
     */
    BattleResult playSingleGame(const SearchLimits& searchLimits = SearchLimits::createDefault());

    /**
     * @brief Execute tournament (multiple games)
     * @param numGames Number of games
     * @param searchLimits Search limits
     * @param progressCallback Progress callback (optional, for GUI updates)
     * @return Tournament result
     */
    TournamentResult playTournament(int numGames,
                                   const SearchLimits& searchLimits = SearchLimits::createDefault(),
                                   ProgressCallback progressCallback = nullptr);

    /**
     * @brief Get black AI name
     */
    std::string getBlackAIName() const;

    /**
     * @brief Get white AI name
     */
    std::string getWhiteAIName() const;

    /**
     * @brief Get black AI description
     */
    std::string getBlackAIDescription() const;

    /**
     * @brief Get white AI description
     */
    std::string getWhiteAIDescription() const;

    /**
     * @brief Set random seed (for reproducibility)
     * @param seed Random seed
     */
    void setRandomSeed(unsigned int seed);

private:
    /**
     * @brief Execute one complete game
     * @param searchLimits Search limits
     * @return Game result
     */
    BattleResult executeGame(const SearchLimits& searchLimits);

    /**
     * @brief Process single move
     * @param board Current board
     * @param currentPlayer Current player
     * @param ai AI algorithm
     * @param searchLimits Search limits
     * @param moveHistory Move history (for recording)
     * @param aiStats AI statistics (for accumulation)
     * @return Whether move executed successfully
     */
    bool executeMove(Board& board, PlayerColor currentPlayer, AIStrategy& ai,
                    const SearchLimits& searchLimits, std::vector<Move>& moveHistory,
                    AIStats& aiStats);

    // AI algorithms
    std::unique_ptr<AIStrategy> blackAI_;
    std::unique_ptr<AIStrategy> whiteAI_;

    // Random number generator (for possible randomness control)
    unsigned int randomSeed_ = 42;
};

/**
 * @class AIBenchmark
 * @brief AI performance benchmark
 *
 * Provides standardized performance tests and comparison
 * Reference: Egaroucid benchmark system
 */
class AIBenchmark {
public:
    /**
     * @brief Benchmark result
     */
    struct BenchmarkResult {
        std::string aiName;
        int testGames = 0;
        std::chrono::milliseconds avgTimePerMove;
        double avgNodesExplored = 0.0;
        double avgBranchingFactor = 0.0;
        double winRate = 0.0;  // Win rate against random AI
    };

    /**
     * @brief Run AI performance benchmark
     * @param ai AI to test
     * @param testGames Number of test games
     * @return Benchmark result
     */
    static BenchmarkResult runBenchmark(std::unique_ptr<AIStrategy> ai, int testGames = 10);

    /**
     * @brief Compare two AI algorithms
     * @param ai1 First AI
     * @param ai2 Second AI
     * @param numGames Number of comparison games
     * @return Comparison result
     */
    static TournamentResult compareAIs(std::unique_ptr<AIStrategy> ai1,
                                      std::unique_ptr<AIStrategy> ai2,
                                      int numGames = 20);
};

} // namespace Reversi
