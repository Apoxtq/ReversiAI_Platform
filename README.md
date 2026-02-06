# ReversiAI_Platform 🎓

|**Version**: v0.4.0  
|**Status**: ✅ 本地多人对战功能已完成  
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
- **MenuWindow**: Game mode selection interface (PvE/PvP)
- **PvEWindow**: Human vs AI gameplay with difficulty selection
- **PvPWindow**: Local two-player mode with undo functionality
- **Real-time Display**: Board visualization and move highlighting
- **Statistics**: Game history and win rate tracking

### 📊 Research Framework (v0.5.0 📅 Planned)
- **Network Multiplayer**: LAN-based online gameplay
- **Benchmarking Suite**: Comprehensive AI algorithm comparison
- **Statistical Analysis**: Performance metrics and significance testing

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
└── PvP Button (本地双人) → PvP Window
    ├── Two-player local gameplay
    ├── Undo functionality
    └── Back to menu option
```

---

## 📚 Documentation

All documentation is available in the `项目计划（文档放置）/` directory:

### Project Tracking
- **[项目进展跟踪.md](项目计划（文档放置）/项目进展跟踪.md)** - Project progress and milestone tracking
- **[ReversiAI_Platform_项目计划.md](项目计划（文档放置）/ReversiAI_Platform_项目计划.md)** - Complete project roadmap

### Version Documentation
- **[v0.4.0-完成报告.md](项目计划（文档放置）/v0.4.0-完成报告.md)** - v0.4.0 completion report
- **[v0.5.0-设计规划.md](项目计划（文档放置）/v0.5.0-设计规划.md)** - v0.5.0 network features design

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
│       ├── GameController.h # Game state management
│       └── StatisticsManager.h # Game statistics
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

#### v0.4.0 (Current - January 2026)
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

#### v0.5.0 (March 2026) - Network Features
| Priority | Feature | Status |
|----------|---------|--------|
| P0 | Message serialization | 📅 Planned |
| P0 | TCP communication | 📅 Planned |
| P0 | Connection state management | 📅 Planned |
| P1 | LAN network discovery | 📅 Planned |
| P1 | Game state synchronization | 📅 Planned |
| P1 | Heartbeat mechanism | 📅 Planned |
| P2 | Room system | 📅 Planned |
| P2 | Reconnection handling | 📅 Planned |

#### v0.6.0 (March 2026) - Research Framework
- AI benchmarking suite
- Statistical analysis tools
- Experiment management

#### v1.0.0 (March 2026) - Final Release
- Complete feature set
- Performance optimization
- Final documentation

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
| **[Egaroucid](https://github.com/Nyanyan/Egaroucid)** | ⭐⭐⭐⭐⭐ | Primary technical reference (world-class Othello AI) |
| **[edax-reversi](https://github.com/abulmo/edax-reversi)** | ⭐⭐⭐⭐ | Classic C implementation guidance |
| **[Reversi(Java)](https://github.com/abulmo/Reversi)** | ⭐⭐⭐ | Clean object-oriented architecture |
| **[MCTS-AI-Reversi](https://github.com/whatlulumomo/MCTS-AI-Reversi)** | ⭐⭐⭐ | Initial framework and Qt integration |

---

## 🧪 Testing

### Current Test Coverage
- **Unit Tests**: Basic game logic tests
- **Integration Tests**: AI vs AI battles
- **Manual Testing**: UI functionality verification

### Test Results (v0.4.0)
- ✅ PvE mode: Working correctly
- ✅ PvP mode: Both players can move
- ✅ Menu navigation: Smooth transitions
- ✅ Undo functionality: Working

---

## 📊 Version Statistics

| Version | Features | Code Lines | Files |
|---------|----------|------------|-------|
| v0.1.0 | Build system | ~800 | 20 |
| v0.2.0 | Core logic | ~2,500 | 30 |
| v0.3.0 | AI algorithms | ~4,000 | 40 |
| v0.4.0 | UI system | ~5,500 | 45 |

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
- **Egaroucid Project**: For providing world-class Othello AI implementation
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
|**Current Version**: v0.4.0 | **Next Version**: v0.5.0 (Network Features)
