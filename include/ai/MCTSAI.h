#pragma once

#include "ai/AIStrategy.h"
#include "ai/Evaluator.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <random>

/**
 * @file MCTSAI.h
 * @brief MCTS AI implementation
 *
 * Based on alpha-zero-general UCT algorithm, adapted for C++ BitBoard
 * Reference: alpha-zero-general/MCTS.py
 *             alpha-zero-general/othello/OthelloGame.py
 */

namespace Reversi {

/**
 * @brief MCTS Node structure
 *
 * Represents a node in the search tree
 * Reference: alpha-zero-general/MCTS.py tree node concept
 */
struct MCTSNode {
    /**
     * @brief Constructor
     * @param parent Parent node pointer
     */
    explicit MCTSNode(MCTSNode* parent = nullptr);

    // Node statistics
    double value_sum = 0.0;        ///< Sum of Q values
    int visit_count = 0;           ///< Visit count N(s,a)
    double prior = 0.0;            ///< Prior probability P(s,a)

    // Tree structure
    MCTSNode* parent;              ///< Parent node
    std::vector<std::unique_ptr<MCTSNode>> children;  ///< Children
    std::vector<Move> child_moves;  ///< Corresponding moves

    /**
     * @brief Get value estimate (Q value)
     * @return Average value
     */
    double getValue() const {
        return visit_count > 0 ? value_sum / visit_count : 0.0;
    }

    /**
     * @brief Check if leaf node
     * @return true if no children
     */
    bool isLeaf() const;

    /**
     * @brief Find child node
     * @param move Move
     * @return Child node pointer, nullptr if not found
     */
    MCTSNode* findChild(const Move& move);

    /**
     * @brief Add child node
     * @param move Move
     * @param node Child node
     */
    void addChild(Move move, std::unique_ptr<MCTSNode> node);

    /**
     * @brief Get number of children
     */
    size_t getChildCount() const { return children.size(); }

    /**
     * @brief Get i-th child node
     */
    MCTSNode* getChild(size_t index) { return children[index].get(); }
    const MCTSNode* getChild(size_t index) const { return children[index].get(); }

    /**
     * @brief Get i-th child move
     */
    const Move& getChildMove(size_t index) const { return child_moves[index]; }

    /**
     * @brief Get UCT value (Upper Confidence Bound)
     * @param c_puct Exploration constant
     * @return UCT value
     *
     * Formula: Q(s,a) + c_puct * P(s,a) * sqrt(N(s)) / (1 + N(s,a))
     * Reference: alpha-zero-general/MCTS.py line 112-113
     */
    double getUCT(double c_puct) const;
};

/**
 * @brief MCTS configuration parameters
 */
struct MCTSConfig {
    int num_simulations = 5000;    ///< Simulations per move
    double c_puct = 1.5;           ///< Exploration constant
    bool use_dirichlet_noise = false;  ///< Dirichlet noise (disabled for performance)
    double dirichlet_alpha = 0.3;  ///< Dirichlet noise parameter
    double dirichlet_epsilon = 0.25;  ///< Noise mixing ratio

    // Time control
    std::chrono::milliseconds time_limit = std::chrono::milliseconds(3000);

    // Improved playout settings
    bool use_smart_playout = true;    ///< Use heuristic-based playout
    int playout_max_depth = 50;       ///< Maximum depth for playout
};

/**
 * @brief MCTS AI implementation
 *
 * Based on alpha-zero-general UCT algorithm, adapted for BitBoard system
 * Reference: alpha-zero-general/MCTS.py
 */
class MCTSAI : public AIStrategy {
public:
    /**
     * @brief Constructor
     * @param config MCTS configuration
     * @param evaluator Evaluator (for leaf node evaluation)
     */
    explicit MCTSAI(MCTSConfig config = MCTSConfig(),
                   std::unique_ptr<Evaluator> evaluator = nullptr);

    // AIStrategy interface
    Move findBestMove(const Board& board, const SearchLimits& limits) override;
    std::string getName() const override { return "MCTSAI"; }
    std::string getDescription() const override;
    Difficulty getDifficulty() const override;
    AIStats getStats() const override;
    std::string getConfigDescription() const override;
    void reset() override;
    bool supportsFeature(const std::string& feature) const override;

    /**
     * @brief Get MCTS configuration
     */
    const MCTSConfig& getConfig() const { return config_; }

    /**
     * @brief Set MCTS configuration
     */
    void setConfig(const MCTSConfig& config) { config_ = config; }

private:
    /**
     * @brief Execute single MCTS search
     * @param board Current board state
     * @param limits Search limits
     *
     * Reference: alpha-zero-general/MCTS.py search() method
     */
    void search(const Board& board, const SearchLimits& limits);

    /**
     * @brief Simulation phase: random rollout or quick evaluation
     * @param board Current board state
     * @param max_depth Maximum simulation depth
     * @return Simulation result (-1 to 1)
     *
     * Reference: alpha-zero-general/MCTS.py random rollout
     */
    double simulate(const Board& board, int max_depth = 50);

    /**
     * @brief Backpropagation: propagate result up
     * @param node Leaf node
     * @param value Simulation result
     *
     * Reference: alpha-zero-general/MCTS.py backpropagation
     */
    void backpropagate(MCTSNode* node, double value);

    /**
     * @brief Get prior probabilities for moves
     * @param board Current board state
     * @return Probability vector for each move
     *
     * Simplified version: all valid moves equal probability.
     * Future: integrate neural network policy.
     */
    std::vector<double> getPriorProbabilities(const Board& board);

    /**
     * @brief Add Dirichlet noise (for root node diversity)
     * @param priors Prior probabilities
     * @return Priors with noise added
     *
     * Reference: alpha-zero-general paper
     */
    std::vector<double> addDirichletNoise(const std::vector<double>& priors);

    /**
     * @brief Generate Dirichlet distribution random numbers
     * @param alpha Concentration parameter
     * @param size Vector size
     * @return Dirichlet distribution sample
     */
    std::vector<double> sampleDirichlet(double alpha, size_t size);

    // Configuration and components
    MCTSConfig config_;
    std::unique_ptr<Evaluator> evaluator_;

    // MCTS tree
    std::unique_ptr<MCTSNode> root_;

    // Statistics
    mutable AIStats stats_;

    // Random number generators
    std::mt19937 rng_;
    std::uniform_real_distribution<double> uniform_dist_;
};

} // namespace Reversi
