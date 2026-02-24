#include "ai/Evaluator.h"
#include "core/PlatformUtils.h"
#include <cmath>

/**
 * @file Evaluator.cpp
 * @brief 评估函数实现
 *
 * 将Reversi(Java)的评估算法转换为C++ BitBoard实现
 */

namespace Reversi {

// StaticEvaluator 实现

int StaticEvaluator::evaluate(const BitBoard& board, PlayerColor player) const {
    int mobility = evaluateMobility(board, player);
    int discDiff = evaluateDiscDifference(board, player);
    int corners = evaluateCorners(board, player);
    int position = evaluatePositionWeights(board, player);

    // 权重分配基于Reversi(Java)的经典公式: 2*mob + sc + 1000*evalCorner
    return MOBILITY_WEIGHT * mobility +
           DISC_DIFF_WEIGHT * discDiff +
           CORNER_WEIGHT * corners +
           POSITION_WEIGHT * position;
}

int StaticEvaluator::evaluateDiscDifference(const BitBoard& board, PlayerColor player) const {
    // 参考: StaticEvaluator.evalDiscDiff()
    int myStones = board.getScore(player);
    PlayerColor opponent = (player == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
    int opStones = board.getScore(opponent);

    if (myStones + opStones == 0) return 0;

    // 归一化到-100到100的范围
    return 100 * (myStones - opStones) / (myStones + opStones);
}

int StaticEvaluator::evaluateMobility(const BitBoard& board, PlayerColor player) const {
    // 参考: StaticEvaluator.evalMobility()
    uint64_t myMoves = board.getValidMoves(player);
    PlayerColor opponent = (player == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
    uint64_t opMoves = board.getValidMoves(opponent);

    int myMoveCount = POPCOUNT64(myMoves);
    int opMoveCount = POPCOUNT64(opMoves);

    // 避免除零错误
    int totalMoves = myMoveCount + opMoveCount;
    if (totalMoves == 0) return 0;

    // 归一化到-100到100的范围
    return 100 * (myMoveCount - opMoveCount) / totalMoves;
}

int StaticEvaluator::evaluateCorners(const BitBoard& board, PlayerColor player) const {
    // 参考: StaticEvaluator.evalCorner()
    PlayerColor opponent = (player == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;

    // 检查四个角落 (0,0), (0,7), (7,0), (7,7)
    int myCorners = 0;
    int opCorners = 0;

    // 角落位置的位掩码
    const uint64_t cornerMasks[4] = {
        (1ULL << 0),    // (0,0)
        (1ULL << 7),    // (0,7)
        (1ULL << 56),   // (7,0)
        (1ULL << 63)    // (7,7)
    };

    for (uint64_t mask : cornerMasks) {
        if (board.getPlayerBits() & mask) myCorners++;
        if (board.getOpponentBits() & mask) opCorners++;
    }

    int totalCorners = myCorners + opCorners;
    if (totalCorners == 0) return 0;

    return 100 * (myCorners - opCorners) / totalCorners;
}

int StaticEvaluator::evaluatePositionWeights(const BitBoard& board, PlayerColor player) const {
    // 参考: StaticEvaluator.evalBoardMap()
    int score = 0;
    uint64_t playerBits = board.getPlayerBits();
    uint64_t opponentBits = board.getOpponentBits();

    // 遍历每个位置
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            int pos = row * 8 + col;
            uint64_t mask = (1ULL << pos);

            if (playerBits & mask) {
                score += POSITION_WEIGHTS[row][col];
            } else if (opponentBits & mask) {
                score -= POSITION_WEIGHTS[row][col];
            }
        }
    }

    return score;
}

// DynamicEvaluator 实现

void DynamicEvaluator::setGamePhase(double phase) {
    game_phase_ = std::max(0.0, std::min(2.0, phase));  // 限制在0-2范围
}

int DynamicEvaluator::evaluate(const BitBoard& board, PlayerColor player) const {
    // 基础静态评估
    int baseScore = static_evaluator_.evaluate(board, player);

    // 根据游戏阶段调整权重
    // 开局: 强调位置和移动性
    // 中局: 平衡各项因素
    // 残局: 强调棋子数量

    double positionWeight = 1.0;
    double mobilityWeight = 1.0;
    double cornerWeight = 1.0;

    if (game_phase_ < 1.0) {
        // 开局阶段
        positionWeight = 1.5;
        mobilityWeight = 1.2;
    } else if (game_phase_ < 2.0) {
        // 中局阶段
        positionWeight = 1.0;
        mobilityWeight = 1.0;
    } else {
        // 残局阶段
        positionWeight = 0.5;
        mobilityWeight = 0.3;
        cornerWeight = 0.7;
    }

    // 这里可以进一步细化评估逻辑
    // 目前返回基础分数，将来可以扩展为多组件评估

    return baseScore;
}

// EvaluatorFactory 实现

std::unique_ptr<Evaluator> EvaluatorFactory::createStaticEvaluator() {
    return std::make_unique<StaticEvaluator>();
}

std::unique_ptr<Evaluator> EvaluatorFactory::createDynamicEvaluator() {
    return std::make_unique<DynamicEvaluator>();
}

} // namespace Reversi
