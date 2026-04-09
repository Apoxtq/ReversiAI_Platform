#include "ai/MinimaxAI.h"
#include "ai/Evaluator.h"
#include "Board.h"
#include "ai/TranspositionTable.h"
#include <algorithm>
#include <limits>
#include <iostream>

/**
 * @file MinimaxAI.cpp
 * @brief Minimax algorithm implementation with Alpha-Beta pruning
 *
 * Converts Reversi(Java) Minimax algorithm to C++ BitBoard implementation
 */

namespace Reversi {

MinimaxAI::MinimaxAI(MinimaxConfig config, std::unique_ptr<Evaluator> evaluator)
    : config_(config), color_(PlayerColor::Black) {
    // Create evaluator in constructor body to avoid UB in member initializer list
    if (evaluator) {
        evaluator_ = std::move(evaluator);
    } else {
        evaluator_ = std::make_unique<StaticEvaluator>();
    }
    // v0.6.0: Initialize transposition table
    initTranspositionTable();

    // v0.7.0: Initialize move orderer
    if (config_.useMoveOrdering) {
        MoveOrdererConfig moConfig;
        moConfig.useKillerMoves = config_.useKillerMoves;
        moConfig.useHistoryHeuristic = config_.useHistoryHeuristic;
        moveOrderer_ = std::make_unique<MoveOrderer>(moConfig, std::make_unique<StaticEvaluator>());
        std::cout << "[MinimaxAI] Move orderer initialized (Killer: "
                  << (config_.useKillerMoves ? "ON" : "OFF")
                  << ", History: " << (config_.useHistoryHeuristic ? "ON" : "OFF") << ")" << std::endl;
    }
}

void MinimaxAI::initTranspositionTable() {
    if (config_.useTranspositionTable) {
        tt_ = std::make_unique<TranspositionTable>(config_.transpositionTableSizeMB);
        std::cout << "[MinimaxAI] Transposition table initialized: "
                  << config_.transpositionTableSizeMB << " MB" << std::endl;
    }
}

Move MinimaxAI::findBestMove(const Board& board, const SearchLimits& limits) {
    // v0.6.0: Initialize transposition table if not yet initialized
    if (!tt_ && config_.useTranspositionTable) {
        initTranspositionTable();
    }

    // v0.7.0: Initialize move orderer if not yet initialized
    if (!moveOrderer_ && config_.useMoveOrdering) {
        MoveOrdererConfig moConfig;
        moConfig.useKillerMoves = config_.useKillerMoves;
        moConfig.useHistoryHeuristic = config_.useHistoryHeuristic;
        moveOrderer_ = std::make_unique<MoveOrderer>(moConfig, EvaluatorFactory::createStaticEvaluator());
    }

    // Reset statistics
    stats_ = AIStats{};
    config_.nodesExplored = 0;
    config_.cutoffs = 0;
    config_.ttHits = 0;
    config_.killerHits = 0;
    config_.historyHits = 0;
    bestMove_ = Move();
    bestScore_ = std::numeric_limits<int>::min();
    lastCompletedDepth_ = 0;

    // Get AI color (set externally, or default to current player)
    PlayerColor originalPlayer = color_;

    searchStartTime_ = std::chrono::steady_clock::now();

    // v0.7.0: Decay move orderer before each search
    if (moveOrderer_) {
        moveOrderer_->decay();
        moveOrderer_->resetStatistics();
    }

    // Get valid moves
    auto validMoves = board.getValidMoves();
    if (validMoves.empty()) {
        return Move::pass();  // No valid moves, skip turn
    }

    // Determine search depth
    int searchDepth = std::min(config_.maxDepth, limits.maxDepth.value_or(config_.maxDepth));

    if (config_.useIterativeDeepening) {
        // Iterative deepening search
        for (int depth = 1; depth <= searchDepth; ++depth) {
            if (shouldStop(limits, searchStartTime_)) {
                break;
            }

            // Search at current depth - use negamax to ensure score consistency
            int alpha = -std::numeric_limits<int>::max();
            int beta = std::numeric_limits<int>::max();

            minimaxAlphaBeta(board, depth, depth, alpha, beta, limits, originalPlayer);
            lastCompletedDepth_ = depth;
        }
    } else {
        // Fixed-depth search
        int alpha = -std::numeric_limits<int>::max();
        int beta = std::numeric_limits<int>::max();

        minimaxAlphaBeta(board, searchDepth, searchDepth, alpha, beta, limits, originalPlayer);
        lastCompletedDepth_ = searchDepth;
    }

    // Update statistics
    auto endTime = std::chrono::steady_clock::now();
    stats_.timeUsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - searchStartTime_);
    stats_.nodesExplored = config_.nodesExplored;
    stats_.evaluationCount = config_.nodesExplored;
    stats_.avgBranching = validMoves.size();

    return bestMove_.isValid() ? bestMove_ : validMoves[0];
}

int MinimaxAI::minimaxAlphaBeta(const Board& board, int depth, int maxSearchDepth,
                               int alpha, int beta, const SearchLimits& limits,
                               PlayerColor originalPlayer) {
    config_.nodesExplored++;

    // In Negamax, we always evaluate from current player's perspective, then negate
    PlayerColor currentPlayer = board.getCurrentTurn();

    // v0.6.0: Transposition table lookup
    if (tt_ && config_.useTranspositionTable) {
        uint32_t hash = ZobristHash::computeHash(
            board.getBitBoard().getPlayerBits(),
            board.getBitBoard().getOpponentBits()
        );
        int storedScore = 0;
        Move storedMove;
        if (tt_->probe(hash, depth, storedScore, storedMove)) {
            config_.ttHits++;
            TTEntryType entryType = tt_->getEntryType(hash, depth);

            if (entryType == TTEntryType::EXACT) {
                if (storedScore >= alpha && storedScore <= beta) {
                    return storedScore;
                }
            } else if (entryType == TTEntryType::LOWER) {
                if (storedScore >= beta) {
                    return beta;
                }
                if (storedScore > alpha) {
                    alpha = storedScore;
                }
            } else if (entryType == TTEntryType::UPPER) {
                if (storedScore <= alpha) {
                    return alpha;
                }
                if (storedScore < beta) {
                    beta = storedScore;
                }
            }
        }
    }

    // Check if search should terminate (time or depth limit)
    if (shouldStop(limits, searchStartTime_) || depth == 0) {
        return evaluator_->evaluate(board.getBitBoard(), currentPlayer);
    }

    // Check if game is over
    if (board.isGameOver()) {
        auto winner = board.getWinner();
        if (!winner.has_value()) {
            return 0;
        }
        bool currentPlayerWon = (winner.value() == currentPlayer);
        int baseScore = currentPlayerWon ? 10000 : -10000;
        int myDiscs = board.getBitBoard().getScore(currentPlayer);
        PlayerColor opponent = (currentPlayer == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
        int opDiscs = board.getBitBoard().getScore(opponent);
        return baseScore + (myDiscs - opDiscs);
    }

    auto validMoves = board.getValidMoves();

    // v0.7.0: Use move orderer to sort moves
    Move pvMove;
    if (tt_ && config_.useTranspositionTable) {
        uint32_t hash = ZobristHash::computeHash(
            board.getBitBoard().getPlayerBits(),
            board.getBitBoard().getOpponentBits()
        );
        Move ttMove;
        int dummyScore;
        if (tt_->probe(hash, depth, dummyScore, ttMove)) {
            pvMove = ttMove;
        }
    }

    if (moveOrderer_ && config_.useMoveOrdering) {
        validMoves = moveOrderer_->orderMoves(board, depth, validMoves, pvMove, Move());
    }

    // Handle no valid moves (pass turn)
    if (validMoves.empty()) {
        Board newBoard = board;
        newBoard.makeMove(Move::pass());
        return -minimaxAlphaBeta(newBoard, depth - 1, maxSearchDepth, -beta, -alpha, limits, currentPlayer);
    }

    int bestScore = std::numeric_limits<int>::min();
    Move localBestMove;

    for (const auto& move : validMoves) {
        if (shouldStop(limits, searchStartTime_)) {
            break;
        }

        Board newBoard = board;
        if (!newBoard.makeMove(move)) {
            continue;
        }

        // Negamax: negate recursive score
        int score = -minimaxAlphaBeta(newBoard, depth - 1, maxSearchDepth, -beta, -alpha, limits, currentPlayer);

        if (score > bestScore) {
            bestScore = score;
            localBestMove = move;
            // Only update bestMove_ at root node
            if (depth == maxSearchDepth) {
                bestMove_ = move;
                bestScore_ = score;
            }
        }

        // Alpha-Beta pruning
        if (bestScore > alpha) {
            alpha = bestScore;
        }
        if (alpha >= beta) {
            config_.cutoffs++;
            // v0.7.0: Record Killer/History
            if (moveOrderer_ && config_.useMoveOrdering) {
                int from = 0;
                int to = move.row * 8 + move.col;
                moveOrderer_->recordCutoff(from, to, depth, true);
            }
            break;
        }
    }

    // v0.6.0: Store result in transposition table
    if (tt_ && config_.useTranspositionTable) {
        uint32_t hash = ZobristHash::computeHash(
            board.getBitBoard().getPlayerBits(),
            board.getBitBoard().getOpponentBits()
        );
        tt_->store(hash, depth, bestScore, alpha, beta, localBestMove);
    }

    return bestScore;
}

bool MinimaxAI::shouldStop(const SearchLimits& limits,
                          std::chrono::steady_clock::time_point startTime) const {
    if (limits.timeLimit.has_value()) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
        if (elapsed >= limits.timeLimit.value()) {
            return true;
        }
    }

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

    if (config_.useTranspositionTable) {
        desc += ", transposition table (" + std::to_string(config_.transpositionTableSizeMB) + " MB)";
    }

    if (config_.useMoveOrdering) {
        desc += ", move ordering";
        if (config_.useKillerMoves) desc += " (Killer)";
        if (config_.useHistoryHeuristic) desc += " (History)";
    }

    desc += ". Searches to depth " + std::to_string(config_.maxDepth);
    desc += " with time limit " + std::to_string(config_.timeLimit.count()) + "ms.";

    return desc;
}

std::string MinimaxAI::getConfigDescription() const {
    std::string configStr = "Depth: " + std::to_string(config_.maxDepth) + ", ";
    configStr += std::string("Alpha-Beta: ") + (config_.useAlphaBeta ? "Yes" : "No") + ", ";
    configStr += std::string("Iterative Deepening: ") + (config_.useIterativeDeepening ? "Yes" : "No") + ", ";
    configStr += std::string("Transposition Table: ") + (config_.useTranspositionTable ? "Yes" : "No") + " ("
                + std::to_string(config_.transpositionTableSizeMB) + " MB), ";
    configStr += std::string("Move Ordering: ") + (config_.useMoveOrdering ? "Yes" : "No") + " (";
    configStr += "Killer: " + std::string(config_.useKillerMoves ? "On" : "Off") + ", ";
    configStr += "History: " + std::string(config_.useHistoryHeuristic ? "On" : "Off") + "), ";
    configStr += "Time Limit: " + std::to_string(config_.timeLimit.count()) + "ms";
    return configStr;
}

void MinimaxAI::reset() {
    stats_ = AIStats{};
    config_.nodesExplored = 0;
    config_.cutoffs = 0;
    config_.ttHits = 0;
    config_.killerHits = 0;
    config_.historyHits = 0;
    bestMove_ = Move();
    bestScore_ = std::numeric_limits<int>::min();
    lastCompletedDepth_ = 0;

    // v0.6.0: Clear transposition table
    if (tt_) {
        tt_->clear();
    }

    // v0.7.0: Reset move orderer
    if (moveOrderer_) {
        moveOrderer_->clear();
        moveOrderer_->resetStatistics();
    }
}

bool MinimaxAI::supportsFeature(const std::string& feature) const {
    if (feature == "alpha_beta") return config_.useAlphaBeta;
    if (feature == "iterative_deepening") return config_.useIterativeDeepening;
    if (feature == "transposition_table") return config_.useTranspositionTable;
    if (feature == "move_ordering") return config_.useMoveOrdering;
    if (feature == "killer_moves") return config_.useKillerMoves;
    if (feature == "history_heuristic") return config_.useHistoryHeuristic;
    if (feature == "time_control") return true;
    if (feature == "depth_limit") return true;
    return AIStrategy::supportsFeature(feature);
}

} // namespace Reversi
