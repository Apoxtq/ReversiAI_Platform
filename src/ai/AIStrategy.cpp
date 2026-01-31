#include "ai/AIStrategy.h"
#include "ai/MinimaxAI.h"
#include "ai/MCTSAI.h"
#include "ai/RandomAI.h"
#include <algorithm>

/**
 * @file AIStrategy.cpp
 * @brief AI策略基础实现
 */

namespace Reversi {

// 工具函数实现

std::string difficultyToString(Difficulty difficulty) {
    switch (difficulty) {
        case Difficulty::EASY: return "Easy";
        case Difficulty::MEDIUM: return "Medium";
        case Difficulty::HARD: return "Hard";
        case Difficulty::CUSTOM: return "Custom";
        default: return "Unknown";
    }
}

Difficulty stringToDifficulty(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "easy") return Difficulty::EASY;
    if (lower == "medium") return Difficulty::MEDIUM;
    if (lower == "hard") return Difficulty::HARD;
    if (lower == "custom") return Difficulty::CUSTOM;

    return Difficulty::MEDIUM;  // 默认中等难度
}

// AIStrategyFactory 实现

std::unique_ptr<AIStrategy> AIStrategyFactory::createMinimaxAI(Difficulty difficulty) {
    MinimaxConfig config;

    switch (difficulty) {
        case Difficulty::EASY:
            config.maxDepth = 2;
            config.useAlphaBeta = false;
            config.timeLimit = std::chrono::milliseconds(1000);
            break;
        case Difficulty::MEDIUM:
            config.maxDepth = 4;
            config.useAlphaBeta = true;
            config.timeLimit = std::chrono::milliseconds(3000);
            break;
        case Difficulty::HARD:
            config.maxDepth = 6;
            config.useAlphaBeta = true;
            config.useIterativeDeepening = true;
            config.timeLimit = std::chrono::milliseconds(8000);
            break;
        case Difficulty::CUSTOM:
            // 使用默认配置
            break;
    }

    return std::make_unique<MinimaxAI>(config);
}

std::unique_ptr<AIStrategy> AIStrategyFactory::createMCTSAI(Difficulty difficulty) {
    // 暂时返回nullptr，MCTS AI正在开发中
    return nullptr;
}

std::unique_ptr<AIStrategy> AIStrategyFactory::createRandomAI() {
    return std::make_unique<RandomAI>();
}

std::vector<std::string> AIStrategyFactory::getAvailableAlgorithms() {
    return {"MinimaxAI", "MCTSAI", "RandomAI"};
}

std::unique_ptr<AIStrategy> AIStrategyFactory::createByName(const std::string& name,
                                                           Difficulty difficulty) {
    if (name == "MinimaxAI") {
        return createMinimaxAI(difficulty);
    } else if (name == "MCTSAI") {
        return createMCTSAI(difficulty);
    } else if (name == "RandomAI") {
        return createRandomAI();
    }

    return nullptr;
}

} // namespace Reversi
