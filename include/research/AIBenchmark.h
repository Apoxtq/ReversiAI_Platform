#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <memory>
#include <sstream>
#include <iomanip>
#include "../ai/AIStrategy.h"
#include "../core/BitBoard.h"
#include "BitboardBenchmark.h"

/**
 * @file AIBenchmark.h
 * @brief AI搜索性能基准测试
 *
 * 测量AI算法的搜索吞吐量:
 * - Minimax吞吐量
 * - MCTS仿真率
 * - 搜索深度性能
 *
 * 参考: Egaroucid src/engine/search.hpp
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @brief AI性能测试结果
 */
struct AISearchBenchmarkResult {
    std::string ai_name;         ///< AI名称
    std::string test_name;       ///< 测试名称
    int depth;                   ///< 搜索深度
    int64_t nodes_searched;       ///< 搜索节点数
    double time_ms;               ///< 耗时(毫秒)
    double nps;                   ///< 每秒节点数
    double throughput;            ///< 吞吐量 (M nodes/sec)
    std::string unit;             ///< 单位
    bool passed;                  ///< 是否通过验收标准
    std::string message;          ///< 结果描述

    /**
     * @brief 转换为字符串
     */
    std::string toString() const;
};

/**
 * @brief AI搜索基准测试类
 *
 * 提供系统化的AI性能测试
 */
class AISearchBenchmark {
public:
    // 验收标准 (基于 Reversi_Proposal.md 8.3.2)
    static constexpr double TARGET_MINIMAX6_NPS = 2.0;   // M nodes/sec @ depth 6
    static constexpr double TARGET_MINIMAX8_NPS = 1.0;   // M nodes/sec @ depth 8
    static constexpr double TARGET_MCTS_SIMS = 200.0;    // K sims/sec

    // 默认测试参数
    static constexpr int DEFAULT_TIME_LIMIT_MS = 5000;
    static constexpr int DEFAULT_POSITION_COUNT = 10;

    /**
     * @brief 构造函数
     */
    AISearchBenchmark();

    /**
     * @brief 运行完整AI基准测试
     * @return 测试结果向量
     */
    std::vector<AISearchBenchmarkResult> runFullBenchmark();

    // =========================================================================
    // Minimax性能测试
    // =========================================================================

    /**
     * @brief Minimax性能测试
     *
     * 测量Minimax AI的搜索吞吐量
     *
     * @param ai AI策略
     * @param depth 搜索深度
     * @param time_limit_ms 时间限制(毫秒)
     * @return 测试结果
     */
    AISearchBenchmarkResult benchmarkMinimax(
        AIStrategy& ai,
        int depth,
        int time_limit_ms = DEFAULT_TIME_LIMIT_MS
    );

    /**
     * @brief 测量Minimax吞吐量
     *
     * @param depth 搜索深度
     * @param time_limit_ms 时间限制
     * @return 吞吐量 (M nodes/sec)
     */
    double measureMinimaxThroughput(int depth, int time_limit_ms = DEFAULT_TIME_LIMIT_MS);

    // =========================================================================
    // MCTS性能测试
    // =========================================================================

    /**
     * @brief MCTS性能测试
     *
     * 测量MCTS AI的仿真率
     *
     * @param ai AI策略
     * @param simulations 仿真次数
     * @param time_limit_ms 时间限制
     * @return 测试结果
     */
    AISearchBenchmarkResult benchmarkMCTS(
        AIStrategy& ai,
        int simulations = 1000,
        int time_limit_ms = DEFAULT_TIME_LIMIT_MS
    );

    /**
     * @brief 测量MCTS仿真率
     *
     * @param time_limit_ms 时间限制
     * @return 仿真率 (K sims/sec)
     */
    double measureMCTSSimulationRate(int time_limit_ms = DEFAULT_TIME_LIMIT_MS);

    // =========================================================================
    // 组合测试
    // =========================================================================

    /**
     * @brief 完整AI对比测试
     *
     * 对比多个AI的性能
     *
     * @param ai1 AI1
     * @param ai2 AI2
     * @param num_games 测试局数
     * @return 对比结果
     */
    std::vector<AISearchBenchmarkResult> compareAI(
        AIStrategy& ai1,
        AIStrategy& ai2,
        int num_games = 10
    );

    /**
     * @brief 基准测试配置
     */
    struct Config {
        int time_limit_ms = DEFAULT_TIME_LIMIT_MS;
        int position_count = DEFAULT_POSITION_COUNT;
        bool verbose = true;
        bool warmup = true;
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
     * @brief 获取测试位置
     */
    static std::vector<BitBoard> getTestPositions();

    /**
     * @brief 生成报告
     */
    static std::string generateReport(const std::vector<AISearchBenchmarkResult>& results);

    /**
     * @brief 导出为JSON
     */
    static std::string toJson(const std::vector<AISearchBenchmarkResult>& results);

private:
    Config config_;
    ProgressCallback progress_callback_;

    /**
     * @brief 预热
     */
    void warmUp();
};

} // namespace Reversi
