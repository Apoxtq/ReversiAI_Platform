#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <chrono>
#include <filesystem>
#include "TestUtils.h"

// 测试夹具类
// 为不同类型的测试提供基础设置和清理

namespace fs = std::filesystem;

/**
 * @brief 基础测试夹具
 * @academic 提供所有测试的通用设置和清理功能
 */
class BaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试输出目录
        TestUtils::EnsureDirectoryExists(TestUtils::GetTestOutputDir());

        // 设置随机种子
        std::srand(TestUtils::GetRandomSeed());

        // 记录测试开始
        test_name_ = ::testing::UnitTest::GetInstance()
                    ->current_test_info()->test_case_name();
        test_name_ += ".";
        test_name_ += ::testing::UnitTest::GetInstance()
                     ->current_test_info()->name();

        test_timer_.Start();
    }

    void TearDown() override {
        // 记录测试耗时
        double duration = test_timer_.Stop();

        // 输出测试信息
        std::cout << "[INFO] " << test_name_ << " completed in "
                  << duration << " ms" << std::endl;

        // 清理测试文件
        CleanupTestFiles();
    }

    /**
     * @brief 获取测试数据目录
     * @return 测试数据目录路径
     */
    std::string GetTestDataDir() const {
        return (fs::path(__FILE__).parent_path().parent_path() / "data").string();
    }

    /**
     * @brief 获取测试输出目录
     * @return 测试输出目录路径
     */
    std::string GetTestOutputDir() const {
        return TestUtils::GetTestOutputDir();
    }

    /**
     * @brief 创建测试输出文件
     * @param filename 文件名
     * @return 完整文件路径
     */
    std::string CreateOutputFile(const std::string& filename) {
        std::string fullPath = (fs::path(GetTestOutputDir()) / filename).string();
        output_files_.push_back(fullPath);
        return fullPath;
    }

private:
    std::string test_name_;
    PerformanceTimer test_timer_;
    std::vector<std::string> output_files_;

    /**
     * @brief 清理测试文件
     */
    void CleanupTestFiles() {
        for (const auto& file : output_files_) {
            TestUtils::CleanupTempFile(file);
        }
        output_files_.clear();
    }
};

/**
 * @brief 核心组件测试夹具
 * @academic 为BitBoard、Board等核心组件测试提供专用设置
 */
class CoreTest : public BaseTest {
protected:
    void SetUp() override {
        BaseTest::SetUp();

        // 初始化标准测试棋盘
        standard_opening_ = TestDataGenerator::GenerateStandardOpening();
        random_board_ = TestDataGenerator::GenerateRandomBoard(20);
    }

    void TearDown() override {
        BaseTest::TearDown();
    }

    // 测试数据
    uint64_t standard_opening_;  // 标准开局
    uint64_t random_board_;      // 随机棋盘
};

/**
 * @brief AI算法测试夹具
 * @academic 为各种AI算法测试提供统一的测试环境
 */
class AITest : public BaseTest {
protected:
    void SetUp() override {
        BaseTest::SetUp();

        // 初始化测试用例
        test_positions_ = {
            TestDataGenerator::GenerateStandardOpening(),
            TestDataGenerator::GenerateRandomBoard(15),
            TestDataGenerator::GenerateRandomBoard(30)
        };

        // 默认搜索限制
        default_limits_ = {2000ms, 4, 1000};
    }

    void TearDown() override {
        BaseTest::TearDown();
        ai_instance_.reset();
    }

    /**
     * @brief 设置AI实例
     * @param ai AI实例指针
     * @academic 统一的AI实例管理
     */
    void SetAIInstance(std::unique_ptr<AIStrategy> ai) {
        ai_instance_ = std::move(ai);
    }

    /**
     * @brief 获取当前AI实例
     * @return AI实例指针
     */
    AIStrategy* GetAI() const {
        return ai_instance_.get();
    }

    /**
     * @brief 测试AI在所有测试位置上的表现
     * @param expectedMinTime 期望的最小思考时间(ms)
     * @param expectedMaxTime 期望的最大思考时间(ms)
     * @academic 统一的AI性能测试框架
     */
    void TestAIPerformance(double expectedMinTime = 0, double expectedMaxTime = 5000) {
        ASSERT_TRUE(ai_instance_ != nullptr) << "AI instance not set";

        for (size_t i = 0; i < test_positions_.size(); ++i) {
            // 这里需要实际的Board和AI接口
            // 暂时用占位符表示
            std::cout << "Testing position " << i << std::endl;

            // 模拟AI思考时间测试
            PerformanceTimer timer;
            timer.Start();

            // 这里会调用实际的AI接口
            // Move move = ai_instance_->findBestMove(board, default_limits_);

            double thinkingTime = timer.Stop();

            // 验证思考时间在合理范围内
            EXPECT_GE(thinkingTime, expectedMinTime);
            EXPECT_LE(thinkingTime, expectedMaxTime);

            std::cout << "Position " << i << " thinking time: " << thinkingTime << " ms" << std::endl;
        }
    }

private:
    std::vector<uint64_t> test_positions_;
    std::unique_ptr<AIStrategy> ai_instance_;
    SearchLimits default_limits_;
};

/**
 * @brief 性能测试夹具
 * @academic 专门为性能基准测试设计的测试环境
 */
class PerformanceTest : public BaseTest {
protected:
    void SetUp() override {
        BaseTest::SetUp();

        // 设置性能测试参数
        iterations_ = 1000;
        warmup_iterations_ = 100;

        // 创建性能结果输出文件
        performance_log_ = CreateOutputFile("performance_results.csv");

        // 初始化统计数据
        results_.clear();
    }

    void TearDown() override {
        BaseTest::TearDown();

        // 输出性能测试总结
        GeneratePerformanceReport();
    }

    /**
     * @brief 运行预热迭代
     * @param testFunction 测试函数
     * @academic 减少冷启动对性能测试的影响
     */
    void Warmup(std::function<void()> testFunction) {
        std::cout << "Running warmup iterations..." << std::endl;
        for (int i = 0; i < warmup_iterations_; ++i) {
            testFunction();
        }
        std::cout << "Warmup completed." << std::endl;
    }

    /**
     * @brief 运行性能测试
     * @param testName 测试名称
     * @param testFunction 测试函数
     * @academic 测量函数执行时间并记录结果
     */
    void RunPerformanceTest(const std::string& testName,
                           std::function<void()> testFunction) {
        std::cout << "Running performance test: " << testName << std::endl;

        Warmup(testFunction);

        PerformanceTimer timer;
        timer.Start();

        for (int i = 0; i < iterations_; ++i) {
            testFunction();
        }

        double totalTime = timer.Stop();
        double avgTime = totalTime / iterations_;

        PerformanceResult result{testName, totalTime, avgTime, iterations_};
        results_.push_back(result);

        std::cout << "Test completed: " << avgTime << " ms per iteration" << std::endl;
    }

    /**
     * @brief 断言性能在目标范围内
     * @param targetTimeMs 目标时间(ms)
     * @param tolerancePercent 可接受的误差百分比
     * @academic 性能测试的断言验证
     */
    void AssertPerformanceTarget(double targetTimeMs, double tolerancePercent = 10.0) {
        ASSERT_FALSE(results_.empty()) << "No performance results available";

        const auto& lastResult = results_.back();
        TestAssertions::AssertPerformanceWithinTolerance(
            lastResult.avgTimePerIteration, targetTimeMs, tolerancePercent);
    }

private:
    struct PerformanceResult {
        std::string testName;
        double totalTime;
        double avgTimePerIteration;
        int iterations;
    };

    int iterations_;
    int warmup_iterations_;
    std::string performance_log_;
    std::vector<PerformanceResult> results_;

    /**
     * @brief 生成性能测试报告
     */
    void GeneratePerformanceReport() {
        std::ofstream report(performance_log_);
        report << "Test Name,Total Time (ms),Avg Time (ms),Iterations\n";

        for (const auto& result : results_) {
            report << result.testName << ","
                   << result.totalTime << ","
                   << result.avgTimePerIteration << ","
                   << result.iterations << "\n";
        }

        report.close();
        std::cout << "Performance report saved to: " << performance_log_ << std::endl;
    }
};

/**
 * @brief 集成测试夹具
 * @academic 为多组件集成测试提供完整环境设置
 */
class IntegrationTest : public BaseTest {
protected:
    void SetUp() override {
        BaseTest::SetUp();

        // 设置临时目录用于集成测试
        temp_dir_ = std::make_unique<TempDirectoryManager>("integration_test");

        // 初始化测试数据
        test_scenario_ = "default";
    }

    void TearDown() override {
        BaseTest::TearDown();

        // 清理临时目录
        temp_dir_.reset();
    }

    /**
     * @brief 加载测试场景
     * @param scenarioName 场景名称
     * @academic 为不同集成测试场景提供配置
     */
    void LoadTestScenario(const std::string& scenarioName) {
        test_scenario_ = scenarioName;

        // 根据场景名称加载相应的测试配置
        // 这里可以加载不同的AI配置、网络设置等
        std::cout << "Loading test scenario: " << scenarioName << std::endl;
    }

    /**
     * @brief 验证系统状态一致性
     * @academic 集成测试中的状态一致性检查
     */
    void VerifySystemConsistency() {
        // 这里会检查各个组件的状态是否一致
        // 例如：棋盘状态、网络状态、UI状态等
        std::cout << "Verifying system consistency..." << std::endl;
        // 实际验证逻辑
    }

    /**
     * @brief 获取临时目录路径
     * @return 临时目录路径
     */
    std::string GetTempDir() const {
        return temp_dir_->GetPath();
    }

private:
    std::unique_ptr<TempDirectoryManager> temp_dir_;
    std::string test_scenario_;
};

/**
 * @brief 系统测试夹具
 * @academic 为端到端系统测试提供完整应用程序环境
 */
class SystemTest : public BaseTest {
protected:
    void SetUp() override {
        BaseTest::SetUp();

        // 这里会启动完整的应用程序实例用于测试
        // 包括GUI、网络服务器等组件
        std::cout << "Setting up system test environment..." << std::endl;
    }

    void TearDown() override {
        BaseTest::TearDown();

        // 清理系统测试环境
        std::cout << "Cleaning up system test environment..." << std::endl;
    }

    /**
     * @brief 模拟用户操作
     * @param action 用户动作描述
     * @academic 系统测试中的用户行为模拟
     */
    void SimulateUserAction(const std::string& action) {
        std::cout << "Simulating user action: " << action << std::endl;
        // 这里会模拟实际的用户界面操作
    }

    /**
     * @brief 验证应用程序状态
     * @param expectedState 期望状态
     * @academic 系统级状态验证
     */
    void VerifyApplicationState(const std::string& expectedState) {
        std::cout << "Verifying application state: " << expectedState << std::endl;
        // 这里会检查应用程序的整体状态
    }
};

/**
 * @brief 回归测试夹具
 * @academic 为历史缺陷回归测试提供专用环境
 */
class RegressionTest : public BaseTest {
protected:
    void SetUp() override {
        BaseTest::SetUp();

        // 加载已知缺陷的测试用例
        LoadRegressionTestCases();
    }

    void TearDown() override {
        BaseTest::TearDown();

        // 记录回归测试结果
        RecordRegressionResults();
    }

private:
    struct RegressionCase {
        std::string defectId;
        std::string description;
        std::function<void()> testFunction;
        bool wasFixed;
    };

    std::vector<RegressionCase> regression_cases_;

    /**
     * @brief 加载回归测试用例
     * @academic 从历史缺陷数据库加载测试用例
     */
    void LoadRegressionTestCases() {
        // 这里会从文件或数据库加载已修复缺陷的测试用例
        // 确保这些缺陷不会重新出现

        // 示例回归测试用例
        regression_cases_ = {
            {
                "DEFECT-001",
                "BitBoard翻转算法边界错误",
                [this]() { /* 测试翻转边界情况 */ },
                true
            },
            {
                "DEFECT-002",
                "AI搜索深度限制失效",
                [this]() { /* 测试搜索深度限制 */ },
                true
            }
        };
    }

    /**
     * @brief 记录回归测试结果
     */
    void RecordRegressionResults() {
        // 将回归测试结果记录到文件或数据库
        std::string reportFile = CreateOutputFile("regression_report.txt");

        std::ofstream report(reportFile);
        report << "Regression Test Report\n";
        report << "======================\n\n";

        for (const auto& testCase : regression_cases_) {
            report << "Defect ID: " << testCase.defectId << "\n";
            report << "Description: " << testCase.description << "\n";
            report << "Status: " << (testCase.wasFixed ? "PASSED" : "FAILED") << "\n\n";
        }

        report.close();
    }
};

// 便捷的测试套件宏定义
#define TEST_CORE_F(test_name) TEST_F(CoreTest, test_name)
#define TEST_AI_F(test_name) TEST_F(AITest, test_name)
#define TEST_PERFORMANCE_F(test_name) TEST_F(PerformanceTest, test_name)
#define TEST_INTEGRATION_F(test_name) TEST_F(IntegrationTest, test_name)
#define TEST_SYSTEM_F(test_name) TEST_F(SystemTest, test_name)
#define TEST_REGRESSION_F(test_name) TEST_F(RegressionTest, test_name)

#endif // TESTFIXTURES_H
