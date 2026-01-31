#include "ai/MCTSAI.h"
#include "ai/Evaluator.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>

/**
 * @file MCTSAI.cpp
 * @brief MCTS算法实现
 *
 * 将alpha-zero-general的MCTS算法转换为C++ BitBoard实现
 */

namespace Reversi {

// MCTSNode 实现

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
        return std::numeric_limits<double>::infinity();  // 未访问的节点优先级最高
    }

    // 计算UCT值: Q(s,a) + c_puct * P(s,a) * sqrt(N(s)) / (1 + N(s,a))
    // 参考: alpha-zero-general/MCTS.py line 112-113
    double parent_visits = parent ? parent->visit_count : 1.0;
    double exploration_term = c_puct * prior * std::sqrt(parent_visits) / (1.0 + visit_count);

    return getValue() + exploration_term;
}

// MCTSAI 实现

MCTSAI::MCTSAI(MCTSConfig config, std::unique_ptr<Evaluator> evaluator)
    : config_(config),
      evaluator_(evaluator ? std::move(evaluator) : EvaluatorFactory::createStaticEvaluator()),
      rng_(std::random_device{}()),
      uniform_dist_(0.0, 1.0) {
}

Move MCTSAI::findBestMove(const Board& board, const SearchLimits& limits) {
    // 重置统计信息
    stats_ = AIStats{};
    auto search_start = std::chrono::steady_clock::now();

    // 初始化或重置根节点
    root_ = std::make_unique<MCTSNode>();

    // 设置搜索限制
    int max_simulations = std::min(config_.num_simulations,
                                 limits.maxNodes.value_or(config_.num_simulations));

    // 执行MCTS搜索
    for (int i = 0; i < max_simulations; ++i) {
        // 检查时间限制
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            current_time - search_start);

        if (limits.timeLimit.has_value() && elapsed >= limits.timeLimit.value()) {
            break;
        }

        search(board, limits);
        stats_.nodesExplored++;
    }

    // 找到访问次数最多的移动
    Move best_move;
    int max_visits = -1;

    for (size_t i = 0; i < root_->getChildCount(); ++i) {
        MCTSNode* child = root_->getChild(i);
        if (child->visit_count > max_visits) {
            max_visits = child->visit_count;
            best_move = root_->getChildMove(i);
        }
    }

    // 如果没有找到有效移动，返回跳过
    if (!best_move.isValid()) {
        best_move = Move::pass();
    }

    // 更新统计信息
    auto end_time = std::chrono::steady_clock::now();
    stats_.timeUsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - search_start);
    stats_.evaluationCount = stats_.nodesExplored;  // 每次仿真算一次评估
    stats_.depthReached = 1;  // MCTS的深度概念不同，这里简化为1

    // 计算平均分支因子（根节点的子节点数量）
    if (root_->getChildCount() > 0) {
        stats_.avgBranching = root_->getChildCount();
    }

    return best_move;
}

void MCTSAI::search(const Board& board, const SearchLimits& limits) {
    // 1. 选择阶段 (Selection) - 找到叶子节点
    MCTSNode* current = root_.get();
    std::vector<MCTSNode*> path;

    // 向下遍历到叶子节点
    while (!current->isLeaf()) {
        path.push_back(current);

        // 选择UCT值最大的子节点
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

    // 2. 扩展阶段 (Expansion) - 如果是叶子节点且游戏未结束
    MCTSNode* leaf_node = current;
    if (leaf_node->isLeaf()) {
        // 扩展叶子节点
        auto valid_moves = board.getValidMoves();
        if (!valid_moves.empty()) {
            // 为每个有效移动创建子节点（简化：只创建几个主要移动）
            size_t max_children = std::min(size_t(4), valid_moves.size()); // 最多4个子节点
            for (size_t i = 0; i < max_children; ++i) {
                auto child = std::make_unique<MCTSNode>(leaf_node);
                child->prior = 1.0 / max_children; // 均匀先验概率
                leaf_node->addChild(valid_moves[i], std::move(child));
            }
        }
    }

    // 选择一个子节点进行仿真（简化：选择第一个）
    MCTSNode* simulation_node = leaf_node;
    if (leaf_node->getChildCount() > 0) {
        simulation_node = leaf_node->getChild(0);
    }

    // 3. 仿真阶段 (Simulation)
    double value = simulate(board);

    // 4. 回传阶段 (Backpropagation)
    backpropagate(simulation_node, value);
}

// 已整合到search方法中

double MCTSAI::simulate(const Board& board, int max_depth) {
    // 随机仿真直到游戏结束或达到最大深度
    // 参考: alpha-zero-general的随机仿真策略

    Board sim_board = board;
    int depth = 0;
    PlayerColor current_player = sim_board.getCurrentTurn();

    while (!sim_board.isGameOver() && depth < max_depth) {
        auto valid_moves = sim_board.getValidMoves();

        if (valid_moves.empty()) {
            // 没有有效移动，跳过回合
            Move pass_move = Move::pass();
            sim_board.makeMove(pass_move);
        } else {
            // 随机选择一个有效移动
            size_t random_index = static_cast<size_t>(
                uniform_dist_(rng_) * valid_moves.size());
            random_index = std::min(random_index, valid_moves.size() - 1);

            sim_board.makeMove(valid_moves[random_index]);
        }

        current_player = (current_player == PlayerColor::Black) ?
                        PlayerColor::White : PlayerColor::Black;
        depth++;
    }

    // 使用评估器评估最终局面
    if (sim_board.isGameOver()) {
        auto winner = sim_board.getWinner();
        if (winner.has_value()) {
            // 根据当前玩家视角返回结果
            return (winner.value() == current_player) ? 1.0 : -1.0;
        } else {
            return 0.0;  // 平局
        }
    } else {
        // 使用评估器评估中间局面
        double eval = evaluator_->evaluate(sim_board.getBitBoard(), current_player);
        // 归一化到-1到1的范围（假设评估器的范围是合理的）
        return std::tanh(eval / 100.0);  // 使用tanh进行平滑归一化
    }
}

void MCTSAI::backpropagate(MCTSNode* node, double value) {
    // 从叶子节点向上传播价值
    // 参考: alpha-zero-general/MCTS.py的回传逻辑

    MCTSNode* current = node;
    while (current != nullptr) {
        current->visit_count++;
        current->value_sum += value;
        value = -value;  // 切换视角（因为下一层是对手）
        current = current->parent;
    }
}

// 简化实现中暂时不需要这些方法

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
