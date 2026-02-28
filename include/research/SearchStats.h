/**
 * @file SearchStats.h
 * @brief 搜索统计数据结构 - v0.9.0可视化增强版
 *
 * 用于在AI搜索过程中收集统计信息，供UI可视化使用
 */

#pragma once

#include <vector>
#include <chrono>
#include <cstdint>
#include <cmath>
#include <string>

namespace Reversi {

/**
 * @brief 搜索树节点信息
 */
struct SearchTreeNodeInfo {
    int move;                    // 走法 (0-63)
    int depth;                   // 深度
    int value;                   // 评估值
    int nodeCount;               // 子节点数
    bool isPV;                   // 是否为PV (Principal Variation) 路线
    int64_t nodesSearched;       // 搜索的节点数
    std::vector<int> childMoves; // 子节点走法列表

    SearchTreeNodeInfo() : move(-1), depth(0), value(0), nodeCount(0),
                          isPV(false), nodesSearched(0) {}
};

/**
 * @brief 实时搜索统计数据
 */
struct SearchStats {
    // 基础统计
    int currentDepth = 0;                    // 当前搜索深度
    int maxDepth = 0;                        // 最大深度
    int64_t nodesExplored = 0;              // 已搜索节点数
    int64_t nodesPerSecond = 0;              // 每秒搜索节点数
    double timeElapsed = 0.0;                // 已用时间(秒)

    // 剪枝和转置表统计
    int cutoffs = 0;                         // 剪枝次数
    double ttHitRate = 0.0;                 // 转置表命中率
    double killerHitRate = 0.0;             // Killer走法命中率
    double historyHitRate = 0.0;            // History启发命中率
    int64_t ttEntries = 0;                  // 转置表条目数

    // 搜索树信息
    std::vector<SearchTreeNodeInfo> pvLine; // PV路线 (主要变化)
    int totalTreeNodes = 0;                 // 搜索树总节点数

    // MCTS特定统计 (仅MCTS使用)
    int64_t simulations = 0;                // 模拟次数
    double winRate = 0.0;                   // 胜率估计

    // 最佳走法信息
    int bestMove = -1;                      // 当前最佳走法
    int bestValue = 0;                      // 最佳走法评估值

    // 搜索状态
    bool isSearching = false;               // 是否正在搜索
    bool isComplete = false;                // 搜索是否完成

    /**
     * @brief 重置所有统计
     */
    void reset() {
        currentDepth = 0;
        maxDepth = 0;
        nodesExplored = 0;
        nodesPerSecond = 0;
        timeElapsed = 0.0;
        cutoffs = 0;
        ttHitRate = 0.0;
        killerHitRate = 0.0;
        historyHitRate = 0.0;
        ttEntries = 0;
        pvLine.clear();
        totalTreeNodes = 0;
        simulations = 0;
        winRate = 0.0;
        bestMove = -1;
        bestValue = 0;
        isSearching = false;
        isComplete = false;
    }

    /**
     * @brief 获取NPS (Nodes Per Second) 格式化字符串
     */
    std::string getNPSString() const {
        if (nodesPerSecond >= 1000000) {
            return std::to_string(nodesPerSecond / 1000000) + "M";
        } else if (nodesPerSecond >= 1000) {
            return std::to_string(nodesPerSecond / 1000) + "K";
        }
        return std::to_string(nodesPerSecond);
    }

    /**
     * @brief 获取时间格式化字符串
     */
    std::string getTimeString() const {
        int minutes = static_cast<int>(timeElapsed) / 60;
        int seconds = static_cast<int>(timeElapsed) % 60;
        int ms = static_cast<int>((timeElapsed - std::floor(timeElapsed)) * 1000);

        if (minutes > 0) {
            return std::to_string(minutes) + ":" +
                   (seconds < 10 ? "0" : "") + std::to_string(seconds);
        }
        return std::to_string(seconds) + "." +
               (ms < 100 ? "0" : "") + (ms < 10 ? "0" : "") + std::to_string(ms);
    }
};

/**
 * @brief 热度图数据类型
 */
enum class HeatmapType {
    PositionValue,     // 静态位置价值
    VisitCount,        // 访问次数 (MCTS)
    WinRate,           // 胜率
    ActionValue        // 动作评估值
};

/**
 * @brief 热度图数据
 */
struct HeatmapData {
    std::vector<double> values;  // 64个格子的值 (0.0 - 1.0)
    HeatmapType type;            // 数据类型
    bool isValid = false;        // 数据是否有效

    HeatmapData() : type(HeatmapType::PositionValue), isValid(false) {
        values.resize(64, 0.0);
    }

    void clear() {
        std::fill(values.begin(), values.end(), 0.0);
        isValid = false;
    }

    bool isValidIndex(int index) const {
        return index >= 0 && index < 64 && isValid;
    }
};

} // namespace Reversi
