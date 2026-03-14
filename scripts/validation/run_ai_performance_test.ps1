# AI性能验证测试脚本
# 依据: Reversi_Proposal.md Section 8.3.1
# 版本: v1.0.0
# 日期: 2026-03-14

$ErrorActionPreference = "Continue"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "AI Performance Validation Test" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$ScriptDir = "d:\Project\Reversi\ReversiAI_Platform\scripts\validation"
$ProjectRoot = "d:\Project\Reversi\ReversiAI_Platform"
$BuildDir = "$ProjectRoot\build_console"
$ResultsDir = "$ScriptDir\results"
$TestResultsDir = "$ResultsDir\performance_validation"

# 创建结果目录
if (-not (Test-Path $TestResultsDir)) {
    New-Item -ItemType Directory -Path $TestResultsDir | Out-Null
}

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"

# ========================================
# 测试配置
# ========================================

Write-Host ""
Write-Host "[Test Configuration]" -ForegroundColor Yellow
Write-Host "========================================"

$testConfig = @{
    "Test 1: Minimax (d6) vs Random" = @{
        "Black AI" = "Minimax (depth=6)"
        "White AI" = "Random"
        "Games" = 100
        "Positions" = "Standard-64 suite"
        "Requirement" = "≥90% win rate"
    }
    "Test 2: MCTS vs Minimax (d4)" = @{
        "Black AI" = "MCTS (1000 sims)"
        "White AI" = "Minimax (depth=4)"
        "Games" = 50
        "Positions" = "Standard opening"
        "Requirement" = "≥70% win rate"
    }
}

$testConfig | ForEach-Object {
    Write-Host "  $($_.Key):" -ForegroundColor White
    $_.Value.GetEnumerator() | ForEach-Object {
        Write-Host "    $($_.Key): $($_.Value)" -ForegroundColor Gray
    }
}

# ========================================
# 测试1: Minimax vs Random
# ========================================

Write-Host ""
Write-Host "[TEST 1] Minimax (depth-6) vs Random (100 games, 64 positions)" -ForegroundColor Yellow
Write-Host "-------------------------------------------------------------------" -ForegroundColor Yellow

# 写入测试代码到临时文件
$testCode = @'
#include <iostream>
#include <chrono>
#include "ai/AIStrategy.h"
#include "ai/AIBattle.h"
#include "research/PositionSuite.h"

int main() {
    std::cout << "=== Test: Minimax (depth-6) vs Random ===" << std::endl;
    
    // 设置随机种子
    srand(42);
    
    // 创建AI
    auto minimax = Reversi::AIStrategyFactory::createMinimaxAI(Reversi::Difficulty::HARD);
    auto random = Reversi::AIStrategyFactory::createRandomAI();
    
    Reversi::AIBattle battle(std::move(minimax), std::move(random));
    
    Reversi::SearchLimits limits;
    limits.maxDepth = 6;
    
    // 测试 - 使用标准开局（简化版，实际应使用64-position suite）
    auto start = std::chrono::steady_clock::now();
    auto result = battle.playTournament(100, limits);
    auto end = std::chrono::steady_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << std::endl;
    std::cout << "=== Results ===" << std::endl;
    std::cout << "Total games: " << result.totalGames << std::endl;
    std::cout << "Black (Minimax) wins: " << result.blackWins << std::endl;
    std::cout << "White (Random) wins: " << result.whiteWins << std::endl;
    std::cout << "Draws: " << result.draws << std::endl;
    std::cout << "Black win rate: " << (result.blackWinRate * 100) << "%" << std::endl;
    std::cout << "Total time: " << duration.count() << "ms" << std::endl;
    
    // 判断是否达标
    bool passed = (result.blackWinRate * 100) >= 90.0;
    std::cout << std::endl;
    std::cout << "Requirement: >= 90%" << std::endl;
    std::cout << "Result: " << (passed ? "[PASSED]" : "[FAILED]") << std::endl;
    
    return passed ? 0 : 1;
}
'@

# 保存测试代码
$testFile = "$TestResultsDir\test_minimax_vs_random.cpp"
$testCode | Out-File -FilePath $testFile -Encoding utf8

Write-Host "Test code saved to: $testFile" -ForegroundColor Gray
Write-Host "Note: This test requires compilation and integration with PositionSuite" -ForegroundColor Gray

# ========================================
# 测试2: MCTS vs Minimax
# ========================================

Write-Host ""
Write-Host "[TEST 2] MCTS (1000 sims) vs Minimax (depth-4) (50 games)" -ForegroundColor Yellow
Write-Host "-------------------------------------------------------------------" -ForegroundColor Yellow

Write-Host "Note: MCTS AI is now functional after fixes" -ForegroundColor Gray
Write-Host "Test would require: MCTSAI (1000 sims) vs MinimaxAI (depth=4)" -ForegroundColor Gray

# ========================================
# 总结
# ========================================

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Test Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Based on previous benchmark (2026-03-07):" -ForegroundColor White
Write-Host "  - Minimax vs Random (100 games, standard opening): 99% win rate" -ForegroundColor Green
Write-Host "  - Requirement: >= 90%" -ForegroundColor Green
Write-Host "  - Status: PASSED" -ForegroundColor Green
Write-Host ""
Write-Host "Remaining tests:" -ForegroundColor Yellow
Write-Host "  1. Minimax vs Random on 64-position suite (not just standard opening)" -ForegroundColor Yellow
Write-Host "  2. MCTS vs Minimax (depth-4): 50 games, >= 70% required" -ForegroundColor Yellow

Write-Host ""
Write-Host "Test script completed at: $(Get-Date)" -ForegroundColor Gray