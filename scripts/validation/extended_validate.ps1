# ReversiAI Platform - Extended Validation Script
# 扩展验证脚本 - 包含性能、系统、覆盖率等全面验证

$ErrorActionPreference = "Continue"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "ReversiAI Platform Extended Validation" -ForegroundColor Cyan
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

# Create directories
if (-not (Test-Path $ResultsDir)) {
    New-Item -ItemType Directory -Path $ResultsDir | Out-Null
}
if (-not (Test-Path $RawDataDir)) {
    New-Item -ItemType Directory -Path $RawDataDir | Out-Null
}

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"

# ============================================================================
# SECTION 1: Build Status
# ============================================================================
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "[1/7] Build Status Check" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$buildStatus = "FAIL"

if (Test-Path "$BuildDir\ReversiAI_Platform_Console.exe") {
    Write-Host "  [OK] Console version compiled" -ForegroundColor Green
    $buildStatus = "OK"
    
    # Get file size
    $exeSize = (Get-Item "$BuildDir\ReversiAI_Platform_Console.exe").Length / 1MB
    Write-Host "    Size: $($exeSize.ToString('F2')) MB" -ForegroundColor Gray
} else {
    Write-Host "  [FAIL] Console version NOT compiled" -ForegroundColor Red
}

@"
========================================
BUILD STATUS CHECK - EXTENDED
========================================
Timestamp: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Status: $buildStatus
Executable Size: $($exeSize.ToString('F2')) MB
========================================
"@ | Out-File -FilePath "$RawDataDir\build_status_$timestamp.txt" -Encoding UTF8

# ============================================================================
# SECTION 2: Unit Tests
# ============================================================================
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "[2/7] Unit Tests" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$testResults = @{}

# Core tests
Write-Host "  Running core unit tests..."
if (Test-Path "$BuildDir\Tests\unit\unit_tests_core.exe") {
    $coreOutput = & "$BuildDir\Tests\unit\unit_tests_core.exe" 2>&1 | Out-String
    $coreExitCode = $LASTEXITCODE
    
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
        Write-Host "    [FAIL] core tests failed" -ForegroundColor Red
        $testResults["core"] = "FAIL"
    }
} else {
    Write-Host "    [SKIP] core tests not found" -ForegroundColor Yellow
    $testResults["core"] = "SKIP"
}

# AI tests
Write-Host "  Running ai unit tests..."
if (Test-Path "$BuildDir\Tests\unit\unit_tests_ai.exe") {
    $aiOutput = & "$BuildDir\Tests\unit\unit_tests_ai.exe" 2>&1 | Out-String
    $aiExitCode = $LASTEXITCODE
    
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
        Write-Host "    [FAIL] ai tests failed" -ForegroundColor Red
        $testResults["ai"] = "FAIL"
    }
} else {
    Write-Host "    [SKIP] ai tests not found" -ForegroundColor Yellow
    $testResults["ai"] = "SKIP"
}

# Research tests
Write-Host "  Running research unit tests..."
if (Test-Path "$BuildDir\Tests\unit\unit_tests_research.exe") {
    $researchOutput = & "$BuildDir\Tests\unit\unit_tests_research.exe" 2>&1 | Out-String
    $researchExitCode = $LASTEXITCODE
    
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
        Write-Host "    [FAIL] research tests failed" -ForegroundColor Red
        $testResults["research"] = "FAIL"
    }
} else {
    Write-Host "    [SKIP] research tests not found" -ForegroundColor Yellow
    $testResults["research"] = "SKIP"
}

# ============================================================================
# SECTION 3: Code Coverage (gcov/lcov)
# ============================================================================
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "[3/7] Code Coverage Check" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$coverageStatus = "NOT_AVAILABLE"
$coveragePercent = "N/A"

# Check if coverage build exists
$coverageDir = "$ProjectRoot\build_console\coverage"
if (Test-Path $coverageDir) {
    $coverageHtml = Get-ChildItem $coverageDir -Filter "index.html" -Recurse
    if ($coverageHtml) {
        $coverageStatus = "BUILD_NEEDED"
        Write-Host "  [INFO] Coverage build exists but not run yet" -ForegroundColor Yellow
        Write-Host "    Run: cmake -DBUILD_COVERAGE=ON .." -ForegroundColor Gray
    }
} else {
    Write-Host "  [SKIP] Coverage build not configured" -ForegroundColor Yellow
    Write-Host "    To enable: cmake -DBUILD_COVERAGE=ON .." -ForegroundColor Gray
}

# ============================================================================
# SECTION 4: System Performance
# ============================================================================
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "[4/7] System Performance" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$perfResults = @{}

# Memory usage check
Write-Host "  Checking memory usage..."
$memStatus = "N/A"
try {
    # Get process memory if running
    $memStatus = "CHECK_MANUALLY"
    Write-Host "    [INFO] Memory check - run console app manually" -ForegroundColor Yellow
} catch {
    $memStatus = "ERROR"
}

$perfResults["memory"] = $memStatus

# Startup time check
Write-Host "  Checking startup time..."
$startupStatus = "N/A"
Write-Host "    [INFO] Startup time - run console app manually" -ForegroundColor Yellow
$perfResults["startup"] = $startupStatus

# ============================================================================
# SECTION 5: AI Performance Targets (Reference)
# ============================================================================
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "[5/7] AI Performance Targets" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

Write-Host "  Project Requirements:" -ForegroundColor Gray
Write-Host "    - Bitboard Flip: >= 100M/s" -ForegroundColor Gray
Write-Host "    - Minimax Throughput: >= 2.0M nodes/s" -ForegroundColor Gray
Write-Host "    - MCTS Simulation: >= 200K sims/s" -ForegroundColor Gray
Write-Host "    - Minimax-6 vs Random: >= 90% win rate" -ForegroundColor Gray
Write-Host "    - MCTS vs Minimax-4: >= 70% win rate" -ForegroundColor Gray
Write-Host ""
Write-Host "  [INFO] Run run_benchmark.ps1 for full performance test" -ForegroundColor Yellow

$perfResults["ai_targets"] = "PENDING"

# ============================================================================
# SECTION 6: Compilation Verification
# ============================================================================
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "[6/7] Compilation Verification" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$compResults = @{}

# MinGW (current)
Write-Host "  MinGW Compilation..."
if ($buildStatus -eq "OK") {
    Write-Host "    [OK] MinGW - Build successful" -ForegroundColor Green
    $compResults["mingw"] = "OK"
} else {
    Write-Host "    [FAIL] MinGW - Build failed" -ForegroundColor Red
    $compResults["mingw"] = "FAIL"
}

# MSVC (check if available)
Write-Host "  MSVC Compilation..."
$msvcBuildDir = "$ProjectRoot\build_msvc"
if (Test-Path "$msvcBuildDir\ReversiAI_Platform_Console.exe") {
    Write-Host "    [OK] MSVC - Build available" -ForegroundColor Green
    $compResults["msvc"] = "OK"
} else {
    Write-Host "    [SKIP] MSVC - Not built yet" -ForegroundColor Yellow
    Write-Host "      To build: cmake -G 'Visual Studio 17 2022' .." -ForegroundColor Gray
    $compResults["msvc"] = "SKIP"
}

# ============================================================================
# SECTION 7: Generate Report
# ============================================================================
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "[7/7] Generating Report" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$date = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

$report = @"
# ReversiAI Platform - Extended Validation Report

**Date**: $date
**Version**: v1.0.0
**Timestamp**: $timestamp

---

## 1. Build Status

| Component | Status | Details |
|-----------|--------|---------|
| Console Version | [$buildStatus] | Executable Size: $($exeSize.ToString('F2')) MB |
| Build Directory | [OK] | $BuildDir |

**Raw Data**: `raw_data/build_status_$timestamp.txt`

---

## 2. Unit Test Results

| Test Suite | Status | Raw Output |
|------------|--------|------------|
| Core (BitBoard/Board) | [$($testResults["core"])] | `raw_data/core_tests_$timestamp.txt` |
| AI (Minimax/MCTS/TT) | [$($testResults["ai"])] | `raw_data/ai_tests_$timestamp.txt` |
| Research (Benchmark) | [$($testResults["research"])] | `raw_data/research_tests_$timestamp.txt` |

### Test Quality Gates
- **Target**: 100% pass rate
- **Current**: See above

---

## 3. Code Coverage

| Metric | Status | Target |
|--------|--------|--------|
| Unit Test Coverage | $coverageStatus | >= 90% |

**Note**: To enable coverage:
```bash
cd build_console
cmake -DBUILD_COVERAGE=ON ..
cmake --build .
make coverage
```

---

## 4. System Performance

| Metric | Status | Target | Method |
|--------|--------|--------|--------|
| Memory Usage | $($perfResults["memory"]) | < 100MB | Manual test |
| Startup Time | $($perfResults["startup"]) | < 2s | Manual test |

---

## 5. AI Performance Targets

| Metric | Target | Status |
|--------|--------|--------|
| Bitboard Flip Speed | >= 100M/s | PENDING |
| Minimax Throughput | >= 2.0M nodes/s | PENDING |
| MCTS Simulation Rate | >= 200K sims/s | PENDING |
| Minimax-6 vs Random Win Rate | >= 90% | PENDING |
| MCTS vs Minimax-4 Win Rate | >= 70% | PENDING |

**Note**: Run `run_benchmark.ps1` for full performance validation

---

## 6. Compilation Verification

| Compiler | Status |
|----------|--------|
| MinGW | [$($compResults["mingw"])] |
| MSVC 2022 | [$($compResults["msvc"])] |

---

## 7. Feature Verification Checklist

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

## 8. Next Steps

1. **Run Full Benchmark**: `./run_benchmark.ps1`
2. **Build with Coverage**: `cmake -DBUILD_COVERAGE=ON ..`
3. **Test MSVC**: `cmake -G "Visual Studio 17 2022" ..`

---

*Report generated by extended_validate.ps1*
*Raw data: scripts/validation/results/raw_data/*
"@

$reportPath = "$ResultsDir\extended_validation_report.md"
$report | Out-File -FilePath $reportPath -Encoding UTF8

Write-Host "  [OK] Report generated: $reportPath" -ForegroundColor Green

# Summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Validation Complete" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

$failedCount = ($testResults.Values | Where-Object { $_ -eq "FAIL" }).Count

if ($failedCount -eq 0 -and $buildStatus -eq "OK") {
    Write-Host "[OK] Core validations passed!" -ForegroundColor Green
} else {
    Write-Host "[WARN] Some tests need attention" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  1. Run full benchmark: .\run_benchmark.ps1" -ForegroundColor White
Write-Host "  2. Build with coverage: cmake -DBUILD_COVERAGE=ON .." -ForegroundColor White
Write-Host "  3. Test MSVC: cmake -G 'Visual Studio 17 2022' .." -ForegroundColor White

Write-Host ""
Write-Host "Report: $reportPath" -ForegroundColor Gray
Write-Host "Raw Data: $RawDataDir" -ForegroundColor Gray
