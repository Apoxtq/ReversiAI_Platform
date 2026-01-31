#include "ai/RandomAI.h"
#include <algorithm>

/**
 * @file RandomAI.cpp
 * @brief 随机AI实现
 */

namespace Reversi {

RandomAI::RandomAI(unsigned int seed) : seed_(seed), rng_(seed) {}

Move RandomAI::findBestMove(const Board& board, const SearchLimits& limits) {
    auto validMoves = board.getValidMoves();

    if (validMoves.empty()) {
        // 没有有效移动，返回跳过
        return Move::pass();
    }

    // 从有效移动中随机选择一个
    std::uniform_int_distribution<size_t> dist(0, validMoves.size() - 1);
    size_t randomIndex = dist(rng_);

    // 更新统计信息
    stats_.nodesExplored = 1;  // 只探索了一个"选择"
    stats_.evaluationCount = 0;  // 不需要评估
    stats_.avgBranching = validMoves.size();

    return validMoves[randomIndex];
}

std::string RandomAI::getDescription() const {
    return "Random AI that selects moves uniformly at random from legal moves. "
           "Used as a baseline for performance comparison.";
}

AIStats RandomAI::getStats() const {
    return stats_;
}

std::string RandomAI::getConfigDescription() const {
    return "Seed: " + std::to_string(seed_);
}

void RandomAI::reset() {
    stats_ = AIStats{};
    rng_.seed(seed_);  // 重新初始化随机数生成器
}

bool RandomAI::supportsFeature(const std::string& feature) const {
    if (feature == "deterministic") return true;  // 给定种子时是确定性的
    if (feature == "fast") return true;           // 执行很快
    return AIStrategy::supportsFeature(feature);
}

void RandomAI::setSeed(unsigned int seed) {
    seed_ = seed;
    rng_.seed(seed);
}

} // namespace Reversi
