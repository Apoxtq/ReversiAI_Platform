#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <filesystem>
#include <chrono>

// 测试框架主入口
// 配置Google Test和自定义测试环境

namespace fs = std::filesystem;

// 测试全局配置
class TestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        std::cout << "=== ReversiAI_Platform Test Suite ===" << std::endl;
        std::cout << "Setting up test environment..." << std::endl;

        // 创建测试输出目录
        fs::create_directories("test_output");

        // 设置随机种子保证可重现性
        std::srand(42);

        // 记录测试开始时间
        start_time_ = std::chrono::steady_clock::now();

        std::cout << "Test environment setup complete." << std::endl;
    }

    void TearDown() override {
        auto end_time = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_);

        std::cout << "\n=== Test Suite Summary ===" << std::endl;
        std::cout << "Total test time: " << duration.count() << " ms" << std::endl;
        std::cout << "Test environment cleanup complete." << std::endl;
    }

private:
    std::chrono::steady_clock::time_point start_time_;
};

// 测试监听器：收集测试结果和性能数据
class TestResultListener : public ::testing::TestEventListener {
public:
    TestResultListener() : test_count_(0), pass_count_(0), fail_count_(0) {}

    void OnTestProgramStart(const ::testing::UnitTest& unit_test) override {
        std::cout << "Starting test program with " << unit_test.total_test_count() << " tests..." << std::endl;
    }

    void OnTestStart(const ::testing::TestInfo& test_info) override {
        test_count_++;
        std::cout << "[" << test_count_ << "] Running: "
                  << test_info.test_case_name() << "." << test_info.name() << std::endl;
    }

    void OnTestEnd(const ::testing::TestInfo& test_info) override {
        if (test_info.result()->Passed()) {
            pass_count_++;
            std::cout << "✓ PASSED" << std::endl;
        } else {
            fail_count_++;
            std::cout << "✗ FAILED" << std::endl;
            // 记录失败详情
            const ::testing::TestResult* result = test_info.result();
            for (int i = 0; i < result->total_part_count(); ++i) {
                if (result->GetTestPartResult(i).failed()) {
                    std::cout << "  Failure: " << result->GetTestPartResult(i).summary() << std::endl;
                }
            }
        }
    }

    void OnTestProgramEnd(const ::testing::UnitTest& /*unit_test*/) override {
        std::cout << "\n=== Final Results ===" << std::endl;
        std::cout << "Total tests: " << test_count_ << std::endl;
        std::cout << "Passed: " << pass_count_ << std::endl;
        std::cout << "Failed: " << fail_count_ << std::endl;
        std::cout << "Success rate: " << (test_count_ > 0 ? (pass_count_ * 100.0 / test_count_) : 0) << "%" << std::endl;

        if (fail_count_ > 0) {
            std::cout << "\n❌ Some tests failed. Please review and fix." << std::endl;
            exit(1);
        } else {
            std::cout << "\n✅ All tests passed!" << std::endl;
        }
    }

    // Required pure virtual functions from TestEventListener
    void OnTestIterationStart(const ::testing::UnitTest& /*unit_test*/, int iteration) override {
        std::cout << "Test iteration " << iteration << " starting..." << std::endl;
    }

    void OnEnvironmentsSetUpStart(const ::testing::UnitTest& /*unit_test*/) override {}
    void OnEnvironmentsSetUpEnd(const ::testing::UnitTest& /*unit_test*/) override {}

    void OnTestPartResult(const ::testing::TestPartResult& test_part_result) override {
        if (test_part_result.failed()) {
            std::cout << "Test part failed: " << test_part_result.summary() << std::endl;
        }
    }

    void OnEnvironmentsTearDownStart(const ::testing::UnitTest& /*unit_test*/) override {}
    void OnEnvironmentsTearDownEnd(const ::testing::UnitTest& /*unit_test*/) override {}

    void OnTestIterationEnd(const ::testing::UnitTest& /*unit_test*/, int /*iteration*/) override {}

private:
    int test_count_;
    int pass_count_;
    int fail_count_;
};

// 内存泄漏检测 (Windows下的替代方案)
#ifdef _WIN32
#include <crtdbg.h>

class MemoryLeakDetector {
public:
    MemoryLeakDetector() {
        // 启用内存泄漏检测
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    }

    ~MemoryLeakDetector() {
        // 检查内存泄漏
        if (_CrtDumpMemoryLeaks()) {
            std::cout << "⚠️  Memory leaks detected!" << std::endl;
        } else {
            std::cout << "✅ No memory leaks detected." << std::endl;
        }
    }
};

static MemoryLeakDetector memory_detector;
#endif

int main(int argc, char** argv) {
    // 初始化Google Test
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);

    // 设置测试环境
    ::testing::AddGlobalTestEnvironment(new TestEnvironment());

    // 添加自定义监听器
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new TestResultListener());

    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            std::cout << "ReversiAI_Platform Test Suite" << std::endl;
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --help, -h          Show this help message" << std::endl;
            std::cout << "  --filter=pattern    Run only tests matching pattern" << std::endl;
            std::cout << "  --output=xml        Generate XML test report" << std::endl;
            return 0;
        }
    }

    // 配置Google Test选项
    ::testing::GTEST_FLAG(filter) = "*";  // 默认运行所有测试
    ::testing::GTEST_FLAG(shuffle) = true;  // 随机测试顺序
    ::testing::GTEST_FLAG(repeat) = 1;     // 运行次数
    ::testing::GTEST_FLAG(break_on_failure) = false;  // 失败时不停止

    // 如果指定了XML输出
    if (argc > 1 && std::string(argv[1]).find("--output") != std::string::npos) {
        ::testing::GTEST_FLAG(output) = "xml:test_results.xml";
    }

    // 运行所有测试
    int result = RUN_ALL_TESTS();

    return result;
}
