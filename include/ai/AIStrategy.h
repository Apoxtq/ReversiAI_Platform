#pragma once

#ifdef _WIN32
#define NOMINMAX
#endif

#include "Board.h"
#include <memory>
#include <string>
#include <chrono>
#include <optional>
#include <algorithm>

/**
 * @file AIStrategy.h
 * @brief AI策略统一接口
 *
 * 定义统一的AI算法接口，实现策略模式
 * 支持多种AI算法的统一管理和切换
 */

namespace Reversi {

/**
 * @enum Difficulty
 * @brief AI难度等级
 */
enum class Difficulty {
    EASY,      ///< 简单难度
    MEDIUM,    ///< 中等难度
    HARD,      ///< 困难难度
    CUSTOM     ///< 自定义配置
};

/**
 * @struct SearchLimits
 * @brief 搜索限制参数
 */
struct SearchLimits {
    std::optional<int> maxDepth;                    ///< 最大搜索深度
    std::optional<std::chrono::milliseconds> timeLimit;  ///< 时间限制
    std::optional<long long> maxNodes;             ///< 最大搜索节点数

    /**
     * @brief 创建默认搜索限制
     */
    static SearchLimits createDefault() {
        SearchLimits limits;
        limits.maxDepth = 4;
        limits.timeLimit = std::chrono::milliseconds(3000);
        limits.maxNodes = std::nullopt;
        return limits;
    }
};

/**
 * @struct AIStats
 * @brief AI算法性能统计
 */
struct AIStats {
    long long nodesExplored = 0;      ///< 探索的节点数
    std::chrono::milliseconds timeUsed = std::chrono::milliseconds(0);  ///< 使用的总时间
    double avgBranching = 0.0;        ///< 平均分支因子
    long long evaluationCount = 0;    ///< 评估函数调用次数
    int cutoffs = 0;                  ///< 剪枝次数
    int depthReached = 0;             ///< 达到的最大深度

    /**
     * @brief 重置统计信息
     */
    void reset() {
        nodesExplored = 0;
        timeUsed = std::chrono::milliseconds(0);
        avgBranching = 0.0;
        evaluationCount = 0;
        cutoffs = 0;
        depthReached = 0;
    }

    /**
     * @brief 合并统计信息
     */
    AIStats& operator+=(const AIStats& other) {
        nodesExplored += other.nodesExplored;
        timeUsed += other.timeUsed;
        avgBranching = (avgBranching + other.avgBranching) / 2.0;  // 简单平均
        evaluationCount += other.evaluationCount;
        cutoffs += other.cutoffs;
        depthReached = (depthReached > other.depthReached) ? depthReached : other.depthReached;
        return *this;
    }
};

/**
 * @class AIStrategy
 * @brief AI策略抽象基类
 *
 * 定义统一的AI算法接口，实现策略模式
 * 所有AI算法都继承此类，提供统一的接口
 */
class AIStrategy {
public:
    virtual ~AIStrategy() = default;

    /**
     * @brief 寻找最佳移动
     *
     * @param board 当前棋盘状态
     * @param limits 搜索限制参数
     * @return 最佳移动决策
     */
    virtual Move findBestMove(const Board& board, const SearchLimits& limits) = 0;

    /**
     * @brief 获取AI算法名称
     *
     * @return 算法名称字符串
     */
    virtual std::string getName() const = 0;

    /**
     * @brief 获取AI算法描述
     *
     * @return 算法描述字符串
     */
    virtual std::string getDescription() const = 0;

    /**
     * @brief 获取难度等级
     *
     * @return 难度等级
     */
    virtual Difficulty getDifficulty() const = 0;

    /**
     * @brief 获取性能统计信息
     *
     * @return 最新的性能统计
     */
    virtual AIStats getStats() const = 0;

    /**
     * @brief 获取当前配置描述
     *
     * @return 配置描述字符串
     */
    virtual std::string getConfigDescription() const = 0;

    /**
     * @brief 重置内部状态
     *
     * 用于清理缓存、统计信息等
     */
    virtual void reset() = 0;

    /**
     * @brief 检查算法是否支持特定功能
     */
    virtual bool supportsFeature(const std::string& feature) const {
        return false;  // 默认不支持
    }
};

/**
 * @class AIStrategyFactory
 * @brief AI策略工厂类
 *
 * 提供创建各种AI策略的工厂方法
 */
class AIStrategyFactory {
public:
    /**
     * @brief 创建Minimax AI
     *
     * @param difficulty 难度等级
     * @return Minimax AI实例
     */
    static std::unique_ptr<AIStrategy> createMinimaxAI(Difficulty difficulty = Difficulty::MEDIUM);

    /**
     * @brief 创建MCTS AI
     *
     * @param difficulty 难度等级
     * @return MCTS AI实例
     */
    static std::unique_ptr<AIStrategy> createMCTSAI(Difficulty difficulty = Difficulty::MEDIUM);

    /**
     * @brief 创建随机AI（用于测试）
     *
     * @return 随机AI实例
     */
    static std::unique_ptr<AIStrategy> createRandomAI();

    /**
     * @brief 获取支持的AI算法列表
     *
     * @return AI算法名称列表
     */
    static std::vector<std::string> getAvailableAlgorithms();

    /**
     * @brief 根据名称创建AI实例
     *
     * @param name AI算法名称
     * @param difficulty 难度等级
     * @return AI实例，如果名称无效返回nullptr
     */
    static std::unique_ptr<AIStrategy> createByName(const std::string& name,
                                                   Difficulty difficulty = Difficulty::MEDIUM);
};

/**
 * @brief 难度等级转换为字符串
 */
std::string difficultyToString(Difficulty difficulty);

/**
 * @brief 字符串转换为难度等级
 */
Difficulty stringToDifficulty(const std::string& str);

} // namespace Reversi
