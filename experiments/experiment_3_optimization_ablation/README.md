# 实验包 3: 优化效果对比实验 (Ablation Study)

## 1. 实验概述

| 属性 | 值 |
|------|-----|
| 实验 ID | exp_003_optimization_ablation |
| 实验日期 | 2026-04-09 |
| 提案参考 | Reversi_Proposal.md Section 8.3.1 |
| 目的 | 量化各优化技术对 Minimax 性能的贡献 |

## 2. 实验设计

### 2.1 消融实验组

| 组别 | 配置 | 说明 |
|------|------|------|
| A | Minimax-6 (Baseline) | 无优化版本 |
| B | Minimax-6 + Alpha-Beta | 添加剪枝 |
| C | Minimax-6 + Alpha-Beta + TT | 添加置换表 |
| D | Minimax-6 + Alpha-Beta + TT + Killer | 添加杀手走法 |
| E | Minimax-6 + Alpha-Beta + TT + Killer + History | 完整版本 |

### 2.2 对战设计

每个实验组与其他所有组对战，记录胜率和搜索效率。

## 3. 预期发现

消融实验将展示：
1. Alpha-Beta 剪枝的剪裁效率
2. 置换表的缓存命中率
3. Killer Moves 的剪裁加速
4. History Heuristic 的走法排序改善

详见: `../../项目计划（文档放置）/v1.0.5-MCTS_vs_Minimax测试结果分析报告.md`

## 4. 数据文件

| 文件 | 描述 |
|------|------|
| `results/raw_data.csv` | 每局详细结果 |
| `results/aggregated.csv` | 聚合统计 |
| `results/ablation_analysis.csv` | 消融分析 |
| `results/analysis/report.md` | 分析报告 |

---

**版本**: v1.0
**更新**: 2026-04-09
