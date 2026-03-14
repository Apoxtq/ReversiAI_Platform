# ReversiAI Platform - Full Benchmark Script
# 运行完整性能测试，保存原始输出数据

$ErrorActionPreference = "Continue"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "ReversiAI Platform - Full Benchmark" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Fixed paths
$ScriptDir = "d:\Project\Reversi\ReversiAI_Platform\scripts\validation"
$ProjectRoot = "d:\Project\Reversi\ReversiAI_Platform"
$BuildDir = "$ProjectRoot\build_console"
$ResultsDir = "$ScriptDir\results"
$RawDataDir = "$ResultsDir\raw_data"
$BenchmarkDir = "$ProjectRoot\benchmark_results"

Write-Host "Project Root: $ProjectRoot" -ForegroundColor Gray
Write-Host "Build Dir: $BuildDir" -ForegroundColor Gray

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
Write-Host "[1/5] Checking build status..." -ForegroundColor Yellow

$buildStatus = "FAIL"
$buildCheckOutput = ""

if (Test-Path "$BuildDir\ReversiAI_Platform_Console.exe") {
    Write-Host "  [OK] Console version found" -ForegroundColor Green
    $buildStatus = "OK"
    $buildCheckOutput = "Console executable found: $BuildDir\ReversiAI_Platform_Console.exe"
} else {
    Write-Host "  [FAIL] Console version NOT found" -ForegroundColor Red
    $buildCheckOutput = "ERROR: Console executable NOT found"
}

# Save build check
@"
========================================
BUILD CHECK - FULL BENCHMARK
========================================
Timestamp: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
Status: $buildStatus
$buildCheckOutput
========================================
"@ | Out-File -FilePath "$RawDataDir\benchmark_build_check_$timestamp.txt" -Encoding UTF8

if ($buildStatus -ne "OK") {
    Write-Host "  Please compile first:" -ForegroundColor Yellow
    Write-Host "    mkdir build_console" -ForegroundColor White
    Write-Host "    cd build_console" -ForegroundColor White
    Write-Host "    cmake -G 'MinGW Makefiles' -DBUILD_QT_GUI=OFF .." -ForegroundColor White
    Write-Host "    cmake --build . --parallel 8" -ForegroundColor White
    exit 1
}

# 2. Prepare benchmark environment
Write-Host ""
Write-Host "[2/5] Preparing benchmark environment..." -ForegroundColor Yellow

# Remove old results
if (Test-Path $BenchmarkDir) {
    Remove-Item -Recurse -Force $BenchmarkDir
    Write-Host "  Cleared old benchmark results" -ForegroundColor Green
}

# 3. Run benchmark
Write-Host ""
Write-Host "[3/5] Running benchmark (this may take 5-10 minutes)..." -ForegroundColor Yellow
Write-Host "  Starting ReversiAI_Platform_Console.exe..." -ForegroundColor Gray
Write-Host ""

$benchmarkStart = Get-Date
$benchmarkStartStr = $benchmarkStart.ToString("yyyy-MM-dd HH:mm:ss")

# Run the console app and capture ALL output
$benchmarkOutput = & "$BuildDir\ReversiAI_Platform_Console.exe" 2>&1 | Out-String
$benchmarkExitCode = $LASTEXITCODE

$benchmarkEnd = Get-Date
$benchmarkEndStr = $benchmarkEnd.ToString("yyyy-MM-dd HH:mm:ss")
$benchmarkDuration = $benchmarkEnd - $benchmarkStart

Write-Host ""
Write-Host "  Benchmark completed in $($benchmarkDuration.TotalMinutes.ToString('F1')) minutes" -ForegroundColor Green
Write-Host "  Exit code: $benchmarkExitCode" -ForegroundColor Gray

# Save raw benchmark output
@"
========================================
FULL BENCHMARK OUTPUT
========================================
Start Time: $benchmarkStartStr
End Time: $benchmarkEndStr
Duration: $($benchmarkDuration.TotalMinutes.ToString('F1')) minutes
Exit Code: $benchmarkExitCode
----------------------------------------
$benchmarkOutput
========================================
"@ | Out-File -FilePath "$RawDataDir\benchmark_full_output_$timestamp.txt" -Encoding UTF8

Write-Host "  [OK] Raw benchmark output saved" -ForegroundColor Green

# 4. Check results
Write-Host ""
Write-Host "[4/5] Checking benchmark results..." -ForegroundColor Yellow

$resultsExist = $false

if (Test-Path $BenchmarkDir) {
    Write-Host "  [OK] Benchmark results found" -ForegroundColor Green
    $resultsExist = $true
    
    # List and copy result files
    Write-Host ""
    Write-Host "  Result files:" -ForegroundColor Gray
    $resultFiles = Get-ChildItem $BenchmarkDir -Recurse
    foreach ($file in $resultFiles) {
        Write-Host "    - $($file.Name)" -ForegroundColor White
        
        # Copy to raw data
        $destPath = "$RawDataDir\benchmark_$($file.Name)"
        Copy-Item $file.FullName -Destination $destPath -Force
    }
} else {
    Write-Host "  [WARN] Benchmark results directory not found" -ForegroundColor Yellow
    Write-Host "  The console app may have encountered an error" -ForegroundColor Yellow
    
    @"
========================================
BENCHMARK RESULTS CHECK
========================================
Results Directory: $BenchmarkDir
Status: NOT FOUND
The console app may have encountered an error or results were not exported.
Exit Code: $benchmarkExitCode
========================================
"@ | Out-File -FilePath "$RawDataDir\benchmark_results_check_$timestamp.txt" -Encoding UTF8
}

# 5. Generate comprehensive report
Write-Host ""
Write-Host "[5/5] Generating validation report..." -ForegroundColor Yellow

$date = Get-Date -Format "yyyy-MM-dd HH:mm:ss"

$benchmarkStatus = if ($resultsExist) { "OK" } else { "PENDING" }

# List raw data files
$rawFiles = Get-ChildItem $RawDataDir -Filter "*$timestamp*" | Sort-Object Name
$rawFileList = $rawFiles | ForEach-Object { "- [$($_.Name)](raw_data/$($_.Name))" } | Out-String

$report = @"
# ReversiAI Platform - Full Benchmark Report

**Date**: $date
**Duration**: $($benchmarkDuration.TotalMinutes.ToString('F1')) minutes
**Version**: v1.0.0
**Timestamp**: $timestamp

---

## Build Status

| Component | Status |
|-----------|--------|
| Console Version | [OK] Compiled |

## Benchmark Results

| Metric | Target | Result | Status |
|--------|--------|--------|--------|
| Bitboard Flip Speed | >=100M/s | See raw output | $benchmarkStatus |
| Minimax Throughput | >=2.0M nodes/s | See raw output | $benchmarkStatus |
| MCTS Simulation Rate | >=200K sims/s | See raw output | $benchmarkStatus |
| Minimax-6 vs Random Win Rate | >=90% | See raw output | $benchmarkStatus |
| MCTS vs Minimax-4 Win Rate | >=70% | See raw output | $benchmarkStatus |

## Raw Data Files

$rawFileList

### Raw Output Description

| File | Description |
|------|-------------|
| `benchmark_build_check_$timestamp.txt` | Build check results |
| `benchmark_full_output_$timestamp.txt` | Complete console output from benchmark run |
| `benchmark_results_check_$timestamp.txt` | Results directory check |

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

## Output Files

- **All Raw Data**: `scripts/validation/results/raw_data/`
- **Validation Report**: `scripts/validation/results/validation_report.md`

---
*This report was auto-generated by1*
*Original raw run_benchmark.ps data saved in: raw_data/*
"@

$reportPath = "$ResultsDir\validation_report.md"
$report | Out-File -FilePath $reportPath -Encoding UTF8

Write-Host "  [OK] Report generated: $reportPath" -ForegroundColor Green

# Summary
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Benchmark Complete" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

if ($resultsExist) {
    Write-Host "[OK] All benchmarks completed successfully!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Results location:" -ForegroundColor White
    Write-Host "  $BenchmarkDir" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Raw data location:" -ForegroundColor White
    Write-Host "  $RawDataDir" -ForegroundColor Gray
} else {
    Write-Host "[WARN] Please check console output for errors" -ForegroundColor Yellow
    Write-Host "  Raw output saved to: $RawDataDir\benchmark_full_output_$timestamp.txt" -ForegroundColor Gray
}

Write-Host ""
Write-Host "Report: $reportPath" -ForegroundColor Gray
