#pragma once

#include "ai/AIStrategy.h"
#include "ai/Evaluator.h"
#include <memory>
#include <chrono>

/**
 * @file MinimaxAI.h
 * @brief Minimax算法AI实现
 *
 * 基于Reversi(Java)的Minimax算法，转换为C++ BitBoard实现
 * 参考: Reversi(Java)/src/player/ai/Minimax.java
 */

namespace Reversi {

/**
 * @brief Minimax AI配置
 */
struct MinimaxConfig {
    int maxDepth = 4;                    ///< 最大搜索深度
    bool useAlphaBeta = true;            ///< 是否使用Alpha-Beta剪枝
    bool useIterativeDeepening = false;  ///< 是否使用迭代深化
    std::chrono::milliseconds timeLimit = std::chrono::milliseconds(3000); ///< 时间限制

    // 统计信息
    mutable int nodesExplored = 0;       ///< 探索的节点数
    mutable int cutoffs = 0;             ///< 剪枝次数
};

/**
 * @brief Minimax AI实现
 *
 * 基于Reversi(Java)的MMAB函数，转换为C++实现
 * 支持Alpha-Beta剪枝和时间控制
 *
 * 继承AIStrategy接口，实现策略模式
 */
class MinimaxAI : public AIStrategy {
public:
    /**
     * @brief 构造函数
     * @param config Minimax配置
     * @param evaluator 评估函数（默认为静态评估器）
     */
    explicit MinimaxAI(MinimaxConfig config = MinimaxConfig(),
                       std::unique_ptr<Evaluator> evaluator = nullptr);

    // AIStrategy接口实现
    Move findBestMove(const Board& board, const SearchLimits& limits) override;
    std::string getName() const override { return "MinimaxAI"; }
    Difficulty getDifficulty() const override;
    AIStats getStats() const override;

    std::string getDescription() const override;
    std::string getConfigDescription() const override;
    void reset() override;
    bool supportsFeature(const std::string& feature) const override;

    /**
     * @brief 获取配置
     */
    const MinimaxConfig& getConfig() const { return config_; }

    /**
     * @brief 设置配置
     */
    void setConfig(const MinimaxConfig& config) { config_ = config; }

private:
    /**
     * @brief Minimax搜索主函数
     *
     * 对应Java的MMAB函数
     * @param board 当前棋盘状态
     * @param currentPlayer 当前玩家
     * @param depth 剩余搜索深度
     * @param isMaximizing 是否是最大化节点
     * @param alpha Alpha值
     * @param beta Beta值
     * @param limits 搜索限制
     * @return 评估分数
     *
     * 参考: Minimax.MMAB(int[][] node, int player, int depth, boolean max, int alpha, int beta, Evaluator e)
     */
    int minimaxAlphaBeta(const Board& board, PlayerColor currentPlayer, int depth,
                         bool isMaximizing, int alpha, int beta, const SearchLimits& limits);

    /**
     * @brief 检查是否应该终止搜索
     * @param limits 搜索限制
     * @param startTime 开始时间
     * @return true表示应该终止
     */
    bool shouldStop(const SearchLimits& limits,
                   std::chrono::steady_clock::time_point startTime) const;

    /**
     * @brief 获取最佳移动（不进行完整搜索）
     * 用于在时间限制内找到最佳移动
     */
    Move getBestMoveFromLastDepth() const;

    // 配置和组件
    MinimaxConfig config_;
    std::unique_ptr<Evaluator> evaluator_;

    // 搜索状态
    mutable AIStats stats_;
    mutable Move bestMove_;
    mutable int bestScore_;
    mutable int lastCompletedDepth_;

    // 统计信息
    mutable std::chrono::steady_clock::time_point searchStartTime_;
};

} // namespace Reversi
