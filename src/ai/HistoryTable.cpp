/**
 * @file HistoryTable.cpp
 * @brief History Heuristic 实现
 *
 * @see HistoryTable.h
 * 参考: Egaroucid/src/engine/move_ordering.hpp
 * 参考: edax-reversi/src/play.c
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
    // 边界检查
    if (!isValidPosition(from) || !isValidPosition(to)) {
        return;
    }

    if (!isCutoff) {
        return;  // 只有导致剪枝的走法才记录
    }

    // 深度加权：越深的搜索历史越重要
    // 使用depth * depth作为权重
    int weight = depth * depth;
    history_[from][to] += weight;

    // 防止溢出
    if (history_[from][to] > MAX_HISTORY_SCORE) {
        // 严重溢出时减半
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
    // 创建带分数的副本
    std::vector<std::pair<int, int>> scoredMoves;
    scoredMoves.reserve(moves.size());

    for (int move : moves) {
        int from = move / NUM_SQUARES;
        int to = move % NUM_SQUARES;
        int score = getHistoryScore(from, to);
        scoredMoves.emplace_back(move, score);
    }

    // 按历史分数降序排序
    std::sort(scoredMoves.begin(), scoredMoves.end(),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.second > b.second;
        });

    // 提取排序后的走法
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
    // 防止factor超出有效范围
    if (factor < 0.0 || factor > 1.0) {
        factor = DEFAULT_DECAY;
    }

    for (int from = 0; from < NUM_SQUARES; ++from) {
        for (int to = 0; to < NUM_SQUARES; ++to) {
            if (history_[from][to] > 0) {
                history_[from][to] = static_cast<int>(history_[from][to] * factor);
                // 如果分数太低，清零
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
    // 如果最高分超过最大值，进行缩放
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
        // 即使没有溢出，如果超过一半也进行衰减
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
