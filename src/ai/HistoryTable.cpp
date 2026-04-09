/**
 * @file HistoryTable.cpp
 * @brief History Heuristic implementation
 */

#include "ai/HistoryTable.h"
#include <algorithm>
#include <numeric>
#include <limits>

namespace Reversi {

HistoryTable::HistoryTable() {
    clear();
}

void HistoryTable::addHistory(int from, int to, int depth, bool isCutoff) {
    if (!isValidPosition(from) || !isValidPosition(to)) {
        return;
    }

    if (!isCutoff) {
        return;
    }

    int weight = depth * depth;
    history_[from][to] += weight;

    if (history_[from][to] > MAX_HISTORY_SCORE) {
        normalize();
    }
}

int HistoryTable::getHistoryScore(int from, int to) const {
    if (!isValidPosition(from) || !isValidPosition(to)) {
        return 0;
    }
    return history_[from][to];
}

std::vector<int> HistoryTable::getSortedMoves(const std::vector<int>& moves) const {
    std::vector<std::pair<int, int>> scoredMoves;
    scoredMoves.reserve(moves.size());

    for (int move : moves) {
        int from = move / NUM_SQUARES;
        int to = move % NUM_SQUARES;
        int score = getHistoryScore(from, to);
        scoredMoves.emplace_back(move, score);
    }

    std::sort(scoredMoves.begin(), scoredMoves.end(),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.second > b.second;
        });

    std::vector<int> result;
    result.reserve(moves.size());
    for (const auto& pair : scoredMoves) {
        result.push_back(pair.first);
    }

    return result;
}

void HistoryTable::clear() {
    for (int from = 0; from < NUM_SQUARES; ++from) {
        for (int to = 0; to < NUM_SQUARES; ++to) {
            history_[from][to] = 0;
        }
    }
}

void HistoryTable::decay(double factor) {
    if (factor < 0.0 || factor > 1.0) {
        factor = DEFAULT_DECAY;
    }

    for (int from = 0; from < NUM_SQUARES; ++from) {
        for (int to = 0; to < NUM_SQUARES; ++to) {
            if (history_[from][to] > 0) {
                history_[from][to] = static_cast<int>(history_[from][to] * factor);
                if (history_[from][to] <= 0) {
                    history_[from][to] = 0;
                }
            }
        }
    }
}

HistoryTable::Statistics HistoryTable::getStatistics() const {
    Statistics stats{};
    stats.maxScore = 0;
    stats.minScore = std::numeric_limits<int>::max();
    stats.totalEntries = 0;
    int totalScore = 0;

    for (int from = 0; from < NUM_SQUARES; ++from) {
        for (int to = 0; to < NUM_SQUARES; ++to) {
            int score = history_[from][to];
            if (score > 0) {
                stats.totalEntries++;
                totalScore += score;
                stats.maxScore = std::max(stats.maxScore, score);
                stats.minScore = std::min(stats.minScore, score);
            }
        }
    }

    if (stats.totalEntries > 0) {
        stats.averageScore = static_cast<double>(totalScore) / stats.totalEntries;
    } else {
        stats.minScore = 0;
        stats.averageScore = 0.0;
    }

    return stats;
}

void HistoryTable::normalize() {
    int currentMax = 0;
    for (int from = 0; from < NUM_SQUARES; ++from) {
        for (int to = 0; to < NUM_SQUARES; ++to) {
            currentMax = std::max(currentMax, history_[from][to]);
        }
    }

    if (currentMax > MAX_HISTORY_SCORE) {
        double scale = static_cast<double>(MAX_HISTORY_SCORE) / currentMax;
        for (int from = 0; from < NUM_SQUARES; ++from) {
            for (int to = 0; to < NUM_SQUARES; ++to) {
                history_[from][to] = static_cast<int>(history_[from][to] * scale);
            }
        }
    } else if (currentMax > MAX_HISTORY_SCORE / 2) {
        double scale = 0.5;
        for (int from = 0; from < NUM_SQUARES; ++from) {
            for (int to = 0; to < NUM_SQUARES; ++to) {
                history_[from][to] = static_cast<int>(history_[from][to] * scale);
            }
        }
    }
}

bool HistoryTable::isValidPosition(int pos) const {
    return pos >= 0 && pos < NUM_SQUARES;
}

} // namespace Reversi
