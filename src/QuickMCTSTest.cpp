// Quick MCTS smoke test - 5 games only
#include <iostream>
#include <chrono>
#include <iomanip>
#include "ai/MCTSAI.h"
#include "research/BattleEngine.h"
#include "research/BenchmarkTargets.h"

using namespace Reversi;

void runTest(const std::string& name,
             std::unique_ptr<AIStrategy> ai1,
             std::unique_ptr<AIStrategy> ai2,
             int num_games,
             double target_winrate) {
    std::cout << "\n========================================\n";
    std::cout << "Running: " << name << "\n";
    std::cout << "========================================\n";

    auto start_time = std::chrono::steady_clock::now();

    BattleConfig config;
    config.player1 = std::move(ai1);
    config.player2 = std::move(ai2);
    config.player1_name = config.player1->getName();
    config.player2_name = config.player2->getName();
    config.num_games = num_games;
    config.verbose = false;
    config.random_seed = 42;

    BattleStats stats = BattleEngine::runBattle(config);

    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

    double win_rate = stats.win_rate1 * 100.0;
    bool passed = (target_winrate < 0) ?
        (stats.games.size() == static_cast<size_t>(num_games)) :
        (stats.win_rate1 >= target_winrate);

    std::cout << "\nResults:\n";
    std::cout << "  Games Played: " << stats.total_games << "\n";
    std::cout << "  " << config.player1_name << " Wins: " << stats.player1_wins
              << " (" << std::fixed << std::setprecision(1) << win_rate << "%)\n";
    std::cout << "  " << config.player2_name << " Wins: " << stats.player2_wins << "\n";
    std::cout << "  Draws: " << stats.draws << "\n";
    if (target_winrate >= 0) {
        std::cout << "  Target: >=" << static_cast<int>(target_winrate * 100) << "%\n";
    }
    std::cout << "  Status: " << (passed ? "PASS" : "FAIL") << "\n";
    std::cout << "  Duration: " << duration.count() << " seconds\n";
    std::cout << "========================================\n";
}

int main() {
    std::cout << "Quick MCTS Smoke Test\n";

    // Test 1: MCTS vs Random (should win >90%)
    {
        MCTSConfig mctsConfig;
        mctsConfig.num_simulations = 100;
        mctsConfig.c_puct = 1.0;
        mctsConfig.use_dirichlet_noise = false;
        mctsConfig.use_dynamic_cpuct = false;
        mctsConfig.use_endgame_solver = false;
        mctsConfig.use_mcts_tt = false;
        mctsConfig.use_puct = false;
        auto ai1 = std::make_unique<MCTSAI>(mctsConfig);
        auto ai2 = AIStrategyFactory::createRandomAI();
        runTest("MCTS-100 vs Random",
                std::move(ai1), std::move(ai2),
                5,  // 5 games only
                0.90);
    }

    // Test 2: MCTS-1000 vs Minimax-4 (5 games)
    {
        MCTSConfig mctsConfig;
        mctsConfig.num_simulations = 1000;
        mctsConfig.c_puct = 1.0;
        mctsConfig.use_dirichlet_noise = false;
        mctsConfig.use_dynamic_cpuct = false;
        mctsConfig.use_endgame_solver = false;
        mctsConfig.use_mcts_tt = false;
        mctsConfig.use_puct = false;
        auto ai1 = std::make_unique<MCTSAI>(mctsConfig);
        auto ai2 = AIStrategyFactory::createMinimaxAI(Difficulty::MEDIUM);
        runTest("MCTS-1k vs Minimax-4",
                std::move(ai1), std::move(ai2),
                5,  // 5 games only
                0.05);
    }

    return 0;
}
