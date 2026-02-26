#include "research/DataExporter.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

namespace Reversi {

// ============================================================================
// DataExporter 实现
// ============================================================================

DataExporter::DataExporter() {
    // 默认配置
}

void DataExporter::setConfig(const ExportConfig& config) {
    config_ = config;
}

const DataExporter::ExportConfig& DataExporter::getConfig() const {
    return config_;
}

// ============================================================================
// Bitboard 结果导出
// ============================================================================

bool DataExporter::exportBitboardResults(
    const std::vector<BenchmarkResult>& results,
    const std::string& filename
) {
    bool success = true;
    
    // 导出为JSON
    success &= exportBitboardToJson(results, makePath(filename, "json"));
    
    // 导出为CSV
    success &= exportBitboardToCsv(results, makePath(filename, "csv"));
    
    // 导出为Markdown
    success &= exportBitboardToMarkdown(results, makePath(filename, "md"));
    
    return success;
}

bool DataExporter::exportBitboardToJson(
    const std::vector<BenchmarkResult>& results,
    const std::string& filepath
) {
    auto json = BitboardBenchmark::toJson(results);
    return writeFile(filepath, json);
}

bool DataExporter::exportBitboardToCsv(
    const std::vector<BenchmarkResult>& results,
    const std::string& filepath
) {
    std::ostringstream oss;
    oss << "Test Name,Value,Unit,Iterations,Time (ms),Passed,Message\n";
    
    for (const auto& r : results) {
        oss << "\"" << r.name << "\","
            << std::fixed << std::setprecision(2) << r.value << ","
            << "\"" << r.unit << "\","
            << r.iterations << ","
            << std::fixed << std::setprecision(2) << r.time_ms << ","
            << (r.passed ? "true" : "false") << ","
            << "\"" << r.message << "\"\n";
    }
    
    return writeFile(filepath, oss.str());
}

bool DataExporter::exportBitboardToMarkdown(
    const std::vector<BenchmarkResult>& results,
    const std::string& filepath
) {
    std::ostringstream oss;
    oss << "# Bitboard Benchmark Results\n\n";
    oss << "Generated: " << getTimestamp() << "\n\n";
    
    oss << "| Test Name | Value | Unit | Iterations | Time (ms) | Status |\n";
    oss << "|-----------|-------|------|------------|----------|--------|\n";
    
    for (const auto& r : results) {
        oss << "| " << r.name << " | "
            << std::fixed << std::setprecision(2) << r.value << " | "
            << r.unit << " | "
            << r.iterations << " | "
            << std::fixed << std::setprecision(2) << r.time_ms << " | "
            << (r.passed ? "PASSED" : "FAILED") << " |\n";
    }
    
    return writeFile(filepath, oss.str());
}

// ============================================================================
// AI 结果导出
// ============================================================================

bool DataExporter::exportAIResults(
    const std::vector<AISearchBenchmarkResult>& results,
    const std::string& filename
) {
    bool success = true;
    
    success &= exportAIToJson(results, makePath(filename, "json"));
    success &= exportAIToCsv(results, makePath(filename, "csv"));
    success &= exportAIToMarkdown(results, makePath(filename, "md"));
    
    return success;
}

bool DataExporter::exportAIToJson(
    const std::vector<AISearchBenchmarkResult>& results,
    const std::string& filepath
) {
    auto json = AISearchBenchmark::toJson(results);
    return writeFile(filepath, json);
}

bool DataExporter::exportAIToCsv(
    const std::vector<AISearchBenchmarkResult>& results,
    const std::string& filepath
) {
    std::ostringstream oss;
    oss << "AI Name,Test Name,Depth,Nodes,Time (ms),Throughput,Unit,Passed\n";
    
    for (const auto& r : results) {
        oss << "\"" << r.ai_name << "\","
            << "\"" << r.test_name << "\","
            << r.depth << ","
            << r.nodes_searched << ","
            << std::fixed << std::setprecision(2) << r.time_ms << ","
            << std::fixed << std::setprecision(2) << r.throughput << ","
            << "\"" << r.unit << "\","
            << (r.passed ? "true" : "false") << "\n";
    }
    
    return writeFile(filepath, oss.str());
}

bool DataExporter::exportAIToMarkdown(
    const std::vector<AISearchBenchmarkResult>& results,
    const std::string& filepath
) {
    std::ostringstream oss;
    oss << "# AI Benchmark Results\n\n";
    oss << "Generated: " << getTimestamp() << "\n\n";
    
    oss << "| AI | Test | Depth | Nodes | Time (ms) | Throughput | Status |\n";
    oss << "|----|------|-------|-------|-----------|------------|--------|\n";
    
    for (const auto& r : results) {
        oss << "| " << r.ai_name << " | "
            << r.test_name << " | "
            << r.depth << " | "
            << r.nodes_searched << " | "
            << std::fixed << std::setprecision(2) << r.time_ms << " | "
            << std::fixed << std::setprecision(2) << r.throughput << " "
            << r.unit << " | "
            << (r.passed ? "PASSED" : "FAILED") << " |\n";
    }
    
    return writeFile(filepath, oss.str());
}

// ============================================================================
// 对战结果导出
// ============================================================================

bool DataExporter::exportBattleResults(
    const BattleStats& stats,
    const std::string& filename
) {
    bool success = true;
    
    success &= exportBattleToJson(stats, makePath(filename, "json"));
    success &= exportBattleToCsv(stats, makePath(filename, "csv"));
    success &= exportBattleToMarkdown(stats, makePath(filename, "md"));
    
    return success;
}

bool DataExporter::exportBattleToJson(
    const BattleStats& stats,
    const std::string& filepath
) {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"battle\": {\n";
    oss << "    \"player1\": \"" << stats.player1_name << "\",\n";
    oss << "    \"player2\": \"" << stats.player2_name << "\",\n";
    oss << "    \"total_games\": " << stats.total_games << ",\n";
    oss << "    \"player1_wins\": " << stats.player1_wins << ",\n";
    oss << "    \"player2_wins\": " << stats.player2_wins << ",\n";
    oss << "    \"draws\": " << stats.draws << ",\n";
    oss << "    \"win_rate1\": " << std::fixed << std::setprecision(4) << stats.win_rate1 << ",\n";
    oss << "    \"win_rate2\": " << std::fixed << std::setprecision(4) << stats.win_rate2 << ",\n";
    oss << "    \"avg_moves\": " << std::fixed << std::setprecision(2) << stats.avg_moves << ",\n";
    oss << "    \"significant\": " << (stats.significant ? "true" : "false") << ",\n";
    oss << "    \"p_value\": " << stats.p_value << "\n";
    oss << "  }\n";
    oss << "}\n";
    
    return writeFile(filepath, oss.str());
}

bool DataExporter::exportBattleToCsv(
    const BattleStats& stats,
    const std::string& filepath
) {
    std::ostringstream oss;
    oss << "Player1,Player2,Total Games,Wins1,Wins2,Draws,Win Rate 1,Win Rate 2,Avg Moves,Significant,P-Value\n";
    oss << "\"" << stats.player1_name << "\","
        << "\"" << stats.player2_name << "\","
        << stats.total_games << ","
        << stats.player1_wins << ","
        << stats.player2_wins << ","
        << stats.draws << ","
        << std::fixed << std::setprecision(4) << stats.win_rate1 << ","
        << std::fixed << std::setprecision(4) << stats.win_rate2 << ","
        << std::fixed << std::setprecision(2) << stats.avg_moves << ","
        << (stats.significant ? "true" : "false") << ","
        << stats.p_value << "\n";
    
    return writeFile(filepath, oss.str());
}

bool DataExporter::exportBattleToMarkdown(
    const BattleStats& stats,
    const std::string& filepath
) {
    std::ostringstream oss;
    oss << "# Battle Results\n\n";
    oss << "**" << stats.player1_name << "** vs **" << stats.player2_name << "**\n\n";
    oss << "Generated: " << getTimestamp() << "\n\n";
    
    oss << "## Summary\n\n";
    oss << "| Metric | Value |\n";
    oss << "|--------|-------|\n";
    oss << "| Total Games | " << stats.total_games << " |\n";
    oss << "| " << stats.player1_name << " Wins | " << stats.player1_wins << " |\n";
    oss << "| " << stats.player2_name << " Wins | " << stats.player2_wins << " |\n";
    oss << "| Draws | " << stats.draws << " |\n";
    oss << "| Win Rate (P1) | " << std::fixed << std::setprecision(2) << stats.win_rate1 * 100 << "% |\n";
    oss << "| Win Rate (P2) | " << std::fixed << std::setprecision(2) << stats.win_rate2 * 100 << "% |\n";
    oss << "| Average Moves | " << std::fixed << std::setprecision(1) << stats.avg_moves << " |\n";
    oss << "| Statistically Significant | " << (stats.significant ? "Yes" : "No") << " |\n";
    oss << "| P-Value | " << std::scientific << stats.p_value << " |\n";
    
    return writeFile(filepath, oss.str());
}

// ============================================================================
// 综合报告
// ============================================================================

bool DataExporter::exportFullReport(
    const std::vector<BenchmarkResult>& bitboard_results,
    const std::vector<AISearchBenchmarkResult>& ai_results,
    const std::vector<BattleStats>& battle_results,
    const std::string& filename
) {
    bool success = true;
    
    // JSON报告
    std::string json = generateExperimentJson(bitboard_results, ai_results, battle_results);
    success &= writeFile(makePath(filename, "json"), json);
    
    // Markdown报告
    std::string md = generateExperimentMarkdown(bitboard_results, ai_results, battle_results);
    success &= writeFile(makePath(filename, "md"), md);
    
    return success;
}

std::string DataExporter::generateExperimentJson(
    const std::vector<BenchmarkResult>& bitboard_results,
    const std::vector<AISearchBenchmarkResult>& ai_results,
    const std::vector<BattleStats>& battle_results
) {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"experiment\": {\n";
    oss << "    \"name\": \"" << config_.experiment_name << "\",\n";
    oss << "    \"timestamp\": \"" << getTimestamp() << "\",\n";
    
    // 系统信息
    auto sys_info = getSystemInfo();
    oss << "    \"system\": {\n";
    for (auto it = sys_info.begin(); it != sys_info.end(); ++it) {
        oss << "      \"" << it->first << "\": \"" << it->second << "\"";
        if (std::next(it) != sys_info.end()) oss << ",";
        oss << "\n";
    }
    oss << "    },\n";
    
    // Bitboard结果
    oss << "    \"bitboard_benchmark\": " << BitboardBenchmark::toJson(bitboard_results) << ",\n";
    
    // AI结果
    oss << "    \"ai_benchmark\": " << AISearchBenchmark::toJson(ai_results) << ",\n";
    
    // 战报
    oss << "    \"battles\": [";
    for (size_t i = 0; i < battle_results.size(); ++i) {
        const auto& b = battle_results[i];
        oss << "{";
        oss << "\"player1\":\"" << b.player1_name << "\",";
        oss << "\"player2\":\"" << b.player2_name << "\",";
        oss << "\"wins1\":" << b.player1_wins << ",";
        oss << "\"wins2\":" << b.player2_wins << ",";
        oss << "\"draws\":" << b.draws << ",";
        oss << "\"win_rate1\":" << std::fixed << std::setprecision(4) << b.win_rate1;
        oss << "}";
        if (i < battle_results.size() - 1) oss << ",";
    }
    oss << "]\n";
    
    oss << "  }\n";
    oss << "}\n";
    
    return oss.str();
}

std::string DataExporter::generateExperimentMarkdown(
    const std::vector<BenchmarkResult>& bitboard_results,
    const std::vector<AISearchBenchmarkResult>& ai_results,
    const std::vector<BattleStats>& battle_results
) {
    std::ostringstream oss;
    oss << "# Experiment Report\n\n";
    oss << "**Experiment**: " << config_.experiment_name << "\n\n";
    oss << "**Generated**: " << getTimestamp() << "\n\n";
    
    // 系统信息
    auto sys_info = getSystemInfo();
    oss << "## System Information\n\n";
    for (const auto& p : sys_info) {
        oss << "- **" << p.first << "**: " << p.second << "\n";
    }
    oss << "\n";
    
    // Bitboard结果
    oss << "## Bitboard Benchmark Results\n\n";
    oss << "| Test | Value | Unit | Status |\n";
    oss << "|------|-------|------|--------|\n";
    for (const auto& r : bitboard_results) {
        oss << "| " << r.name << " | "
            << std::fixed << std::setprecision(2) << r.value << " | "
            << r.unit << " | "
            << (r.passed ? "PASSED" : "FAILED") << " |\n";
    }
    oss << "\n";
    
    // AI结果
    oss << "## AI Benchmark Results\n\n";
    oss << "| AI | Test | Depth | Throughput | Status |\n";
    oss << "|----|------|-------|------------|--------|\n";
    for (const auto& r : ai_results) {
        oss << "| " << r.ai_name << " | "
            << r.test_name << " | "
            << r.depth << " | "
            << std::fixed << std::setprecision(2) << r.throughput << " "
            << r.unit << " | "
            << (r.passed ? "PASSED" : "FAILED") << " |\n";
    }
    oss << "\n";
    
    // 战报
    if (!battle_results.empty()) {
        oss << "## Battle Results\n\n";
        for (const auto& b : battle_results) {
            oss << "### " << b.player1_name << " vs " << b.player2_name << "\n\n";
            oss << "- **Total Games**: " << b.total_games << "\n";
            oss << "- **" << b.player1_name << " Wins**: " << b.player1_wins
                << " (" << std::fixed << std::setprecision(1) << b.win_rate1 * 100 << "%)\n";
            oss << "- **" << b.player2_name << " Wins**: " << b.player2_wins
                << " (" << std::fixed << std::setprecision(1) << b.win_rate2 * 100 << "%)\n";
            oss << "- **Draws**: " << b.draws << "\n";
            oss << "- **Significant**: " << (b.significant ? "Yes" : "No")
                << " (p=" << std::scientific << b.p_value << ")\n\n";
        }
    }
    
    return oss.str();
}

// ============================================================================
// 工具函数
// ============================================================================

bool DataExporter::ensureDirectory(const std::string& path) {
#ifdef _WIN32
    return CreateDirectoryA(path.c_str(), NULL) != 0 || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

std::string DataExporter::formatTableAsCsv(
    const std::vector<std::vector<std::string>>& rows
) {
    std::ostringstream oss;
    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            oss << row[i];
            if (i < row.size() - 1) oss << ",";
        }
        oss << "\n";
    }
    return oss.str();
}

std::map<std::string, std::string> DataExporter::getSystemInfo() {
    std::map<std::string, std::string> info;
    
    // 平台
#ifdef _WIN32
    info["Platform"] = "Windows";
#elif __linux__
    info["Platform"] = "Linux";
#elif __APPLE__
    info["Platform"] = "macOS";
#else
    info["Platform"] = "Unknown";
#endif
    
    // 编译器
#ifdef _MSC_VER
    std::ostringstream oss;
    oss << "MSVC " << _MSC_VER;
    info["Compiler"] = oss.str();
#elif __GNUC__
    info["Compiler"] = "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__);
#elif __clang__
    info["Compiler"] = "Clang " + std::to_string(__clang_major__) + "." + std::to_string(__clang_minor__);
#else
    info["Compiler"] = "Unknown";
#endif
    
    info["Timestamp"] = getTimestamp();
    
    return info;
}

// ============================================================================
// 私有函数
// ============================================================================

std::string DataExporter::makePath(const std::string& filename, const std::string& extension) {
    std::string name = filename;
    if (config_.include_timestamp) {
        name += "_" + getTimestamp();
    }
    name += "." + extension;
    
    if (!config_.output_dir.empty()) {
        return config_.output_dir + "/" + name;
    }
    return name;
}

bool DataExporter::writeFile(const std::string& path, const std::string& content) {
    // 确保目录存在
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        std::string dir = path.substr(0, pos);
        if (!dir.empty() && !ensureDirectory(dir)) {
            std::cerr << "[DataExporter] Failed to create directory: " << dir << std::endl;
            return false;
        }
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "[DataExporter] Failed to open file: " << path << std::endl;
        return false;
    }
    
    file << content;
    file.close();
    
    return true;
}

} // namespace Reversi
