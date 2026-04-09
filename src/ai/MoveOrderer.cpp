/**
 * @file MoveOrderer.cpp
 * @brief Move Ordering implementation
 */

#include "ai/MoveOrderer.h"
#include "Board.h"
#include <algorithm>
#include <numeric>

namespace Reversi {

constexpr int PV_MOVE_BONUS = 10000000;
constexpr int KILLER_BONUS = 1000000;
constexpr int HISTORY_BONUS = 100000;

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

    std::vector<std::pair<Move, int>> scoredMoves;
    scoredMoves.reserve(moves.size());

    bool hasKiller = false;
    bool hasHistory = false;

    for (const Move& move : moves) {
        int score = calculateMoveScore(move, depth, pvMove, prevMove);

        if (config_.useMobilityOrdering) {
            score += calculateMobilityScore(board, move);
        }

        scoredMoves.emplace_back(move, score);

        if (config_.useKillerMoves) {
            int movePos = move.row * 8 + move.col;
            if (killerTable_.isKiller(depth, movePos)) {
                hasKiller = true;
            }
        }
        if (config_.useHistoryHeuristic) {
            int from = prevMove.isValid() ? (prevMove.row * 8 + prevMove.col) : 0;
            int to = move.row * 8 + move.col;
            if (historyTable_.getHistoryScore(from, to) > 0) {
                hasHistory = true;
            }
        }
    }

    if (hasKiller) stats_.movesWithKiller++;
    if (hasHistory) stats_.movesWithHistory++;

    if (pvMove.isValid()) {
        for (const auto& pair : scoredMoves) {
            if (pair.first == pvMove) {
                stats_.pvHits++;
                break;
            }
        }
    }

    std::sort(scoredMoves.begin(), scoredMoves.end(),
        [](const std::pair<Move, int>& a, const std::pair<Move, int>& b) {
            return a.second > b.second;
        });

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

    std::vector<std::pair<Move, int>> scoredMoves;
    scoredMoves.reserve(moves.size());

    for (const Move& move : moves) {
        int score = calculateMoveScore(move, depth, pvMove, prevMove);
        scoredMoves.emplace_back(move, score);
    }

    std::sort(scoredMoves.begin(), scoredMoves.end(),
        [](const std::pair<Move, int>& a, const std::pair<Move, int>& b) {
            return a.second > b.second;
        });

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

    if (config_.useKillerMoves) {
        addKiller(depth, to);
        stats_.killerHits++;
    }

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

    if (pvMove.isValid() && move == pvMove) {
        score += PV_MOVE_BONUS;
    }

    if (config_.useKillerMoves) {
        int killerScore = killerTable_.getKillerScore(depth, movePos);
        if (killerScore > 0) {
            score += killerScore * config_.killerWeight;
        }
    }

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
    Board tempBoard = board;

    if (!tempBoard.makeMove(move)) {
        return 0;
    }

    if (tempBoard.isGameOver()) {
        return 10000;
    }

    uint64_t opponentMoves = tempBoard.getBitBoard().getValidMoves(tempBoard.getCurrentTurn());

    int mobility;
#if defined(_MSC_VER)
    mobility = __popcnt64(opponentMoves);
#else
    mobility = __builtin_popcountll(opponentMoves);
#endif

    return -mobility * config_.mobilityWeight;
}

} // namespace Reversi
