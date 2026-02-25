/**
 * @file KillerTable.cpp
 * @brief Killer Moves 实现
 *
 * @see KillerTable.h
 * 参考: Egaroucid/src/engine/move_ordering.hpp
 */

#include "ai/KillerTable.h"
#include <algorithm>
#include <numeric>

namespace Reversi {

KillerTable::KillerTable() {
    clear();
}

void KillerTable::addKiller(int depth, int move, int score) {
    // 边界检查
    if (depth < 0 || depth >= MAX_DEPTH || move < 0 || move >= 64) {
        return;
    }

    // 检查是否已存在
    int existingIdx = findExistingIndex(depth, move);
    if (existingIdx >= 0) {
        // 已存在，增加分数
        killers_[depth][existingIdx].score += score;
        return;
    }

    // 找到最低分数位置
    int lowestIdx = findLowestScoreIndex(depth);

    // 只有当新分数更高时才替换
    if (score > killers_[depth][lowestIdx].score) {
        killers_[depth][lowestIdx] = KillerMove(move, score, depth);
    }
}

std::vector<int> KillerTable::getKillers(int depth) const {
    std::vector<int> result;

    if (depth < 0 || depth >= MAX_DEPTH) {
        return result;
    }

    // 收集有效的杀手走法
    for (int i = 0; i < MAX_KILLER_COUNT; ++i) {
        if (killers_[depth][i].isValid()) {
            result.push_back(killers_[depth][i].move);
        }
    }

    // 按分数降序排序
    std::sort(result.begin(), result.end(), [this, depth](int a, int b) {
        return getKillerScore(depth, a) > getKillerScore(depth, b);
    });

    return result;
}

bool KillerTable::isKiller(int depth, int move) const {
    if (depth < 0 || depth >= MAX_DEPTH || move < 0 || move >= 64) {
        return false;
    }

    for (int i = 0; i < MAX_KILLER_COUNT; ++i) {
        if (killers_[depth][i].move == move && killers_[depth][i].score > 0) {
            return true;
        }
    }
    return false;
}

int KillerTable::getKillerScore(int depth, int move) const {
    if (depth < 0 || depth >= MAX_DEPTH || move < 0 || move >= 64) {
        return 0;
    }

    for (int i = 0; i < MAX_KILLER_COUNT; ++i) {
        if (killers_[depth][i].move == move) {
            return killers_[depth][i].score;
        }
    }
    return 0;
}

void KillerTable::clear() {
    for (int d = 0; d < MAX_DEPTH; ++d) {
        for (int i = 0; i < MAX_KILLER_COUNT; ++i) {
            killers_[d][i] = KillerMove();
        }
    }
}

void KillerTable::decay(double factor) {
    // 防止factor超出有效范围
    if (factor < 0.0 || factor > 1.0) {
        factor = 0.99;
    }

    for (int d = 0; d < MAX_DEPTH; ++d) {
        for (int i = 0; i < MAX_KILLER_COUNT; ++i) {
            if (killers_[d][i].isValid()) {
                killers_[d][i].score = static_cast<int>(killers_[d][i].score * factor);
                // 如果分数太低，清除
                if (killers_[d][i].score <= 0) {
                    killers_[d][i] = KillerMove();
                }
            }
        }
    }
}

KillerTable::Statistics KillerTable::getStatistics() const {
    Statistics stats{};
    stats.totalKillers = 0;

    for (int d = 0; d < MAX_DEPTH; ++d) {
        stats.depthDistribution[d] = 0;
        for (int i = 0; i < MAX_KILLER_COUNT; ++i) {
            if (killers_[d][i].isValid() && killers_[d][i].score > 0) {
                stats.totalKillers++;
                stats.depthDistribution[d]++;
            }
        }
    }

    return stats;
}

int KillerTable::findLowestScoreIndex(int depth) const {
    int lowestIdx = 0;
    int lowestScore = killers_[depth][0].score;

    for (int i = 1; i < MAX_KILLER_COUNT; ++i) {
        if (killers_[depth][i].score < lowestScore) {
            lowestScore = killers_[depth][i].score;
            lowestIdx = i;
        }
    }

    return lowestIdx;
}

int KillerTable::findExistingIndex(int depth, int move) const {
    for (int i = 0; i < MAX_KILLER_COUNT; ++i) {
        if (killers_[depth][i].move == move) {
            return i;
        }
    }
    return -1;
}

} // namespace Reversi
