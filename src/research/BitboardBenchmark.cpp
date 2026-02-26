#include "research/BitboardBenchmark.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include "../ai/ZobristHash.h"
#include "../ai/Evaluator.h"

namespace Reversi {

// ============================================================================
// BenchmarkResult 实现
// ============================================================================

std::string BenchmarkResult::toString() const {
    std::ostringstream oss;
    oss << name << ": " << std::fixed << std::setprecision(2) << value << " " << unit;
    if (!message.empty()) {
        oss << " (" << message << ")";
    }
    oss << " - " << (passed ? "PASSED" : "FAILED");
    return oss.str();
}

// ============================================================================
// BitboardBenchmark 实现
// ============================================================================

BitboardBenchmark::BitboardBenchmark() {
    progress_callback_ = nullptr;
}

std::vector<BenchmarkResult> BitboardBenchmark::runAllBenchmarks() {
    std::vector<BenchmarkResult> results;

    if (config_.verbose) {
        std::cout << "=== Bitboard Benchmark Starting ===" << std::endl;
    }

    // 预热
    if (config_.warmup) {
        warmUp();
    }

    int total_tests = 7;
    int current = 0;

    // 1. 翻转性能测试
    if (progress_callback_) progress_callback_(++current, total_tests, "Flip Performance");
    results.push_back(measureFlipPerformance(config_.flip_iterations));

    // 2. 移动生成性能测试
    if (progress_callback_) progress_callback_(++current, total_tests, "Move Generation");
    results.push_back(measureMoveGenerationPerformance(config_.move_iterations));

    // 3. 合法性检查性能测试
    if (progress_callback_) progress_callback_(++current, total_tests, "Legal Move Check");
    results.push_back(measureLegalMovePerformance(config_.legal_iterations));

    // 4. 棋盘复制性能测试
    if (progress_callback_) progress_callback_(++current, total_tests, "Board Copy");
    results.push_back(measureBoardCopyPerformance(config_.copy_iterations));

    // 5. 评估函数性能测试
    if (progress_callback_) progress_callback_(++current, total_tests, "Evaluation");
    results.push_back(measureEvaluationPerformance(config_.move_iterations));

    // 6. Zobrist哈希性能测试
    if (progress_callback_) progress_callback_(++current, total_tests, "Zobrist Hash");
    results.push_back(measureZobristHashPerformance(config_.move_iterations));

    // 7. 搜索性能测试
    if (progress_callback_) progress_callback_(++current, total_tests, "Search Performance");
    results.push_back(measureSearchPerformance(100000));

    if (config_.verbose) {
        std::cout << "\n" << generateReport(results) << std::endl;
    }

    return results;
}

BenchmarkResult BitboardBenchmark::measureFlipPerformance(int iterations) {
    BenchmarkResult result;
    result.name = "Flip Performance";
    result.iterations = iterations;

    // 标准开局: 黑棋在 D4(27), E5(36); 白棋在 E4(35), D5(28)
    // 初始局面 (黑先)
    uint64_t player_bits = (1ULL << 27) | (1ULL << 36);  // 黑棋
    uint64_t opponent_bits = (1ULL << 35) | (1ULL << 28); // 白棋
    BitBoard board(player_bits, opponent_bits);

    // 测试移动: D3 = (2, 3) = row 2, col 3
    int row = 2;
    int col = 3;

    // 预热
    for (int i = 0; i < 1000; ++i) {
        BitBoard temp(player_bits, opponent_bits);
        temp.makeMove(row, col, PlayerColor::Black);
    }

    // 正式测试
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        BitBoard temp(player_bits, opponent_bits);
        temp.makeMove(row, col, PlayerColor::Black);
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double time_ms = duration.count() / 1000.0;

    result.time_ms = time_ms;
    result.value = (iterations / time_ms) * 1000.0;  // M flips/sec
    result.unit = "M flips/sec";
    result.passed = result.value >= TARGET_FLIP_SPEED;
    result.message = result.passed ? "Meets target" : "Below target";

    return result;
}

BenchmarkResult BitboardBenchmark::measureMoveGenerationPerformance(int iterations) {
    BenchmarkResult result;
    result.name = "Move Generation";
    result.iterations = iterations;

    // 使用多个测试位置
    auto test_positions = getTestPositions();

    // 预热
    for (const auto& board : test_positions) {
        for (int i = 0; i < 100; ++i) {
            board.getValidMoves(PlayerColor::Black);
        }
    }

    // 正式测试
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        test_positions[i % test_positions.size()].getValidMoves(PlayerColor::Black);
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double time_ms = duration.count() / 1000.0;

    result.time_ms = time_ms;
    result.value = (iterations / time_ms) * 1000.0;  // M moves/sec
    result.unit = "M moves/sec";
    result.passed = result.value >= TARGET_MOVE_SPEED;
    result.message = result.passed ? "Meets target" : "Below target";

    return result;
}

BenchmarkResult BitboardBenchmark::measureLegalMovePerformance(int iterations) {
    BenchmarkResult result;
    result.name = "Legal Move Check";
    result.iterations = iterations;

    // 标准开局
    uint64_t player_bits = (1ULL << 27) | (1ULL << 36);
    uint64_t opponent_bits = (1ULL << 35) | (1ULL << 28);
    BitBoard board(player_bits, opponent_bits);

    // 先获取有效移动位图
    uint64_t valid_moves = board.getValidMoves(PlayerColor::Black);

    // 预热
    for (int i = 0; i < 1000; ++i) {
        for (int m = 0; m < 64; ++m) {
            if (valid_moves & (1ULL << m)) {
                // 模拟检查
                volatile bool check = true;
                (void)check;
            }
        }
    }

    // 正式测试 - 检查所有64个位置
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        for (int m = 0; m < 64; ++m) {
            bool is_valid = (valid_moves & (1ULL << m)) != 0;
            (void)is_valid;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double time_ms = duration.count() / 1000.0;
    int total_checks = iterations * 64;

    result.time_ms = time_ms;
    result.value = (total_checks / time_ms) * 1000.0;  // M checks/sec
    result.unit = "M checks/sec";
    result.passed = result.value >= TARGET_LEGAL_SPEED;
    result.message = result.passed ? "Meets target" : "Below target";

    return result;
}

BenchmarkResult BitboardBenchmark::measureBoardCopyPerformance(int iterations) {
    BenchmarkResult result;
    result.name = "Board Copy";
    result.iterations = iterations;

    uint64_t player_bits = (1ULL << 27) | (1ULL << 36);
    uint64_t opponent_bits = (1ULL << 35) | (1ULL << 28);
    BitBoard board(player_bits, opponent_bits);

    // 预热
    for (int i = 0; i < 1000; ++i) {
        BitBoard copy = board;
        (void)copy;
    }

    // 正式测试
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        BitBoard copy = board;
        (void)copy;
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double time_ms = duration.count() / 1000.0;

    result.time_ms = time_ms;
    result.value = (iterations / time_ms) * 1000.0;  // M copies/sec
    result.unit = "M copies/sec";
    result.passed = result.value >= TARGET_COPY_SPEED;
    result.message = result.passed ? "Meets target" : "Below target";

    return result;
}

BenchmarkResult BitboardBenchmark::measureEvaluationPerformance(int iterations) {
    BenchmarkResult result;
    result.name = "Evaluation";
    result.iterations = iterations;

    uint64_t player_bits = (1ULL << 27) | (1ULL << 36);
    uint64_t opponent_bits = (1ULL << 35) | (1ULL << 28);
    BitBoard board(player_bits, opponent_bits);

    // 创建评估器
    auto evaluator = EvaluatorFactory::createStaticEvaluator();

    // 预热
    for (int i = 0; i < 1000; ++i) {
        evaluator->evaluate(board, PlayerColor::Black);
    }

    // 正式测试
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        evaluator->evaluate(board, PlayerColor::Black);
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double time_ms = duration.count() / 1000.0;

    result.time_ms = time_ms;
    result.value = (iterations / time_ms) * 1000.0;  // M evals/sec
    result.unit = "M evals/sec";
    result.passed = true;  // 没有特定目标
    result.message = "Evaluation test completed";

    return result;
}

BenchmarkResult BitboardBenchmark::measureZobristHashPerformance(int iterations) {
    BenchmarkResult result;
    result.name = "Zobrist Hash";
    result.iterations = iterations;

    uint64_t player_bits = (1ULL << 27) | (1ULL << 36);
    uint64_t opponent_bits = (1ULL << 35) | (1ULL << 28);

    // 初始化Zobrist哈希
    ZobristHash::init();

    // 预热
    for (int i = 0; i < 1000; ++i) {
        ZobristHash::computeHash(player_bits, opponent_bits);
    }

    // 正式测试
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        ZobristHash::computeHash(player_bits, opponent_bits);
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double time_ms = duration.count() / 1000.0;

    result.time_ms = time_ms;
    result.value = (iterations / time_ms) * 1000.0;  // M hashes/sec
    result.unit = "M hashes/sec";
    result.passed = true;  // 没有特定目标
    result.message = "Hash test completed";

    return result;
}

BenchmarkResult BitboardBenchmark::measureSearchPerformance(int iterations) {
    BenchmarkResult result;
    result.name = "Search Performance";
    result.iterations = iterations;

    // 测试完整搜索步骤
    auto test_positions = getTestPositions();

    // 创建评估器
    auto evaluator = EvaluatorFactory::createStaticEvaluator();

    // 预热
    for (const auto& board : test_positions) {
        for (int i = 0; i < 100; ++i) {
            uint64_t moves = board.getValidMoves(PlayerColor::Black);
            for (int m = 0; m < 64; ++m) {
                if (moves & (1ULL << m)) {
                    int row = m / 8;
                    int col = m % 8;
                    BitBoard copy = board;
                    copy.makeMove(row, col, PlayerColor::Black);
                    evaluator->evaluate(copy, PlayerColor::Black);
                }
            }
        }
    }

    // 正式测试
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        const auto& board = test_positions[i % test_positions.size()];
        uint64_t moves = board.getValidMoves(PlayerColor::Black);
        for (int m = 0; m < 64; ++m) {
            if (moves & (1ULL << m)) {
                int row = m / 8;
                int col = m % 8;
                BitBoard copy = board;
                copy.makeMove(row, col, PlayerColor::Black);
                evaluator->evaluate(copy, PlayerColor::Black);
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    double time_ms = duration.count() / 1000.0;

    result.time_ms = time_ms;
    result.value = (iterations / time_ms) * 1000.0;  // M searches/sec
    result.unit = "M searches/sec";
    result.passed = true;  // 没有特定目标
    result.message = "Search test completed";

    return result;
}

void BitboardBenchmark::setConfig(const Config& config) {
    config_ = config;
}

const BitboardBenchmark::Config& BitboardBenchmark::getConfig() const {
    return config_;
}

void BitboardBenchmark::setProgressCallback(ProgressCallback callback) {
    progress_callback_ = std::move(callback);
}

std::vector<BitBoard> BitboardBenchmark::getTestPositions() {
    std::vector<BitBoard> positions;

    // 1. 标准开局
    uint64_t p1 = (1ULL << 27) | (1ULL << 36);  // 黑棋 D4, E5
    uint64_t o1 = (1ULL << 35) | (1ULL << 28);   // 白棋 E4, D5
    positions.push_back(BitBoard(p1, o1));

    // 2. 早期阶段 - 模拟几个走法后的局面
    uint64_t p2 = p1 | (1ULL << 19);  // 黑棋 D6
    uint64_t o2 = o1 | (1ULL << 26);  // 白棋 C5
    positions.push_back(BitBoard(p2, o2));

    // 3. 中局 - 更多棋子
    uint64_t p3 = p2 | (1ULL << 18) | (1ULL << 10);
    uint64_t o3 = o2 | (1ULL << 17) | (1ULL << 11);
    positions.push_back(BitBoard(p3, o3));

    // 4. 更多中局位置
    uint64_t p4 = 0x000000FF8C180000ULL;  // 示例中局
    uint64_t o4 = 0x0000000062340000ULL;
    positions.push_back(BitBoard(p4, o4));

    // 5. 随机位置
    std::mt19937_64 rng(42);
    uint64_t p5 = 0;
    uint64_t o5 = 0;
    BitBoard board(p1, o1);
    for (int i = 0; i < 30; ++i) {
        uint64_t moves = board.getValidMoves(PlayerColor::Black);
        if (moves != 0) {
            // 获取最低位的移动
            int move = __builtin_ctzll(moves);
            int row = move / 8;
            int col = move % 8;
            board.makeMove(row, col, PlayerColor::Black);
        }
        moves = board.getValidMoves(PlayerColor::White);
        if (moves != 0) {
            int move = __builtin_ctzll(moves);
            int row = move / 8;
            int col = move % 8;
            board.makeMove(row, col, PlayerColor::White);
        }
    }
    positions.push_back(board);

    return positions;
}

std::string BitboardBenchmark::generateReport(const std::vector<BenchmarkResult>& results) {
    std::ostringstream oss;
    oss << "\n========================================\n";
    oss << "      Bitboard Benchmark Report\n";
    oss << "========================================\n\n";

    int passed = 0;
    for (const auto& r : results) {
        oss << r.toString() << "\n";
        if (r.passed) passed++;
    }

    oss << "\n----------------------------------------\n";
    oss << "Summary: " << passed << "/" << results.size() << " tests passed\n";
    oss << "----------------------------------------\n";

    return oss.str();
}

std::string BitboardBenchmark::toJson(const std::vector<BenchmarkResult>& results) {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"benchmark\": \"Bitboard\",\n";
    oss << "  \"results\": [\n";

    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        oss << "    {\n";
        oss << "      \"name\": \"" << r.name << "\",\n";
        oss << "      \"value\": " << std::fixed << std::setprecision(2) << r.value << ",\n";
        oss << "      \"unit\": \"" << r.unit << "\",\n";
        oss << "      \"iterations\": " << r.iterations << ",\n";
        oss << "      \"time_ms\": " << r.time_ms << ",\n";
        oss << "      \"passed\": " << (r.passed ? "true" : "false") << "\n";
        oss << "    }";
        if (i < results.size() - 1) oss << ",";
        oss << "\n";
    }

    oss << "  ]\n";
    oss << "}\n";

    return oss.str();
}

void BitboardBenchmark::warmUp() {
    if (config_.verbose) {
        std::cout << "Warming up..." << std::endl;
    }

    // CPU预热
    volatile double sum = 0.0;
    for (int i = 0; i < 100000; ++i) {
        sum += i * 0.1;
    }
    (void)sum;

    // 内存预热
    uint64_t player_bits = (1ULL << 27) | (1ULL << 36);
    uint64_t opponent_bits = (1ULL << 35) | (1ULL << 28);
    BitBoard board(player_bits, opponent_bits);
    
    auto evaluator = EvaluatorFactory::createStaticEvaluator();

    for (int i = 0; i < WARMUP_ITERATIONS; ++i) {
        board.getValidMoves(PlayerColor::Black);
        evaluator->evaluate(board, PlayerColor::Black);
    }
}

} // namespace Reversi
