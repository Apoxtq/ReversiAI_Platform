/**
 *.h
 * @ @file HistoryTablebrief History Heuristic (历史启发) 实现
 *
 * 记录每个走法在历史上导致剪枝的次数，用于全局走法排序
 *
 * 参考: Egaroucid/src/engine/move_ordering.hpp
 * 参考: edax-reversi/src/play.c
 * v0.7.0 - AI算法优化版
 */

#pragma once

#include <vector>
#include <array>
#include <algorithm>
#include <cstdint>

namespace Reversi {

/**
 * @brief History Table (历史启发表)
 *
 * 记录每个走法在历史上导致剪枝的次数，用于全局走法排序
 *
 * 设计要点:
 * - 64x64 矩阵存储所有可能的走法
 * - 每次Beta剪枝时增加历史得分
 * - 搜索前进行衰减处理
 * - 结合位置和目标格进行索引
 */
class HistoryTable {
public:
    static constexpr int NUM_SQUARES = 64;    ///< 棋盘格子数
    static constexpr int MAX_HISTORY_SCORE = 1000000;  ///< 最大历史分数
    static constexpr double DEFAULT_DECAY = 0.99;  ///< 默认衰减因子

public:
    /**
     * @brief 构造函数
     */
    HistoryTable();

    /**
     * @brief 增加走法历史得分
     *
     * 当一个走法导致剪枝时调用，增加其历史得分
     * 使用深度加权：越深的搜索历史越重要
     *
     * @param from 起始位置 (0-63)
     * @param to 目标位置 (0-63)
     * @param depth 搜索深度
     * @param isCutoff 是否导致剪枝 (默认true)
     */
    void addHistory(int from, int to, int depth, bool isCutoff = true);

    /**
     * @brief 获取走法历史得分
     * @param from 起始位置
     * @param to 目标位置
     * @return 历史得分
     */
    int getHistoryScore(int from, int to) const;

    /**
     * @brief 获取排序后的走法列表
     * @param moves 原始走法列表 (格式: from * 64 + to)
     * @return 排序后的走法列表
     */
    std::vector<int> getSortedMoves(const std::vector<int>& moves) const;

    /**
     * @brief 清空历史表
     */
    void clear();

    /**
     * @brief 衰减历史得分
     * 防止历史数据过时
     * @param factor 衰减因子 (0.0-1.0)
     */
    void decay(double factor = DEFAULT_DECAY);

    /**
     * @brief 获取统计信息
     */
    struct Statistics {
        int maxScore;           ///< 最高历史分数
        int minScore;           ///< 最低非零分数
        int totalEntries;       ///< 非零条目数
        double averageScore;    ///< 平均分数
    };
    Statistics getStatistics() const;

    /**
     * @brief 规范化所有历史分数
     * 防止分数溢出
     */
    void normalize();

private:
    /**
     * @brief 检查位置是否有效
     * @param pos 位置 (0-63)
     * @return true表示有效
     */
    bool isValidPosition(int pos) const;

    // History矩阵: [from][to] = 历史得分
    // history_[from][to] 表示从from到to的走法历史得分
    std::array<std::array<int, NUM_SQUARES>, NUM_SQUARES> history_;
};

} // namespace Reversi
