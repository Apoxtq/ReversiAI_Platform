# ReversiAI_Platform

**Reversi AI Algorithm Benchmarking and Research Platform**

基于MCTS-AI-Reversi项目构建的现代化黑白棋AI研究平台。

---

## 🎓 学术项目声明

**重要声明**: 本项目为 **University of Liverpool COMP390 Honours Year Project**。

- **学生**: Tianqixing (201821852)
- **学位**: BSc Computer Science
- **学年**: 2025–2026
- **项目类型**: 学术研究 + 软件工程

### 📜 学术诚信声明
- **原创性**: 本项目的所有原创代码和设计为学生独立完成
- **开源参考**: 开源代码参考已明确标注和引用，遵守相应许可证
- **学术道德**: 严格遵循英国高等教育学术诚信准则
- **知识产权**: 尊重所有参考项目的知识产权和贡献者权益

### 📊 研究合规
- **伦理批准**: 已通过大学伦理审查程序 (Ref #12779)
- **BCS标准**: 符合英国计算机学会本科项目评估准则
- **研究方法**: 采用系统性的科学研究方法和实验设计
- **数据透明**: 所有实验数据和结果可重现和验证

## 项目结构

```
ReversiAI_Platform/
├── CMakeLists.txt          # 现代CMake构建系统
├── ReversiAI_Platform.pro  # Qt Creator项目文件
├── main.cpp               # 程序入口
├── .project-config/       # 项目开发配置 (规则和钩子)
├── include/               # 头文件
│   └── mainwindow.h
├── ui/                    # 用户界面模块
│   └── mainwindow.cpp
├── ai/                    # AI算法模块
│   └── MCTS.cpp
├── core/                  # 核心游戏逻辑 (待开发)
├── network/               # 网络功能 (待开发)
├── research/              # 研究工具 (待开发)
├── tests/                 # 测试用例 (待开发)
├── docs/                  # 文档 (待开发)
├── scripts/               # 构建脚本 (待开发)
└── exe/                   # 可执行文件和依赖
```

## 开发路线图

### Phase 1: 核心稳定化 (1-2周)
- [x] 基于MCTS-AI-Reversi创建项目
- [x] 建立现代目录结构
- [x] 配置CMake和Qt构建
- [ ] 测试编译和运行
- [ ] 代码清理和重构

### Phase 2: 功能扩展 (2-3周)
- [ ] 实现位棋盘 (bitboard)
- [ ] 添加Minimax算法
- [ ] 实现难度等级系统
- [ ] 添加游戏统计和指标显示

### Phase 3: 研究功能 (2-3周)
- [ ] 集成基准测试框架
- [ ] 实现算法对比功能
- [ ] 添加网络多人对战
- [ ] 完善实验数据收集

### Phase 4: 优化完善 (1-2周)
- [ ] 性能优化
- [ ] UI/UX改进
- [ ] 文档编写
- [ ] 项目演示准备

## 技术栈

- **语言**: C++17/20
- **界面**: Qt6/Qt5 (备选ImGui)
- **构建**: CMake
- **算法**: MCTS, Minimax, Alpha-Beta剪枝
- **架构**: 模块化设计，易于扩展

## 依赖项目

本项目基于以下开源项目构建：

- **MCTS-AI-Reversi**: 核心MCTS实现和Qt界面
- **Egaroucid**: 专业引擎和测试套件参考
- **Reversi(Java)**: 复杂评估函数设计
- **QtReversi**: 界面设计灵感

## 快速开始

### 使用Qt Creator
1. 打开 `ReversiAI_Platform.pro`
2. 构建并运行项目

### 使用CMake
```bash
mkdir build
cd build
cmake ..
make
./ReversiAI_Platform
```

## 项目目标

按照`项目设计/Reversi_Proposal.md`的要求，实现：

- ✅ Minimax/Negamax和MCTS AI策略
- 🚧 位棋盘表示
- 🚧 本地双人对战
- 🚧 LAN网络对战
- 🚧 基准测试框架
- ✅ 基础UI界面

## 贡献指南

1. 遵循现有的代码风格
2. 为新功能添加相应的测试
3. 更新文档
4. 提交前进行代码审查

## 许可证

本项目基于多个开源项目构建，请参考各模块的许可证信息。
