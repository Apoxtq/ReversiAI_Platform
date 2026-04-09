/**
 * @file KillerTable.cpp
 * @brief Killer Moves implementation
 */

#include "ai/KillerTable.h"
#include <algorithm>
#include <numeric>

namespace Reversi {

KillerTable::KillerTable() {
    clear();
}

void KillerTable::addKiller(int depth, int move, int score) {
    if (depth < 0 || depth >= MAX_DEPTH || move < 0 || move >= 64) {
        return;
    }

    int existingIdx = findExistingIndex(depth, move);
    if (existingIdx >= 0) {
        killers_[depth][existingIdx].score += score;
        return;
    }

    int lowestIdx = findLowestScoreIndex(depth);

    if (score > killers_[depth][lowestIdx].score) {
        killers_[depth][lowestIdx] = KillerMove(move, score, depth);
    }
}

std::vector<int> KillerTable::getKillers(int depth) const {
    std::vector<int> result;

    if (depth < 0 || depth >= MAX_DEPTH) {
        return result;
    }

    for (int i = 0; i < MAX_KILLER_COUNT; ++i) {
        if (killers_[depth][i].isValid()) {
            result.push_back(killers_[depth][i].move);
        }
    }

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
    if (factor < 0.0 || factor > 1.0) {
        factor = 0.99;
    }

    for (int d = 0; d < MAX_DEPTH; ++d) {
        for (int i = 0; i < MAX_KILLER_COUNT; ++i) {
            if (killers_[d][i].isValid()) {
                killers_[d][i].score = static_cast<int>(killers_[d][i].score * factor);
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
