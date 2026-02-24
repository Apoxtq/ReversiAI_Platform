#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

class TestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        std::cout << "=== ReversiAI_Platform Test Suite ===" << std::endl;
        std::cout << "Setting up test environment..." << std::endl;
        fs::create_directories("test_output");
        std::srand(42);
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
            std::cout << "[PASSED]" << std::endl;
        } else {
            fail_count_++;
            std::cout << "[FAILED]" << std::endl;
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
            std::cout << "\nSome tests failed. Please review and fix." << std::endl;
            exit(1);
        } else {
            std::cout << "\nAll tests passed!" << std::endl;
        }
    }

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

#ifdef _WIN32
#include <crtdbg.h>

class MemoryLeakDetector {
public:
    MemoryLeakDetector() {
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    }

    ~MemoryLeakDetector() {
        if (_CrtDumpMemoryLeaks()) {
            std::cout << "[WARNING] Memory leaks detected!" << std::endl;
        } else {
            std::cout << "[OK] No memory leaks detected." << std::endl;
        }
    }
};

static MemoryLeakDetector memory_detector;
#endif

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::InitGoogleMock(&argc, argv);

    ::testing::AddGlobalTestEnvironment(new TestEnvironment());

    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new TestResultListener());

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

    ::testing::GTEST_FLAG(filter) = "*";
    ::testing::GTEST_FLAG(shuffle) = true;
    ::testing::GTEST_FLAG(repeat) = 1;
    ::testing::GTEST_FLAG(break_on_failure) = false;

    if (argc > 1 && std::string(argv[1]).find("--output") != std::string::npos) {
        ::testing::GTEST_FLAG(output) = "xml:test_results.xml";
    }

    int result = RUN_ALL_TESTS();
    return result;
}
