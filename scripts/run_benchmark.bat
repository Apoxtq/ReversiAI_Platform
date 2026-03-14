@echo off
REM ReversiAI Platform - Run Full Benchmark
REM This runs the complete performance validation

echo ========================================
echo ReversiAI Platform - Performance Test
echo ========================================
echo.

cd /d "d:\Project\Reversi\ReversiAI_Platform\build_console"

echo Running benchmark (this may take a few minutes)...
echo.

REM Run the console app which will execute all benchmarks
REM Note: This will generate benchmark_results folder

.\ReversiAI_Platform_Console.exe

echo.
echo ========================================
echo Benchmark Complete
echo ========================================
echo.

if exist "d:\Project\Reversi\ReversiAI_Platform\benchmark_results" (
    echo Results saved to: benchmark_results\
    dir "d:\Project\Reversi\ReversiAI_Platform\benchmark_results"
) else (
    echo Note: benchmark_results folder not found
    echo The console app may need to run longer
)

pause
