/**
 * @file HistoryTable.h
 * @brief History Heuristic implementation
 *
 * Records how often each move caused cutoffs in the search tree,
 * used for global move ordering.
 *
 * Reference: Egaroucid/src/engine/move_ordering.hpp
 *           edax-reversi/src/play.c
 * v0.7.0 - AI algorithm optimization version
 */

#pragma once

#include <vector>
#include <array>
#include <algorithm>
#include <cstdint>

namespace Reversi {

/**
 * @brief History Table
 *
 * Records how often each move caused beta cutoffs in the search history,
 * used for global move ordering.
 *
 * Design highlights:
 * - 64x64 matrix storing all possible moves
 * - Score increases each time a move causes cutoff
 * - Decay applied before each search to prevent stale data
 * - Indexed by from/to position
 */
class HistoryTable {
public:
    static constexpr int NUM_SQUARES = 64;       ///< Number of board squares
    static constexpr int MAX_HISTORY_SCORE = 1000000;  ///< Maximum history score
    static constexpr double DEFAULT_DECAY = 0.99;  ///< Default decay factor

public:
    /**
     * @brief Constructor
     */
    HistoryTable();

    /**
     * @brief Add history score for a move
     *
     * Called when a move causes cutoff, increases its history score.
     * Uses depth weighting: deeper search = more important history.
     *
     * @param from Start position (0-63)
     * @param to Target position (0-63)
     * @param depth Search depth
     * @param isCutoff Whether this caused cutoff (default true)
     */
    void addHistory(int from, int to, int depth, bool isCutoff = true);

    /**
     * @brief Get history score for a move
     * @param from Start position
     * @param to Target position
     * @return History score
     */
    int getHistoryScore(int from, int to) const;

    /**
     * @brief Get sorted move list
     * @param moves Original move list (format: from * 64 + to)
     * @return Sorted move list
     */
    std::vector<int> getSortedMoves(const std::vector<int>& moves) const;

    /**
     * @brief Clear history table
     */
    void clear();

    /**
     * @brief Decay all history scores
     * Prevents stale history data from dominating
     * @param factor Decay factor (0.0-1.0)
     */
    void decay(double factor = DEFAULT_DECAY);

    /**
     * @brief Get statistics
     */
    struct Statistics {
        int maxScore;           ///< Maximum history score
        int minScore;           ///< Minimum non-zero score
        int totalEntries;       ///< Number of non-zero entries
        double averageScore;    ///< Average score
    };
    Statistics getStatistics() const;

    /**
     * @brief Normalize all history scores
     * Prevents score overflow
     */
    void normalize();

private:
    /**
     * @brief Check if position is valid
     * @param pos Position (0-63)
     * @return true if valid
     */
    bool isValidPosition(int pos) const;

    // History matrix: [from][to] = history score
    std::array<std::array<int, NUM_SQUARES>, NUM_SQUARES> history_;
};

} // namespace Reversi
