/**
 * @file SearchStats.h
 * @brief Search Statistics Data Structure - v0.9.0 Visualization Enhanced
 *
 * Used to collect statistics during AI search for UI visualization
 */

#pragma once

#include <vector>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <string>

namespace Reversi {

/**
 * @brief Search tree node info
 */
struct SearchTreeNodeInfo {
    int move;                    // Move (0-63)
    int depth;                   // Depth
    int value;                   // Evaluation value
    int nodeCount;               // Number of child nodes
    bool isPV;                   // Whether PV (Principal Variation) line
    int64_t nodesSearched;       // Number of searched nodes
    std::vector<int> childMoves; // Child move list

    SearchTreeNodeInfo() : move(-1), depth(0), value(0), nodeCount(0),
                          isPV(false), nodesSearched(0) {}
};

/**
 * @brief Real-time search statistics
 */
struct SearchStats {
    // Basic statistics
    int currentDepth = 0;                    // Current search depth
    int maxDepth = 0;                        // Maximum depth
    int64_t nodesExplored = 0;              // Explored nodes
    int64_t nodesPerSecond = 0;            // Nodes per second
    double timeElapsed = 0.0;                // Elapsed time (seconds)

    // Pruning and transposition table statistics
    int cutoffs = 0;                         // Number of cutoffs
    double ttHitRate = 0.0;                 // Transposition table hit rate
    double killerHitRate = 0.0;             // Killer move hit rate
    double historyHitRate = 0.0;            // History heuristic hit rate
    int64_t ttEntries = 0;                  // Transposition table entries

    // Search tree info
    std::vector<SearchTreeNodeInfo> pvLine; // PV line (Principal Variation)
    int totalTreeNodes = 0;                 // Total tree nodes

    // MCTS specific statistics (only used by MCTS)
    int64_t simulations = 0;                // Number of simulations
    double winRate = 0.0;                   // Win rate estimate

    // Best move info
    int bestMove = -1;                      // Current best move
    int bestValue = 0;                      // Best move evaluation value

    // Search state
    bool isSearching = false;               // Whether searching
    bool isComplete = false;                // Whether search complete

    /**
     * @brief Reset all statistics
     */
    void reset() {
        currentDepth = 0;
        maxDepth = 0;
        nodesExplored = 0;
        nodesPerSecond = 0;
        timeElapsed = 0.0;
        cutoffs = 0;
        ttHitRate = 0.0;
        killerHitRate = 0.0;
        historyHitRate = 0.0;
        ttEntries = 0;
        pvLine.clear();
        totalTreeNodes = 0;
        simulations = 0;
        winRate = 0.0;
        bestMove = -1;
        bestValue = 0;
        isSearching = false;
        isComplete = false;
    }

    /**
     * @brief Get NPS (Nodes Per Second) formatted string
     */
    std::string getNPSString() const {
        if (nodesPerSecond >= 1000000) {
            return std::to_string(nodesPerSecond / 1000000) + "M";
        } else if (nodesPerSecond >= 1000) {
            return std::to_string(nodesPerSecond / 1000) + "K";
        }
        return std::to_string(nodesPerSecond);
    }

    /**
     * @brief Get time formatted string
     */
    std::string getTimeString() const {
        int minutes = static_cast<int>(timeElapsed) / 60;
        int seconds = static_cast<int>(timeElapsed) % 60;
        int ms = static_cast<int>((timeElapsed - std::floor(timeElapsed)) * 1000);

        if (minutes > 0) {
            return std::to_string(minutes) + ":" +
                   (seconds < 10 ? "0" : "") + std::to_string(seconds);
        }
        return std::to_string(seconds) + "." +
               (ms < 100 ? "0" : "") + (ms < 10 ? "0" : "") + std::to_string(ms);
    }
};

/**
 * @brief Heatmap data type
 */
enum class HeatmapType {
    PositionValue,     // Static position value
    VisitCount,        // Visit count (MCTS)
    WinRate,           // Win rate
    ActionValue        // Action evaluation value
};

/**
 * @brief Heatmap data
 */
struct HeatmapData {
    std::vector<double> values;  // Values for 64 cells (0.0 - 1.0)
    HeatmapType type;            // Data type
    bool isValid = false;        // Whether data is valid

    HeatmapData() : type(HeatmapType::PositionValue), isValid(false) {
        values.resize(64, 0.0);
    }

    void clear() {
        std::fill(values.begin(), values.end(), 0.0);
        isValid = false;
    }

    bool isValidIndex(int index) const {
        return index >= 0 && index < 64 && isValid;
    }
};

} // namespace Reversi
