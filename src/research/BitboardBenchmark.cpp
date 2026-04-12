#include "research/BitboardBenchmark.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <cassert>
#include "../ai/ZobristHash.h"
#include "../ai/Evaluator.h"

namespace Reversi {

// ============================================================================
// BenchmarkResult Implementation
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
// BitboardBenchmark Implementation
// ============================================================================

BitboardBenchmark::BitboardBenchmark() {
    progress_callback_ = nullptr;
}

std::vector<BenchmarkResult> BitboardBenchmark::runAllBenchmarks() {
    std::vector<BenchmarkResult> results;

    if (config_.verbose) {
        std::cout << "=== Bitboard Benchmark Starting ===" << std::endl;
    }

    // Warmup
    if (config_.warmup) {
        std::cout << "Running warmup..." << std::endl;
        warmUp();
        std::cout << "Warmup complete." << std::endl;
    } else {
        std::cout << "Warmup disabled, skipping." << std::endl;
    }

    int total_tests = 7;
    int current = 0;

    // 1. Flip Performance Test
    if (progress_callback_) progress_callback_(++current, total_tests, "Flip Performance");
    std::cout << "Running test 1: Flip Performance..." << std::endl;
    try {
        results.push_back(measureFlipPerformance(config_.flip_iterations));
        std::cout << "Test 1 complete." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR in test 1] " << e.what() << std::endl;
        throw;
    }

    // 2. Move Generation Performance Test
    if (progress_callback_) progress_callback_(++current, total_tests, "Move Generation");
    std::cout << "Running test 2: Move Generation..." << std::endl;
    results.push_back(measureMoveGenerationPerformance(config_.move_iterations));

    // 3. Legal Move Check Performance Test
    if (progress_callback_) progress_callback_(++current, total_tests, "Legal Move Check");
    results.push_back(measureLegalMovePerformance(config_.legal_iterations));

    // 4. Board Copy Performance Test
    if (progress_callback_) progress_callback_(++current, total_tests, "Board Copy");
    results.push_back(measureBoardCopyPerformance(config_.copy_iterations));

    // 5. Evaluation Function Performance Test
    if (progress_callback_) progress_callback_(++current, total_tests, "Evaluation");
    results.push_back(measureEvaluationPerformance(config_.move_iterations));

    // 6. Zobrist Hash Performance Test
    if (progress_callback_) progress_callback_(++current, total_tests, "Zobrist Hash");
    results.push_back(measureZobristHashPerformance(config_.move_iterations));

    // 7. Search Performance Test
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

    // Standard opening: Black at D4(27), E5(36); White at E4(35), D5(28)
    // Initial position (Black moves first) - Note: player=Black, opponent=White
    uint64_t player_bits = (1ULL << 28) | (1ULL << 35);     // Black positions
    uint64_t opponent_bits = (1ULL << 27) | (1ULL << 36);   // White positions
    BitBoard board(player_bits, opponent_bits);

    // Test move: D3 = (2, 3) = row 2, col 3
    int row = 2;
    int col = 3;

    // Warmup
    for (int i = 0; i < 1000; ++i) {
        BitBoard temp(player_bits, opponent_bits);
        temp.makeMove(row, col, PlayerColor::Black);
    }

    // Actual test
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

    // Use multiple test positions
    std::cout << "  Getting test positions..." << std::endl;
    auto test_positions = getTestPositions();
    std::cout << "  Got " << test_positions.size() << " test positions" << std::endl;

    // Warmup
    std::cout << "  Warming up move generation..." << std::endl;
    for (size_t idx = 0; idx < test_positions.size(); ++idx) {
        const auto& board = test_positions[idx];
        for (int i = 0; i < 100; ++i) {
            board.getValidMoves(PlayerColor::Black);
        }
    }
    std::cout << "  Warmup complete." << std::endl;

    // Actual test
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

    // Standard opening
    uint64_t player_bits = (1ULL << 28) | (1ULL << 35);     // Black positions
    uint64_t opponent_bits = (1ULL << 27) | (1ULL << 36);   // White positions
    BitBoard board(player_bits, opponent_bits);

    // Get valid move bitmap first
    uint64_t valid_moves = board.getValidMoves(PlayerColor::Black);

    // Warmup
    for (int i = 0; i < 1000; ++i) {
        for (int m = 0; m < 64; ++m) {
            if (valid_moves & (1ULL << m)) {
                // Simulate check
                volatile bool check = true;
                (void)check;
            }
        }
    }

    // Actual test - check all 64 positions
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

    uint64_t player_bits = (1ULL << 28) | (1ULL << 35);     // Black positions
    uint64_t opponent_bits = (1ULL << 27) | (1ULL << 36);   // White positions
    BitBoard board(player_bits, opponent_bits);

    // Warmup
    for (int i = 0; i < 1000; ++i) {
        BitBoard copy = board;
        (void)copy;
    }

    // Actual test
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

    uint64_t player_bits = (1ULL << 28) | (1ULL << 35);     // Black positions
    uint64_t opponent_bits = (1ULL << 27) | (1ULL << 36);   // White positions
    BitBoard board(player_bits, opponent_bits);

    // Create evaluator
    auto evaluator = EvaluatorFactory::createStaticEvaluator();

    // Warmup
    for (int i = 0; i < 1000; ++i) {
        evaluator->evaluate(board, PlayerColor::Black);
    }

    // Actual test
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
    result.passed = true;  // No specific target
    result.message = "Evaluation test completed";

    return result;
}

BenchmarkResult BitboardBenchmark::measureZobristHashPerformance(int iterations) {
    BenchmarkResult result;
    result.name = "Zobrist Hash";
    result.iterations = iterations;

    uint64_t player_bits = (1ULL << 28) | (1ULL << 35);     // Black positions
    uint64_t opponent_bits = (1ULL << 27) | (1ULL << 36);   // White positions

    // Initialize Zobrist hash
    ZobristHash::init();

    // Warmup
    for (int i = 0; i < 1000; ++i) {
        ZobristHash::computeHash(player_bits, opponent_bits);
    }

    // Actual test
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
    result.passed = true;  // No specific target
    result.message = "Hash test completed";

    return result;
}

BenchmarkResult BitboardBenchmark::measureSearchPerformance(int iterations) {
    BenchmarkResult result;
    result.name = "Search Performance";
    result.iterations = iterations;

    // Test complete search steps
    auto test_positions = getTestPositions();

    // Create evaluator
    auto evaluator = EvaluatorFactory::createStaticEvaluator();

    // Warmup
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

    // Actual test
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
    result.passed = true;  // No specific target
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

    // 1. Standard opening - Note: player=Black, opponent=White
    uint64_t p1 = (1ULL << 28) | (1ULL << 35);   // Black positions (3,4),(4,3)
    uint64_t o1 = (1ULL << 27) | (1ULL << 36);   // White positions (3,3),(4,4)
    std::cout << "  Position 1: p1=0x" << std::hex << p1 << ", o1=0x" << o1 << std::dec << std::endl;
    positions.push_back(BitBoard(p1, o1));

    // 2. Early stage - position after a few moves
    uint64_t p2 = p1 | (1ULL << 19);  // Black D6
    uint64_t o2 = o1 | (1ULL << 26);  // White C5
    std::cout << "  Position 2: p2=0x" << std::hex << p2 << ", o2=0x" << o2 << std::dec << std::endl;
    positions.push_back(BitBoard(p2, o2));

    // 3. Mid-game - more pieces
    // Fix: o2 initially contains bit 10, conflicts with bit 10 in p3
    // Reallocated: p3 adds bit 11, o3 adds bit 12 (both don't conflict with initial bits {10,19,26,27,35})
    uint64_t p3 = p2 | (1ULL << 18) | (1ULL << 11);
    uint64_t o3 = o2 | (1ULL << 17) | (1ULL << 12);
    std::cout << "  Position 3: p3=0x" << std::hex << p3 << ", o3=0x" << o3 << ", overlap=" << ((p3 & o3) ? "YES" : "no") << std::dec << std::endl;
    positions.push_back(BitBoard(p3, o3));

    // 4. More mid-game positions - use bitmaps known to not overlap (ensure within 64 squares)
    // p4: bits 40-47 (0x00FF000000), o4: bits 48-55 (0xFF00000000)
    uint64_t p4 = 0x00FF000000ULL;  // Black (bits 40-47)
    uint64_t o4 = 0xFF00000000ULL;  // White (bits 48-55)
    // Verify no overlap
    std::cout << "  Position 4: p4=0x" << std::hex << p4 << ", o4=0x" << o4 << ", overlap=" << ((p4 & o4) ? "YES" : "no") << std::dec << std::endl;
    assert((p4 & o4) == 0 && "Player and opponent bits must not overlap");
    positions.push_back(BitBoard(p4, o4));

    // 5. Random positions
    std::mt19937_64 rng(42);
    uint64_t p5 = 0;
    uint64_t o5 = 0;
    BitBoard board(p1, o1);
    for (int i = 0; i < 30; ++i) {
        uint64_t moves = board.getValidMoves(PlayerColor::Black);
        if (moves != 0) {
            // Get least significant bit move (cross-platform compatible)
#ifdef _MSC_VER
            unsigned long move_index;
            _BitScanForward64(&move_index, moves);
            int move = static_cast<int>(move_index);
#else
            int move = __builtin_ctzll(moves);
#endif
            int row = move / 8;
            int col = move % 8;
            board.makeMove(row, col, PlayerColor::Black);
        }
        moves = board.getValidMoves(PlayerColor::White);
        if (moves != 0) {
#ifdef _MSC_VER
            unsigned long move_index;
            _BitScanForward64(&move_index, moves);
            int move = static_cast<int>(move_index);
#else
            int move = __builtin_ctzll(moves);
#endif
            int row = move / 8;
            int col = move % 8;
            board.makeMove(row, col, PlayerColor::White);
        }
    }
    std::cout << "  Position 5: p5=0x" << std::hex << board.getPlayerBits() << ", o5=0x" << board.getOpponentBits() << std::dec << std::endl;
    positions.push_back(board);

    std::cout << "  Total positions created: " << positions.size() << std::endl;
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

    // CPU warmup
    volatile double sum = 0.0;
    for (int i = 0; i < 100000; ++i) {
        sum += i * 0.1;
    }
    (void)sum;

    // Memory warmup - use resetToStandardOpening() to create a valid board
    BitBoard board;
    board.resetToStandardOpening();
    
    auto evaluator = EvaluatorFactory::createStaticEvaluator();

    for (int i = 0; i < WARMUP_ITERATIONS; ++i) {
        board.getValidMoves(PlayerColor::Black);
        evaluator->evaluate(board, PlayerColor::Black);
    }
}

} // namespace Reversi
