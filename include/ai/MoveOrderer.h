/**
 * @file MoveOrderer.h
 * @brief Move Ordering implementation
 *
 * Unified management of all move ordering strategies:
 * Killer Moves + History Heuristic
 *
 * Ordering priority:
 * 1. PV move (Principal Variation) - known best move
 * 2. Killer moves - moves that caused cutoffs
 * 3. History heuristic - moves with high history scores
 * 4. Static evaluation - evaluator score
 * 5. Others - original order
 *
 * Reference: Egaroucid/src/engine/move_ordering.hpp
 * v0.7.0 - AI algorithm optimization version
 */

#pragma once

#include "ai/KillerTable.h"
#include "ai/HistoryTable.h"
#include "ai/Evaluator.h"
#include "core/BitBoard.h"
#include "Board.h"
#include <vector>
#include <memory>

namespace Reversi {

/**
 * @brief Move Orderer Configuration
 */
struct MoveOrdererConfig {
    bool useKillerMoves = true;           ///< Whether to use Killer Moves
    bool useHistoryHeuristic = true;      ///< Whether to use History Heuristic
    bool useMobilityOrdering = true;       ///< Whether to use mobility ordering
    int killerWeight = 8;                 ///< Killer weight (based on Egaroucid)
    int historyWeight = 6;                 ///< History weight (based on Egaroucid)
    int mobilityWeight = 35;               ///< Mobility weight (based on Egaroucid)
    double decayFactor = 0.99;             ///< Decay factor
};

/**
 * @brief Move Orderer
 *
 * Unified management of all move ordering strategies.
 *
 * Design highlights:
 * - Integrates KillerTable and HistoryTable
 * - Supports combined ordering strategies
 * - Provides statistics
 */
class MoveOrderer {
public:
    /**
     * @brief Constructor
     * @param config Configuration
     * @param evaluator Evaluator
     */
    explicit MoveOrderer(MoveOrdererConfig config = MoveOrdererConfig(),
                        std::unique_ptr<Evaluator> evaluator = nullptr);

    /**
     * @brief Main ordering function
     *
     * Orders moves based on current search state.
     *
     * @param board Current board
     * @param depth Current search depth
     * @param moves Moves to order
     * @param pvMove PV move (optional)
     * @param prevMove Previous move (for Counter Move)
     * @return Ordered moves
     */
    std::vector<Move> orderMoves(const Board& board, int depth,
                                const std::vector<Move>& moves,
                                const Move& pvMove = Move(), const Move& prevMove = Move());

    /**
     * @brief Static ordering (fast)
     *
     * Uses only Killer and History, no depth evaluation.
     *
     * @param depth Current search depth
     * @param moves Moves to order
     * @param pvMove PV move
     * @param prevMove Previous move
     * @return Ordered moves
     */
    std::vector<Move> orderMovesStatic(int depth,
                                      const std::vector<Move>& moves,
                                      const Move& pvMove = Move(), const Move& prevMove = Move());

    /**
     * @brief Add Killer move
     * Called after beta cutoff.
     *
     * @param depth Search depth
     * @param move Move
     */
    void addKiller(int depth, int move);

    /**
     * @brief Add History score
     * Called after beta cutoff.
     *
     * @param from Start position
     * @param to Target position
     * @param depth Search depth
     */
    void addHistory(int from, int to, int depth);

    /**
     * @brief Record cutoff move
     * Updates both Killer and History.
     *
     * @param from Start position
     * @param to Target position
     * @param depth Search depth
     * @param isBetaCutoff Whether beta cutoff occurred
     */
    void recordCutoff(int from, int to, int depth, bool isBetaCutoff = true);

    /**
     * @brief Clear all ordering data
     */
    void clear();

    /**
     * @brief Apply decay
     * Called before each search to prevent stale data.
     */
    void decay();

    /**
     * @brief Get configuration
     */
    const MoveOrdererConfig& getConfig() const { return config_; }

    /**
     * @brief Set configuration
     */
    void setConfig(const MoveOrdererConfig& config) { config_ = config; }

    /**
     * @brief Get Killer table
     */
    const KillerTable& getKillerTable() const { return killerTable_; }

    /**
     * @brief Get History table
     */
    const HistoryTable& getHistoryTable() const { return historyTable_; }

    /**
     * @brief Get statistics
     */
    struct Statistics {
        int killerHits;       ///< Killer hit count
        int historyHits;      ///< History hit count
        int pvHits;           ///< PV move hit count
        int totalMoves;       ///< Total moves ordered
        int movesWithKiller;  ///< Moves containing killer
        int movesWithHistory; ///< Moves containing history
    };
    Statistics getStatistics() const;

    /**
     * @brief Reset statistics
     */
    void resetStatistics();

private:
    /**
     * @brief Calculate move score
     *
     * @param move Move
     * @param depth Search depth
     * @param pvMove PV move
     * @param prevMove Previous move
     * @return Ordering score
     */
    int calculateMoveScore(const Move& move, int depth, const Move& pvMove, const Move& prevMove) const;

    /**
     * @brief Calculate mobility score
     *
     * @param board Board
     * @param move Move
     * @return Mobility score
     */
    int calculateMobilityScore(const Board& board, const Move& move) const;

    // Configuration
    MoveOrdererConfig config_;

    // Components
    KillerTable killerTable_;
    HistoryTable historyTable_;
    std::unique_ptr<Evaluator> evaluator_;

    // Statistics
    mutable Statistics stats_;
};

} // namespace Reversi
