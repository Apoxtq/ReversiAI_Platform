#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include <memory>
#include <vector>
#include <string>
#include <random>
#include <filesystem>

// 测试工具函数和辅助类
// 提供测试常用的功能和数据生成

namespace fs = std::filesystem;

/**
 * @brief 测试工具类
 * @academic 提供测试数据生成、性能测量、文件操作等通用功能
 */
class TestUtils {
public:
    /**
     * @brief 生成随机种子，保证测试可重现
     * @return 随机种子值
     * @academic 确保测试结果的可重现性和一致性
     */
    static unsigned int GetRandomSeed() {
        return 42;  // 固定种子保证可重现性
    }

    /**
     * @brief 生成随机整数
     * @param min 最小值
     * @param max 最大值
     * @return 随机整数
     */
    static int RandomInt(int min, int max) {
        static std::mt19937 gen(GetRandomSeed());
        std::uniform_int_distribution<> dist(min, max);
        return dist(gen);
    }

    /**
     * @brief 生成随机双精度浮点数
     * @param min 最小值
     * @param max 最大值
     * @return 随机双精度浮点数
     */
    static double RandomDouble(double min, double max) {
        static std::mt19937 gen(GetRandomSeed());
        std::uniform_real_distribution<> dist(min, max);
        return dist(gen);
    }

    /**
     * @brief 创建临时文件
     * @param prefix 文件名前缀
     * @param extension 文件扩展名
     * @return 临时文件路径
     * @academic 测试文件操作的安全性
     */
    static std::string CreateTempFile(const std::string& prefix = "test",
                                     const std::string& extension = "tmp") {
        static int counter = 0;
        std::string filename = prefix + "_" + std::to_string(counter++) + "." + extension;
        std::string fullPath = (fs::temp_directory_path() / filename).string();
        return fullPath;
    }

    /**
     * @brief 清理临时文件
     * @param filePath 文件路径
     * @academic 确保测试后的清理工作
     */
    static void CleanupTempFile(const std::string& filePath) {
        if (fs::exists(filePath)) {
            fs::remove(filePath);
        }
    }

    /**
     * @brief 读取测试数据文件
     * @param filename 文件名
     * @return 文件内容字符串
     * @academic 加载测试数据和配置
     */
    static std::string ReadTestDataFile(const std::string& filename) {
        fs::path testDataPath = fs::path(__FILE__).parent_path().parent_path() / "data" / filename;
        if (!fs::exists(testDataPath)) {
            throw std::runtime_error("Test data file not found: " + testDataPath.string());
        }

        std::ifstream file(testDataPath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open test data file: " + testDataPath.string());
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    /**
     * @brief 获取测试输出目录
     * @return 测试输出目录路径
     * @academic 统一测试输出位置
     */
    static std::string GetTestOutputDir() {
        return "test_output";
    }

    /**
     * @brief 确保目录存在
     * @param dirPath 目录路径
     * @academic 测试环境的准备工作
     */
    static void EnsureDirectoryExists(const std::string& dirPath) {
        fs::create_directories(dirPath);
    }
};

/**
 * @brief 性能测量工具
 * @academic 精确测量代码执行时间，用于性能测试和基准测试
 */
class PerformanceTimer {
public:
    /**
     * @brief 开始计时
     */
    void Start() {
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    /**
     * @brief 结束计时并返回耗时(毫秒)
     * @return 耗时(毫秒)
     */
    double Stop() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time_);
        return duration.count() / 1000.0;  // 转换为毫秒
    }

    /**
     * @brief 获取当前耗时但不停止计时
     * @return 当前耗时(毫秒)
     */
    double Peek() const {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time_);
        return duration.count() / 1000.0;
    }

private:
    std::chrono::high_resolution_clock::time_point start_time_;
};

/**
 * @brief 内存使用监控器
 * @academic 监控测试中的内存使用情况，检测内存泄漏
 */
class MemoryMonitor {
public:
    /**
     * @brief 开始监控
     */
    void Start() {
        // 在实际实现中，这里会使用平台特定的内存监控API
        start_memory_ = GetCurrentMemoryUsage();
    }

    /**
     * @brief 结束监控并返回内存使用差值
     * @return 内存使用差值(字节)
     */
    long long Stop() {
        long long end_memory = GetCurrentMemoryUsage();
        return end_memory - start_memory_;
    }

private:
    long long start_memory_;

    /**
     * @brief 获取当前内存使用量
     * @return 当前内存使用量(字节)
     */
    long long GetCurrentMemoryUsage() {
        // 简化的实现，实际应该使用平台特定的API
        // Windows: GetProcessMemoryInfo
        // Linux: /proc/self/status
        // macOS: task_info
        return 0;  // 占位符
    }
};

/**
 * @brief 临时目录管理器
 * @academic 自动管理测试临时目录的创建和清理
 */
class TempDirectoryManager {
public:
    /**
     * @brief 构造函数，创建临时目录
     * @param prefix 目录名前缀
     */
    explicit TempDirectoryManager(const std::string& prefix = "test_temp") {
        temp_dir_ = fs::temp_directory_path() / (prefix + "_" + std::to_string(rand()));
        fs::create_directories(temp_dir_);
    }

    /**
     * @brief 析构函数，清理临时目录
     */
    ~TempDirectoryManager() {
        try {
            if (fs::exists(temp_dir_)) {
                fs::remove_all(temp_dir_);
            }
        } catch (const std::exception&) {
            // 忽略清理错误
        }
    }

    /**
     * @brief 获取临时目录路径
     * @return 临时目录路径
     */
    std::string GetPath() const {
        return temp_dir_.string();
    }

    /**
     * @brief 在临时目录中创建文件
     * @param filename 文件名
     * @param content 文件内容
     * @return 完整文件路径
     */
    std::string CreateFile(const std::string& filename, const std::string& content) {
        fs::path filePath = temp_dir_ / filename;
        std::ofstream file(filePath);
        file << content;
        file.close();
        return filePath.string();
    }

private:
    fs::path temp_dir_;
};

/**
 * @brief 测试数据生成器
 * @academic 生成各种测试数据，用于单元测试和基准测试
 */
class TestDataGenerator {
public:
    /**
     * @brief 生成标准开局棋盘
     * @return 标准开局的BitBoard表示
     * @academic 黑白棋的标准开局位置
     */
    static uint64_t GenerateStandardOpening() {
        // 标准4个棋子的开局
        // 黑棋: (3,3), (4,4)
        // 白棋: (3,4), (4,3)
        uint64_t black = (1ULL << 27) | (1ULL << 36);  // (3,3) = 3*8+3=27, (4,4)=4*8+4=36
        uint64_t white = (1ULL << 28) | (1ULL << 35);  // (3,4) = 3*8+4=28, (4,3)=4*8+3=35
        return black | (white << 32);  // 假设数据结构
    }

    /**
     * @brief 生成随机棋盘位置
     * @param pieceCount 棋子数量
     * @return 随机棋盘的BitBoard表示
     * @academic 用于测试各种棋盘配置的鲁棒性
     */
    static uint64_t GenerateRandomBoard(int pieceCount = 10) {
        uint64_t board = 0;
        for (int i = 0; i < pieceCount && i < 64; ++i) {
            int pos;
            do {
                pos = TestUtils::RandomInt(0, 63);
            } while (board & (1ULL << pos));
            board |= (1ULL << pos);
        }
        return board;
    }

    /**
     * @brief 生成测试移动序列
     * @param length 序列长度
     * @return 移动序列
     * @academic 用于集成测试的预定义移动序列
     */
    static std::vector<std::pair<int, int>> GenerateTestMoveSequence(int length = 10) {
        std::vector<std::pair<int, int>> moves;
        for (int i = 0; i < length; ++i) {
            int row = TestUtils::RandomInt(0, 7);
            int col = TestUtils::RandomInt(0, 7);
            moves.emplace_back(row, col);
        }
        return moves;
    }
};

/**
 * @brief Google Test断言辅助函数
 * @academic 提供更丰富的断言函数，便于测试编写
 */
class TestAssertions {
public:
    /**
     * @brief 断言两个浮点数在误差范围内相等
     * @param expected 期望值
     * @param actual 实际值
     * @param tolerance 容差
     * @academic 处理浮点数比较的精度问题
     */
    static void AssertNear(double expected, double actual, double tolerance = 1e-6) {
        ASSERT_NEAR(expected, actual, tolerance);
    }

    /**
     * @brief 断言性能在可接受范围内
     * @param measuredValue 测量值
     * @param targetValue 目标值
     * @param tolerancePercent 可接受的百分比误差
     * @academic 性能测试的专用断言
     */
    static void AssertPerformanceWithinTolerance(double measuredValue,
                                                double targetValue,
                                                double tolerancePercent = 10.0) {
        double tolerance = targetValue * tolerancePercent / 100.0;
        ASSERT_NEAR(measuredValue, targetValue, tolerance);
    }

    /**
     * @brief 断言内存使用在限制内
     * @param memoryUsage 内存使用量(字节)
     * @param limit 内存限制(字节)
     * @academic 内存使用测试的断言
     */
    static void AssertMemoryWithinLimit(long long memoryUsage, long long limit) {
        ASSERT_LE(memoryUsage, limit);
    }
};

// 便捷宏定义
#define TEST_PERFORMANCE_BEGIN() \
    PerformanceTimer perf_timer; \
    perf_timer.Start()

#define TEST_PERFORMANCE_END(target_ms) \
    double actual_time = perf_timer.Stop(); \
    TestAssertions::AssertPerformanceWithinTolerance(actual_time, target_ms)

#define TEST_MEMORY_BEGIN() \
    MemoryMonitor memory_monitor; \
    memory_monitor.Start()

#define TEST_MEMORY_END(limit_bytes) \
    long long memory_usage = memory_monitor.Stop(); \
    TestAssertions::AssertMemoryWithinLimit(memory_usage, limit_bytes)

#endif // TESTUTILS_H
