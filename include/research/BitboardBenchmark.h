#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include "../core/BitBoard.h"
#include "../Board.h"

/**
 * @file BitboardBenchmark.h
 * @brief Bitboard Performance Benchmark
 *
 * Measures bitboard operation performance limits including:
 * - Flip speed
 * - Move generation
 * - Legality check
 * - Board copy
 *
 * Reference: Egaroucid src/tools/benchmark/
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @brief Performance test result
 */
struct BenchmarkResult {
    std::string name;             ///< Test name
    double value;                 ///< Measured value
    std::string unit;             ///< Unit
    int iterations;               ///< Number of iterations
    double time_ms;               ///< Time elapsed (milliseconds)
    bool passed;                  ///< Whether passed acceptance criteria
    std::string message;          ///< Result description

    /**
     * @brief Get formatted string
     */
    std::string toString() const;
};

/**
 * @brief Bitboard benchmark class
 *
 * Provides systematic performance testing for bitboard operations
 */
class BitboardBenchmark {
public:
    // Warmup iterations
    static constexpr int WARMUP_ITERATIONS = 1000;
    // Default test iterations
    static constexpr int DEFAULT_FLIP_ITERATIONS = 10000000;
    static constexpr int DEFAULT_MOVE_ITERATIONS = 1000000;
    static constexpr int DEFAULT_LEGAL_ITERATIONS = 1000000;
    static constexpr int DEFAULT_COPY_ITERATIONS = 1000000;

    // Acceptance criteria (based on Reversi_Proposal.md 8.3.2)
    static constexpr double TARGET_FLIP_SPEED = 100.0;    // M flips/sec
    static constexpr double TARGET_MOVE_SPEED = 50.0;     // M moves/sec
    static constexpr double TARGET_LEGAL_SPEED = 100.0;   // M checks/sec
    static constexpr double TARGET_COPY_SPEED = 20.0;     // M copies/sec

    /**
     * @brief Constructor
     */
    BitboardBenchmark();

    /**
     * @brief Run all bitboard benchmarks
     * @return Test result vector
     */
    std::vector<BenchmarkResult> runAllBenchmarks();

    // =========================================================================
    // Core Performance Tests
    // =========================================================================

    /**
     * @brief Flip performance test
     *
     * Measures flip() method performance
     *
     * @param iterations Number of iterations
     * @return Test result
     */
    BenchmarkResult measureFlipPerformance(int iterations = DEFAULT_FLIP_ITERATIONS);

    /**
     * @brief Move generation performance test
     *
     * Measures generateMoves() method performance
     *
     * @param iterations Number of iterations
     * @return Test result
     */
    BenchmarkResult measureMoveGenerationPerformance(int iterations = DEFAULT_MOVE_ITERATIONS);

    /**
     * @brief Legality check performance test
     *
     * Measures isValidMove() method performance
     *
     * @param iterations Number of iterations
     * @return Test result
     */
    BenchmarkResult measureLegalMovePerformance(int iterations = DEFAULT_LEGAL_ITERATIONS);

    /**
     * @brief Board copy performance test
     *
     * Measures copy constructor performance
     *
     * @param iterations Number of iterations
     * @return Test result
     */
    BenchmarkResult measureBoardCopyPerformance(int iterations = DEFAULT_COPY_ITERATIONS);

    /**
     * @brief Evaluation function performance test
     *
     * Measures evaluate() method performance
     *
     * @param iterations Number of iterations
     * @return Test result
     */
    BenchmarkResult measureEvaluationPerformance(int iterations = DEFAULT_MOVE_ITERATIONS);

    /**
     * @brief Zobrist hash performance test
     *
     * Measures zobrist hashing performance
     *
     * @param iterations Number of iterations
     * @return Test result
     */
    BenchmarkResult measureZobristHashPerformance(int iterations = DEFAULT_MOVE_ITERATIONS);

    // =========================================================================
    // Combined Tests
    // =========================================================================

    /**
     * @brief Search performance test
     *
     * Measures full search performance (generate moves + flip + evaluate)
     *
     * @param iterations Number of iterations
     * @return Test result
     */
    BenchmarkResult measureSearchPerformance(int iterations = 100000);

    /**
     * @brief Benchmark configuration
     */
    struct Config {
        int flip_iterations = DEFAULT_FLIP_ITERATIONS;
        int move_iterations = DEFAULT_MOVE_ITERATIONS;
        int legal_iterations = DEFAULT_LEGAL_ITERATIONS;
        int copy_iterations = DEFAULT_COPY_ITERATIONS;
        bool verbose = true;           ///< Verbose output
        bool warmup = true;           ///< Warmup
    };

    /**
     * @brief Set test configuration
     */
    void setConfig(const Config& config);

    /**
     * @brief Get test configuration
     */
    const Config& getConfig() const;

    /**
     * @brief Test callback function type
     */
    using ProgressCallback = std::function<void(int current, int total, const std::string& name)>;

    /**
     * @brief Set progress callback
     */
    void setProgressCallback(ProgressCallback callback);

    /**
     * @brief Get test position list
     *
     * Standard positions for performance testing
     *
     * @return Board state list
     */
    static std::vector<BitBoard> getTestPositions();

    /**
     * @brief Generate summary report
     *
     * @param results Test results
     * @return Formatted report string
     */
    static std::string generateReport(const std::vector<BenchmarkResult>& results);

    /**
     * @brief Export results as JSON
     *
     * @param results Test results
     * @return JSON string
     */
    static std::string toJson(const std::vector<BenchmarkResult>& results);

private:
    Config config_;
    ProgressCallback progress_callback_;

    /**
     * @brief Warmup CPU cache
     */
    void warmUp();

    /**
     * @brief Execute timed test
     */
    template<typename Func>
    double measureTime(Func func, int iterations);
};

// 模板实现
template<typename Func>
double BitboardBenchmark::measureTime(Func func, int iterations) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        func();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / 1000.0;  // 转换为毫秒
}

} // namespace Reversi
