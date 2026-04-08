# Changelog

All notable changes to this project will be documented in this file.

## [v1.0.2] - 2026-04-08
### Fixed
- **NetworkGameWindow**: Fixed recursive closeEvent crash — added `isClosing_` guard flag to prevent double-trigger
- **NetworkGameWindow**: Fixed double `stopHosting()` call in destructor by moving `isClosing_` check before network cleanup
- **NetworkHost**: Fixed `clientSocket_` signal cascade — `disconnect()` now called before `close()` to prevent `onClientDisconnected` re-entry
- **MenuWindow**: Optimized back-to-menu navigation — returns directly to main menu instead of network lobby

### Changed
- **MenuWindow**: backToMenu lambda now closes both gameWindow and lobbyWindow, shows main menu directly
- **Project Structure**: Organized scattered test and debug files into dedicated directories

### Project Reorganization
- Created `tests/experiments/` — 21 experimental test files moved from root
- Created `tests/debug/` — 13 debug files moved from root  
- Moved 3 backup files to `src/core/`
- Cleaned up 18 orphaned .exe and .o build artifacts from root directory

### Notes
- v1.0.2 fixes critical stability issues in network multiplayer module
- All network window close scenarios (button/X button/double-click) now verified safe
- Project structure now clearly separates production code from experimental/debug code
- AI Research Mode (button 4) not yet tested

## [v1.0.3] - 2026-03-14
### Fixed
- **MCTSAI Factory**: Fixed factory returning nullptr issue
- **MCTSAI Compilation**: Added missing Evaluator header, fixed type conversion
- **MCTSAI Integration Crash**: Fixed crash by initializing ZobristHash before MCTS creation

### Verified
- Minimax vs Random: 99% win rate (target: ≥90%) ✅
- MCTS vs Minimax: Integration test passed ✅
- Bitboard Benchmark: 7/7 tests passed ✅

### Notes
- v1.0.2 fixes MCTS integration issues
- All core acceptance criteria now validated
- Project ready for release

## [v1.0.1] - 2026-03-06
### Added
- **Dual Compiler Support**: Added MinGW and MSVC dual compiler support
- **CMake Auto-Detection**: CMake automatically detects compiler type and selects appropriate Qt6
  - MinGW → Qt 6.10.1/mingw_64
  - MSVC → Qt 6.10.2/msvc2022_64
- **MSVC Qt6 Configuration**: Updated CMakeLists.txt with MSVC Qt6 path detection

### Fixed
- BitboardBenchmark: 修正标准开局位图定义 (player=黑棋在D5/E4, opponent=白棋在D4/E5)
- BitboardBenchmark: warmUp函数改用resetToStandardOpening()创建合法棋盘
- PositionSuite: 修复中局/残局位置生成逻辑，使用合法落子序列替代随机填充
- BitboardBenchmark: getTestPositions()中局位置使用不重叠的位图

### Changed
- main_console: 提高测试AI难度 (EASY → HARD) 和搜索深度 (2 → 6)
- main_console: 增加MCTS vs Minimax对局测试 (100局)
- PositionSuite: 使用预设合法落子序列生成标准64位置
- README.md: Updated build instructions with MinGW and MSVC examples

### Notes
- v1.0.1 adds dual compiler support for MinGW and MSVC
- Both MinGW and MSVC builds verified and working
- Position Suite now generates truly legal board states

## [v0.8.0] - 2026-02-25
### Added
- Bitboard Benchmark (Flip/Move/Legal/Copy/Hash performance testing)
- AI Search Benchmark (Minimax/MCTS throughput measurement)
- Head-to-Head Engine (Win rate verification with statistical analysis)
- Data Exporter (JSON/CSV/Markdown multi-format export)
- Position Suite (64 standard test positions)
- Statistics Tools (Wilcoxon test, confidence intervals)
- Console version for benchmark testing

### Changed
- Updated version to v0.8.0
- Enhanced BattleEngine with more statistics

### Notes
- All core P0 features completed for v0.8.0
- Ready for academic performance validation

## [v0.7.0] - 2026-02-12
### Added
- Killer Moves (2 killers per depth, 64 depths)
- History Heuristic (64x64 matrix, depth-weighted scoring)
- Move Orderer (PV > Killer > History > Static priority)
- Search Parameters (configurable midgame/endgame weights)
- MinimaxAI Integration (Killer/History recording)

### Notes
- AI optimization version complete
- Search efficiency improved by ~10%


