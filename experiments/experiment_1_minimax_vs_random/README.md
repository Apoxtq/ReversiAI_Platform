# 实验包 1: Minimax-6 vs Random AI 对战实验

## 1. 实验概述

| 属性 | 值 |
|------|-----|
| 实验 ID | exp_001_minimax6_vs_random |
| 实验日期 | 2026-04-09 |
| 提案参考 | Reversi_Proposal.md Section 8.3.1 |
| 假设 | Minimax-6 胜率 ≥90% |
| 游戏数 | 100 局 |

## 2. 实验配置

### 2.1 玩家配置

| 玩家 | AI 类型 | 参数 |
|------|---------|------|
| 白方 | MinimaxAI | depth=6, TT=ON, Killer=ON, History=ON |
| 黑方 | RandomAI | random_seed=12345 |

### 2.2 环境配置

| 参数 | 值 |
|------|-----|
| CPU | Intel Core i7-1165G7 |
| 操作系统 | Windows 10 |
| 编译器 | MSVC 2022 |
| 优化等级 | -O2 (Release) |
| 随机种子 | 42 |

## 3. 运行方法

```bash
# 进入项目构建目录
cd D:\Project\Reversi\ReversiAI_Platform\build_qt

# 运行基准测试（包含本实验）
Release\ReversiAI_Platform.exe --benchmark

# 或运行 AI 对战模式
Release\ReversiAI_Platform.exe --ai-battle Minimax6 Random 100
```

## 4. 预期结果

| 指标 | 目标 | 实际 |
|------|------|------|
| Minimax 胜率 | ≥90% | 100% |
| p 值 | <0.05 | 4.9e-12 |
| 95% CI | - | [96.3%, 100%] |

## 5. 数据文件

| 文件 | 描述 |
|------|------|
| `results/raw_data.csv` | 每局详细结果 |
| `results/aggregated.csv` | 聚合统计 |
| `results/statistics.csv` | 统计分析 |
| `results/analysis/report.md` | 分析报告 |

## 6. 复现说明

本实验可通过以下方式复现：

1. 克隆仓库并编译
2. 设置随机种子为 42
3. 运行 `ReversiAI_Platform.exe --benchmark`
4. 或使用 Python 脚本 `run_experiment.py`

---

**版本**: v1.0
**更新**: 2026-04-09
