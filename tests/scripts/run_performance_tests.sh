#!/bin/bash

# ReversiAI_Platform 性能测试运行脚本
# 专门用于性能基准测试和性能回归检测

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

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

# 性能目标定义
declare -A PERFORMANCE_TARGETS=(
    ["BM_BitBoard_GetValidMoves"]="100"  # 微秒
    ["BM_BitBoard_FlipBits"]="50"        # 微秒
    ["BM_Board_MakeMove"]="200"          # 微秒
)

# 检查性能结果
check_performance_targets() {
    local results_file="$1"
    local all_passed=true

    log_info "Checking performance against targets..."

    while IFS=',' read -r benchmark_name time_unit time_value iterations_or_other; do
        # 跳过标题行
        if [[ "$benchmark_name" == "name" ]]; then
            continue
        fi

        # 只检查我们定义的目标基准测试
        for target_benchmark in "${!PERFORMANCE_TARGETS[@]}"; do
            if [[ "$benchmark_name" == *"$target_benchmark"* ]]; then
                target_time="${PERFORMANCE_TARGETS[$target_benchmark]}"

                # 转换时间单位 (假设结果是微秒)
                if (( $(echo "$time_value > $target_time" | bc -l 2>/dev/null || echo "0") )); then
                    log_error "Performance target FAILED for $benchmark_name: $time_value us (target: $target_time us)"
                    all_passed=false
                else
                    log_success "Performance target PASSED for $benchmark_name: $time_value us (target: $target_time us)"
                fi
                break
            fi
        done
    done < "$results_file"

    return $([ "$all_passed" = true ] && echo 0 || echo 1)
}

# 主函数
main() {
    log_info "Starting ReversiAI_Platform Performance Test Suite"
    log_info "==================================================="

    # 检查是否在正确的目录
    if [ ! -f "CMakeLists.txt" ]; then
        log_error "Please run this script from the project root directory"
        exit 1
    fi

    # 创建构建目录
    if [ ! -d "build" ]; then
        mkdir build
    fi

    cd build

    # 配置项目 (Release模式以获得最佳性能)
    log_info "Configuring project for performance testing..."
    cmake .. -DCMAKE_BUILD_TYPE=Release -DPERFORMANCE_TESTING=ON

    # 构建项目
    log_info "Building project..."
    make -j$(nproc) 2>&1
    if [ $? -ne 0 ]; then
        log_error "Build failed!"
        exit 1
    fi

    # 查找性能测试可执行文件
    performance_tests=$(find . -name "*performance*" -type f -executable 2>/dev/null)
    benchmark_tests=$(find . -name "*benchmark*" -type f -executable 2>/dev/null)

    if [ -z "$performance_tests" ] && [ -z "$benchmark_tests" ]; then
        log_error "No performance test executables found!"
        exit 1
    fi

    # 创建结果目录
    results_dir="performance_results"
    mkdir -p "$results_dir"

    # 运行Google Benchmark测试
    if [ -n "$benchmark_tests" ]; then
        log_info "Running Google Benchmark tests..."

        for benchmark in $benchmark_tests; do
            test_name=$(basename "$benchmark")
            log_info "Running benchmark: $test_name"

            # 运行基准测试并捕获输出
            output_file="$results_dir/${test_name}_results.csv"
            "$benchmark" --benchmark_format=csv > "$output_file" 2>&1

            if [ $? -eq 0 ]; then
                log_success "Benchmark $test_name completed"
            else
                log_error "Benchmark $test_name failed"
            fi
        done
    fi

    # 运行传统的性能测试
    if [ -n "$performance_tests" ]; then
        log_info "Running traditional performance tests..."

        for perf_test in $performance_tests; do
            test_name=$(basename "$perf_test")
            log_info "Running performance test: $test_name"

            # 运行测试
            "$perf_test" 2>&1 | tee "$results_dir/${test_name}.log"

            if [ $? -eq 0 ]; then
                log_success "Performance test $test_name completed"
            else
                log_warning "Performance test $test_name had issues"
            fi
        done
    fi

    # 合并和分析结果
    log_info "Analyzing performance results..."

    # 查找所有基准测试结果文件
    benchmark_results=$(find "$results_dir" -name "*results.csv" 2>/dev/null)

    if [ -n "$benchmark_results" ]; then
        # 合并所有基准测试结果
        merged_results="$results_dir/merged_benchmark_results.csv"
        echo "name,real_time,time_unit,iterations" > "$merged_results"

        for result_file in $benchmark_results; do
            # 跳过标题行，追加数据行
            tail -n +2 "$result_file" >> "$merged_results" 2>/dev/null || true
        done

        log_info "Merged benchmark results saved to: $merged_results"

        # 检查性能目标
        if check_performance_targets "$merged_results"; then
            log_success "All performance targets met!"
            performance_status="PASSED"
        else
            log_warning "Some performance targets not met"
            performance_status="FAILED"
        fi
    else
        log_warning "No benchmark results found"
        performance_status="NO_DATA"
    fi

    # 生成性能摘要报告
    summary_file="$results_dir/performance_summary.txt"
    {
        echo "ReversiAI_Platform Performance Test Summary"
        echo "=========================================="
        echo "Test Date: $(date)"
        echo "System: $(uname -a)"
        echo "CPU: $(nproc) cores"
        echo ""
        echo "Performance Status: $performance_status"
        echo ""

        if [ -f "$merged_results" ]; then
            echo "Benchmark Results Summary:"
            echo "-------------------------"
            # 显示前几个最相关的基准测试结果
            head -20 "$merged_results" | while IFS=',' read -r name real_time unit iterations; do
                if [[ "$name" != "name" ]]; then
                    echo "$name: $real_time $unit ($iterations iterations)"
                fi
            done
        fi

        echo ""
        echo "Performance Targets:"
        echo "-------------------"
        for benchmark in "${!PERFORMANCE_TARGETS[@]}"; do
            target="${PERFORMANCE_TARGETS[$benchmark]}"
            echo "$benchmark: ≤ $target microseconds per operation"
        done

        echo ""
        echo "Raw results saved in: $results_dir/"

    } > "$summary_file"

    log_success "Performance summary saved to: $summary_file"

    # 显示最终状态
    log_info "==================================================="
    if [ "$performance_status" = "PASSED" ]; then
        log_success "🎉 All performance targets achieved!"
        exit 0
    elif [ "$performance_status" = "FAILED" ]; then
        log_warning "⚠️  Performance targets not fully met. Review optimization opportunities."
        exit 2
    else
        log_info "ℹ️  No performance data collected. Run benchmark tests to get results."
        exit 3
    fi
}

# 参数处理
case "$1" in
    "help"|"-h"|"--help")
        echo "ReversiAI_Platform Performance Test Runner"
        echo "Usage: $0 [options]"
        echo ""
        echo "This script runs all performance tests and benchmarks,"
        echo "then checks results against predefined performance targets."
        echo ""
        echo "Options:"
        echo "  help    Show this help message"
        echo ""
        echo "Exit codes:"
        echo "  0  All performance targets met"
        echo "  2  Some performance targets not met"
        echo "  3  No performance data collected"
        ;;
    *)
        main
        ;;
esac
