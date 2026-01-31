#include "ai/MinimaxAI.h"
#include "Board.h"
#include <algorithm>
#include <limits>
#include <iostream>

/**
 * @file MinimaxAI.cpp
 * @brief Minimax算法实现
 *
 * 将Reversi(Java)的Minimax算法转换为C++ BitBoard实现
 */

namespace Reversi {

MinimaxAI::MinimaxAI(MinimaxConfig config, std::unique_ptr<Evaluator> evaluator)
    : config_(config),
      evaluator_(evaluator ? std::move(evaluator) : EvaluatorFactory::createStaticEvaluator()) {
}

Move MinimaxAI::findBestMove(const Board& board, const SearchLimits& limits) {
    // 重置统计信息
    stats_ = AIStats{};
    config_.nodesExplored = 0;
    config_.cutoffs = 0;
    bestMove_ = Move();
    bestScore_ = std::numeric_limits<int>::min();
    lastCompletedDepth_ = 0;

    searchStartTime_ = std::chrono::steady_clock::now();

    // 获取有效移动
    auto validMoves = board.getValidMoves();
    if (validMoves.empty()) {
        return Move::pass();  // 无有效移动，跳过回合
    }

    // 确定搜索深度
    int searchDepth = std::min(config_.maxDepth, limits.maxDepth.value_or(config_.maxDepth));

    if (config_.useIterativeDeepening) {
        // 迭代深化搜索
        for (int depth = 1; depth <= searchDepth; ++depth) {
            if (shouldStop(limits, searchStartTime_)) {
                break;
            }

            // 在当前深度搜索
            int alpha = std::numeric_limits<int>::min();
            int beta = std::numeric_limits<int>::max();

            minimaxAlphaBeta(board, board.getCurrentTurn(), depth, true, alpha, beta, limits);
            lastCompletedDepth_ = depth;
        }
    } else {
        // 固定深度搜索
        int alpha = std::numeric_limits<int>::min();
        int beta = std::numeric_limits<int>::max();

        minimaxAlphaBeta(board, board.getCurrentTurn(), searchDepth, true, alpha, beta, limits);
        lastCompletedDepth_ = searchDepth;
    }

    // 更新统计信息
    auto endTime = std::chrono::steady_clock::now();
    stats_.timeUsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - searchStartTime_);
    stats_.nodesExplored = config_.nodesExplored;
    stats_.evaluationCount = config_.nodesExplored;  // 简化处理
    stats_.avgBranching = validMoves.size();  // 简化计算

    return bestMove_.isValid() ? bestMove_ : validMoves[0];  // 返回最佳移动或第一个有效移动
}

int MinimaxAI::minimaxAlphaBeta(const Board& board, PlayerColor currentPlayer, int depth,
                               bool isMaximizing, int alpha, int beta, const SearchLimits& limits) {
    config_.nodesExplored++;

    // 检查是否应该终止搜索（时间或深度限制）
    if (shouldStop(limits, searchStartTime_) || depth == 0) {
        // 到达叶子节点，使用评估函数
        return evaluator_->evaluate(board.getBitBoard(), currentPlayer);
    }

    // 检查游戏是否结束
    if (board.isGameOver()) {
        auto winner = board.getWinner();
        if (!winner.has_value()) {
            return 0;  // 平局
        }
        // 根据获胜者返回分数
        int score = (winner.value() == currentPlayer) ? 10000 : -10000;
        // 根据剩余棋子数调整分数
        int discDiff = board.getBitBoard().getScore(currentPlayer) -
                      board.getBitBoard().getScore(currentPlayer == PlayerColor::Black ? PlayerColor::White : PlayerColor::Black);
        return score + discDiff;
    }

    auto validMoves = board.getValidMoves();

    // 处理无有效移动的情况（跳过回合）
    if (validMoves.empty()) {
        Board newBoard = board;  // 复制棋盘
        Move passMove = Move::pass();
        newBoard.makeMove(passMove);  // 执行跳过

        // 递归搜索，但切换玩家
        return minimaxAlphaBeta(newBoard, currentPlayer, depth - 1, !isMaximizing, alpha, beta, limits);
    }

    int bestScore = isMaximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

    for (const auto& move : validMoves) {
        if (shouldStop(limits, searchStartTime_)) {
            break;  // 时间不足，停止搜索
        }

        // 创建新棋盘状态
        Board newBoard = board;  // 复制棋盘
        bool moveSuccess = newBoard.makeMove(move);

        if (!moveSuccess) {
            continue;  // 移动失败，跳过
        }

        // 递归搜索
        int score = minimaxAlphaBeta(newBoard, currentPlayer, depth - 1, !isMaximizing, alpha, beta, limits);

        if (isMaximizing) {
            if (score > bestScore) {
                bestScore = score;
                if (depth == config_.maxDepth) {  // 根节点
                    bestMove_ = move;
                    bestScore_ = score;
                }
            }
            alpha = std::max(alpha, bestScore);
            if (beta <= alpha) {
                config_.cutoffs++;  // Beta剪枝
                break;
            }
        } else {
            if (score < bestScore) {
                bestScore = score;
            }
            beta = std::min(beta, bestScore);
            if (beta <= alpha) {
                config_.cutoffs++;  // Alpha剪枝
                break;
            }
        }
    }

    return bestScore;
}

bool MinimaxAI::shouldStop(const SearchLimits& limits,
                          std::chrono::steady_clock::time_point startTime) const {
    // 检查时间限制
    if (limits.timeLimit.has_value()) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
        if (elapsed >= limits.timeLimit.value()) {
            return true;
        }
    }

    // 检查节点限制
    if (limits.maxNodes.has_value() && config_.nodesExplored >= limits.maxNodes.value()) {
        return true;
    }

    return false;
}

Difficulty MinimaxAI::getDifficulty() const {
    if (config_.maxDepth <= 2) return Difficulty::EASY;
    if (config_.maxDepth <= 4) return Difficulty::MEDIUM;
    return Difficulty::HARD;
}

AIStats MinimaxAI::getStats() const {
    return stats_;
}

std::string MinimaxAI::getDescription() const {
    std::string desc = "Minimax algorithm with ";
    if (config_.useAlphaBeta) {
        desc += "Alpha-Beta pruning";
    } else {
        desc += "no pruning";
    }

    if (config_.useIterativeDeepening) {
        desc += " and iterative deepening";
    }

    desc += ". Searches to depth " + std::to_string(config_.maxDepth);
    desc += " with time limit " + std::to_string(config_.timeLimit.count()) + "ms.";

    return desc;
}

std::string MinimaxAI::getConfigDescription() const {
    std::string configStr = "Depth: " + std::to_string(config_.maxDepth) + ", ";
    configStr += std::string("Alpha-Beta: ") + (config_.useAlphaBeta ? "Yes" : "No") + ", ";
    configStr += std::string("Iterative Deepening: ") + (config_.useIterativeDeepening ? "Yes" : "No") + ", ";
    configStr += "Time Limit: " + std::to_string(config_.timeLimit.count()) + "ms";
    return configStr;
}

void MinimaxAI::reset() {
    stats_ = AIStats{};
    config_.nodesExplored = 0;
    config_.cutoffs = 0;
    bestMove_ = Move();
    bestScore_ = std::numeric_limits<int>::min();
    lastCompletedDepth_ = 0;
}

bool MinimaxAI::supportsFeature(const std::string& feature) const {
    if (feature == "alpha_beta") return config_.useAlphaBeta;
    if (feature == "iterative_deepening") return config_.useIterativeDeepening;
    if (feature == "time_control") return true;
    if (feature == "depth_limit") return true;
    return AIStrategy::supportsFeature(feature);
}

} // namespace Reversi
