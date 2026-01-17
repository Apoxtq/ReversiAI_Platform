# Changelog

All notable changes to this project will be documented in this file.

## [v0.2.0-dev] - 2026-01-17
### Added
- BitBoard core (位运算, getValidMoves, makeMove, calculateFlips)
- Board wrapper (game rules, undo/redo support)
- Qt GUI integration (WBChessQT)
- Unit tests: BitBoardTest, BoardTest (GoogleTest)
- Performance benchmark: perft_performance
- CI: cross-platform build, scheduled perft, coverage workflow

### Changed
- CMakeLists: enable test integration and FetchContent for GoogleTest

### Notes
- MCTS tests and full coverage postponed to next iteration


