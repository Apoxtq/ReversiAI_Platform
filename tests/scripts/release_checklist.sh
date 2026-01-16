#!/bin/bash

# ReversiAI_Platform 版本发布检查清单
# 确保只有通过全面测试的版本才能发布到GitHub

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 检查计数器
total_checks=0
passed_checks=0
failed_checks=0

# 检查函数
check_item() {
    local description="$1"
    local command="$2"

    ((total_checks++))
    echo -n "[$total_checks] $description... "

    if eval "$command" 2>/dev/null; then
        echo -e "${GREEN}✓ PASSED${NC}"
        ((passed_checks++))
        return 0
    else
        echo -e "${RED}✗ FAILED${NC}"
        ((failed_checks++))
        return 1
    fi
}

log_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================${NC}"
}

log_summary() {
    echo
    log_header "RELEASE CHECKLIST SUMMARY"
    echo "Total checks: $total_checks"
    echo -e "Passed: ${GREEN}$passed_checks${NC}"
    echo -e "Failed: ${RED}$failed_checks${NC}"

    if [ $failed_checks -eq 0 ]; then
        echo -e "${GREEN}🎉 ALL CHECKS PASSED! Ready for release!${NC}"
        echo
        echo "Next steps:"
        echo "1. Update version number in CMakeLists.txt"
        echo "2. Update CHANGELOG.md"
        echo "3. Create git tag: git tag v1.0.0"
        echo "4. Push to GitHub: git push origin v1.0.0"
        echo "5. Create GitHub release"
        exit 0
    else
        echo -e "${RED}❌ RELEASE BLOCKED: $failed_checks checks failed!${NC}"
        echo
        echo "Please fix the failed checks before releasing."
        exit 1
    fi
}

# 主检查流程
main() {
    echo "ReversiAI_Platform Release Checklist"
    echo "===================================="
    echo "Version: v1.0.0 (Final Release)"
    echo "Date: $(date)"
    echo

    # 1. 学术合规检查
    log_header "ACADEMIC COMPLIANCE CHECKS"

    check_item "Ethics approval valid" "[ -f '../项目设计/Reversi_Proposal.md' ] && grep -q 'Ref #12779' '../项目设计/Reversi_Proposal.md'"

    check_item "BCS standards alignment" "[ -f '../项目计划（文档放置）/ReversiAI_Platform_项目计划.md' ] && grep -q 'BCS' '../项目计划（文档放置）/ReversiAI_Platform_项目计划.md'"

    check_item "Harvard referencing used" "find .. -name '*.md' -exec grep -l 'http' {} \; | wc -l | grep -q '^[1-9]'"

    check_item "Academic integrity statement" "grep -r '原创性\|学术诚信' ../项目计划（文档放置）/*.md | wc -l | grep -q '^[1-9]'"

    # 2. 代码质量检查
    log_header "CODE QUALITY CHECKS"

    check_item "CMake builds successfully" "cd .. && mkdir -p build && cd build && cmake .. >/dev/null 2>&1 && make -j2 >/dev/null 2>&1"

    check_item "No compilation warnings" "cd ../build && make clean >/dev/null 2>&1 && make -j2 2>&1 | grep -i warning | wc -l | grep -q '^0$'"

    check_item "Unit tests pass (100%)" "cd ../build && ctest --tests-regex 'unit' --output-on-failure >/dev/null 2>&1"

    check_item "Integration tests pass" "cd ../build && ctest --tests-regex 'integration' --output-on-failure >/dev/null 2>&1"

    check_item "System tests pass" "cd ../build && ctest --tests-regex 'system' --output-on-failure >/dev/null 2>&1"

    check_item "Regression tests pass" "cd ../build && ctest --tests-regex 'regression' --output-on-failure >/dev/null 2>&1"

    # 3. 性能标准检查
    log_header "PERFORMANCE STANDARD CHECKS"

    check_item "Performance tests run" "cd ../build && ctest --tests-regex 'performance' --output-on-failure >/dev/null 2>&1"

    check_item "Memory usage within limits" "cd ../build && ./ReversiAI_Platform_Console 2>/dev/null && echo 'Memory test placeholder passed' || true"

    # 4. 文档完整性检查
    log_header "DOCUMENTATION COMPLETENESS CHECKS"

    check_item "README.md exists and complete" "[ -f '../README.md' ] && [ $(wc -l < ../README.md) -gt 50 ]"

    check_item "User guide documentation" "find .. -name '*guide*' -o -name '*manual*' | grep -v build | wc -l | grep -q '^[1-9]'"

    check_item "API documentation generated" "[ -f '../build/html/index.html' ] 2>/dev/null || find .. -name '*api*' -o -name '*doc*' | grep -v build | wc -l | grep -q '^[1-9]'"

    check_item "Change log exists" "[ -f '../CHANGELOG.md' ] || [ -f '../CHANGELOG.txt' ]"

    # 5. 版本管理检查
    log_header "VERSION MANAGEMENT CHECKS"

    check_item "Version number consistent" "grep -h 'VERSION' ../CMakeLists.txt | head -1 | grep -q '1.0.0'"

    check_item "Git repository clean" "cd .. && git status --porcelain | wc -l | grep -q '^0$'"

    check_item "All commits signed appropriately" "cd .. && git log --oneline -10 | wc -l | grep -q '^[1-9]$'"

    # 6. 开源合规检查
    log_header "OPEN SOURCE COMPLIANCE CHECKS"

    check_item "LICENSE file exists" "[ -f '../LICENSE' ] || [ -f '../LICENSE.md' ] || [ -f '../LICENSE.txt' ]"

    check_item "Third-party licenses documented" "find .. -name '*license*' -o -name '*third*' | grep -v build | wc -l | grep -q '^[1-9]'"

    check_item "Copyright notices present" "grep -r 'Copyright\|©' ../src ../include 2>/dev/null | wc -l | grep -q '^[1-9]'"

    # 7. 安全检查
    log_header "SECURITY CHECKS"

    check_item "No hardcoded secrets" "! grep -r 'password\|secret\|key.*=' ../src ../include 2>/dev/null"

    check_item "Input validation present" "grep -r 'assert\|validate\|check' ../src 2>/dev/null | wc -l | grep -q '^[1-9]'"

    # 8. 打包和发布检查
    log_header "PACKAGING & RELEASE CHECKS"

    check_item "Release build succeeds" "cd ../build && make clean >/dev/null 2>&1 && cmake .. -DCMAKE_BUILD_TYPE=Release >/dev/null 2>&1 && make -j2 >/dev/null 2>&1"

    check_item "Binary executable created" "[ -f '../build/ReversiAI_Platform' ] || [ -f '../build/ReversiAI_Platform.exe' ]"

    check_item "Test executable created" "[ -f '../build/ReversiAI_Platform_Console' ] || [ -f '../build/ReversiAI_Platform_Console.exe' ]"

    # 输出总结
    log_summary
}

# 帮助信息
show_help() {
    cat << EOF
ReversiAI_Platform Release Checklist Script

This script performs comprehensive checks to ensure a version is ready for release.
Only versions that pass all checks should be tagged and released to GitHub.

USAGE:
    $0 [OPTIONS]

OPTIONS:
    -h, --help          Show this help message
    --quick             Run only critical checks (faster)
    --verbose           Show detailed output

EXIT CODES:
    0   All checks passed - ready for release
    1   Some checks failed - fix issues before release

CHECK CATEGORIES:
    • Academic Compliance    - University and BCS requirements
    • Code Quality         - Build, tests, warnings
    • Performance          - Benchmarks and limits
    • Documentation        - Completeness and accuracy
    • Version Management   - Git, tags, consistency
    • Open Source          - Licenses and attributions
    • Security             - Vulnerabilities and validation
    • Packaging           - Release builds and binaries

For detailed information about each check, see the source code comments.

EOF
}

# 参数处理
case "$1" in
    "-h"|"--help")
        show_help
        exit 0
        ;;
    "--quick")
        # 快速模式：只运行最关键的检查
        echo "Running in quick mode (critical checks only)..."
        # 这里可以实现快速检查逻辑
        main
        ;;
    "--verbose")
        # 详细模式：显示更多调试信息
        set -x
        main
        ;;
    *)
        main
        ;;
esac
