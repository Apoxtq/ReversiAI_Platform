# Changelog

All notable changes to this project will be documented in this file.

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


