#pragma once

#include "ai/AIStrategy.h"
#include "ai/Evaluator.h"
#include "ai/TranspositionTable.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <random>
#include <optional>
#include <atomic>

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
    int num_simulations = 5000;         ///< Simulations per move
    double c_puct = 1.4142;            ///< Base exploration constant (default: sqrt(2))
    bool use_dirichlet_noise = false;   ///< Dirichlet noise for root exploration
    double dirichlet_alpha = 0.3;      ///< Dirichlet noise parameter
    double dirichlet_epsilon = 0.25;    ///< Noise mixing ratio

    // Time control
    std::chrono::milliseconds time_limit = std::chrono::milliseconds(3000);

    // AI player color for simulation result evaluation
    std::optional<PlayerColor> ai_player_color;

    // === v1.1.0: MCTS Enhancement ===

    // Dynamic c_puct thresholds (empty squares count)
    double c_puct_early = 1.5;      ///< Exploration constant for early game (>40 empty)
    double c_puct_mid = 1.3;        ///< Exploration constant for mid game (20-40 empty)
    double c_puct_late = 1.1;       ///< Exploration constant for late game (12-20 empty)
    double c_puct_endgame = 0.8;    ///< Exploration constant for endgame (<=12 empty)
    bool use_dynamic_cpuct = false;   ///< Enable dynamic c_puct adjustment

    // Endgame solver: use Minimax when empty squares <= this threshold
    // Default 0 (disabled) for automated testing performance.
    // Set to 8-12 for better endgame play in human vs AI games.
    int endgame_solver_threshold = 0;
    bool use_endgame_solver = false;

    // Transposition table for MCTS
    // Note: Full integration pending. Basic TT storing is available.
    bool use_mcts_tt = false;
    size_t mcts_tt_size_mb = 16;    ///< MCTS TT size (MB)

    // Selection policy
    bool use_puct = true;             ///< Use PUCT (AlphaZero-style) instead of UCB1
};

/**
 * @brief MCTS-specific transposition table entry
 *
 * Stores aggregated MCTS statistics (Q-value sum, visit count, best move)
 * for previously visited board states. Used for tree reuse and node merging.
 */
struct MCTSTTEntry {
    uint32_t hash = 0;           ///< Board Zobrist hash
    double q_sum = 0.0;          ///< Sum of Q values (for averaging)
    int visit_count = 0;         ///< Total visit count
    Move best_move;               ///< Best move from this position
    uint8_t used = 0;            ///< Whether entry is valid

    void clear() {
        hash = 0; q_sum = 0.0; visit_count = 0; used = 0;
    }
    bool match(uint32_t h) const { return used && hash == h; }
    double getQ() const { return visit_count > 0 ? q_sum / visit_count : 0.0; }
};

/**
 * @brief MCTS AI implementation
 *
 * Uses UCT (Upper Confidence Bound for Trees) algorithm with hybrid playout strategy:
 * - Mid-game (empty squares > 12): uses heuristic evaluation for better simulation quality
 * - Endgame (empty squares <= 12): plays to terminal state with pure random moves
 *
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
    void setColor(PlayerColor color) override;
    PlayerColor getColor() const override;

    /**
     * @brief Get MCTS configuration
     */
    const MCTSConfig& getConfig() const { return config_; }

    /**
     * @brief Set MCTS configuration
     */
    void setConfig(const MCTSConfig& config) { config_ = config; }

private:
    void search(const Board& board);
    double simulateWithHeuristic(Board board);
    double getTerminalResult(const Board& board);
    void backpropagate(MCTSNode* node, double value);

    // === v1.1.0: Enhancement ===

    /**
     * @brief Get dynamic c_puct based on game phase
     * @param empty_count Number of empty squares on board
     * @return c_puct value for current game phase
     */
    double getDynamicCPuct(int empty_count) const;

    /**
     * @brief Get effective c_puct for selection
     * @param board Current board state
     * @return c_puct value
     */
    double getEffectiveCPuct(const Board& board) const;

    /**
     * @brief Endgame solver: use Minimax for terminal search
     * @param board Board state
     * @return Terminal game result (+1, 0, -1)
     *
     * Called when empty squares <= endgame_solver_threshold.
     * Delegates to MinimaxAI with max depth for perfect reading.
     */
    double solveWithMinimax(const Board& board);

    /**
     * @brief Get prior probabilities for moves
     */
    std::vector<double> getPriorProbabilities(const Board& board);

    /**
     * @brief Add Dirichlet noise (for root node diversity)
     */
    std::vector<double> addDirichletNoise(const std::vector<double>& priors);

    /**
     * @brief Generate Dirichlet distribution random numbers
     */
    std::vector<double> sampleDirichlet(double alpha, size_t size);

    // === MCTS Transposition Table ===
    /**
     * @brief Probe MCTS transposition table
     * @param hash Board hash
     * @return MCTSTTEntry pointer if found, nullptr otherwise
     */
    const MCTSTTEntry* probeMCTSTT(uint32_t hash) const;

    /**
     * @brief Store to MCTS transposition table
     * @param hash Board hash
     * @param q_sum Q value sum
     * @param visits Visit count
     * @param move Best move
     */
    void storeMCTSTT(uint32_t hash, double q_sum, int visits, const Move& move);

    /**
     * @brief Get board Zobrist hash
     */
    uint32_t getBoardHash(const Board& board) const;

    // Configuration and components
    MCTSConfig config_;
    std::unique_ptr<Evaluator> evaluator_;
    mutable std::unique_ptr<TranspositionTable> mcts_tt_;  ///< MCTS transposition table

    // MCTS tree
    std::unique_ptr<MCTSNode> root_;

    // Statistics
    mutable AIStats stats_;

    // Random number generators
    std::mt19937 rng_;
    std::uniform_real_distribution<double> uniform_dist_;
};

} // namespace Reversi
