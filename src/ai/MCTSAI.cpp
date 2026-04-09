#include "ai/MCTSAI.h"
#include "ai/Evaluator.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>

namespace Reversi {

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

MCTSAI::MCTSAI(MCTSConfig config, std::unique_ptr<Evaluator> evaluator)
    : config_(config),
      evaluator_(evaluator ? std::move(evaluator) : nullptr),
      rng_(42),
      uniform_dist_(0.0, 1.0) {
    if (!evaluator_) {
        evaluator_ = std::make_unique<StaticEvaluator>();
    }
}

Move MCTSAI::findBestMove(const Board& board, const SearchLimits& limits) {
    stats_ = AIStats{};
    auto search_start = std::chrono::steady_clock::now();

    root_ = std::make_unique<MCTSNode>();

    int max_simulations = std::min(config_.num_simulations,
                                 static_cast<int>(limits.maxNodes.value_or(config_.num_simulations)));

    for (int i = 0; i < max_simulations; ++i) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - search_start);

        if (limits.timeLimit.has_value() && elapsed >= limits.timeLimit.value()) {
            break;
        }

        search(board, limits);
        stats_.nodesExplored++;
    }

    Move best_move;
    int max_visits = -1;

    for (size_t i = 0; i < root_->getChildCount(); ++i) {
        MCTSNode* child = root_->getChild(i);
        if (child->visit_count > max_visits) {
            max_visits = child->visit_count;
            best_move = root_->getChildMove(i);
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

void MCTSAI::search(const Board& board, const SearchLimits& limits) {
    if (!root_) {
        std::cerr << "[MCTSAI] Warning: root_ is null" << std::endl;
        return;
    }

    MCTSNode* current = root_.get();
    std::vector<MCTSNode*> path;

    while (current && !current->isLeaf()) {
        path.push_back(current);

        double best_uct = -std::numeric_limits<double>::infinity();
        MCTSNode* best_child = nullptr;

        for (size_t i = 0; i < current->getChildCount(); ++i) {
            MCTSNode* child = current->getChild(i);
            double uct_value = child->getUCT(config_.c_puct);
            if (uct_value > best_uct) {
                best_uct = uct_value;
                best_child = child;
            }
        }

        if (!best_child) break;
        current = best_child;
    }

    MCTSNode* leaf_node = current;
    if (leaf_node->isLeaf()) {
        auto valid_moves = board.getValidMoves();
        if (!valid_moves.empty()) {
            size_t max_children = std::min(size_t(4), valid_moves.size());
            for (size_t i = 0; i < max_children; ++i) {
                auto child = std::make_unique<MCTSNode>(leaf_node);
                child->prior = 1.0 / max_children;
                Move move = valid_moves[i];
                leaf_node->addChild(move, std::move(child));
            }
        }
    }

    MCTSNode* simulation_node = leaf_node;
    if (leaf_node->getChildCount() > 0) {
        simulation_node = leaf_node->getChild(0);
        double value = simulate(board);
        backpropagate(simulation_node, value);
    }
}

double MCTSAI::simulate(const Board& board, int max_depth) {
    try {
        Board sim_board = board;
        int depth = 0;
        PlayerColor current_player = sim_board.getCurrentTurn();

        while (!sim_board.isGameOver() && depth < max_depth) {
            auto valid_moves = sim_board.getValidMoves();

            if (valid_moves.empty()) {
                Move pass_move = Move::pass();
                if (!sim_board.makeMove(pass_move)) {
                    break;
                }
            } else {
                if (valid_moves.size() == 0) break;
                size_t random_index = static_cast<size_t>(uniform_dist_(rng_) * valid_moves.size());
                random_index = std::min(random_index, valid_moves.size() - 1);
                sim_board.makeMove(valid_moves[random_index]);
            }

            current_player = (current_player == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
            depth++;
        }

        if (sim_board.isGameOver()) {
            auto winner = sim_board.getWinner();
            if (winner.has_value()) {
                return (winner.value() == current_player) ? 1.0 : -1.0;
            } else {
                return 0.0;
            }
        } else {
            if (!evaluator_) {
                return 0.0;
            }
            double eval = evaluator_->evaluate(sim_board.getBitBoard(), current_player);
            return std::tanh(eval / 1000.0);
        }
    } catch (const std::exception& e) {
        std::cerr << "[MCTSAI] Simulation error: " << e.what() << std::endl;
        return 0.0;
    } catch (...) {
        std::cerr << "[MCTSAI] Unknown simulation error" << std::endl;
        return 0.0;
    }
}

void MCTSAI::backpropagate(MCTSNode* node, double value) {
    MCTSNode* current = node;
    while (current != nullptr) {
        current->visit_count++;
        current->value_sum += value;
        value = -value;
        current = current->parent;
    }
}

std::string MCTSAI::getDescription() const {
    std::string desc = "Monte Carlo Tree Search (UCT) algorithm. ";
    desc += "Simulations: " + std::to_string(config_.num_simulations) + ", ";
    desc += "C_puct: " + std::to_string(config_.c_puct) + ", ";
    desc += "Time limit: " + std::to_string(config_.time_limit.count()) + "ms.";

    if (config_.use_dirichlet_noise) {
        desc += " Uses Dirichlet noise for exploration.";
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
    configStr += "Time: " + std::to_string(config_.time_limit.count()) + "ms";
    return configStr;
}

void MCTSAI::reset() {
    stats_ = AIStats{};
    root_.reset();
}

bool MCTSAI::supportsFeature(const std::string& feature) const {
    if (feature == "uct") return true;
    if (feature == "monte_carlo") return true;
    if (feature == "time_control") return true;
    if (feature == "simulation_limit") return true;
    if (feature == "dirichlet_noise") return config_.use_dirichlet_noise;
    return AIStrategy::supportsFeature(feature);
}

} // namespace Reversi
