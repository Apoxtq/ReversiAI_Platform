#include "ai/Evaluator.h"
#include "core/PlatformUtils.h"
#include <cmath>

/**
 * @file Evaluator.cpp
 * @brief Evaluation function implementation
 *
 * Converts Reversi(Java) evaluation algorithm to C++ BitBoard implementation
 */

namespace Reversi {

// StaticEvaluator Implementation

int StaticEvaluator::evaluate(const BitBoard& board, PlayerColor player) const {
    int mobility = evaluateMobility(board, player);
    int discDiff = evaluateDiscDifference(board, player);
    int corners = evaluateCorners(board, player);
    int position = evaluatePositionWeights(board, player);

    // Weights based on Reversi(Java) classic formula: 2*mob + sc + 1000*evalCorner
    return MOBILITY_WEIGHT * mobility +
           DISC_DIFF_WEIGHT * discDiff +
           CORNER_WEIGHT * corners +
           POSITION_WEIGHT * position;
}

int StaticEvaluator::evaluateDiscDifference(const BitBoard& board, PlayerColor player) const {
    // Reference: StaticEvaluator.evalDiscDiff()
    int myStones = board.getScore(player);
    PlayerColor opponent = (player == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
    int opStones = board.getScore(opponent);

    if (myStones + opStones == 0) return 0;

    // Normalize to range [-100, 100]
    return 100 * (myStones - opStones) / (myStones + opStones);
}

int StaticEvaluator::evaluateMobility(const BitBoard& board, PlayerColor player) const {
    // Reference: StaticEvaluator.evalMobility()
    uint64_t myMoves = board.getValidMoves(player);
    PlayerColor opponent = (player == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
    uint64_t opMoves = board.getValidMoves(opponent);

    int myMoveCount = POPCOUNT64(myMoves);
    int opMoveCount = POPCOUNT64(opMoves);

    // Avoid division by zero
    int totalMoves = myMoveCount + opMoveCount;
    if (totalMoves == 0) return 0;

    // Normalize to range [-100, 100]
    return 100 * (myMoveCount - opMoveCount) / totalMoves;
}

int StaticEvaluator::evaluateCorners(const BitBoard& board, PlayerColor player) const {
    // Reference: StaticEvaluator.evalCorner()
    PlayerColor opponent = (player == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;

    // Check four corners: (0,0), (0,7), (7,0), (7,7)
    int myCorners = 0;
    int opCorners = 0;

    // Bit masks for corner positions
    const uint64_t cornerMasks[4] = {
        (1ULL << 0),    // (0,0)
        (1ULL << 7),    // (0,7)
        (1ULL << 56),   // (7,0)
        (1ULL << 63)    // (7,7)
    };

    // Get correct player/opponent bits from player's perspective
    uint64_t myBits = (player == PlayerColor::Black) ? board.getPlayerBits() : board.getOpponentBits();
    uint64_t opBits = (player == PlayerColor::Black) ? board.getOpponentBits() : board.getPlayerBits();

    for (uint64_t mask : cornerMasks) {
        if (myBits & mask) myCorners++;
        if (opBits & mask) opCorners++;
    }

    int totalCorners = myCorners + opCorners;
    if (totalCorners == 0) return 0;

    return 100 * (myCorners - opCorners) / totalCorners;
}

int StaticEvaluator::evaluatePositionWeights(const BitBoard& board, PlayerColor player) const {
    // Reference: StaticEvaluator.evalBoardMap()
    int score = 0;
    // Get correct player/opponent bits from player's perspective
    uint64_t playerBits = (player == PlayerColor::Black) ? board.getPlayerBits() : board.getOpponentBits();
    uint64_t opponentBits = (player == PlayerColor::Black) ? board.getOpponentBits() : board.getPlayerBits();

    // Iterate over each position
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

// DynamicEvaluator Implementation

void DynamicEvaluator::setGamePhase(double phase) {
    game_phase_ = std::max(0.0, std::min(2.0, phase));  // Clamp to [0, 2] range
}

int DynamicEvaluator::evaluate(const BitBoard& board, PlayerColor player) const {
    // Base static evaluation
    int baseScore = static_evaluator_.evaluate(board, player);

    // Adjust weights based on game phase
    // Opening: emphasize position and mobility
    // Mid-game: balance all factors
    // End-game: emphasize disc count

    double positionWeight = 1.0;
    double mobilityWeight = 1.0;
    double cornerWeight = 1.0;

    if (game_phase_ < 1.0) {
        // Opening phase
        positionWeight = 1.5;
        mobilityWeight = 1.2;
    } else if (game_phase_ < 2.0) {
        // Mid-game phase
        positionWeight = 1.0;
        mobilityWeight = 1.0;
    } else {
        // End-game phase
        positionWeight = 0.5;
        mobilityWeight = 0.3;
        cornerWeight = 0.7;
    }

    // Further refinements can be added here
    // Currently returns base score; extensible to multi-component evaluation

    return baseScore;
}

// EvaluatorFactory Implementation

std::unique_ptr<Evaluator> EvaluatorFactory::createStaticEvaluator() {
    return std::make_unique<StaticEvaluator>();
}

std::unique_ptr<Evaluator> EvaluatorFactory::createDynamicEvaluator() {
    return std::make_unique<DynamicEvaluator>();
}

} // namespace Reversi
