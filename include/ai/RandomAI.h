#pragma once

#include "ai/AIStrategy.h"
#include <random>

/**
 * @file RandomAI.h
 * @brief 随机AI实现
 *
 * 简单的随机移动AI，用于测试和基准对比
 * 参考: alpha-zero-general的随机策略
 */

namespace Reversi {

/**
 * @class RandomAI
 * @brief 随机AI策略
 *
 * 从有效移动中随机选择一个移动
 * 用于基准测试和作为弱AI对手
 */
class RandomAI : public AIStrategy {
public:
    /**
     * @brief 构造函数
     * @param seed 随机种子（用于重现结果）
     */
    explicit RandomAI(unsigned int seed = 42);

    // AIStrategy接口实现
    Move findBestMove(const Board& board, const SearchLimits& limits) override;
    std::string getName() const override { return "RandomAI"; }
    std::string getDescription() const override;
    Difficulty getDifficulty() const override { return Difficulty::EASY; }
    AIStats getStats() const override;
    std::string getConfigDescription() const override;
    void reset() override;
    bool supportsFeature(const std::string& feature) const override;

    /**
     * @brief 设置随机种子
     * @param seed 随机种子
     */
    void setSeed(unsigned int seed);

private:
    std::mt19937 rng_;                    ///< 随机数生成器
    mutable AIStats stats_;               ///< 统计信息
    unsigned int seed_;                   ///< 当前种子
};

} // namespace Reversi
