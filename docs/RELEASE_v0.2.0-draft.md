# ReversiAI_Platform — Release Draft v0.2.0-dev

Release draft for `v0.2.0-dev`. This document records the snapshot of the repository prepared for release and instructions for verification.

Summary
- Version: v0.2.0-dev
- Date: 2026-01-17
- Type: Core game logic + basic GUI + testing and CI integration

Highlights
- BitBoard core implemented and validated (位运算 + 翻转逻辑)
- Board wrapper implemented for game rules and undo support
- Qt GUI integration (WBChessQT) working (Debug GUI tested)
- Unit tests added (GoogleTest): `BitBoard` and `Board`
- Performance benchmark (`perft_performance`) added under `Tests/performance/`
- CI workflows:
  - `ci.yml`: cross-platform build and test (Ubuntu/Windows)
  - `perft.yml`: scheduled daily perft benchmark and artifact upload
  - `coverage.yml`: coverage generation and report upload
- Test framework automatically fetches GoogleTest if missing (CMake FetchContent)

Build & Verification (recommended)
1. Local quick verify (Debug):
   mkdir build && cd build
   cmake .. -DBUILD_TESTS=ON
   cmake --build . --config Debug -- -j4
   ctest --test-dir . --output-on-failure -C Debug

2. Performance smoke test (Release):
   cmake -S . -B build_perft -DBUILD_TESTS=ON -DPERFORMANCE_TESTING=ON -DCMAKE_BUILD_TYPE=Release
   cmake --build build_perft --target perft_performance
   ./build_perft/bin/perft_performance 2000000

3. CI:
   - Pushing a tag `v0.2.0-dev` will trigger `ci.yml`. Review artifacts and ctest logs.
   - Perft and coverage workflows upload artifacts; download and inspect CSV / coverage_report.

Known issues & notes
- Tests cover core paths (initial position, basic moves, undo). Full coverage and MCTS unit tests are postponed until feature freeze.  
- Some warnings remain (signedness, deprecated QMouseEvent API); they do not block function but should be cleaned in next iteration.  
- GUI tested on Windows MinGW and basic interactions validated; platform differences should be checked in CI artifacts.

Next steps before final v0.2.0
1. Increase unit test coverage for MCTS and edge cases.  
2. Run perft benchmarks across multiple hardware (collect CSVs) and compare with Egaroucid/edax baselines.  
3. Clean warnings and finish documentation for experiments.  

Artifacts to attach (after CI finishes)
- build artifacts (Debug/Release)  
- ctest logs (LastTest.log)  
- perft_results.csv  
- coverage_report


