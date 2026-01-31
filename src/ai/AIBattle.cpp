#include "ai/AIBattle.h"
#include "ai/AIStrategy.h"
#include <algorithm>
#include <random>
#include <iostream>

/**
 * @file AIBattle.cpp
 * @brief AI对战系统实现
 *
 * 实现AI算法之间的自动对战和性能基准测试
 * 支持GUI环境下的进度回调和线程安全
 */

namespace Reversi {

// AIBattle 实现

AIBattle::AIBattle(std::unique_ptr<AIStrategy> blackAI, std::unique_ptr<AIStrategy> whiteAI)
    : blackAI_(std::move(blackAI)), whiteAI_(std::move(whiteAI)) {
    if (!blackAI_ || !whiteAI_) {
        throw std::invalid_argument("AI strategies cannot be null");
    }
}

BattleResult AIBattle::playSingleGame(const SearchLimits& searchLimits) {
    return executeGame(searchLimits);
}

TournamentResult AIBattle::playTournament(int numGames, const SearchLimits& searchLimits,
                                        ProgressCallback progressCallback) {
    TournamentResult result;
    result.blackAIName = getBlackAIName();
    result.whiteAIName = getWhiteAIName();
    result.totalGames = numGames;

    auto tournamentStart = std::chrono::steady_clock::now();

    // 执行多局对战
    for (int game = 0; game < numGames; ++game) {
        BattleResult gameResult = executeGame(searchLimits);

        // 更新锦标赛统计
        result.gameResults.push_back(gameResult);
        result.totalDuration += gameResult.duration;

        if (gameResult.isDraw) {
            result.draws++;
        } else if (gameResult.winner == PlayerColor::Black) {
            result.blackWins++;
        } else {
            result.whiteWins++;
        }

        // 累积AI统计信息
        result.avgBlackStats.nodesExplored += gameResult.blackAIStats.nodesExplored;
        result.avgBlackStats.timeUsed += gameResult.blackAIStats.timeUsed;
        result.avgBlackStats.evaluationCount += gameResult.blackAIStats.evaluationCount;
        result.avgBlackStats.avgBranching += gameResult.blackAIStats.avgBranching;

        result.avgWhiteStats.nodesExplored += gameResult.whiteAIStats.nodesExplored;
        result.avgWhiteStats.timeUsed += gameResult.whiteAIStats.timeUsed;
        result.avgWhiteStats.evaluationCount += gameResult.whiteAIStats.evaluationCount;
        result.avgWhiteStats.avgBranching += gameResult.whiteAIStats.avgBranching;

        // 调用进度回调（适合GUI环境）
        if (progressCallback) {
            progressCallback(game + 1, numGames, gameResult);
        }
    }

    // 计算胜率和平均统计
    result.blackWinRate = static_cast<double>(result.blackWins) / numGames;
    result.whiteWinRate = static_cast<double>(result.whiteWins) / numGames;
    result.drawRate = static_cast<double>(result.draws) / numGames;

    // 计算平均统计
    if (numGames > 0) {
        result.avgBlackStats.nodesExplored /= numGames;
        result.avgBlackStats.timeUsed = std::chrono::milliseconds(
            result.avgBlackStats.timeUsed.count() / numGames);
        result.avgBlackStats.evaluationCount /= numGames;
        result.avgBlackStats.avgBranching /= numGames;

        result.avgWhiteStats.nodesExplored /= numGames;
        result.avgWhiteStats.timeUsed = std::chrono::milliseconds(
            result.avgWhiteStats.timeUsed.count() / numGames);
        result.avgWhiteStats.evaluationCount /= numGames;
        result.avgWhiteStats.avgBranching /= numGames;
    }

    auto tournamentEnd = std::chrono::steady_clock::now();
    result.totalDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
        tournamentEnd - tournamentStart);

    return result;
}

std::string AIBattle::getBlackAIName() const {
    return blackAI_->getName();
}

std::string AIBattle::getWhiteAIName() const {
    return whiteAI_->getName();
}

std::string AIBattle::getBlackAIDescription() const {
    return blackAI_->getDescription();
}

std::string AIBattle::getWhiteAIDescription() const {
    return whiteAI_->getDescription();
}

void AIBattle::setRandomSeed(unsigned int seed) {
    randomSeed_ = seed;
}

BattleResult AIBattle::executeGame(const SearchLimits& searchLimits) {
    BattleResult result;
    auto gameStart = std::chrono::steady_clock::now();

    Board board;
    PlayerColor currentPlayer = PlayerColor::Black;
    std::vector<Move> moveHistory;

    // 对战循环
    while (!board.isGameOver()) {
        AIStrategy& currentAI = (currentPlayer == PlayerColor::Black) ? *blackAI_ : *whiteAI_;
        AIStats& currentStats = (currentPlayer == PlayerColor::Black) ?
                               result.blackAIStats : result.whiteAIStats;

        bool moveSuccess = executeMove(board, currentPlayer, currentAI,
                                     searchLimits, moveHistory, currentStats);

        if (!moveSuccess) {
            // 无法移动，可能是因为没有有效移动或AI出错
            // 这里可以选择跳过回合或结束游戏
            break;
        }

        result.totalMoves++;
        currentPlayer = (currentPlayer == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
    }

    // 确定获胜者
    auto winner = board.getWinner();
    if (winner.has_value()) {
        result.winner = winner.value();
        result.isDraw = false;
    } else {
        result.isDraw = true;
    }

    auto gameEnd = std::chrono::steady_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(gameEnd - gameStart);
    result.moveHistory = std::move(moveHistory);

    return result;
}

bool AIBattle::executeMove(Board& board, PlayerColor currentPlayer, AIStrategy& ai,
                          const SearchLimits& searchLimits, std::vector<Move>& moveHistory,
                          AIStats& aiStats) {
    try {
        // 获取AI的移动决策
        Move move = ai.findBestMove(board, searchLimits);

        // 验证移动是否有效
        auto validMoves = board.getValidMoves();
        bool isValidMove = std::find(validMoves.begin(), validMoves.end(), move) != validMoves.end();

        if (!isValidMove && !move.is_pass) {
            // AI选择了无效移动，这是一个错误
            std::cerr << "AI选择了无效移动: (" << move.row << "," << move.col << ")" << std::endl;
            return false;
        }

        // 执行移动
        bool success = board.makeMove(move);
        if (success || move.is_pass) {  // pass移动也算成功
            moveHistory.push_back(move);

            // 累积AI统计信息
            AIStats moveStats = ai.getStats();
            aiStats.nodesExplored += moveStats.nodesExplored;
            aiStats.timeUsed += moveStats.timeUsed;
            aiStats.evaluationCount += moveStats.evaluationCount;
            aiStats.avgBranching += moveStats.avgBranching;
            aiStats.cutoffs += moveStats.cutoffs;
            aiStats.depthReached = std::max(aiStats.depthReached, moveStats.depthReached);

            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "AI移动执行出错: " << e.what() << std::endl;
    }

    return false;
}

// AIBenchmark 实现

AIBenchmark::BenchmarkResult AIBenchmark::runBenchmark(std::unique_ptr<AIStrategy> ai, int testGames) {
    BenchmarkResult result;
    result.aiName = ai->getName();
    result.testGames = testGames;

    // 创建随机AI作为对手
    auto randomAI = AIStrategyFactory::createRandomAI();
    if (!randomAI) {
        // 如果没有随机AI，使用简单Minimax作为基准
        randomAI = AIStrategyFactory::createMinimaxAI(Difficulty::EASY);
    }

    // 执行测试对战
    AIBattle battle(std::move(ai), std::move(randomAI));
    TournamentResult tournament = battle.playTournament(testGames);

    // 计算基准结果
    if (tournament.blackAIName == result.aiName) {
        result.winRate = tournament.blackWinRate;
        result.avgTimePerMove = std::chrono::milliseconds(
            tournament.avgBlackStats.timeUsed.count() / std::max(1, tournament.totalGames));
        result.avgNodesExplored = tournament.avgBlackStats.nodesExplored;
        result.avgBranchingFactor = tournament.avgBlackStats.avgBranching;
    } else {
        result.winRate = tournament.whiteWinRate;
        result.avgTimePerMove = std::chrono::milliseconds(
            tournament.avgWhiteStats.timeUsed.count() / std::max(1, tournament.totalGames));
        result.avgNodesExplored = tournament.avgWhiteStats.nodesExplored;
        result.avgBranchingFactor = tournament.avgWhiteStats.avgBranching;
    }

    return result;
}

TournamentResult AIBenchmark::compareAIs(std::unique_ptr<AIStrategy> ai1,
                                        std::unique_ptr<AIStrategy> ai2,
                                        int numGames) {
    AIBattle battle(std::move(ai1), std::move(ai2));
    return battle.playTournament(numGames);
}

} // namespace Reversi
