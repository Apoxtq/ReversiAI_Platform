# Changelog

All notable changes to this project will be documented in this file.

## [v1.0.1] - 2026-03-01
### Fixed
- BitboardBenchmark: 修正标准开局位图定义 (player=黑棋在D5/E4, opponent=白棋在D4/E5)
- BitboardBenchmark: warmUp函数改用resetToStandardOpening()创建合法棋盘
- PositionSuite: 修复中局/残局位置生成逻辑，使用合法落子序列替代随机填充
- BitboardBenchmark: getTestPositions()中局位置使用不重叠的位图

### Changed
- main_console: 提高测试AI难度 (EASY → HARD) 和搜索深度 (2 → 6)
- main_console: 增加MCTS vs Minimax对局测试 (100局)
- PositionSuite: 使用预设合法落子序列生成标准64位置

### Notes
- v1.0.1 是一个维护版本，修复了位图相关的潜在bug
- Position Suite 现在生成的是真正合法棋盘状态

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


