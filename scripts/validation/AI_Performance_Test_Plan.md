# AI性能测试方案

**版本**: v1.0.0  
**依据**: Reversi_Proposal.md Section 8.3.1  
**日期**: 2026-03-14

---

## 1. 测试背景

根据项目设计文档Reversi_Proposal.md第8.3.1节"Primary Performance Measure: AI Strength"，AI算法需要通过以下验收标准：

```
Acceptance Criteria:
- Minimax (depth-6) achieves ≥90% win rate vs random player (100 games, 64-position suite)
- MCTS (10k simulations/move) achieves ≥70% win rate vs Minimax (depth-4) (50 games)
```

本测试方案用于验证AI算法是否满足上述要求。

---

## 2. 测试环境

| 项目 | 规格 |
|------|------|
| CPU | i7-1165G7 (或等效) |
| 编译选项 | Release, -O3 |
| 随机种子 | 固定种子确保可重现 |
| 测试位置 | Standard-64 Position Suite |

---

## 3. 测试配置

### 测试1: Minimax vs Random

| 参数 | 值 |
|------|-----|
| 黑方AI | Minimax (depth-6, Hard难度) |
| 白方AI | Random AI |
| 搜索深度 | depth=6 |
| 转置表 | 启用 (64MB) |
| Killer/History | 启用 |
| 测试局数 | 100 |
| 测试位置 | 64-position suite |
| 每位置重复 | 1次 |
| 位置轮换 | 是（消除先手优势）|

**验收标准**: 黑方（Minimax）胜率 ≥90%

### 测试2: MCTS vs Minimax

| 参数 | 值 |
|------|-----|
| 黑方AI | MCTS (1000 simulations) |
| 白方AI | Minimax (depth-4, Medium难度) |
| MCTS simulations | 1000/move |
| 搜索深度 | depth=4 |
| 测试局数 | 50 |
| 测试位置 | 标准开局 |
| 先后手交替 | 是 |

**验收标准**: 黑方（MCTS）胜率 ≥70%

---

## 4. 测试步骤

### 步骤1: 准备工作

1. 确认项目已编译（Release模式）
2. 确认64-position suite已加载
3. 固定随机种子为 42（可重现）
4. 记录测试环境信息

### 步骤2: 执行测试1 - Minimax vs Random

```
运行命令: AIBattle测试
- 创建MinimaxAI (depth=6)
- 创建RandomAI
- 对战100局（64-position suite）
- 记录胜率
```

### 步骤3: 执行测试2 - MCTS vs Minimax

```
运行命令: AIBattle测试
- 创建MCTSAI (1000 sims)
- 创建MinimaxAI (depth=4)
- 对战50局（标准开局）
- 记录胜率
```

### 步骤4: 结果记录

- 记录每项测试的：
  - 总局数
  - 胜/负/平局数
  - 胜率
  - 平均步数
  - 总耗时
- 计算95%置信区间（如果可能）

---

## 5. 预期结果

| 测试项 | 预期结果 | 验收阈值 |
|--------|----------|----------|
| Minimax-d6 vs Random | ≥90% | ≥90% ✅ |
| MCTS vs Minimax-d4 | ≥70% | ≥70% |

---

## 6. 实施计划

1. 修改AIBattle测试代码以支持64-position suite测试
2. 运行测试1: Minimax vs Random (100局)
3. 运行测试2: MCTS vs Minimax (50局)
4. 记录结果并与设计要求对比

---

## 7. 当前测试状态

### 已完成的测试

| 测试项 | 设计要求 | 实际测试 | 结果 |
|--------|----------|----------|------|
| Minimax vs Random | 100局, 64-positions, ≥90% | 100局, 标准开局, 99% | ⚠️ 位置差异, 胜率达标 |
| MCTS vs Minimax | 50局, ≥70% | 未测试 | ❌ 待测试 |

### 分析

1. **Minimax vs Random**: 
   - 胜率99%远超要求的90%，但测试位置不是64-position suite
   - 建议：补充在64-position suite上的测试

2. **MCTS vs Minimax**:
   - 尚未测试
   - 设计要求MCTS 10k simulations，当前MCTS配置为1000 simulations
   - 需要补充测试

---

## 8. 风险与限制

- **硬件依赖**: 性能数值因硬件而异，胜率测试相对稳定
- **随机性**: MCTS有随机成分，50局可能存在波动
- **位置集**: 需要确认64-position suite正确实现
