# ReversiAI_Platform 🎓

|**Version**: v1.0.0
|**Status**: ✅ Final Release Completed
|**Project**: University of Liverpool COMP390 Honours Year Project

A comprehensive Othello/Reversi AI research and benchmarking platform implemented in modern C++, demonstrating academic excellence in open-source learning and software engineering.

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-6.10-blue.svg)](https://www.qt.io/)
[![CMake](https://img.shields.io/badge/CMake-3.16+-green.svg)](https://cmake.org/)
[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Academic Project](https://img.shields.io/badge/Academic-University%20of%20Liverpool-red.svg)](https://www.liverpool.ac.uk/)

---

## ✨ Core Features

### 🔢 BitBoard System (v0.2.0 ✅ Completed)
- **Efficient Implementation**: uint64_t bitboard with 10M+ ops/sec performance
- **Complete Game Logic**: Precise Othello flip rules and move generation
- **Cross-Platform**: CMake build system supporting Windows/Linux/macOS
- **Academic Validation**: Thoroughly tested with standard opening scenarios

### 🤖 AI Algorithms (v0.3.0 ✅ Completed)
- **MinimaxAI**: Classical game tree search with Alpha-Beta pruning
- **MCTSAI**: Monte Carlo Tree Search probabilistic algorithm
- **RandomAI**: Baseline AI for benchmarking and testing
- **Evaluation Functions**: Dynamic heuristic assessment
- **Difficulty Levels**: Easy/Medium/Hard AI opponents

### 🎮 User Interface (v0.4.0 ✅ Completed)
- **MenuWindow**: Game mode selection interface (PvE/PvP/Network)
- **PvEWindow**: Human vs AI gameplay with difficulty selection
- **PvPWindow**: Local two-player mode with undo functionality
- **Real-time Display**: Board visualization and move highlighting
- **Statistics**: Game history and win rate tracking

### 🌐 Network Multiplayer (v0.5.0 ✅ Completed)
- **TCP Communication**: Stable client-server architecture
- **LAN Discovery**: Automatic host detection via UDP broadcast
- **Game State Synchronization**: Real-time board sync with Zobrist hashing
- **Room System**: Create/Join rooms with customizable settings
- **Reconnection Handling**: Exponential backoff reconnection with auto-retry
- **Heartbeat Mechanism**: 30-second keep-alive for connection stability
- **Network Lobby**: Room browser with auto-refresh
- **Network Game Window**: Full-featured online gameplay with chat
- **Latency Monitoring**: Real-time ping display

### 📊 Research Framework (v0.6.0 ✅ Completed)
- **Zobrist Hashing**: 64-bit position encoding, 2MB memory, 33M entries
- **Transposition Table**: 112MB cache, 4.2M entries, search optimization
- **Position Suite**: 64 standard test positions for reproducible experiments
- **Battle Engine**: Head-to-head AI battle system with win rate statistics
- **Statistics Tools**: Wilcoxon test, confidence intervals, statistical significance
- **AI vs AI Window**: Visual interface for automated benchmark testing

### ⚡ AI Optimization (v0.7.0 ✅ Completed)
- **Killer Moves**: Record pruned moves for future优先 ordering
- **History Heuristic**: 64x64 matrix for move history scoring
- **Move Orderer**: Unified sorting: PV > Killer > History > Static
- **Search Parameters**: Configurable weights for midgame/endgame
- **Performance Target**: ≥10% search efficiency improvement

### 📊 Performance Benchmarking (v0.8.0 ✅ Completed)
- **Bitboard Benchmark**: Flip/Move/Legal/Copy performance testing
- **AI Search Benchmark**: Minimax/MCTS throughput measurement
- **Head-to-Head Engine**: Win rate verification with statistical analysis
- **Data Exporter**: JSON/CSV/Markdown multi-format export
- **Position Suite**: 64 standard test positions
- **Statistics**: Wilcoxon test, confidence intervals

### 📈 Visualization Enhancement (v0.9.0 ✅ Completed)
- **Search Tree Visualization**: Textual display of search tree and PV line
- **Real-time Statistics Panel**: Live AI search statistics during gameplay
- **Board Heatmap**: Visual representation of position evaluation
- **Game Replay System**: Load/Save game records with playback controls
- **PGN/SGF Support**: Standard game notation import/export
- **Replay Analysis Window**: Dedicated interface for game analysis

---

## 🚀 Quick Start

### Prerequisites
- **C++ Compiler**: GCC 9+, Clang 10+, or MSVC 2019+
- **CMake**: Version 3.16 or higher
- **Qt6**: Version 6.10 or higher (for GUI)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/Apoxtq/ReversiAI_Platform.git
cd ReversiAI_Platform

# Create build directory
mkdir build && cd build

# Configure with CMake (GUI mode)
cmake .. -DBUILD_QT_GUI=ON -DCMAKE_BUILD_TYPE=Release

# Build the project
cmake --build . --config Release

# Deploy Qt dependencies (Windows)
windeployqt ReversiAI_Platform.exe

# Run the GUI application
./ReversiAI_Platform
```

### GUI Features

```
Main Menu (目录界面)
├── PvE Button (人机对战) → PvE Window
│   ├── Select AI difficulty (Easy/Medium/Hard)
│   ├── Choose who plays first (AI First / Player First)
│   ├── View AI thinking stats (nodes, time)
│   └── Back to menu option
├── PvP Button (本地双人) → PvP Window
│   ├── Two-player local gameplay
│   ├── Undo functionality
│   └── Back to menu option
├── Network Button (线上对战) → Network Lobby
│   ├── Create new room
│   ├── Browse and join existing rooms
│   ├── Online multiplayer gameplay
│   ├── Real-time chat
│   └── Latency display
└── Replay Analysis (棋谱回放分析) → Replay Analysis Window
    ├── Load/Save game records (PGN/SGF)
    ├── Playback controls (play/pause/step)
    ├── Board heatmap visualization
    └── Search tree analysis
```

---

## 📚 Documentation

All documentation is available in the `项目计划（文档放置）/` directory:

### Project Tracking
- **[项目进展跟踪.md](项目计划（文档放置）/项目进展跟踪.md)** - Project progress and milestone tracking
- **[ReversiAI_Platform_项目计划.md](项目计划（文档放置）/ReversiAI_Platform_项目计划.md)** - Complete project roadmap

### Version Documentation
- **[v0.9.0-完成报告.md](项目计划（文档放置）/v0.9.0-完成报告.md)** - v0.9.0 completion report (Visualization Enhancement)
- **[v0.9.0-设计规划.md](项目计划（文档放置）/v0.9.0-设计规划.md)** - v0.9.0 visualization design
- **[v0.8.0-完成报告.md](项目计划（文档放置）/v0.8.0-完成报告.md)** - v0.8.0 completion report (Performance Benchmarking)
- **[v0.8.0-设计规划.md](项目计划（文档放置）/v0.8.0-设计规划.md)** - v0.8.0 performance benchmarking design
- **[v0.7.0-完成报告.md](项目计划（文档放置）/v0.7.0-完成报告.md)** - v0.7.0 completion report (AI Optimization)
- **[v0.7.0-设计规划.md](项目计划（文档放置）/v0.7.0-设计规划.md)** - v0.7.0 AI optimization design
- **[v0.6.0-完成报告.md](项目计划（文档放置）/v0.6.0-完成报告.md)** - v0.6.0 completion report
- **[v0.6.0-设计规划.md](项目计划（文档放置）/v0.6.0-设计规划.md)** - v0.6.0 research framework design

### Core Documents
- **[技术债务与开源参考.md](项目计划（文档放置）/技术债务与开源参考.md)** - Open-source reference guide
- **[学术项目检查清单.md](项目计划（文档放置）/学术项目检查清单.md)** - Academic compliance verification
- **[API设计规范.md](项目计划（文档放置）/API设计规范.md)** - Code design standards

---

## 🏗️ Architecture

```
ReversiAI_Platform/
├── include/
│   ├── core/              # Core game logic
│   │   ├── Board.h       # Board representation
│   │   ├── BitBoard.h    # BitBoard optimization
│   │   └── Move.h        # Move structure
│   ├── ai/               # AI algorithms
│   │   ├── AIStrategy.h  # AI interface
│   │   ├── MinimaxAI.h   # Minimax algorithm
│   │   ├── MCTSAI.h      # MCTS algorithm
│   │   └── RandomAI.h    # Random AI
│   └── ui/               # User interface
│       ├── MenuWindow.h  # Main menu
│       ├── PvEWindow.h   # PvE game window
│       ├── PvPWindow.h   # PvP game window
│       ├── NetworkLobbyWindow.h  # Network lobby
│       ├── NetworkGameWindow.h   # Network game
│       ├── GameController.h       # Game state management
│       └── StatisticsManager.h    # Game statistics
├── network/               # Network module (v0.5.0)
│   ├── include/network/
│   │   ├── message.hpp           # Message serialization
│   │   ├── networkclient.hpp     # TCP client
│   │   ├── networkdiscovery.hpp  # UDP discovery
│   │   ├── gamesynchronizer.hpp  # State sync
│   │   ├── roommanager.hpp       # Room management
│   │   └── reconnectionmanager.hpp # Reconnection
│   └── src/
│       ├── message.cpp
│       ├── networkclient.cpp
│       ├── networkdiscovery.cpp
│       ├── gamesynchronizer.cpp
│       ├── roommanager.cpp
│       └── reconnectionmanager.cpp
├── src/
│   ├── core/             # Core implementation
│   ├── ai/               # AI implementation
│   └── ui/               # UI implementation
├── ui/                   # Qt UI files (.ui)
└── 项目计划（文档放置）/   # Academic documentation
```

### Design Principles
- **Academic Rigor**: Harvard citation standards and ethical compliance
- **Modern C++**: C++17 features with RAII and smart pointers
- **Modular Design**: Clean separation of concerns for research and extension
- **Qt Framework**: Signal/slot mechanism for inter-object communication
- **Cross-Platform**: Native support for Windows, Linux, and macOS

---

## 📈 Development Roadmap

### ✅ Completed Versions

#### v0.9.0 (February 2026) - Visualization Enhancement
- **Features**: Search tree visualization, Real-time statistics panel, Board heatmap, Game replay
- **Status**: 100% Complete
- **Code Lines**: ~1,800 new lines
- **Key Features**:
  - ✅ Search Tree Visualization (Textual display of PV line and search tree)
  - ✅ Real-time Statistics Panel (Nodes, depth, time, NPS, TT hit rate)
  - ✅ Board Heatmap (Evaluation visualization with multiple modes)
  - ✅ Game Replay System (Load/Save game records)
  - ✅ PGN/SGF Support (Standard game notation import/export)
  - ✅ Replay Analysis Window (Dedicated analysis interface)
  - ✅ Compilation: MinGW verified

#### v0.8.0 (February 2026) - Performance Benchmarking
- **Features**: Bitboard benchmark, AI search benchmark, Head-to-Head engine, Data export
- **Status**: 100% Complete
- **Code Lines**: ~1,400 new lines
- **Key Features**:
  - ✅ Bitboard Benchmark (Flip/Move/Legal/Copy/Hash performance)
  - ✅ AI Search Benchmark (Minimax/MCTS throughput)
  - ✅ Head-to-Head Engine (Win rate verification with statistics)
  - ✅ Data Exporter (JSON/CSV/Markdown multi-format)
  - ✅ Position Suite (64 standard test positions)
  - ✅ Statistics Tools (Wilcoxon test, confidence intervals)
  - ✅ Compilation: MinGW + MSVC verified

#### v0.7.0 (February 2026) - AI Optimization
- **Features**: Killer Moves, History Heuristic, Move Orderer, Search Parameters
- **Status**: 100% Complete
- **Code Lines**: ~13,100 new lines
- **Key Features**:
  - ✅ Killer Moves (2 killers per depth, 64 depths)
  - ✅ History Heuristic (64x64 matrix, depth-weighted scoring)
  - ✅ Move Orderer (PV > Killer > History > Static priority)
  - ✅ Search Parameters (configurable midgame/endgame weights)
  - ✅ MinimaxAI Integration (Killer/History recording)
  - ✅ Compilation: MinGW + MSVC verified

#### v0.6.0 (February 2026) - Research Framework
- **Features**: Zobrist hashing, Transposition table, Position suite, Battle engine, Statistics
- **Status**: 100% Complete
- **Code Lines**: ~2,000 new lines
- **Key Features**:
  - ✅ Zobrist Hashing (64-bit position encoding, 2MB memory, 33M entries)
  - ✅ Transposition Table (112MB cache, 4.2M entries, search optimization)
  - ✅ Position Suite (64 standard test positions for reproducible experiments)
  - ✅ Battle Engine (Head-to-head AI battle system with win rate statistics)
  - ✅ Statistics Tools (Wilcoxon test, confidence intervals, statistical significance)
  - ✅ AI vs AI Window (Visual interface for automated benchmark testing)
  - ✅ MinimaxAI Integration (Transposition table search optimization)

#### v0.5.0 (February 2026)
- **Features**: LAN network multiplayer, TCP/UDP communication, Room system
- **Status**: 100% Complete
- **Git Commit**: `Network module implemented UDP discovery, room management`
 with TCP client,- **Code Lines**: ~5,000 new lines
- **Key Features**:
  - ✅ TCP Communication Framework
  - ✅ LAN Network Discovery (UDP Broadcast)
  - ✅ Game State Synchronization (Zobrist Hashing)
  - ✅ Room System (Create/Join/Leave)
  - ✅ Heartbeat Mechanism (30s keep-alive)
  - ✅ Reconnection Handling (Exponential backoff)
  - ✅ Network Lobby UI
  - ✅ Network Game Window with Chat
  - ✅ Latency Monitoring

#### v0.4.0 (January 2026)
- **Features**: Menu interface, PvE mode, PvP mode
- **Status**: 100% Complete
- **Git Commit**: `259d367` - feat: v0.4.0 - 实现本地多人对战功能

#### v0.3.0 (January 2026)
- **Features**: AIStrategy pattern, MinimaxAI, MCTSAI, RandomAI
- **Status**: 100% Complete

#### v0.2.0 (January 2026)
- **Features**: BitBoard system, Board logic, Move validation
- **Status**: 100% Complete

#### v0.1.0 (January 2026)
- **Features**: CMake build system, project structure, compilation verification
- **Status**: 100% Complete

### 📅 Planned Versions

#### v1.0.0 (March 2026) - Final Release
- Complete feature set
- Performance optimization
- Final documentation
- Academic presentation materials
- **Status**: ✅ Completed

---

## 🎓 Academic Background

This project is part of the **COMP390 Honours Year Project** at the University of Liverpool, demonstrating:

- **Systematic Open-Source Learning**: How to study and integrate knowledge from mature projects
- **Academic Implementation**: Proper citation and intellectual property management
- **Research Methodology**: Combining multiple approaches into a cohesive research platform
- **Engineering Excellence**: Modern C++ practices in academic software development

### Learning Sources

This project learns from and builds upon excellent open-source implementations:

| Project | Reference | Contribution |
|---------|-----------|--------------|
| **[Egaroucid](https://github.com/Nyanyan/Egaroucid)** | ⭐⭐⭐⭐⭐ | Primary technical reference (world-class Othello AI) - GGS protocol, GTP commands, network architecture |
| **[edax-reversi](https://github.com/abulmo/edax-reversi)** | ⭐⭐⭐⭐ | Classic C implementation guidance - XBoard protocol |
| **[Reversi(Java)](https://github.com/abulmo/Reversi)** | ⭐⭐⭐ | Clean object-oriented architecture |
| **[MCTS-AI-Reversi](https://github.com/whatlulumomo/MCTS-AI-Reversi)** | ⭐⭐⭐ | Initial framework and Qt integration |
| **[QtReversi](OtherProjects/QtReversi)** | ⭐⭐⭐ | Qt UI patterns and widget layout |

---

## 🧪 Testing

### Current Test Coverage
- **Unit Tests**: Network message serialization, TCP communication
- **Integration Tests**: AI vs AI battles, Network discovery
- **Manual Testing**: UI functionality verification, Network gameplay
- **GUI Tests**: Menu navigation, Lobby operations, Gameplay

### Test Results (v0.5.0)
- ✅ TCP connection: Working correctly
- ✅ UDP discovery: Hosts detected reliably
- ✅ Room creation/joining: All operations functional
- ✅ Game sync: Real-time board synchronization
- ✅ Reconnection: Auto-retry with exponential backoff
- ✅ Chat: Message sending/receiving
- ✅ Latency: Accurate ping measurement

---

## 📊 Version Statistics

| Version | Features | Code Lines | Files |
|---------|----------|------------|-------|
| v0.1.0 | Build system | ~800 | 20 |
| v0.2.0 | Core logic | ~2,500 | 30 |
| v0.3.0 | AI algorithms | ~4,000 | 40 |
| v0.4.0 | UI system | ~5,500 | 45 |
| v0.5.0 | Network features | ~10,500 | 69 |
| v0.6.0 | Research framework | ~12,500 | 80 |
| v0.7.0 | AI optimization | ~13,100 | 86 |
| v0.8.0 | Performance benchmark | ~14,500 | 92 |
| v0.9.0 | Visualization enhancement | ~16,300 | 100 |

---

## 🤝 Contributing

This is an academic project, but contributions and feedback are welcome:

1. **Fork** the repository
2. **Create** a feature branch
3. **Implement** your changes with proper documentation
4. **Add tests** for new functionality
5. **Submit** a pull request

### Academic Collaboration
- **Code Review**: All contributions undergo academic peer review
- **Citation Requirements**: New features must include proper attribution
- **Testing Standards**: All code must pass the established test suite
- **Documentation**: New features require complete technical documentation

---

## 📧 Contact & Academic Supervision

| Field | Information |
|-------|-------------|
| **Student** | Tianqixing (201821852) |
| **Institution** | University of Liverpool, Department of Computer Science |
| **Course** | COMP390 Honours Year Project |
| **Supervisor** | [To be assigned by department] |
| **Ethical Approval** | Reference #12779 (Low Risk, Category B/Participant 2) |

---

## 🙏 Acknowledgments

This project stands on the shoulders of excellent open-source projects and academic research in the field of game AI and Othello algorithms.

### Special Thanks
- **Egaroucid Project**: For providing world-class Othello AI implementation and network protocol reference
- **edax-reversi Team**: For the classic C implementation that inspired many
- **University of Liverpool**: For the academic environment and support
- **Open-Source Community**: For making knowledge freely available

---

## 📄 License

This project is released under the MIT License. See [LICENSE](LICENSE) for details.

### Academic Licensing Notes
- **Open-Source Respect**: All referenced projects' licenses are respected
- **Academic Freedom**: Research and educational use is encouraged
- **Commercial Use**: Check individual component licenses for commercial applications

---

## 🎓 Educational Impact

This project serves as an excellent example for:

- **Computer Science Students**: Learning modern C++ and algorithm implementation
- **AI Researchers**: Understanding Othello as a research domain
- **Open-Source Contributors**: Studying proper attribution and academic integrity
- **Academic Projects**: Template for university-level software engineering

### Learning Outcomes Demonstrated
- **Systematic Research**: How to approach complex software projects academically
- **Open-Source Integration**: Proper methods for learning from and building upon existing work
- **Engineering Excellence**: Modern software development practices in academic context
- **Documentation Standards**: Professional documentation and project management

---

|**⭐ Star this repository to support academic open-source development!**  
|**🎓 University of Liverpool COMP390 Honours Year Project**  
|**Current Version**: v1.0.0 | **Final Release Completed**
