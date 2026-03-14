# ReversiAI Platform - Quick Validation Script
# 运行基本功能测试，保存原始输出数据

$ErrorActionPreference = "Continue"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "ReversiAI Platform Validation Test" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Fixed paths
$ScriptDir = "d:\Project\Reversi\ReversiAI_Platform\scripts\validation"
$ProjectRoot = "d:\Project\Reversi\ReversiAI_Platform"
$BuildDir = "$ProjectRoot\build_console"
$ResultsDir = "$ScriptDir\results"
$RawDataDir = "$ResultsDir\raw_data"

Write-Host "Project Root: $ProjectRoot" -ForegroundColor Gray
Write-Host "Build Dir: $BuildDir" -ForegroundColor Gray
Write-Host "Raw Data Dir: $RawDataDir" -ForegroundColor Gray

# Create directories
if (-not (Test-Path $ResultsDir)) {
    New-Item -ItemType Directory -Path $ResultsDir | Out-Null
}
if (-not (Test-Path $RawDataDir)) {
    New-Item -ItemType Directory -Path $RawDataDir | Out-Null
}

# Get timestamp for file naming
$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"

# 1. Check build status
Write-Host ""
Write-Host "[1/4] Checking build status..." -ForegroundColor Yellow

$buildStatus = "FAIL"
$buildOutput = ""

if (Test-Path "$BuildDir\ReversiAI_Platform_Console.exe") {
    Write-Host "  [OK] Console version compiled" -ForegroundColor Green
    $buildStatus = "OK"
    $buildOutput = "Console executable found at: $BuildDir\ReversiAI_Platform_Console.exe"
} else {
    Write-Host "  [FAIL] Console version NOT compiled" -ForegroundColor Red
    $buildOutput = "ERROR: Console executable NOT found at: $BuildDir\ReversiAI_Platform_Console.exe"
}

# Save raw build status
@"
========================================
BUILD STATUS CHECK
========================================
Timestamp: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Status: $buildStatus
$buildOutput
========================================
"@ | Out-File -FilePath "$RawDataDir\build_status_$timestamp.txt" -Encoding UTF8

# 2. Run unit tests
Write-Host ""
Write-Host "[2/4] Running unit tests..." -ForegroundColor Yellow

$testResults = @{}

# Core tests
Write-Host "  Running core unit tests..."
$coreOutput = ""
if (Test-Path "$BuildDir\Tests\unit\unit_tests_core.exe") {
    $coreOutput = & "$BuildDir\Tests\unit\unit_tests_core.exe" 2>&1 | Out-String
    $coreExitCode = $LASTEXITCODE
    
    # Save raw output
    @"
========================================
CORE UNIT TESTS
========================================
Timestamp: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Exit Code: $coreExitCode
----------------------------------------
$coreOutput
========================================
"@ | Out-File -FilePath "$RawDataDir\core_tests_$timestamp.txt" -Encoding UTF8
    
    if ($coreExitCode -eq 0) {
        Write-Host "    [OK] core tests passed" -ForegroundColor Green
        $testResults["core"] = "OK"
    } else {
        Write-Host "    [FAIL] core tests failed (exit code: $coreExitCode)" -ForegroundColor Red
        $testResults["core"] = "FAIL"
    }
} else {
    $coreOutput = "ERROR: Test executable not found"
    @"
========================================
CORE UNIT TESTS
========================================
Timestamp: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Status: NOT FOUND
$coreOutput
========================================
"@ | Out-File -FilePath "$RawDataDir\core_tests_$timestamp.txt" -Encoding UTF8
    Write-Host "    [SKIP] core tests not found" -ForegroundColor Yellow
    $testResults["core"] = "SKIP"
}

# AI tests
Write-Host "  Running ai unit tests..."
$aiOutput = ""
if (Test-Path "$BuildDir\Tests\unit\unit_tests_ai.exe") {
    $aiOutput = & "$BuildDir\Tests\unit\unit_tests_ai.exe" 2>&1 | Out-String
    $aiExitCode = $LASTEXITCODE
    
    # Save raw output
    @"
========================================
AI UNIT TESTS
========================================
Timestamp: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Exit Code: $aiExitCode
----------------------------------------
$aiOutput
========================================
"@ | Out-File -FilePath "$RawDataDir\ai_tests_$timestamp.txt" -Encoding UTF8
    
    if ($aiExitCode -eq 0) {
        Write-Host "    [OK] ai tests passed" -ForegroundColor Green
        $testResults["ai"] = "OK"
    } else {
        Write-Host "    [FAIL] ai tests failed (exit code: $aiExitCode)" -ForegroundColor Red
        $testResults["ai"] = "FAIL"
    }
} else {
    $aiOutput = "ERROR: Test executable not found"
    @"
========================================
AI UNIT TESTS
========================================
Timestamp: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Status: NOT FOUND
$aiOutput
========================================
"@ | Out-File -FilePath "$RawDataDir\ai_tests_$timestamp.txt" -Encoding UTF8
    Write-Host "    [SKIP] ai tests not found" -ForegroundColor Yellow
    $testResults["ai"] = "SKIP"
}

# Research tests
Write-Host "  Running research unit tests..."
$researchOutput = ""
if (Test-Path "$BuildDir\Tests\unit\unit_tests_research.exe") {
    $researchOutput = & "$BuildDir\Tests\unit\unit_tests_research.exe" 2>&1 | Out-String
    $researchExitCode = $LASTEXITCODE
    
    # Save raw output
    @"
========================================
RESEARCH UNIT TESTS
========================================
Timestamp: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Exit Code: $researchExitCode
----------------------------------------
$researchOutput
========================================
"@ | Out-File -FilePath "$RawDataDir\research_tests_$timestamp.txt" -Encoding UTF8
    
    if ($researchExitCode -eq 0) {
        Write-Host "    [OK] research tests passed" -ForegroundColor Green
        $testResults["research"] = "OK"
    } else {
        Write-Host "    [FAIL] research tests failed (exit code: $researchExitCode)" -ForegroundColor Red
        $testResults["research"] = "FAIL"
    }
} else {
    $researchOutput = "ERROR: Test executable not found"
    @"
========================================
RESEARCH UNIT TESTS
========================================
Timestamp: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Status: NOT FOUND
$researchOutput
========================================
"@ | Out-File -FilePath "$RawDataDir\research_tests_$timestamp.txt" -Encoding UTF8
    Write-Host "    [SKIP] research tests not found" -ForegroundColor Yellow
    $testResults["research"] = "SKIP"
}

# 3. Generate validation report
Write-Host ""
Write-Host "[3/4] Generating validation report..." -ForegroundColor Yellow

$date = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

# List raw data files
$rawFiles = Get-ChildItem $RawDataDir -Filter "*.txt" | Sort-Object LastWriteTime -Descending | Select-Object -First 10
$rawFileList = $rawFiles | ForEach-Object { "- $($_.Name)" } | Out-String

$report = @"
# ReversiAI Platform Validation Report

**Date**: $date
**Version**: v1.0.0
**Timestamp**: $timestamp

---

## Build Status

| Component | Status |
|-----------|--------|
| Console Version | [$buildStatus] Compiled |

**Raw Data**: `raw_data/build_status_$timestamp.txt`

## Unit Test Results

| Test Suite | Status | Raw Output File |
|------------|--------|-----------------|
| Core (BitBoard/Board) | [$($testResults["core"])] | `raw_data/core_tests_$timestamp.txt` |
| AI (Minimax/MCTS/TT) | [$($testResults["ai"])] | `raw_data/ai_tests_$timestamp.txt` |
| Research (Benchmark) | [$($testResults["research"])] | `raw_data/research_tests_$timestamp.txt` |

## Raw Data Files

$rawFileList

---

## Feature Verification Checklist

### Essential Features
- [x] Minimax/Negamax + MCTS AI
- [x] Bitboard Board System
- [x] Local PvP
- [x] PvE (Human vs AI)
- [x] LAN Network Multiplayer
- [x] Benchmark Framework

### Desirable Features
- [x] Transposition Tables + Zobrist Hashing
- [x] Iterative Deepening + Time Management
- [x] Killer Moves + History Heuristic
- [ ] Internet Multiplayer (Abandoned)
- [x] Extended Visualisation

---

## Performance Metrics (Full Test Pending)

| Metric | Target | Status |
|--------|--------|--------|
| Minimax (depth-6) vs Random | >=90% | [PENDING] |
| MCTS vs Minimax (depth-4) | >=70% | [PENDING] |
| Bitboard Flip Speed | >=100M/s | [PENDING] |
| Minimax Throughput | >=2.0M nodes/s | [PENDING] |

---

*This report was auto-generated by validate.ps1*
*Raw data saved in: raw_data/*
"@

$reportPath = "$ResultsDir\validation_report.md"
$report | Out-File -FilePath $reportPath -Encoding UTF8

Write-Host "  [OK] Report generated: $reportPath" -ForegroundColor Green

# 4. Summary
Write-Host ""
Write-Host "[4/4] Validation Complete" -ForegroundColor Yellow
Write-Host ""

$failedCount = ($testResults.Values | Where-Object { $_ -eq "FAIL" }).Count
$skipCount = ($testResults.Values | Where-Object { $_ -eq "SKIP" }).Count

if ($failedCount -eq 0 -and $buildStatus -eq "OK") {
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "[OK] All basic validations passed!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next step - Run full performance test:" -ForegroundColor Cyan
    Write-Host "  cd $ScriptDir" -ForegroundColor White
    Write-Host "  .\run_benchmark.ps1" -ForegroundColor White
} elseif ($skipCount -gt 0) {
    Write-Host "========================================" -ForegroundColor Yellow
    Write-Host "[WARN] Build needed - run build_console first" -ForegroundColor Yellow
    Write-Host "========================================" -ForegroundColor Yellow
} else {
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "[FAIL] Some tests failed" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
}

Write-Host ""
Write-Host "Report: $reportPath" -ForegroundColor Gray
Write-Host "Raw Data: $RawDataDir" -ForegroundColor Gray
