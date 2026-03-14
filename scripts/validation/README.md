# ReversiAI Platform Validation Suite
# 验证套件 - 用于验证项目功能和性能指标

## 目录结构

```
validation/
├── README.md                   # 本说明文档
├── validate.ps1               # 快速验证脚本 (单元测试)
├── extended_validate.ps1      # 扩展验证脚本 (编译+单元测试+覆盖率检查)
├── run_benchmark.ps1         # 完整性能测试脚本
└── results/
    ├── validation_report.md          # 快速验证报告
    ├── extended_validation_report.md # 扩展验证报告
    └── raw_data/                     # 原始验证数据 (第一手证据)
        ├── build_status_YYYYMMDD_HHMMSS.txt
        ├── core_tests_YYYYMMDD_HHMMSS.txt
        ├── ai_tests_YYYYMMDD_HHMMSS.txt
        ├── research_tests_YYYYMMDD_HHMMSS.txt
        └── benchmark_full_output_YYYYMMDD_HHMMSS.txt
```

## 验证脚本说明

### 1. validate.ps1 - 快速验证
- 运行单元测试 (Core/AI/Research)
- 保存原始测试输出
- 生成简洁验证报告

### 2. extended_validate.ps1 - 扩展验证
- 编译状态检查
- 单元测试
- 代码覆盖率检查 (需要额外配置)
- 系统性能检查
- AI性能目标参考
- MSVC编译验证

### 3. run_benchmark.ps1 - 完整性能测试
- 完整AI性能基准测试
- 胜率验证
- 生成详细性能报告

## 项目计划要求的验证项

| 验证项 | 要求 | 状态 |
|--------|------|------|
| **单元测试通过率** | 100% | ✅ 已验证 |
| **MinGW编译** | 成功 | ✅ 已验证 |
| **MSVC编译** | 成功 | ⚠️ 需手动 |
| **测试覆盖率** | ≥90% | ⚠️ 需配置 |
| **Bitboard性能** | ≥100M/s | ⚠️ run_benchmark.ps1 |
| **Minimax吞吐量** | ≥2.0M nodes/s | ⚠️ run_benchmark.ps1 |
| **MCTS仿真率** | ≥200K sims/s | ⚠️ run_benchmark.ps1 |
| **Minimax-6 vs Random** | ≥90%胜率 | ⚠️ run_benchmark.ps1 |
| **MCTS vs Minimax-4** | ≥70%胜率 | ⚠️ run_benchmark.ps1 |
| **内存占用** | <100MB | ⚠️ 需手动 |
| **启动时间** | <2秒 | ⚠️ 需手动 |

## 快速开始

### 1. 快速验证 (单元测试)
```powershell
cd scripts/validation
.\validate.ps1
```

### 2. 扩展验证 (推荐)
```powershell
cd scripts/validation
.\extended_validate.ps1
```

### 3. 完整性能测试
```powershell
cd scripts/validation
.\run_benchmark.ps1
```

## 重要特性：保留原始数据

**每个验证测试的原始输出都会被完整保存**，包括：
- 编译状态检查结果
- 单元测试的完整控制台输出
- 性能基准测试的全部输出
- 时间戳记录

这些原始数据文件可以作为**第一手证据**，比总结报告更具说服力。

## 输出结果

验证结果保存在 `results/` 目录：

1. **validation_report.md** - 快速验证总结报告
2. **extended_validation_report.md** - 扩展验证总结报告
3. **raw_data/** - 原始验证数据
   - 每个测试的完整控制台输出
   - 时间戳标记
   - 退出代码记录

这些原始数据可作为学术/项目验证的**可信证据**。

---
*For University of Liverpool COMP390 Honours Year Project*
