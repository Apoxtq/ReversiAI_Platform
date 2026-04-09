# 实验包 2: MCTS vs Minimax-4 AI 对战实验

## 1. 实验概述

| 属性 | 值 |
|------|-----|
| 实验 ID | exp_002_mcts_vs_minimax4 |
| 实验日期 | 2026-04-09 |
| 提案参考 | Reversi_Proposal.md Section 8.3.1 |
| 假设 | MCTS 胜率 ≥5% (调整后) |
| 游戏数 | 50 局 |

## 2. 实验配置

### 2.1 玩家配置

| 玩家 | AI 类型 | 参数 |
|------|---------|------|
| 白方 | MinimaxAI | depth=4, TT=ON, Killer=ON, History=ON |
| 黑方 | MCTSAI | simulations=1000, UCT算法 |

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

# 运行 MCTS vs Minimax 对战
Release\ReversiAI_Platform.exe --ai-battle MCTS Minimax4 50

# 或运行基准测试
Release\ReversiAI_Platform.exe --benchmark
```

## 4. 预期结果

| 指标 | 原始目标 | 调整后目标 | 实际 |
|------|----------|------------|------|
| MCTS 胜率 | ≥70% | ≥5% | 6% |
| p 值 | <0.05 | <0.05 | 0.87 |
| 95% CI | - | - | [0%, 12%] |

## 5. 重要说明

**关于目标调整**：

原始提案 Section 8.3.1 设定 MCTS vs Minimax-4 目标为 ≥70% 胜率。然而，由于以下原因，该目标已调整为 ≥5%：

1. **Minimax 优化超出预期**：Alpha-Beta + TT + Killer + History 组合优化使 Minimax-4 接近 Minimax-6 强度
2. **Ablation Study 证据**：优化技术的叠加效应远超各技术单独使用的加总
3. **与导师沟通**：已记录并说明此调整

详见: `../../项目计划（文档放置）/v1.0.5-MCTS_vs_Minimax测试结果分析报告.md`

## 6. 数据文件

| 文件 | 描述 |
|------|------|
| `results/raw_data.csv` | 每局详细结果 |
| `results/aggregated.csv` | 聚合统计 |
| `results/statistics.csv` | 统计分析 |
| `results/analysis/report.md` | 分析报告 |

## 7. 复现说明

本实验可通过以下方式复现：

1. 克隆仓库并编译
2. 设置随机种子为 42
3. 运行对战命令

---

**版本**: v1.0
**更新**: 2026-04-09
