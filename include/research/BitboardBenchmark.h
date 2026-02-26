#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include "../core/BitBoard.h"
#include "../Board.h"

/**
 * @file BitboardBenchmark.h
 * @brief 位棋盘性能基准测试
 *
 * 测量位棋盘操作的极限性能，包括:
 * - 翻转速度
 * - 移动生成
 * - 合法性检查
 * - 棋盘复制
 *
 * 参考: Egaroucid src/tools/benchmark/
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @brief 性能测试结果
 */
struct BenchmarkResult {
    std::string name;             ///< 测试名称
    double value;                 ///< 测量值
    std::string unit;             ///< 单位
    int iterations;               ///< 迭代次数
    double time_ms;               ///< 耗时(毫秒)
    bool passed;                  ///< 是否通过验收标准
    std::string message;          ///< 结果描述

    /**
     * @brief 获取格式化字符串
     */
    std::string toString() const;
};

/**
 * @brief 位棋盘基准测试类
 *
 * 提供系统化的性能测试，用于验证和优化位棋盘操作
 */
class BitboardBenchmark {
public:
    // 预热迭代次数
    static constexpr int WARMUP_ITERATIONS = 1000;
    // 默认测试迭代次数
    static constexpr int DEFAULT_FLIP_ITERATIONS = 10000000;
    static constexpr int DEFAULT_MOVE_ITERATIONS = 1000000;
    static constexpr int DEFAULT_LEGAL_ITERATIONS = 1000000;
    static constexpr int DEFAULT_COPY_ITERATIONS = 1000000;

    // 验收标准 (基于 Reversi_Proposal.md 8.3.2)
    static constexpr double TARGET_FLIP_SPEED = 100.0;    // M flips/sec
    static constexpr double TARGET_MOVE_SPEED = 50.0;     // M moves/sec
    static constexpr double TARGET_LEGAL_SPEED = 100.0;   // M checks/sec
    static constexpr double TARGET_COPY_SPEED = 20.0;     // M copies/sec

    /**
     * @brief 构造函数
     */
    BitboardBenchmark();

    /**
     * @brief 运行所有Bitboard基准测试
     * @return 测试结果向量
     */
    std::vector<BenchmarkResult> runAllBenchmarks();

    // =========================================================================
    // 核心性能测试
    // =========================================================================

    /**
     * @brief 翻转性能测试
     *
     * 测量 flip() 方法的性能
     *
     * @param iterations 迭代次数
     * @return 测试结果
     */
    BenchmarkResult measureFlipPerformance(int iterations = DEFAULT_FLIP_ITERATIONS);

    /**
     * @brief 移动生成性能测试
     *
     * 测量 generateMoves() 方法的性能
     *
     * @param iterations 迭代次数
     * @return 测试结果
     */
    BenchmarkResult measureMoveGenerationPerformance(int iterations = DEFAULT_MOVE_ITERATIONS);

    /**
     * @brief 合法性检查性能测试
     *
     * 测量 isValidMove() 方法的性能
     *
     * @param iterations 迭代次数
     * @return 测试结果
     */
    BenchmarkResult measureLegalMovePerformance(int iterations = DEFAULT_LEGAL_ITERATIONS);

    /**
     * @brief 棋盘复制性能测试
     *
     * 测量 copy constructor 的性能
     *
     * @param iterations 迭代次数
     * @return 测试结果
     */
    BenchmarkResult measureBoardCopyPerformance(int iterations = DEFAULT_COPY_ITERATIONS);

    /**
     * @brief 评估函数性能测试
     *
     * 测量 evaluate() 方法的性能
     *
     * @param iterations 迭代次数
     * @return 测试结果
     */
    BenchmarkResult measureEvaluationPerformance(int iterations = DEFAULT_MOVE_ITERATIONS);

    /**
     * @brief Zobrist哈希性能测试
     *
     * 测量 zobrist hashing 的性能
     *
     * @param iterations 迭代次数
     * @return 测试结果
     */
    BenchmarkResult measureZobristHashPerformance(int iterations = DEFAULT_MOVE_ITERATIONS);

    // =========================================================================
    // 组合测试
    // =========================================================================

    /**
     * @brief 搜索性能测试
     *
     * 测量完整搜索的性能 (生成走法 + 翻转 + 评估)
     *
     * @param iterations 迭代次数
     * @return 测试结果
     */
    BenchmarkResult measureSearchPerformance(int iterations = 100000);

    /**
     * @brief 基准测试配置
     */
    struct Config {
        int flip_iterations = DEFAULT_FLIP_ITERATIONS;
        int move_iterations = DEFAULT_MOVE_ITERATIONS;
        int legal_iterations = DEFAULT_LEGAL_ITERATIONS;
        int copy_iterations = DEFAULT_COPY_ITERATIONS;
        bool verbose = true;           ///< 详细输出
        bool warmup = true;            ///< 预热
    };

    /**
     * @brief 设置测试配置
     */
    void setConfig(const Config& config);

    /**
     * @brief 获取测试配置
     */
    const Config& getConfig() const;

    /**
     * @brief 测试回调函数类型
     */
    using ProgressCallback = std::function<void(int current, int total, const std::string& name)>;

    /**
     * @brief 设置进度回调
     */
    void setProgressCallback(ProgressCallback callback);

    /**
     * @brief 获取测试位置列表
     *
     * 用于性能测试的标准位置
     *
     * @return 棋盘状态列表
     */
    static std::vector<BitBoard> getTestPositions();

    /**
     * @brief 获取摘要报告
     *
     * @param results 测试结果
     * @return 格式化的报告字符串
     */
    static std::string generateReport(const std::vector<BenchmarkResult>& results);

    /**
     * @brief 导出结果为JSON格式
     *
     * @param results 测试结果
     * @return JSON字符串
     */
    static std::string toJson(const std::vector<BenchmarkResult>& results);

private:
    Config config_;
    ProgressCallback progress_callback_;

    /**
     * @brief 预热CPU缓存
     */
    void warmUp();

    /**
     * @brief 执行计时测试
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
