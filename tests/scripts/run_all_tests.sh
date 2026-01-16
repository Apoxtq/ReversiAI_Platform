#!/bin/bash

# ReversiAI_Platform 完整测试套件运行脚本
# 用于版本发布前的全面质量验证

set -e  # 遇到错误立即退出

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查命令是否存在
check_command() {
    if ! command -v "$1" &> /dev/null; then
        log_error "$1 command not found. Please install $1."
        exit 1
    fi
}

# 清理函数
cleanup() {
    log_info "Cleaning up test artifacts..."
    # 清理测试生成的文件
    find . -name "*.gcov" -delete 2>/dev/null || true
    find . -name "*.gcda" -delete 2>/dev/null || true
    find . -name "*.gcno" -delete 2>/dev/null || true
    find . -name "test_output" -type d -exec rm -rf {} + 2>/dev/null || true
}

# 主函数
main() {
    log_info "Starting ReversiAI_Platform Comprehensive Test Suite"
    log_info "=================================================="

    # 检查必要工具
    log_info "Checking required tools..."
    check_command "cmake"
    check_command "make"
    check_command "gcc"
    check_command "g++"

    # 设置陷阱以确保清理
    trap cleanup EXIT

    # 创建构建目录
    log_info "Setting up build environment..."
    if [ ! -d "build" ]; then
        mkdir build
    fi
    cd build

    # 配置项目
    log_info "Configuring project with CMake..."
    if [ "$1" = "coverage" ]; then
        cmake .. -DCMAKE_BUILD_TYPE=Debug -DCOVERAGE=ON
    else
        cmake .. -DCMAKE_BUILD_TYPE=Release
    fi

    # 构建项目
    log_info "Building project..."
    make -j$(nproc) 2>&1
    if [ $? -ne 0 ]; then
        log_error "Build failed!"
        exit 1
    fi
    log_success "Build completed successfully"

    # 运行单元测试
    log_info "Running unit tests..."
    ctest --output-on-failure --tests-regex "unit" -j$(nproc) 2>&1
    if [ $? -ne 0 ]; then
        log_error "Unit tests failed!"
        exit 1
    fi
    log_success "Unit tests passed"

    # 运行集成测试
    log_info "Running integration tests..."
    ctest --output-on-failure --tests-regex "integration" -j$(nproc) 2>&1
    if [ $? -ne 0 ]; then
        log_error "Integration tests failed!"
        exit 1
    fi
    log_success "Integration tests passed"

    # 运行系统测试
    log_info "Running system tests..."
    ctest --output-on-failure --tests-regex "system" -j$(nproc) 2>&1
    if [ $? -ne 0 ]; then
        log_error "System tests failed!"
        exit 1
    fi
    log_success "System tests passed"

    # 运行回归测试
    log_info "Running regression tests..."
    ctest --output-on-failure --tests-regex "regression" -j$(nproc) 2>&1
    if [ $? -ne 0 ]; then
        log_error "Regression tests failed!"
        exit 1
    fi
    log_success "Regression tests passed"

    # 运行性能测试
    log_info "Running performance tests..."
    ctest --output-on-failure --tests-regex "performance" -j$(nproc) 2>&1
    perf_exit_code=$?
    if [ $perf_exit_code -eq 0 ]; then
        log_success "Performance tests passed"
    else
        log_warning "Performance tests failed - check performance targets"
        # 性能测试失败不阻止发布，但需要标记
        echo "PERFORMANCE_WARNING=1" >> $GITHUB_ENV 2>/dev/null || true
    fi

    # 生成覆盖率报告 (如果启用)
    if [ "$1" = "coverage" ]; then
        log_info "Generating coverage report..."
        if command -v lcov &> /dev/null; then
            lcov --capture --directory . --output-file coverage.info
            lcov --remove coverage.info '/usr/*' '*/tests/*' '*/build/*' --output-file coverage.info
            genhtml coverage.info --output-directory coverage_report
            log_success "Coverage report generated: coverage_report/index.html"
        else
            log_warning "lcov not found, skipping coverage report generation"
        fi
    fi

    # 最终总结
    log_info "=================================================="
    log_success "All tests completed!"
    log_info ""
    log_info "Test Results Summary:"
    echo "  ✅ Unit Tests: PASSED"
    echo "  ✅ Integration Tests: PASSED"
    echo "  ✅ System Tests: PASSED"
    echo "  ✅ Regression Tests: PASSED"
    if [ $perf_exit_code -eq 0 ]; then
        echo "  ✅ Performance Tests: PASSED"
    else
        echo "  ⚠️  Performance Tests: FAILED (check targets)"
    fi

    if [ "$1" = "coverage" ]; then
        echo "  📊 Coverage Report: coverage_report/index.html"
    fi

    log_info ""
    log_success "🎉 ReversiAI_Platform is ready for release!"

    # 返回适当的退出码
    if [ $perf_exit_code -eq 0 ]; then
        exit 0
    else
        exit 2  # 性能测试失败但其他通过
    fi
}

# 参数处理
case "$1" in
    "coverage")
        log_info "Running tests with coverage analysis"
        main "coverage"
        ;;
    "help"|"-h"|"--help")
        echo "ReversiAI_Platform Test Suite Runner"
        echo "Usage: $0 [options]"
        echo ""
        echo "Options:"
        echo "  coverage    Run tests with coverage analysis"
        echo "  help        Show this help message"
        echo ""
        echo "Exit codes:"
        echo "  0  All tests passed"
        echo "  1  Critical tests failed (unit/integration/system/regression)"
        echo "  2  Performance tests failed but others passed"
        ;;
    *)
        main
        ;;
esac
