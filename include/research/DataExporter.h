#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include "BitboardBenchmark.h"
#include "AIBenchmark.h"
#include "BattleEngine.h"

/**
 * @file DataExporter.h
 * @brief 实验数据导出模块
 *
 * 支持将实验数据导出为多种格式:
 * - JSON (机器可读)
 * - CSV (表格数据)
 * - Markdown (文档格式)
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @brief 实验数据导出器
 *
 * 提供统一的接口导出各种实验数据
 */
class DataExporter {
public:
    /**
     * @brief 导出配置
     */
    struct ExportConfig {
        std::string output_dir = "benchmark_results";  ///< 输出目录
        std::string experiment_name = "experiment";      ///< 实验名称
        bool include_timestamp = true;                   ///< 包含时间戳
        bool pretty_print = true;                       ///< 格式化输出
    };

    /**
     * @brief 构造函数
     */
    DataExporter();

    /**
     * @brief 设置导出配置
     */
    void setConfig(const ExportConfig& config);

    /**
     * @brief 获取导出配置
     */
    const ExportConfig& getConfig() const;

    // =========================================================================
    // Bitboard基准测试导出
    // =========================================================================

    /**
     * @brief 导出Bitboard基准测试结果
     *
     * @param results 基准测试结果
     * @param filename 文件名 (不含扩展名)
     * @return 是否成功
     */
    bool exportBitboardResults(
        const std::vector<BenchmarkResult>& results,
        const std::string& filename = "bitboard_benchmark"
    );

    /**
     * @brief 导出Bitboard结果为JSON
     */
    bool exportBitboardToJson(
        const std::vector<BenchmarkResult>& results,
        const std::string& filepath
    );

    /**
     * @brief 导出Bitboard结果为CSV
     */
    bool exportBitboardToCsv(
        const std::vector<BenchmarkResult>& results,
        const std::string& filepath
    );

    /**
     * @brief 导出Bitboard结果为Markdown
     */
    bool exportBitboardToMarkdown(
        const std::vector<BenchmarkResult>& results,
        const std::string& filepath
    );

    // =========================================================================
    // AI基准测试导出
    // =========================================================================

    /**
     * @brief 导出AI基准测试结果
     */
    bool exportAIResults(
        const std::vector<AISearchBenchmarkResult>& results,
        const std::string& filename = "ai_benchmark"
    );

    /**
     * @brief 导出AI结果为JSON
     */
    bool exportAIToJson(
        const std::vector<AISearchBenchmarkResult>& results,
        const std::string& filepath
    );

    /**
     * @brief 导出AI结果为CSV
     */
    bool exportAIToCsv(
        const std::vector<AISearchBenchmarkResult>& results,
        const std::string& filepath
    );

    /**
     * @brief 导出AI结果为Markdown
     */
    bool exportAIToMarkdown(
        const std::vector<AISearchBenchmarkResult>& results,
        const std::string& filepath
    );

    // =========================================================================
    // 对战结果导出
    // =========================================================================

    /**
     * @brief 导出对战结果
     */
    bool exportBattleResults(
        const BattleStats& stats,
        const std::string& filename = "battle_results"
    );

    /**
     * @brief 导出战报为JSON
     */
    bool exportBattleToJson(
        const BattleStats& stats,
        const std::string& filepath
    );

    /**
     * @brief 导出战报为CSV
     */
    bool exportBattleToCsv(
        const BattleStats& stats,
        const std::string& filepath
    );

    /**
     * @brief 导出战报为Markdown
     */
    bool exportBattleToMarkdown(
        const BattleStats& stats,
        const std::string& filepath
    );

    // =========================================================================
    // 综合报告导出
    // =========================================================================

    /**
     * @brief 导出完整实验报告
     *
     * 包含所有测试结果的综合报告
     *
     * @param bitboard_results Bitboard结果
     * @param ai_results AI结果
     * @param battle_results 对战结果
     * @param filename 文件名
     */
    bool exportFullReport(
        const std::vector<BenchmarkResult>& bitboard_results,
        const std::vector<AISearchBenchmarkResult>& ai_results,
        const std::vector<BattleStats>& battle_results,
        const std::string& filename = "full_report"
    );

    /**
     * @brief 生成实验报告JSON
     */
    std::string generateExperimentJson(
        const std::vector<BenchmarkResult>& bitboard_results,
        const std::vector<AISearchBenchmarkResult>& ai_results,
        const std::vector<BattleStats>& battle_results
    );

    /**
     * @brief 生成实验报告Markdown
     */
    std::string generateExperimentMarkdown(
        const std::vector<BenchmarkResult>& bitboard_results,
        const std::vector<AISearchBenchmarkResult>& ai_results,
        const std::vector<BattleStats>& battle_results
    );

    // =========================================================================
    // 工具函数
    // =========================================================================

    /**
     * @brief 获取当前时间戳字符串
     */
    static std::string getTimestamp();

    /**
     * @brief 创建带时间戳的文件名
     */
    static std::string createTimestampedFilename(
        const std::string& base,
        const std::string& extension
    );

    /**
     * @brief 确保目录存在
     */
    static bool ensureDirectory(const std::string& path);

    /**
     * @brief 格式化表格数据为CSV字符串
     */
    static std::string formatTableAsCsv(
        const std::vector<std::vector<std::string>>& rows
    );

    /**
     * @brief 获取系统信息
     */
    static std::map<std::string, std::string> getSystemInfo();

private:
    ExportConfig config_;

    /**
     * @brief 生成文件路径
     */
    std::string makePath(const std::string& filename, const std::string& extension);

    /**
     * @brief 写入文件
     */
    bool writeFile(const std::string& path, const std::string& content);
};

// ============================================================================
// 工具函数实现
// ============================================================================

inline std::string DataExporter::getTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", std::localtime(&now));
    return std::string(buf);
}

inline std::string DataExporter::createTimestampedFilename(
    const std::string& base,
    const std::string& extension
) {
    return base + "_" + getTimestamp() + "." + extension;
}

} // namespace Reversi
