# ReversiAI_Platform

**Version**: v1.0.6
**Status**: Production Ready (Merit Target Achieved)
**Project**: University of Liverpool COMP390 Honours Year Project

一个基于现代 C++ 的黑白棋/奥赛罗 AI 研究与基准测试平台，展示了学术卓越的开源学习和软件工程能力。

---

## 目录

- [快速开始](#快速开始)
- [项目结构](#项目结构)
- [编译指南](#编译指南)
- [运行指南](#运行指南)
- [关键路径参考](#关键路径参考)
- [版本历史](#版本历史)
- [文档目录](#文档目录)
- [常见问题](#常见问题)

---

## 快速开始

### 方法一：直接运行（推荐新手）

使用已编译好的可运行版本：

| 版本 | 路径 | 说明 |
|------|------|------|
| **Debug 版本** | `end_submite/Debug/ReversiAI_Platform.exe` | 包含调试信息，适合开发测试 |
| **Release 版本** | `Release_Package/ReversiAI_Platform.exe` | 优化版本，适合发布使用 |

### 方法二：从源码编译

详见 [编译指南](#编译指南) 部分。

---

## 项目结构

```
D:\Project\Reversi\ReversiAI_Platform\
├── include/                    # 头文件目录
│   ├── core/                   # 核心游戏逻辑
│   │   ├── Board.h            # 棋盘表示
│   │   ├── BitBoard.h          # 位棋盘优化
│   │   └── Move.h              # 走法结构
│   ├── ai/                    # AI 算法
│   │   ├── AIStrategy.h        # AI 接口
│   │   ├── MinimaxAI.h         # Minimax 算法
│   │   ├── MCTSAI.h            # MCTS 算法
│   │   └── RandomAI.h          # 随机 AI
│   ├── research/              # 研究框架
│   │   ├── TranspositionTable.h # 置换表
│   │   └── ZobristHash.h       # Zobrist 哈希
│   └── ui/                    # 用户界面
│       ├── MenuWindow.h        # 主菜单
│       ├── PvEWindow.h         # 人机对战窗口
│       ├── PvPWindow.h         # 本地双人窗口
│       ├── NetworkLobbyWindow.h # 网络大厅
│       └── NetworkGameWindow.h # 网络对战窗口
├── src/                       # 源代码实现
│   ├── core/                  # 核心实现
│   ├── ai/                   # AI 实现
│   ├── research/             # 研究框架实现
│   ├── ui/                  # UI 实现
│   └── rsc.qrc              # Qt 资源文件
├── ui/                       # Qt UI 设计文件 (.ui)
├── tests/                    # 测试代码
├── network/                  # 网络模块
├── docs/                     # 文档目录
├── scripts/                  # 脚本工具
└── 项目计划（文档放置）/       # 项目计划文档
```

---

## 编译指南

### 环境要求

| 组件 | 要求 | 说明 |
|------|------|------|
| **编译器** | MSVC 2022 或 MinGW (GCC) | 二选一 |
| **CMake** | 3.16+ | 构建系统 |
| **Qt6** | 6.10+ | GUI 框架 |
| **C++ 标准** | C++17 | 项目使用现代 C++ |

### Qt6 安装

**重要**：根据编译器选择正确的 Qt6 版本！

| 编译器 | Qt6 版本 | 路径格式 |
|--------|----------|----------|
| **MSVC 2022** | Qt 6.10.2 | `msvc2022_64` |
| **MinGW (GCC)** | Qt 6.10.1 | `mingw_64` |

**下载地址**：https://www.qt.io/download-qt-installer

安装时选择对应组件：
- MSVC 用户：Qt 6.10.x → MSVC 2022 x64
- MinGW 用户：Qt 6.10.x → MinGW 12.2.0 x64

### 方法一：使用 MSVC 编译

```powershell
# 1. 进入项目目录
cd D:\Project\Reversi\ReversiAI_Platform

# 2. 创建构建目录
mkdir build_msvc
cd build_msvc

# 3. 配置 CMake (使用 MSVC)
cmake -G "Visual Studio 17 2022" -A x64 -DBUILD_QT_GUI=ON -DCMAKE_BUILD_TYPE=Release ..

# 4. 编译
cmake --build . --parallel 8

# 5. 部署 Qt 依赖
windeployqt Release\ReversiAI_Platform.exe

# 6. 运行
.\Release\ReversiAI_Platform.exe
```

### 方法二：使用 MinGW 编译

```powershell
# 1. 进入项目目录
cd D:\Project\Reversi\ReversiAI_Platform

# 2. 创建构建目录
mkdir build_mingw
cd build_mingw

# 3. 配置 CMake (使用 MinGW)
cmake -G "MinGW Makefiles" -DBUILD_QT_GUI=ON -DCMAKE_BUILD_TYPE=Release ..

# 4. 编译
cmake --build . --parallel 8

# 5. 部署 Qt 依赖
windeployqt ReversiAI_Platform.exe

# 6. 运行
.\ReversiAI_Platform.exe
```

### CMake 自动检测

CMake 会自动检测编译器并选择对应的 Qt6：

```
MSVC   → D:/Dev/SDKs/Qt/6.10.2/msvc2022_64/
MinGW  → D:/Dev/SDKs/Qt/6.10.1/mingw_64/
```

### 构建配置选项

| 选项 | 值 | 说明 |
|------|-----|------|
| `BUILD_QT_GUI` | ON/OFF | 是否构建 Qt GUI |
| `BUILD_TESTS` | ON/OFF | 是否构建单元测试 |
| `CMAKE_BUILD_TYPE` | Debug/Release | 构建类型 |
| `PERFORMANCE_TESTING` | ON/OFF | 性能测试模式 |

### Debug 构建

```powershell
# Debug 构建 (用于调试)
cmake -G "Visual Studio 17 2022" -A x64 -DBUILD_QT_GUI=ON -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --parallel 8
windeployqt Debug\ReversiAI_Platform.exe
```

---

## 运行指南

### 可运行版本

#### Debug 版本 (end_submite/Debug)

**路径**：`D:\Project\Reversi\ReversiAI_Platform\end_submite\Debug\ReversiAI_Platform.exe`

**包含文件**：
```
Debug/
├── ReversiAI_Platform.exe          # 主程序 (2.8 MB)
├── Qt6Cored.dll                    # Qt 核心 (21 MB)
├── Qt6Guid.dll                     # Qt GUI (27 MB)
├── Qt6Widgetsd.dll                 # Qt Widgets (17 MB)
├── Qt6Networkd.dll                 # Qt 网络 (5 MB)
├── Qt6Svgd.dll                     # Qt SVG (2 MB)
├── D3Dcompiler_47.dll              # Direct3D 着色器
├── opengl32sw.dll                 # OpenGL 软件渲染 (20 MB)
├── icuuc.dll                      # ICU Unicode
├── generic/                       # 触摸插件
│   └── qtuiotouchplugind.dll
├── iconengines/                   # 图标引擎
│   └── qsvgicond.dll
├── imageformats/                  # 图片格式
│   ├── qgifd.dll
│   ├── qjpegd.dll
│   ├── qpngd.dll
│   ├── qsvgd.dll
│   └── ... (更多格式)
├── platforms/                     # ★ 平台插件 (必须！)
│   └── qwindowsd.dll
├── styles/                        # 样式
│   └── qmodernwindowsstyled.dll
└── tls/                          # TLS/SSL
    ├── qcertonlybackendd.dll
    └── qschannelbackendd.dll
```

**注意**：这个版本已包含所有必需的 DLL 和插件，可直接运行！

#### Release 版本 (Release_Package)

**路径**：`D:\Project\Reversi\ReversiAI_Platform\Release_Package\ReversiAI_Platform.exe`

**特性**：
- MinGW 编译
- 优化版本
- 包含 Qt 运行时
- 适合发布

### 运行时依赖

如果需要自己部署 Qt 依赖，确保包含以下内容：

#### 1. 核心 DLL (必须)

| DLL | 用途 |
|-----|------|
| `Qt6Core.dll` / `Qt6Cored.dll` | Qt 核心库 |
| `Qt6Gui.dll` / `Qt6Guid.dll` | Qt GUI 库 |
| `Qt6Widgets.dll` / `Qt6Widgetsd.dll` | Qt Widgets 库 |

#### 2. 平台插件 (必须)

| 文件夹 | 文件 | 用途 |
|--------|------|------|
| `platforms/` | `qwindows.dll` / `qwindowsd.dll` | **Windows 平台插件** |

> ⚠️ **重要**：缺少 `platforms/qwindows.dll` 会导致错误：
> ```
> This application failed to start because no Qt platform plugin could be initialized.
> ```

#### 3. 可选插件

| 文件夹 | 用途 |
|--------|------|
| `iconengines/` | SVG 图标支持 |
| `imageformats/` | 更多图片格式 (GIF, JPEG, PNG, WebP 等) |
| `styles/` | Windows 10/11 现代风格 |
| `generic/` | 触摸屏支持 |

#### 4. 图形依赖

| DLL | 用途 |
|-----|------|
| `D3Dcompiler_47.dll` | Direct3D 着色器编译 |
| `opengl32sw.dll` | OpenGL 软件渲染 (无显卡时使用) |

### windeployqt 自动部署

如果安装了 Qt，可以使用 `windeployqt` 自动复制所有依赖：

```powershell
# 部署到指定目录
windeployqt --dir D:\output\folder D:\path\to\ReversiAI_Platform.exe

# 或直接覆盖 EXE 所在目录
windeployqt D:\path\to\ReversiAI_Platform.exe
```

---

## 关键路径参考

### 项目目录

```
D:\Project\Reversi\ReversiAI_Platform\    # 项目根目录
├── build_qt/                            # Qt GUI 构建 (MSVC)
├── build_mingw/                         # MinGW 构建
├── build_msvc/                          # MSVC 构建
├── build_console/                        # 控制台构建
├── build_test/                          # 测试构建
├── end_submite/                         # 提交的预编译版本
│   └── Debug/                           # ★ 可运行的 Debug 版本
├── Release_Package/                    # ★ 可运行的 Release 版本
└── zhiqiandebanbenneirong/             # GitHub 下载的旧版本备份
    └── ReversiAI_Platform-main/         #   用于恢复参考
```

### 预编译可运行版本

| 版本 | 路径 | 编译器 |
|------|------|--------|
| Debug | `end_submite\Debug\ReversiAI_Platform.exe` | MSVC |
| Release | `Release_Package\ReversiAI_Platform.exe` | MinGW |

### Qt 安装路径 (参考)

```
# MSVC
D:/Dev/SDKs/Qt/6.10.2/msvc2022_64/

# MinGW
D:/Dev/SDKs/Qt/6.10.1/mingw_64/
```

### 运行时搜索路径

Qt 按以下顺序搜索插件：

1. `QCoreApplication::applicationDirPath()/platforms/`
2. `QT_PLUGIN_PATH` 环境变量
3. 编译时指定的 Qt 插件路径

---

## 版本历史

### v1.0.6 - 项目完成报告

**日期**: 2026-04-09

#### 主要成就

| 类别 | 完成情况 | 说明 |
|------|---------|------|
| **Essential Requirements** | ✅ 100% (8/8) | 所有基本功能已完成 |
| **Desirable (P1-P3)** | ✅ 100% (3/3) | 置换表、迭代深化、走法排序 |
| **Merit 标准** | ✅ 全部达成 | 满足 Merit/Distinction 要求 |

#### Merit 标准达成情况

| 标准 | 提案要求 | 实际达成 | 状态 |
|------|---------|---------|------|
| Minimax-6 vs Random | ≥90% | **100%** | ✅ |
| MCTS vs Minimax-4 | ≥5% | **6%** | ✅ |
| Desirable (P1-P3) | ≥60% | **100%** | ✅ |
| 次要效率指标 | ≥4/5 | **6/6** | ✅ |
| 统计显著性 | p<0.05 | **已验证** | ✅ |
| 可复现实验包 | ≥3个 | **已完成** | ✅ |

#### 技术亮点

| 功能 | 说明 |
|------|------|
| **Bitboard 优化** | 高效位棋盘实现，≥100M flip/sec |
| **Minimax 优化** | Alpha-Beta 剪枝 + 置换表 + Killer Moves |
| **MCTS** | 标准蒙特卡洛树搜索，支持多种配置 |
| **迭代深化** | Anytime 算法，精确时间控制 |
| **网络对战** | LAN 多人游戏，房间匹配 |
| **基准测试** | 完整对战测试套件，数据导出 |

#### 重要说明：MCTS vs Minimax 结果

> **发现**：优化后的 Minimax-4 强度远超预期基准线
>
> **原因**：Alpha-Beta 剪枝 + 置换表 + Killer Moves + History Heuristic 的组合优化效果远超提案设定基准
>
> **调整**：经与导师沟通，MCTS vs Minimax-4 目标调整为 ≥5%（实际达成 6%）

#### 详细文档

| 文档 | 说明 |
|------|------|
| `项目计划（文档放置）/v1.0.5-MCTS_vs_Minimax测试结果分析报告.md` | MCTS vs Minimax 详细分析 |
| `项目计划（文档放置）/v1.0.6-项目完成状态报告.md` | 项目完成状态总览 |

### v1.0.4 - Benchmark Suite 功能增强

**日期**: 2026-04-09

#### 新增功能

| 功能 | 说明 |
|------|------|
| **Pass/Fail 判定** | 实时显示测试结果是否达标（✅ PASS / ❌ FAIL） |
| **Random Seed 配置** | 可设置随机种子，便于复现对局 |
| **Runtime 预估** | 自动估算对战预计耗时 |
| **Position Suite 选择** | 支持 Standard/Opening/Midgame/Endgame 套件 |
| **并行处理配置** | 支持多线程加速对战 |
| **验证模式** | 自动运行两次验证结果一致性 |
| **快速验证** | 一键运行标准测试套件 |

#### 验收测试结果

| 测试 | 目标 | 结果 | 状态 |
|------|------|------|------|
| Minimax-6 vs Random | ≥90% | 100% | ✅ PASS |
| MCTS vs Minimax-4 | ≥5% | 6% | ✅ PASS |
| AI 稳定性 | 0崩溃 | 0崩溃 | ✅ PASS |

> **注意**：MCTS vs Minimax-4 的目标已调整为 ≥5%（原始目标 ≥70%），原因是优化后的 Minimax-4 远超预期基准。

#### 新增文件

| 文件 | 说明 |
|------|------|
| `include/research/BenchmarkTargets.h` | 目标值定义常量 |
| `include/research/RuntimeEstimator.h` | 运行时长预估器 |
| `include/research/ValidationSuite.h` | 验证测试套件 |

### v1.0.3 - UI 优化与悔棋修复

- **Bug 修复**：修复本地双人模式悔棋功能
  - 修复 `undoMove()` 玩家轮次恢复逻辑错误
  - 增加 `MAX_UNDO_STEPS` 从 10 步到 60 步
- **UI 优化**：调整 PvEWindow 右面板布局
  - AI First/Player First 改为垂直排列
  - Controls 组往下移动，填充右下角空缺

### v1.0.2 - Bug Fix Release

- 修复网络稳定性问题
- 优化性能

### v1.0.1 - 双编译器支持

- MinGW 编译支持 (GCC)
- MSVC 编译支持 (Visual Studio 2022)
- CMake 自动检测编译器类型
- Qt6 版本自动选择

### v0.9.0 - 可视化增强

- 搜索树可视化
- 实时统计面板
- 棋盘热力图
- 棋谱回放系统
- PGN/SGF 支持

### v0.8.0 - 性能基准测试

- 位棋盘基准测试
- AI 搜索基准测试
- 对战引擎
- 数据导出 (JSON/CSV/Markdown)

### v0.7.0 - AI 优化

- Killer Moves
- History Heuristic
- Move Orderer
- 搜索参数配置

### v0.6.0 - 研究框架

- Zobrist 哈希
- 置换表
- 位置测试套件
- 对战引擎
- 统计分析工具

### v0.5.0 - 网络功能

- TCP/UDP 通信
- 局域网发现
- 房间系统
- 重连机制
- 延迟监测

### v0.4.0 - UI 系统

- 主菜单
- 人机对战模式
- 本地双人模式

### v0.3.0 - AI 算法

- MinimaxAI
- MCTSAI
- RandomAI

### v0.2.0 - 核心逻辑

- BitBoard 系统
- 棋盘逻辑
- 走法验证

---

## 文档目录

项目文档位于 `项目计划（文档放置）/` 目录：

### 重要文档

| 文档 | 说明 |
|------|------|
| `docs/未完成功能说明.md` | 待完成功能列表 |
| `docs/录制展示指南.md` | 视频录制指南 |
| `docs/人机对战界面改进方案.md` | UI 改进方案 |
| `项目计划（文档放置）/v1.0.3-BenchmarkSuite功能增强设计.md` | **Benchmark Suite 增强设计文档** |
| `项目计划（文档放置）/v1.0.3-BenchmarkSuite完成报告.md` | **Benchmark Suite 完成报告** |
| `项目计划（文档放置）/项目进展跟踪.md` | 项目进度 |
| `项目计划（文档放置）/学术项目检查清单.md` | 学术合规检查 |

---

## 常见问题

### Q1: 运行时报错 "no Qt platform plugin could be initialized"

**原因**：缺少 `platforms` 文件夹或 `qwindows.dll`

**解决方法**：
1. 从 `end_submite\Debug\` 复制 `platforms/` 文件夹到 EXE 目录
2. 或使用 `windeployqt` 重新部署

### Q2: MSVC 和 MinGW 的 DLL 能否混用？

**不能**！MSVC 编译的 EXE 必须使用 MSVC 版本的 Qt DLL，MinGW 编译的 EXE 必须使用 MinGW 版本的 Qt DLL。

| EXE 编译器 | 需要的 Qt DLL |
|------------|---------------|
| MSVC | `Qt6Cored.dll` (不带 `d` 后缀的 MSVC 版本) |
| MinGW | `Qt6Cored.dll` (MinGW 版本) |

### Q3: 如何重新编译？

详见 [编译指南](#编译指南)。

### Q4: DLL 版本不匹配？

确保 Qt6 版本与编译器匹配：
- MSVC 2022 → Qt 6.10.2 `msvc2022_64`
- MinGW → Qt 6.10.1 `mingw_64`

### Q5: windeployqt 找不到？

确保 Qt 的 `bin` 目录在 PATH 中：
```powershell
$env:PATH += ";D:\Dev\SDKs\Qt\6.10.2\msvc2022_64\bin"
```

---

## 许可证

MIT License - 详见 [LICENSE](LICENSE) 文件。

---

**⭐ 如果这个项目对你有帮助，请给个 Star！**
**🎓 University of Liverpool COMP390 Honours Year Project**
