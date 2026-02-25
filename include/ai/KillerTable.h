/**
 * @file KillerTable.h
 * @brief Killer Moves (杀手走法) 实现
 *
 * 记录在搜索树中导致Beta剪枝的走法，在相同深度搜索时优先尝试这些走法
 *
 * 参考: Egaroucid/src/engine/move_ordering.hpp
 * v0.7.0 - AI算法优化版
 */

#pragma once

#include <vector>
#include <array>
#include <algorithm>

namespace Reversi {

/**
 * @brief Killer Move结构
 *
 * 记录导致剪枝的走法信息
 */
struct KillerMove {
    int move;          ///< 走法位置 (0-63)
    int score;         ///< 杀手系数 (导致剪枝的次数)
    int depth;         ///< 搜索深度

    KillerMove() : move(-1), score(0), depth(0) {}
    KillerMove(int m, int s, int d) : move(m), score(s), depth(d) {}

    bool isValid() const { return move >= 0 && move < 64; }
};

/**
 * @brief Killer Table (杀手走法表)
 *
 * 每个深度维护多个最佳杀手走法，用于走法排序
 *
 * 设计要点:
 * - 每个深度维护2个最佳杀手走法 (MAX_KILLER_COUNT)
 * - 使用计数机制避免同一走法重复添加
 * - 深度迭代时保留上一层的杀手信息
 */
class KillerTable {
public:
    static constexpr int MAX_KILLER_COUNT = 2;  ///< 每个深度最多杀手走法数
    static constexpr int MAX_DEPTH = 64;         ///< 最大搜索深度
    static constexpr int INITIAL_SCORE = 1;      ///< 初始分数

public:
    /**
     * @brief 构造函数
     */
    KillerTable();

    /**
     * @brief 添加杀手走法
     * @param depth 搜索深度
     * @param move 走法位置
     * @param score 杀手系数 (默认1)
     */
    void addKiller(int depth, int move, int score = INITIAL_SCORE);

    /**
     * @brief 获取深度的杀手走法
     * @param depth 搜索深度
     * @return 杀手走法向量
     */
    std::vector<int> getKillers(int depth) const;

    /**
     * @brief 判断是否为杀手走法
     * @param depth 搜索深度
     * @param move 走法位置
     * @return true表示是杀手走法
     */
    bool isKiller(int depth, int move) const;

    /**
     * @brief 获取杀手走法分数
     * @param depth 搜索深度
     * @param move 走法位置
     * @return 杀手分数 (0表示不是杀手走法)
     */
    int getKillerScore(int depth, int move) const;

    /**
     * @brief 清空表
     */
    void clear();

    /**
     * @brief 衰减所有杀手分数
     * 在长期搜索中使用，防止历史数据过时
     * @param factor 衰减因子 (0.0-1.0)
     */
    void decay(double factor = 0.99);

    /**
     * @brief 获取统计信息
     */
    struct Statistics {
        int totalKillers;       ///< 总杀手走法数
        int depthDistribution[MAX_DEPTH];  ///< 每层杀手数
    };
    Statistics getStatistics() const;

private:
    /**
     * @brief 找到最低分数的位置
     * @param depth 搜索深度
     * @return 最低分数位置的索引
     */
    int findLowestScoreIndex(int depth) const;

    /**
     * @brief 检查走法是否已存在
     * @param depth 搜索深度
     * @param move 走法位置
     * @return 存在的索引，-1表示不存在
     */
    int findExistingIndex(int depth, int move) const;

    // 杀手走法表: [depth][count]
    std::array<std::array<KillerMove, MAX_KILLER_COUNT>, MAX_DEPTH> killers_;
};

} // namespace Reversi
