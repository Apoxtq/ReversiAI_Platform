#include "ai/RandomAI.h"
#include <algorithm>

/**
 * @file RandomAI.cpp
 * @brief Random AI implementation
 */

namespace Reversi {

RandomAI::RandomAI(unsigned int seed) : seed_(seed), rng_(seed) {}

Move RandomAI::findBestMove(const Board& board, const SearchLimits& limits) {
    auto validMoves = board.getValidMoves();

    if (validMoves.empty()) {
        return Move::pass();
    }

    std::uniform_int_distribution<size_t> dist(0, validMoves.size() - 1);
    size_t randomIndex = dist(rng_);

    stats_.nodesExplored = 1;
    stats_.evaluationCount = 0;
    stats_.avgBranching = static_cast<double>(validMoves.size());

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
    rng_.seed(seed_);
}

bool RandomAI::supportsFeature(const std::string& feature) const {
    if (feature == "deterministic") return true;
    if (feature == "fast") return true;
    return AIStrategy::supportsFeature(feature);
}

void RandomAI::setColor(PlayerColor color) {
    (void)color;
}

PlayerColor RandomAI::getColor() const {
    return PlayerColor::Black;
}

void RandomAI::setSeed(unsigned int seed) {
    seed_ = seed;
    rng_.seed(seed);
}

} // namespace Reversi
