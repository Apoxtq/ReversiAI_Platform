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
 * @brief Experiment Data Export Module
 *
 * Supports exporting experiment data in multiple formats:
 * - JSON (machine readable)
 * - CSV (tabular data)
 * - Markdown (document format)
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @brief Experiment data exporter
 *
 * Provides unified interface for exporting various experiment data
 */
class DataExporter {
public:
    /**
     * @brief Export configuration
     */
    struct ExportConfig {
        std::string output_dir = "benchmark_results";  ///< Output directory
        std::string experiment_name = "experiment";      ///< Experiment name
        bool include_timestamp = true;                   ///< Include timestamp
        bool pretty_print = true;                       ///< Formatted output
    };

    /**
     * @brief Constructor
     */
    DataExporter();

    /**
     * @brief Set export configuration
     */
    void setConfig(const ExportConfig& config);

    /**
     * @brief Get export configuration
     */
    const ExportConfig& getConfig() const;

    // =========================================================================
    // Bitboard Benchmark Export
    // =========================================================================

    /**
     * @brief Export bitboard benchmark results
     *
     * @param results Benchmark results
     * @param filename Filename (without extension)
     * @return Whether successful
     */
    bool exportBitboardResults(
        const std::vector<BenchmarkResult>& results,
        const std::string& filename = "bitboard_benchmark"
    );

    /**
     * @brief Export bitboard results as JSON
     */
    bool exportBitboardToJson(
        const std::vector<BenchmarkResult>& results,
        const std::string& filepath
    );

    /**
     * @brief Export bitboard results as CSV
     */
    bool exportBitboardToCsv(
        const std::vector<BenchmarkResult>& results,
        const std::string& filepath
    );

    /**
     * @brief Export bitboard results as Markdown
     */
    bool exportBitboardToMarkdown(
        const std::vector<BenchmarkResult>& results,
        const std::string& filepath
    );

    // =========================================================================
    // AI Benchmark Export
    // =========================================================================

    /**
     * @brief Export AI benchmark results
     */
    bool exportAIResults(
        const std::vector<AISearchBenchmarkResult>& results,
        const std::string& filename = "ai_benchmark"
    );

    /**
     * @brief Export AI results as JSON
     */
    bool exportAIToJson(
        const std::vector<AISearchBenchmarkResult>& results,
        const std::string& filepath
    );

    /**
     * @brief Export AI results as CSV
     */
    bool exportAIToCsv(
        const std::vector<AISearchBenchmarkResult>& results,
        const std::string& filepath
    );

    /**
     * @brief Export AI results as Markdown
     */
    bool exportAIToMarkdown(
        const std::vector<AISearchBenchmarkResult>& results,
        const std::string& filepath
    );

    // =========================================================================
    // Battle Result Export
    // =========================================================================

    /**
     * @brief Export battle results
     */
    bool exportBattleResults(
        const BattleStats& stats,
        const std::string& filename = "battle_results"
    );

    /**
     * @brief Export battle report as JSON
     */
    bool exportBattleToJson(
        const BattleStats& stats,
        const std::string& filepath
    );

    /**
     * @brief Export battle report as CSV
     */
    bool exportBattleToCsv(
        const BattleStats& stats,
        const std::string& filepath
    );

    /**
     * @brief Export battle report as Markdown
     */
    bool exportBattleToMarkdown(
        const BattleStats& stats,
        const std::string& filepath
    );

    // =========================================================================
    // Comprehensive Report Export
    // =========================================================================

    /**
     * @brief Export complete experiment report
     *
     * Contains comprehensive report of all test results
     *
     * @param bitboard_results Bitboard results
     * @param ai_results AI results
     * @param battle_results Battle results
     * @param filename Filename
     */
    bool exportFullReport(
        const std::vector<BenchmarkResult>& bitboard_results,
        const std::vector<AISearchBenchmarkResult>& ai_results,
        const std::vector<BattleStats>& battle_results,
        const std::string& filename = "full_report"
    );

    /**
     * @brief Generate experiment report JSON
     */
    std::string generateExperimentJson(
        const std::vector<BenchmarkResult>& bitboard_results,
        const std::vector<AISearchBenchmarkResult>& ai_results,
        const std::vector<BattleStats>& battle_results
    );

    /**
     * @brief Generate experiment report Markdown
     */
    std::string generateExperimentMarkdown(
        const std::vector<BenchmarkResult>& bitboard_results,
        const std::vector<AISearchBenchmarkResult>& ai_results,
        const std::vector<BattleStats>& battle_results
    );

    // =========================================================================
    // Utility Functions
    // =========================================================================

    /**
     * @brief Get current timestamp string
     */
    static std::string getTimestamp();

    /**
     * @brief Create filename with timestamp
     */
    static std::string createTimestampedFilename(
        const std::string& base,
        const std::string& extension
    );

    /**
     * @brief Ensure directory exists
     */
    static bool ensureDirectory(const std::string& path);

    /**
     * @brief Format table data as CSV string
     */
    static std::string formatTableAsCsv(
        const std::vector<std::vector<std::string>>& rows
    );

    /**
     * @brief Get system information
     */
    static std::map<std::string, std::string> getSystemInfo();

private:
    ExportConfig config_;

    /**
     * @brief Generate file path
     */
    std::string makePath(const std::string& filename, const std::string& extension);

    /**
     * @brief Write file
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
