/**
 * @file MoveOrderer.cpp
 * @brief Move Ordering 实现
 *
 * @see MoveOrderer.h
 * 参考: Egaroucid/src/engine/move_ordering.hpp
 */

#include "ai/MoveOrderer.h"
#include "Board.h"
#include <algorithm>
#include <numeric>

namespace Reversi {

// 走法排序优先级常量 (基于Egaroucid)
constexpr int PV_MOVE_BONUS = 10000000;    // PV走法优先级
constexpr int KILLER_BONUS = 1000000;       // Killer优先级
constexpr int HISTORY_BONUS = 100000;       // History优先级

MoveOrderer::MoveOrderer(MoveOrdererConfig config, std::unique_ptr<Evaluator> evaluator)
    : config_(config),
      evaluator_(evaluator ? std::move(evaluator) : EvaluatorFactory::createStaticEvaluator()) {
    resetStatistics();
}

std::vector<Move> MoveOrderer::orderMoves(const Board& board, int depth,
                                        const std::vector<Move>& moves,
                                        const Move& pvMove, const Move& prevMove) {
    if (moves.empty()) {
        return moves;
    }

    stats_.totalMoves += moves.size();

    // 创建带分数的走法列表
    std::vector<std::pair<Move, int>> scoredMoves;
    scoredMoves.reserve(moves.size());

    bool hasKiller = false;
    bool hasHistory = false;

    for (const Move& move : moves) {
        int score = calculateMoveScore(move, depth, pvMove, prevMove);

        // 添加灵活度得分
        if (config_.useMobilityOrdering) {
            score += calculateMobilityScore(board, move);
        }

        scoredMoves.emplace_back(move, score);

        // 统计
        if (config_.useKillerMoves) {
            int movePos = move.row * 8 + move.col;
            if (killerTable_.isKiller(depth, movePos)) {
                hasKiller = true;
            }
        }
        if (config_.useHistoryHeuristic) {
            // 简化：使用上一手作为from
            int from = prevMove.isValid() ? (prevMove.row * 8 + prevMove.col) : 0;
            int to = move.row * 8 + move.col;
            if (historyTable_.getHistoryScore(from, to) > 0) {
                hasHistory = true;
            }
        }
    }

    if (hasKiller) stats_.movesWithKiller++;
    if (hasHistory) stats_.movesWithHistory++;

    // 统计PV命中
    if (pvMove.isValid()) {
        for (const auto& pair : scoredMoves) {
            if (pair.first == pvMove) {
                stats_.pvHits++;
                break;
            }
        }
    }

    // 按分数降序排序
    std::sort(scoredMoves.begin(), scoredMoves.end(),
        [](const std::pair<Move, int>& a, const std::pair<Move, int>& b) {
            return a.second > b.second;
        });

    // 提取排序后的走法
    std::vector<Move> result;
    result.reserve(moves.size());
    for (const auto& pair : scoredMoves) {
        result.push_back(pair.first);
    }

    return result;
}

std::vector<Move> MoveOrderer::orderMovesStatic(int depth,
                                               const std::vector<Move>& moves,
                                               const Move& pvMove, const Move& prevMove) {
    if (moves.empty()) {
        return moves;
    }

    stats_.totalMoves += moves.size();

    // 创建带分数的走法列表
    std::vector<std::pair<Move, int>> scoredMoves;
    scoredMoves.reserve(moves.size());

    for (const Move& move : moves) {
        int score = calculateMoveScore(move, depth, pvMove, prevMove);
        scoredMoves.emplace_back(move, score);
    }

    // 按分数降序排序
    std::sort(scoredMoves.begin(), scoredMoves.end(),
        [](const std::pair<Move, int>& a, const std::pair<Move, int>& b) {
            return a.second > b.second;
        });

    // 提取排序后的走法
    std::vector<Move> result;
    result.reserve(moves.size());
    for (const auto& pair : scoredMoves) {
        result.push_back(pair.first);
    }

    return result;
}

void MoveOrderer::addKiller(int depth, int move) {
    if (config_.useKillerMoves && move >= 0 && move < 64) {
        killerTable_.addKiller(depth, move, 1);
    }
}

void MoveOrderer::addHistory(int from, int to, int depth) {
    if (config_.useHistoryHeuristic) {
        historyTable_.addHistory(from, to, depth, true);
    }
}

void MoveOrderer::recordCutoff(int from, int to, int depth, bool isBetaCutoff) {
    if (!isBetaCutoff) {
        return;
    }

    // 更新Killer表
    if (config_.useKillerMoves) {
        addKiller(depth, to);
        stats_.killerHits++;
    }

    // 更新History表
    if (config_.useHistoryHeuristic) {
        addHistory(from, to, depth);
        stats_.historyHits++;
    }
}

void MoveOrderer::clear() {
    killerTable_.clear();
    historyTable_.clear();
}

void MoveOrderer::decay() {
    if (config_.useKillerMoves) {
        killerTable_.decay(config_.decayFactor);
    }
    if (config_.useHistoryHeuristic) {
        historyTable_.decay(config_.decayFactor);
    }
}

MoveOrderer::Statistics MoveOrderer::getStatistics() const {
    return stats_;
}

void MoveOrderer::resetStatistics() {
    stats_ = Statistics{};
}

int MoveOrderer::calculateMoveScore(const Move& move, int depth, const Move& pvMove, const Move& prevMove) const {
    int score = 0;
    int movePos = move.row * 8 + move.col;

    // 1. PV走法优先级最高
    if (pvMove.isValid() && move == pvMove) {
        score += PV_MOVE_BONUS;
    }

    // 2. Killer Moves
    if (config_.useKillerMoves) {
        int killerScore = killerTable_.getKillerScore(depth, movePos);
        if (killerScore > 0) {
            score += killerScore * config_.killerWeight;
        }
    }

    // 3. History Heuristic
    if (config_.useHistoryHeuristic) {
        int from = prevMove.isValid() ? (prevMove.row * 8 + prevMove.col) : 0;
        int to = movePos;
        int historyScore = historyTable_.getHistoryScore(from, to);
        if (historyScore > 0) {
            score += historyScore * config_.historyWeight;
        }
    }

    return score;
}

int MoveOrderer::calculateMobilityScore(const Board& board, const Move& move) const {
    // 灵活度评估：评估执行该走法后，对手的可选走法数量
    // 对手可选走法越少，我们的走法越好
    
    // 创建一个临时棋盘来模拟走法
    Board tempBoard = board;
    
    // 执行走法
    if (!tempBoard.makeMove(move)) {
        return 0;  // 无效走法
    }
    
    // 检查游戏是否结束
    if (tempBoard.isGameOver()) {
        // 游戏结束，奖励高分数
        return 10000;
    }
    
    // 获取对手的合法走法数量
    uint64_t opponentMoves = tempBoard.getBitBoard().getValidMoves(tempBoard.getCurrentTurn());
    
    // 计算对手合法走法数量（使用popcount）
    int mobility;
    #if defined(_MSC_VER)
        mobility = __popcnt64(opponentMoves);
    #else
        mobility = __builtin_popcountll(opponentMoves);
    #endif
    
    // 灵活度得分 = 对手可选走法越少越好
    // 范围：0-60 (最多60个合法位置)
    // 使用负值，因为走法越少越好
    return -mobility * config_.mobilityWeight;
}

} // namespace Reversi
