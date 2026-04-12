// Performance Benchmark Test Runner
// Tests all performance metrics required by Reversi_Proposal.md Section 8.3.2

#include <iostream>
#include <iomanip>
#include <chrono>
#include "research/BitboardBenchmark.h"
#include "research/AIBenchmark.h"
#include "ai/AIStrategy.h"
#include "ai/MinimaxAI.h"
#include "ai/MCTSAI.h"
#include "ai/RandomAI.h"
#include "research/BenchmarkTargets.h"

using namespace Reversi;

void printSeparator() {
    std::cout << "========================================\n";
}

void printHeader(const std::string& title) {
    printSeparator();
    std::cout << "  " << title << "\n";
    printSeparator();
    std::cout << "\n";
}

void printResult(const std::string& name, double value, const std::string& unit,
                 double target, bool passed) {
    std::cout << std::left << std::setw(30) << name
              << std::right << std::setw(12) << std::fixed << std::setprecision(2) << value << " " << unit
              << std::setw(12) << ">= " << target << " " << unit
              << std::setw(10) << (passed ? "[PASS]" : "[FAIL]") << "\n";
}

int main() {
    std::cout << "\n";
    printHeader("ReversiAI Platform Performance Benchmark");
    std::cout << "Based on: Reversi_Proposal.md Section 8.3.2\n";
    std::cout << "Date: " << __DATE__ << " " << __TIME__ << "\n\n";

    int total_pass = 0;
    int total_tests = 0;

    // ========================================================================
    // Test 1: Bitboard Performance
    // ========================================================================
    printHeader("1. Bitboard Performance Tests");

    BitboardBenchmark bb;
    BitboardBenchmark::Config bbConfig;
    bbConfig.verbose = false;
    bbConfig.warmup = true;
    bb.setConfig(bbConfig);

    auto bbResults = bb.runAllBenchmarks();

    for (const auto& result : bbResults) {
        double target = 0;
        if (result.name.find("Flip") != std::string::npos) {
            target = BenchmarkTargets::BITBOARD_FLIP_SPEED;
        } else if (result.name.find("Move") != std::string::npos) {
            target = BenchmarkTargets::BITBOARD_MOVE_GEN;
        } else if (result.name.find("Legal") != std::string::npos) {
            target = BenchmarkTargets::BITBOARD_LEGAL_CHECK;
        } else if (result.name.find("Copy") != std::string::npos) {
            target = BenchmarkTargets::BITBOARD_COPY;
        } else {
            continue; // Skip other tests
        }

        total_tests++;
        if (result.passed) total_pass++;

        printResult(result.name, result.value, result.unit, target, result.passed);
    }

    std::cout << "\n";

    // ========================================================================
    // Test 2: AI Performance - Minimax Throughput
    // ========================================================================
    printHeader("2. AI Performance Tests");

    // Test Minimax at depth 6
    std::cout << "Testing Minimax (depth 6) throughput...\n";
    auto minimaxAI = AIStrategyFactory::createMinimaxAI(Difficulty::HARD);

    AISearchBenchmark aiBench;
    AISearchBenchmark::Config aiConfig;
    aiConfig.verbose = false;
    aiConfig.warmup = true;
    aiConfig.time_limit_ms = 5000;
    aiBench.setConfig(aiConfig);

    auto minimaxResult = aiBench.benchmarkMinimax(*minimaxAI, 6, 5000);

    total_tests++;
    bool minimaxPassed = minimaxResult.throughput >= BenchmarkTargets::MINIMAX_THROUGHPUT;
    if (minimaxPassed) total_pass++;

    std::cout << std::left << std::setw(30) << "Minimax-6 Throughput"
              << std::right << std::setw(12) << std::fixed << std::setprecision(2)
              << minimaxResult.throughput << " M nodes/s"
              << std::setw(12) << ">= " << BenchmarkTargets::MINIMAX_THROUGHPUT << " M/s"
              << std::setw(10) << (minimaxPassed ? "[PASS]" : "[FAIL]") << "\n";

    std::cout << "\n";

    // ========================================================================
    // Test 3: AI Performance - MCTS Simulation Rate
    // ========================================================================
    printHeader("3. MCTS Simulation Rate Test");

    std::cout << "Testing MCTS (10000 sims) simulation rate...\n";

    MCTSConfig mctsConfig;
    mctsConfig.num_simulations = 10000;
    auto mctsAI = std::make_unique<MCTSAI>(mctsConfig);

    auto mctsResult = aiBench.benchmarkMCTS(*mctsAI, 10000, 5000);

    total_tests++;
    bool mctsPassed = mctsResult.nps >= BenchmarkTargets::MCTS_SIMULATION_RATE;
    if (mctsPassed) total_pass++;

    std::cout << std::left << std::setw(30) << "MCTS Simulation Rate"
              << std::right << std::setw(12) << std::fixed << std::setprecision(2)
              << mctsResult.nps << " K sims/s"
              << std::setw(12) << ">= " << BenchmarkTargets::MCTS_SIMULATION_RATE << " K/s"
              << std::setw(10) << (mctsPassed ? "[PASS]" : "[FAIL]") << "\n";

    std::cout << "\n";

    // ========================================================================
    // Summary
    // ========================================================================
    printHeader("Benchmark Summary");

    std::cout << "Total Tests: " << total_tests << "\n";
    std::cout << "Passed:      " << total_pass << "\n";
    std::cout << "Failed:      " << (total_tests - total_pass) << "\n";
    std::cout << "\n";

    if (total_pass == total_tests) {
        std::cout << "========================================\n";
        std::cout << "  ALL BENCHMARKS PASSED!\n";
        std::cout << "========================================\n";
    } else {
        std::cout << "========================================\n";
        std::cout << "  SOME BENCHMARKS FAILED\n";
        std::cout << "========================================\n";
    }

    std::cout << "\n";
    return (total_pass == total_tests) ? 0 : 1;
}
