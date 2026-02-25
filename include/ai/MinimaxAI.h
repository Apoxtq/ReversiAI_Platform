#pragma once

#include "ai/AIStrategy.h"
#include "ai/Evaluator.h"
#include "ai/TranspositionTable.h"
#include "ai/MoveOrderer.h"
#include <memory>
#include <chrono>

/**
 * @file MinimaxAI.h
 * @brief Minimax算法AI实现
 *
 * 基于Reversi(Java)的Minimax算法，转换为C++ BitBoard实现
 * 参考: Reversi(Java)/src/player/ai/Minimax.java
 *
 * v0.6.0更新: 添加转置表支持
 */

namespace Reversi {

/**
 * @brief Minimax AI配置
 */
struct MinimaxConfig {
    int maxDepth = 4;                    ///< 最大搜索深度
    bool useAlphaBeta = true;            ///< 是否使用Alpha-Beta剪枝
    bool useIterativeDeepening = false;  ///< 是否使用迭代深化
    bool useTranspositionTable = true;   ///< v0.6.0: 是否使用转置表

    // v0.7.0: Killer Moves & History Heuristic
    bool useKillerMoves = true;          ///< 是否使用Killer Moves
    bool useHistoryHeuristic = true;     ///< 是否使用历史启发
    bool useMoveOrdering = true;         ///< 是否使用走法排序

    std::chrono::milliseconds timeLimit = std::chrono::milliseconds(3000); ///< 时间限制

    // 转置表配置 (v0.6.0)
    size_t transpositionTableSizeMB = 64; ///< 转置表大小 (MB)

    // 统计信息
    mutable int nodesExplored = 0;       ///< 探索的节点数
    mutable int cutoffs = 0;             ///< 剪枝次数
    mutable int ttHits = 0;              ///< v0.6.0: 转置表命中次数

    // v0.7.0: Killer/History统计
    mutable int killerHits = 0;          ///< Killer走法命中次数
    mutable int historyHits = 0;         ///< History启发命中次数
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

    // v0.6.0: 转置表相关方法
    /**
     * @brief 获取转置表命中率
     */
    double getTranspositionTableHitRate() const {
        return tt_ ? tt_->getHitRate() : 0.0;
    }

    /**
     * @brief 清空转置表
     */
    void clearTranspositionTable() {
        if (tt_) tt_->clear();
    }

    // v0.7.0: Killer Moves & History Heuristic
    /**
     * @brief 获取走法排序器
     */
    MoveOrderer* getMoveOrderer() { return moveOrderer_.get(); }

    /**
     * @brief 获取走法排序器 (const)
     */
    const MoveOrderer* getMoveOrderer() const { return moveOrderer_.get(); }

    /**
     * @brief 获取Killer命中率
     */
    double getKillerHitRate() const {
        if (!config_.useKillerMoves || !moveOrderer_) return 0.0;
        const auto& stats = moveOrderer_->getStatistics();
        return stats.totalMoves > 0 ? static_cast<double>(stats.killerHits) / stats.totalMoves : 0.0;
    }

    /**
     * @brief 获取History命中率
     */
    double getHistoryHitRate() const {
        if (!config_.useHistoryHeuristic || !moveOrderer_) return 0.0;
        const auto& stats = moveOrderer_->getStatistics();
        return stats.totalMoves > 0 ? static_cast<double>(stats.historyHits) / stats.totalMoves : 0.0;
    }

    /**
     * @brief 清空走法排序数据
     */
    void clearMoveOrderer() {
        if (moveOrderer_) moveOrderer_->clear();
    }

private:
    /**
     * @brief 初始化转置表
     */
    void initTranspositionTable();

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

    // v0.6.0: 转置表
    std::unique_ptr<TranspositionTable> tt_;  ///< 转置表

    // v0.7.0: 走法排序器 (Killer + History)
    std::unique_ptr<MoveOrderer> moveOrderer_;  ///< 走法排序器

    // 搜索状态
    mutable AIStats stats_;
    mutable Move bestMove_;
    mutable int bestScore_;
    mutable int lastCompletedDepth_;

    // 统计信息
    mutable std::chrono::steady_clock::time_point searchStartTime_;
};

} // namespace Reversi
