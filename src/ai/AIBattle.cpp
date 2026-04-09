#include "ai/AIBattle.h"
#include "ai/AIStrategy.h"
#include <algorithm>
#include <random>
#include <iostream>

/**
 * @file AIBattle.cpp
 * @brief AI battle system implementation
 */

namespace Reversi {

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

    for (int game = 0; game < numGames; ++game) {
        BattleResult gameResult = executeGame(searchLimits);

        result.gameResults.push_back(gameResult);
        result.totalDuration += gameResult.duration;

        if (gameResult.isDraw) {
            result.draws++;
        } else if (gameResult.winner == PlayerColor::Black) {
            result.blackWins++;
        } else {
            result.whiteWins++;
        }

        result.avgBlackStats.nodesExplored += gameResult.blackAIStats.nodesExplored;
        result.avgBlackStats.timeUsed += gameResult.blackAIStats.timeUsed;
        result.avgBlackStats.evaluationCount += gameResult.blackAIStats.evaluationCount;
        result.avgBlackStats.avgBranching += gameResult.blackAIStats.avgBranching;

        result.avgWhiteStats.nodesExplored += gameResult.whiteAIStats.nodesExplored;
        result.avgWhiteStats.timeUsed += gameResult.whiteAIStats.timeUsed;
        result.avgWhiteStats.evaluationCount += gameResult.whiteAIStats.evaluationCount;
        result.avgWhiteStats.avgBranching += gameResult.whiteAIStats.avgBranching;

        if (progressCallback) {
            progressCallback(game + 1, numGames, gameResult);
        }
    }

    result.blackWinRate = static_cast<double>(result.blackWins) / numGames;
    result.whiteWinRate = static_cast<double>(result.whiteWins) / numGames;
    result.drawRate = static_cast<double>(result.draws) / numGames;

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

    while (!board.isGameOver()) {
        AIStrategy& currentAI = (currentPlayer == PlayerColor::Black) ? *blackAI_ : *whiteAI_;
        AIStats& currentStats = (currentPlayer == PlayerColor::Black) ?
                               result.blackAIStats : result.whiteAIStats;

        bool moveSuccess = executeMove(board, currentPlayer, currentAI,
                                     searchLimits, moveHistory, currentStats);

        if (!moveSuccess) {
            break;
        }

        result.totalMoves++;
        currentPlayer = (currentPlayer == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
    }

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
        Move move = ai.findBestMove(board, searchLimits);

        auto validMoves = board.getValidMoves();
        bool isValidMove = std::find(validMoves.begin(), validMoves.end(), move) != validMoves.end();

        if (!isValidMove && !move.is_pass) {
            std::cerr << "AI selected invalid move: (" << move.row << "," << move.col << ")" << std::endl;
            return false;
        }

        bool success = board.makeMove(move);
        if (success || move.is_pass) {
            moveHistory.push_back(move);

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
        std::cerr << "AI move error: " << e.what() << std::endl;
    }

    return false;
}

AIBenchmark::BenchmarkResult AIBenchmark::runBenchmark(std::unique_ptr<AIStrategy> ai, int testGames) {
    BenchmarkResult result;
    result.aiName = ai->getName();
    result.testGames = testGames;

    auto randomAI = AIStrategyFactory::createRandomAI();
    if (!randomAI) {
        randomAI = AIStrategyFactory::createMinimaxAI(Difficulty::EASY);
    }

    AIBattle battle(std::move(ai), std::move(randomAI));
    TournamentResult tournament = battle.playTournament(testGames);

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
