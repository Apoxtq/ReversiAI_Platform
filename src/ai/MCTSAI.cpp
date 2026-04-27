#include "ai/MCTSAI.h"
#include "ai/Evaluator.h"
#include "ai/MinimaxAI.h"
#include "ai/ZobristHash.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>

namespace Reversi {

// ============================================================================
// MCTSNode Implementation
// ============================================================================

MCTSNode::MCTSNode(MCTSNode* parent) : parent(parent) {}

bool MCTSNode::isLeaf() const {
    return children.empty();
}

MCTSNode* MCTSNode::findChild(const Move& move) {
    for (size_t i = 0; i < children.size(); ++i) {
        if (child_moves[i] == move) {
            return children[i].get();
        }
    }
    return nullptr;
}

void MCTSNode::addChild(Move move, std::unique_ptr<MCTSNode> node) {
    child_moves.push_back(move);
    children.push_back(std::move(node));
}

double MCTSNode::getUCT(double c_puct) const {
    if (visit_count == 0) {
        return std::numeric_limits<double>::infinity();
    }
    double parent_visits = parent ? parent->visit_count : 1.0;
    double exploration_term = c_puct * prior * std::sqrt(parent_visits) / (1.0 + visit_count);
    return getValue() + exploration_term;
}

// ============================================================================
// MCTSAI Implementation
// ============================================================================

// v1.1.0: Compute c_puct based on game phase
double MCTSAI::getDynamicCPuct(int empty_count) const {
    if (empty_count > 40) return config_.c_puct_early;
    if (empty_count > 20) return config_.c_puct_mid;
    if (empty_count > 12) return config_.c_puct_late;
    return config_.c_puct_endgame;
}

double MCTSAI::getEffectiveCPuct(const Board& board) const {
    if (!config_.use_dynamic_cpuct) {
        return config_.c_puct;
    }
    return getDynamicCPuct(board.getBitBoard().getEmptyCount());
}

MCTSAI::MCTSAI(MCTSConfig config, std::unique_ptr<Evaluator> evaluator)
    : config_(config),
      evaluator_(evaluator ? std::move(evaluator) : nullptr),
      rng_(42),
      uniform_dist_(0.0, 1.0) {
    if (!evaluator_) {
        evaluator_ = std::make_unique<StaticEvaluator>();
    }
    if (config_.use_mcts_tt) {
        mcts_tt_ = std::make_unique<TranspositionTable>(config_.mcts_tt_size_mb);
    }
}

Move MCTSAI::findBestMove(const Board& board, const SearchLimits& limits) {
    stats_ = AIStats{};
    auto search_start = std::chrono::steady_clock::now();

    if (config_.use_mcts_tt && !mcts_tt_) {
        mcts_tt_ = std::make_unique<TranspositionTable>(config_.mcts_tt_size_mb);
    }

    root_ = std::make_unique<MCTSNode>();

    int max_simulations = std::min(config_.num_simulations,
                                   static_cast<int>(limits.maxNodes.value_or(config_.num_simulations)));

    for (int i = 0; i < max_simulations; ++i) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - search_start);

        if (limits.timeLimit.has_value() && elapsed >= limits.timeLimit.value()) {
            break;
        }

        search(board);
        stats_.nodesExplored++;
    }

    // v1.1.0: Endgame solver - use Minimax when board is deep enough
    int empty_count = board.getBitBoard().getEmptyCount();
    Move best_move;
    double best_score = -1e9;

    if (config_.use_endgame_solver && empty_count <= config_.endgame_solver_threshold) {
        for (size_t i = 0; i < root_->getChildCount(); ++i) {
            MCTSNode* child = root_->getChild(i);
            const Move& child_move = root_->getChildMove(i);
            Board test_board = board;
            if (!test_board.makeMove(child_move)) continue;
            double minimax_score = solveWithMinimax(test_board);
            double combined = minimax_score + 0.001 * child->visit_count;
            if (combined > best_score) {
                best_score = combined;
                best_move = child_move;
            }
        }
        if (!best_move.isValid()) {
            for (size_t i = 0; i < root_->getChildCount(); ++i) {
                if (root_->getChild(i)->visit_count > 0) {
                    best_move = root_->getChildMove(i);
                    break;
                }
            }
        }
    } else {
        int max_visits = -1;
        for (size_t i = 0; i < root_->getChildCount(); ++i) {
            MCTSNode* child = root_->getChild(i);
            if (child->visit_count > max_visits) {
                max_visits = child->visit_count;
                best_move = root_->getChildMove(i);
            }
        }
    }

    if (!best_move.isValid()) {
        best_move = Move::pass();
    }

    auto end_time = std::chrono::steady_clock::now();
    stats_.timeUsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - search_start);
    stats_.evaluationCount = stats_.nodesExplored;
    stats_.depthReached = 1;

    if (root_->getChildCount() > 0) {
        stats_.avgBranching = root_->getChildCount();
    }

    return best_move;
}

void MCTSAI::search(const Board& board) {
    if (!root_) return;

    MCTSNode* current = root_.get();
    Board sim_board = board;
    double c_puct = getEffectiveCPuct(board);

    // Selection: traverse tree using UCB1
    while (current && !current->isLeaf()) {
        double best_uct = -std::numeric_limits<double>::infinity();
        MCTSNode* best_child = nullptr;
        size_t best_move_idx = 0;

        for (size_t i = 0; i < current->getChildCount(); ++i) {
            MCTSNode* child = current->getChild(i);
            double uct_value = child->getUCT(c_puct);
            if (uct_value > best_uct) {
                best_uct = uct_value;
                best_child = child;
                best_move_idx = i;
            }
        }

        if (!best_child) break;

        const Move& move = current->getChildMove(best_move_idx);
        sim_board.makeMove(move);
        current = best_child;

        if (sim_board.isGameOver()) break;

        // v1.1.0: Update c_puct as board changes
        c_puct = getEffectiveCPuct(sim_board);
    }

    // Expansion: add new child nodes
    if (current->isLeaf() && !sim_board.isGameOver()) {
        auto valid_moves = sim_board.getValidMoves();
        if (!valid_moves.empty()) {
            std::vector<double> priors = getPriorProbabilities(sim_board);

            for (size_t i = 0; i < valid_moves.size(); ++i) {
                auto child = std::make_unique<MCTSNode>(current);
                double prior = (i < priors.size()) ? priors[i] : 1.0;
                child->prior = prior;
                current->addChild(valid_moves[i], std::move(child));
            }
        }
    }

    // Simulation: play random moves to terminal or depth limit
    if (!sim_board.isGameOver() && current->getChildCount() > 0) {
        std::uniform_int_distribution<size_t> dist(0, current->getChildCount() - 1);
        size_t selected_idx = dist(rng_);

        MCTSNode* sim_node = current->getChild(selected_idx);
        const Move& sim_move = current->getChildMove(selected_idx);
        sim_board.makeMove(sim_move);

        double result = simulateWithHeuristic(sim_board);
        backpropagate(sim_node, result);
    } else if (sim_board.isGameOver()) {
        double result = getTerminalResult(sim_board);
        backpropagate(root_.get(), result);
    }
}

double MCTSAI::simulateWithHeuristic(Board board) {
    int consecutive_passes = 0;
    int move_count = 0;

    while (!board.isGameOver() && consecutive_passes < 2) {
        auto valid_moves = board.getValidMoves();

        if (valid_moves.empty()) {
            Move pass_move = Move::pass();
            if (!board.makeMove(pass_move)) break;
            consecutive_passes++;
            move_count++;
        } else {
            consecutive_passes = 0;

            int empty_count = board.getBitBoard().getEmptyCount();

            if (empty_count > 12) {
                // Mid-game: use evaluator-based selection
                double best_score = -1e9;
                Move best_move = valid_moves[0];
                PlayerColor player = board.getCurrentTurn();

                for (const Move& move : valid_moves) {
                    Board test_board = board;
                    test_board.makeMove(move);

                    double score = evaluator_->evaluate(test_board.getBitBoard(), player);
                    score += (uniform_dist_(rng_) - 0.5) * 2.0;

                    if (score > best_score) {
                        best_score = score;
                        best_move = move;
                    }
                }

                if (!board.makeMove(best_move)) break;
            } else {
                // Endgame: play to completion with pure random
                std::uniform_int_distribution<size_t> dist(0, valid_moves.size() - 1);
                size_t move_idx = dist(rng_);
                Move selected = valid_moves[move_idx];
                if (!board.makeMove(selected)) break;
            }

            move_count++;
        }
    }

    return getTerminalResult(board);
}

double MCTSAI::getTerminalResult(const Board& board) {
    auto winner = board.getWinner();
    if (!winner.has_value()) return 0.0;

    PlayerColor ai_player = config_.ai_player_color.value_or(PlayerColor::Black);
    return (winner.value() == ai_player) ? 1.0 : -1.0;
}

void MCTSAI::backpropagate(MCTSNode* node, double value) {
    while (node != nullptr) {
        node->visit_count++;
        node->value_sum += value;
        value = -value;
        node = node->parent;
    }
}

std::vector<double> MCTSAI::getPriorProbabilities(const Board& board) {
    std::vector<Move> valid_moves = board.getValidMoves();
    std::vector<double> priors(valid_moves.size(), 1.0);

    for (size_t i = 0; i < valid_moves.size(); ++i) {
        const Move& move = valid_moves[i];
        bool is_corner = (move.row == 0 || move.row == 7) &&
                         (move.col == 0 || move.col == 7);
        bool is_edge = (move.row == 0 || move.row == 7 ||
                        move.col == 0 || move.col == 7) && !is_corner;
        bool is_cursed = false;
        if (!is_corner && !is_edge) {
            if ((move.row == 0 && (move.col == 1 || move.col == 6)) ||
                (move.row == 7 && (move.col == 1 || move.col == 6)) ||
                (move.col == 0 && (move.row == 1 || move.row == 6)) ||
                (move.col == 7 && (move.row == 1 || move.row == 6))) {
                is_cursed = true;
            }
        }

        if (is_corner) {
            priors[i] = 3.0;
        } else if (is_edge) {
            priors[i] = 1.5;
        } else if (is_cursed) {
            priors[i] = 0.1;
        }
    }

    double sum = 0.0;
    for (double p : priors) sum += p;
    if (sum > 0) {
        for (double& p : priors) p /= sum;
    }
    return priors;
}

std::vector<double> MCTSAI::addDirichletNoise(const std::vector<double>& priors) {
    std::vector<double> noisy_priors = priors;
    std::vector<double> noise = sampleDirichlet(config_.dirichlet_alpha, priors.size());

    for (size_t i = 0; i < noisy_priors.size(); ++i) {
        noisy_priors[i] = (1.0 - config_.dirichlet_epsilon) * priors[i]
                        + config_.dirichlet_epsilon * noise[i];
    }
    return noisy_priors;
}

std::vector<double> MCTSAI::sampleDirichlet(double alpha, size_t size) {
    std::vector<double> samples(size);
    double sum = 0.0;
    std::gamma_distribution<double> gamma_dist(alpha, 1.0);

    for (size_t i = 0; i < size; ++i) {
        samples[i] = gamma_dist(rng_);
        sum += samples[i];
    }

    if (sum > 0) {
        for (double& s : samples) s /= sum;
    } else {
        for (double& s : samples) s = 1.0 / size;
    }
    return samples;
}

std::string MCTSAI::getDescription() const {
    std::string desc = "Monte Carlo Tree Search v1.1.0 (Enhanced). ";
    desc += "Simulations: " + std::to_string(config_.num_simulations) + ", ";
    if (config_.use_dynamic_cpuct) {
        desc += "Dynamic c_puct ("
              + std::to_string(config_.c_puct_early) + "/"
              + std::to_string(config_.c_puct_mid) + "/"
              + std::to_string(config_.c_puct_late) + "/"
              + std::to_string(config_.c_puct_endgame) + "), ";
    } else {
        desc += "C_puct: " + std::to_string(config_.c_puct) + ", ";
    }
    desc += "Time limit: " + std::to_string(config_.time_limit.count()) + "ms. ";
    if (config_.use_endgame_solver) {
        desc += "Endgame solver (threshold: " + std::to_string(config_.endgame_solver_threshold) + " empty squares). ";
    }
    if (config_.use_mcts_tt) {
        desc += "MCTS TT enabled. ";
    }
    if (config_.use_dirichlet_noise) {
        desc += "Dirichlet noise enabled.";
    }
    return desc;
}

Difficulty MCTSAI::getDifficulty() const {
    if (config_.num_simulations <= 500) return Difficulty::EASY;
    if (config_.num_simulations <= 2000) return Difficulty::MEDIUM;
    return Difficulty::HARD;
}

AIStats MCTSAI::getStats() const {
    return stats_;
}

std::string MCTSAI::getConfigDescription() const {
    std::string configStr = "Simulations: " + std::to_string(config_.num_simulations) + ", ";
    configStr += "C_puct: " + std::to_string(config_.c_puct) + ", ";
    configStr += "Time: " + std::to_string(config_.time_limit.count()) + "ms, ";
    configStr += "Hybrid playout: evaluator (mid) + random (endgame)";
    return configStr;
}

void MCTSAI::reset() {
    stats_ = AIStats{};
    root_.reset();
}

void MCTSAI::setColor(PlayerColor color) {
    config_.ai_player_color = color;
}

PlayerColor MCTSAI::getColor() const {
    return config_.ai_player_color.value_or(PlayerColor::Black);
}

bool MCTSAI::supportsFeature(const std::string& feature) const {
    if (feature == "uct") return true;
    if (feature == "monte_carlo") return true;
    if (feature == "time_control") return true;
    if (feature == "simulation_limit") return true;
    if (feature == "hybrid_playout") return true;
    if (feature == "dirichlet_noise") return config_.use_dirichlet_noise;
    if (feature == "dynamic_cpuct") return config_.use_dynamic_cpuct;
    if (feature == "endgame_solver") return config_.use_endgame_solver;
    if (feature == "mcts_tt") return config_.use_mcts_tt;
    if (feature == "puct") return config_.use_puct;
    return AIStrategy::supportsFeature(feature);
}

// ============================================================================
// v1.1.0: Endgame Solver
// ============================================================================

double MCTSAI::solveWithMinimax(const Board& board) {
    MinimaxConfig minimaxConfig;
    minimaxConfig.maxDepth = 60;
    minimaxConfig.useTranspositionTable = true;
    minimaxConfig.transpositionTableSizeMB = 16;
    minimaxConfig.timeLimit = std::chrono::milliseconds(100);

    MinimaxAI solver(minimaxConfig, std::make_unique<StaticEvaluator>());
    PlayerColor aiPlayer = config_.ai_player_color.value_or(PlayerColor::Black);
    solver.setColor(aiPlayer);

    SearchLimits limits;
    limits.maxDepth = 60;
    limits.timeLimit = std::chrono::milliseconds(100);

    solver.findBestMove(board, limits);

    int minimaxScore = solver.getLastScore();
    PlayerColor currentPlayer = board.getCurrentTurn();
    int effectiveScore = (currentPlayer == aiPlayer) ? minimaxScore : -minimaxScore;

    if (effectiveScore > 0) return 1.0;
    if (effectiveScore < 0) return -1.0;
    return 0.0;
}

// ============================================================================
// v1.1.0: MCTS Transposition Table
// ============================================================================

uint32_t MCTSAI::getBoardHash(const Board& board) const {
    return ZobristHash::computeHash(
        board.getBitBoard().getPlayerBits(),
        board.getBitBoard().getOpponentBits()
    );
}

const MCTSTTEntry* MCTSAI::probeMCTSTT(uint32_t hash) const {
    if (!mcts_tt_) return nullptr;
    static thread_local std::unordered_map<uint32_t, MCTSTTEntry> mcts_cache;
    auto it = mcts_cache.find(hash);
    if (it != mcts_cache.end()) {
        return &it->second;
    }
    return nullptr;
}

void MCTSAI::storeMCTSTT(uint32_t hash, double q_sum, int visits, const Move& move) {
    if (!mcts_tt_) return;
    int scaled_score = static_cast<int>(q_sum * 1000);
    mcts_tt_->store(hash, 0, scaled_score, -1000, 1000, move);
    static thread_local std::unordered_map<uint32_t, MCTSTTEntry> mcts_cache;
    MCTSTTEntry entry;
    entry.hash = hash;
    entry.q_sum = q_sum;
    entry.visit_count = visits;
    entry.best_move = move;
    entry.used = 1;
    mcts_cache[hash] = entry;
}

} // namespace Reversi
