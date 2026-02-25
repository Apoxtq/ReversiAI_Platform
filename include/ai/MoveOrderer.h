/**
 * @file MoveOrderer.h
 * @brief Move Ordering (走法排序器) 实现
 *
 * 统一管理所有走法排序策略：Killer Moves + History Heuristic
 *
 * 排序优先级:
 * 1. PV走法 (Principal Variation) - 已知最佳走法
 * 2. 杀手走法 - 历史导致剪枝的走法
 * 3. 历史启发 - 历史得分高的走法
 * 4. 静态评估 - 评估函数得分
 * 5. 其他 - 按原始顺序
 *
 * 参考: Egaroucid/src/engine/move_ordering.hpp
 * v0.7.0 - AI算法优化版
 */

#pragma once

#include "ai/KillerTable.h"
#include "ai/HistoryTable.h"
#include "ai/Evaluator.h"
#include "core/BitBoard.h"
#include "Board.h"
#include <vector>
#include <memory>

namespace Reversi {

/**
 * @brief Move Orderer Configuration (走法排序器配置)
 */
struct MoveOrdererConfig {
    bool useKillerMoves = true;           ///< 是否使用Killer Moves
    bool useHistoryHeuristic = true;      ///< 是否使用历史启发
    bool useMobilityOrdering = true;      ///< 是否使用灵活度排序
    int killerWeight = 8;                  ///< Killer权重 (基于Egaroucid)
    int historyWeight = 6;                 ///< History权重 (基于Egaroucid)
    double decayFactor = 0.99;             ///< 衰减因子
};

/**
 * @brief Move Orderer (走法排序器)
 *
 * 统一管理所有走法排序策略
 *
 * 设计要点:
 * - 整合KillerTable和HistoryTable
 * - 支持多种排序策略组合
 * - 提供统计信息
 */
class MoveOrderer {
public:
    /**
     * @brief 构造函数
     * @param config 配置
     * @param evaluator 评估函数
     */
    explicit MoveOrderer(MoveOrdererConfig config = MoveOrdererConfig(),
                        std::unique_ptr<Evaluator> evaluator = nullptr);

    /**
     * @brief 主排序函数
     *
     * 根据当前搜索状态对走法进行排序
     *
     * @param board 当前棋盘
     * @param depth 当前搜索深度
     * @param moves 待排序走法
     * @param pvMove PV走法 (可选)
     * @param prevMove 上一手走法 (用于Counter Move)
     * @return 排序后的走法
     */
    std::vector<Move> orderMoves(const Board& board, int depth,
                                const std::vector<Move>& moves,
                                const Move& pvMove = Move(), const Move& prevMove = Move());

    /**
     * @brief 静态排序 (快速排序)
     *
     * 只使用Killer和History，不进行深度评估
     *
     * @param depth 当前搜索深度
     * @param moves 待排序走法
     * @param pvMove PV走法
     * @param prevMove 上一手走法
     * @return 排序后的走法
     */
    std::vector<Move> orderMovesStatic(int depth,
                                      const std::vector<Move>& moves,
                                      const Move& pvMove = Move(), const Move& prevMove = Move());

    /**
     * @brief 添加Killer走法
     * 在Beta剪枝后调用
     *
     * @param depth 搜索深度
     * @param move 走法
     */
    void addKiller(int depth, int move);

    /**
     * @brief 添加History得分
     * 在Beta剪枝后调用
     *
     * @param from 起始位置
     * @param to 目标位置
     * @param depth 搜索深度
     */
    void addHistory(int from, int to, int depth);

    /**
     * @brief 记录导致剪枝的走法
     * 同时更新Killer和History
     *
     * @param from 起始位置
     * @param to 目标位置
     * @param depth 搜索深度
     * @param isBetaCutoff 是否是Beta剪枝
     */
    void recordCutoff(int from, int to, int depth, bool isBetaCutoff = true);

    /**
     * @brief 清空所有排序数据
     */
    void clear();

    /**
     * @brief 执行衰减
     * 在每次搜索开始前调用，防止历史数据过时
     */
    void decay();

    /**
     * @brief 获取配置
     */
    const MoveOrdererConfig& getConfig() const { return config_; }

    /**
     * @brief 设置配置
     */
    void setConfig(const MoveOrdererConfig& config) { config_ = config; }

    /**
     * @brief 获取Killer表
     */
    const KillerTable& getKillerTable() const { return killerTable_; }

    /**
     * @brief 获取History表
     */
    const HistoryTable& getHistoryTable() const { return historyTable_; }

    /**
     * @brief 获取统计数据
     */
    struct Statistics {
        int killerHits;       ///< Killer命中次数
        int historyHits;      ///< History命中次数
        int pvHits;          ///< PV走法命中次数
        int totalMoves;       ///< 排序的总走法数
        int movesWithKiller;  ///< 包含Killer走法的次数
        int movesWithHistory; ///< 包含History走法的次数
    };
    Statistics getStatistics() const;

    /**
     * @brief 重置统计
     */
    void resetStatistics();

private:
    /**
     * @brief 计算走法得分
     *
     * @param move 走法
     * @param depth 搜索深度
     * @param pvMove PV走法
     * @param prevMove 上一手走法
     * @return 排序得分
     */
    int calculateMoveScore(const Move& move, int depth, const Move& pvMove, const Move& prevMove) const;

    /**
     * @brief 计算灵活度得分
     *
     * @param board 棋盘
     * @param move 走法
     * @return 灵活度得分
     */
    int calculateMobilityScore(const Board& board, const Move& move) const;

    // 配置
    MoveOrdererConfig config_;

    // 组件
    KillerTable killerTable_;
    HistoryTable historyTable_;
    std::unique_ptr<Evaluator> evaluator_;

    // 统计
    mutable Statistics stats_;
};

} // namespace Reversi
