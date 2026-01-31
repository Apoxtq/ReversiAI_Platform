#pragma once

#include <memory>
#include "core/BitBoard.h"

/**
 * @file Evaluator.h
 * @brief 评估函数接口和实现
 *
 * 基于Reversi(Java)的评估函数设计，转换为C++实现
 * 参考: Reversi(Java)/src/player/ai/Evaluator.java
 *       Reversi(Java)/src/player/ai/StaticEvaluator.java
 */

namespace Reversi {

class Evaluator;

/**
 * @brief 评估函数接口
 *
 * 参考Reversi(Java)的Evaluator接口设计
 */
class Evaluator {
public:
    virtual ~Evaluator() = default;

    /**
     * @brief 评估棋盘状态
     *
     * @param board 当前棋盘状态
     * @param player 评估的玩家 (1或2，对应PlayerColor)
     * @return 评估分数，越高表示对该玩家越有利
     *
     * 参考: Evaluator.eval(int[][] board, int player)
     */
    virtual int evaluate(const BitBoard& board, PlayerColor player) const = 0;

    /**
     * @brief 获取评估器名称
     */
    virtual std::string getName() const = 0;
};

/**
 * @brief 静态权重评估器
 *
 * 基于Reversi(Java)的StaticEvaluator实现
 * 综合评估: 移动性(2倍权重) + 棋子差异 + 角落控制(1000倍权重)
 */
class StaticEvaluator : public Evaluator {
public:
    int evaluate(const BitBoard& board, PlayerColor player) const override;

    std::string getName() const override { return "StaticEvaluator"; }

private:
    /**
     * @brief 评估棋子数量差异
     *
     * 参考: StaticEvaluator.evalDiscDiff()
     */
    int evaluateDiscDifference(const BitBoard& board, PlayerColor player) const;

    /**
     * @brief 评估移动性
     *
     * 参考: StaticEvaluator.evalMobility()
     */
    int evaluateMobility(const BitBoard& board, PlayerColor player) const;

    /**
     * @brief 评估角落控制
     *
     * 参考: StaticEvaluator.evalCorner()
     */
    int evaluateCorners(const BitBoard& board, PlayerColor player) const;

    /**
     * @brief 评估位置权重
     *
     * 参考: StaticEvaluator.evalBoardMap()
     */
    int evaluatePositionWeights(const BitBoard& board, PlayerColor player) const;

    // 位置权重表 - 基于Reversi(Java)的经典权重
    static constexpr int POSITION_WEIGHTS[8][8] = {
        {200 , -100, 100,  50,  50, 100, -100,  200},
        {-100, -200, -50, -50, -50, -50, -200, -100},
        {100 ,  -50, 100,   0,   0, 100,  -50,  100},
        {50  ,  -50,   0,   0,   0,   0,  -50,   50},
        {50  ,  -50,   0,   0,   0,   0,  -50,   50},
        {100 ,  -50, 100,   0,   0, 100,  -50,  100},
        {-100, -200, -50, -50, -50, -50, -200, -100},
        {200 , -100, 100,  50,  50, 100, -100,  200}
    };

    // 权重因子 - 基于Reversi(Java)的权重分配
    static constexpr int MOBILITY_WEIGHT = 2;
    static constexpr int DISC_DIFF_WEIGHT = 1;
    static constexpr int CORNER_WEIGHT = 1000;
    static constexpr int POSITION_WEIGHT = 1;
};

/**
 * @brief 动态评估器
 *
 * 基于Reversi(Java)的RealtimeEvaluator概念
 * 可以根据游戏阶段动态调整权重
 */
class DynamicEvaluator : public Evaluator {
public:
    /**
     * @brief 设置游戏阶段
     * @param phase 0.0=开局, 1.0=中局, 2.0=残局
     */
    void setGamePhase(double phase);

    int evaluate(const BitBoard& board, PlayerColor player) const override;
    std::string getName() const override { return "DynamicEvaluator"; }

private:
    double game_phase_ = 0.0;  // 游戏阶段
    mutable StaticEvaluator static_evaluator_;  // 基础评估器
};

/**
 * @brief 评估器工厂
 */
class EvaluatorFactory {
public:
    static std::unique_ptr<Evaluator> createStaticEvaluator();
    static std::unique_ptr<Evaluator> createDynamicEvaluator();
};

} // namespace Reversi
