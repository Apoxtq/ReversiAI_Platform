/**
 * @file KillerTable.h
 * @brief Killer Moves implementation
 *
 * Records moves that caused beta cutoffs in the search tree.
 * These moves are tried first at the same depth.
 *
 * Reference: Egaroucid/src/engine/move_ordering.hpp
 * v0.7.0 - AI algorithm optimization version
 */

#pragma once

#include <vector>
#include <array>
#include <algorithm>

namespace Reversi {

/**
 * @brief Killer Move structure
 *
 * Records information about a move that caused cutoff
 */
struct KillerMove {
    int move;          ///< Move position (0-63)
    int score;         ///< Killer coefficient (cutoff count)
    int depth;         ///< Search depth

    KillerMove() : move(-1), score(0), depth(0) {}
    KillerMove(int m, int s, int d) : move(m), score(s), depth(d) {}

    bool isValid() const { return move >= 0 && move < 64; }
};

/**
 * @brief Killer Table
 *
 * Maintains best killer moves for each search depth.
 *
 * Design highlights:
 * - Each depth maintains MAX_KILLER_COUNT best killer moves
 * - Uses count mechanism to avoid duplicate entries
 * - Killer info from previous layer preserved during iterative deepening
 */
class KillerTable {
public:
    static constexpr int MAX_KILLER_COUNT = 2;  ///< Max killer moves per depth
    static constexpr int MAX_DEPTH = 64;         ///< Max search depth
    static constexpr int INITIAL_SCORE = 1;      ///< Initial score

public:
    /**
     * @brief Constructor
     */
    KillerTable();

    /**
     * @brief Add killer move
     * @param depth Search depth
     * @param move Move position
     * @param score Killer coefficient (default 1)
     */
    void addKiller(int depth, int move, int score = INITIAL_SCORE);

    /**
     * @brief Get killer moves for a depth
     * @param depth Search depth
     * @return Vector of killer moves
     */
    std::vector<int> getKillers(int depth) const;

    /**
     * @brief Check if a move is a killer
     * @param depth Search depth
     * @param move Move position
     * @return true if killer move
     */
    bool isKiller(int depth, int move) const;

    /**
     * @brief Get killer score for a move
     * @param depth Search depth
     * @param move Move position
     * @return Killer score (0 if not a killer move)
     */
    int getKillerScore(int depth, int move) const;

    /**
     * @brief Clear the table
     */
    void clear();

    /**
     * @brief Decay all killer scores
     * Prevents stale data from dominating long searches
     * @param factor Decay factor (0.0-1.0)
     */
    void decay(double factor = 0.99);

    /**
     * @brief Get statistics
     */
    struct Statistics {
        int totalKillers;       ///< Total killer moves
        int depthDistribution[MAX_DEPTH];  ///< Killer count per depth
    };
    Statistics getStatistics() const;

private:
    /**
     * @brief Find index with lowest score
     * @param depth Search depth
     * @return Index of lowest score position
     */
    int findLowestScoreIndex(int depth) const;

    /**
     * @brief Check if move already exists
     * @param depth Search depth
     * @param move Move position
     * @return Existing index, -1 if not found
     */
    int findExistingIndex(int depth, int move) const;

    // Killer table: [depth][count]
    std::array<std::array<KillerMove, MAX_KILLER_COUNT>, MAX_DEPTH> killers_;
};

} // namespace Reversi
