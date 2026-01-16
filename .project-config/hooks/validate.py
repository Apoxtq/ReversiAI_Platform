#!/usr/bin/env python3
"""
项目代码质量验证钩子
在每次Cursor操作后自动运行，检查代码质量

学术项目声明: 本验证系统为University of Liverpool COMP390 Honours Year Project
设计目标: 确保代码质量符合BCS项目评估标准和学术诚信要求
"""

import os
import sys
import subprocess
import json
from pathlib import Path

def check_compilation():
    """检查项目是否能成功编译"""
    build_dir = Path("build")
    if not build_dir.exists():
        return {"error": "构建目录不存在，请先运行CMake配置"}

    try:
        result = subprocess.run(
            ["cmake", "--build", "build"],
            capture_output=True,
            text=True,
            timeout=300  # 5分钟超时
        )

        if result.returncode != 0:
            return {
                "error": "编译失败",
                "details": result.stderr,
                "stdout": result.stdout
            }
    except subprocess.TimeoutExpired:
        return {"error": "编译超时"}
    except FileNotFoundError:
        return {"error": "CMake未找到，请检查环境配置"}

    return None

def check_architecture_violations():
    """检查架构约束违反"""
    violations = []

    # 检查UI层是否直接访问核心数据层
    ui_files = list(Path("ui").glob("**/*.cpp")) + list(Path("ui").glob("**/*.h"))
    for ui_file in ui_files:
        try:
            with open(ui_file, 'r', encoding='utf-8') as f:
                content = f.read()
                if '#include "core/' in content or '#include "../core/' in content:
                    violations.append({
                        "type": "架构违反",
                        "file": str(ui_file),
                        "rule": "UI层不得直接访问核心数据层",
                        "suggestion": "通过业务逻辑层接口访问数据"
                    })
        except UnicodeDecodeError:
            continue  # 跳过二进制文件

    # 检查循环依赖
    # 这里可以添加更复杂的依赖分析

    return violations

def check_academic_integrity():
    """检查学术诚信要求"""
    issues = []

    # 检查所有C++源文件和头文件
    cpp_files = list(Path(".").glob("**/*.cpp")) + list(Path(".").glob("**/*.hpp")) + list(Path(".").glob("**/*.h"))

    for cpp_file in cpp_files:
        if "build" in str(cpp_file) or "exe" in str(cpp_file):
            continue  # 跳过构建目录

        try:
            with open(cpp_file, 'r', encoding='utf-8') as f:
                content = f.read()

                # 检查是否有学术引用注释
                if "/**" in content and "@file" in content:
                    # 检查是否包含参考来源信息
                    if "参考实现:" not in content and "Reference:" not in content and "Based on:" not in content:
                        issues.append({
                            "type": "学术诚信",
                            "file": str(cpp_file),
                            "issue": "缺少开源参考标注",
                            "suggestion": "在文件头部添加参考来源和许可证信息"
                        })

                    # 检查是否有修改说明
                    if "修改点:" not in content and "Changes:" not in content and "Modifications:" not in content:
                        issues.append({
                            "type": "学术诚信",
                            "file": str(cpp_file),
                            "issue": "缺少修改贡献说明",
                            "suggestion": "说明基于参考实现的创新和修改点"
                        })
                else:
                    # 新文件缺少文档
                    issues.append({
                        "type": "学术诚信",
                        "file": str(cpp_file),
                        "issue": "缺少学术文档",
                        "suggestion": "添加完整的Doxygen文档头，包含参考来源"
                    })

        except UnicodeDecodeError:
            continue

    return issues

def check_code_style():
    """检查代码风格"""
    issues = []

    # 检查头文件保护符
    for header_file in Path("include").glob("**/*.h"):
        try:
            with open(header_file, 'r', encoding='utf-8') as f:
                content = f.read()
                if "#pragma once" not in content and "#ifndef" not in content:
                    issues.append({
                        "type": "代码风格",
                        "file": str(header_file),
                        "issue": "缺少头文件保护符"
                    })
        except UnicodeDecodeError:
            continue

    return issues

def main():
    """主验证函数"""
    errors = []
    warnings = []

    # 1. 编译检查
    compile_error = check_compilation()
    if compile_error:
        errors.append(compile_error)

    # 2. 架构检查
    arch_violations = check_architecture_violations()
    if arch_violations:
        errors.extend(arch_violations)

    # 3. 学术诚信检查 (BCS项目标准)
    academic_issues = check_academic_integrity()
    if academic_issues:
        errors.extend(academic_issues)

    # 4. 代码风格检查
    style_issues = check_code_style()
    if style_issues:
        warnings.extend(style_issues)

    # 输出结果
    if errors:
        result = {
            "status": "failed",
            "errors": errors,
            "warnings": warnings,
            "message": f"发现 {len(errors)} 个错误，{len(warnings)} 个警告"
        }
        print(json.dumps(result, indent=2, ensure_ascii=False))
        sys.exit(1)
    else:
        result = {
            "status": "passed",
            "warnings": warnings,
            "message": "代码质量检查通过"
        }
        print(json.dumps(result, indent=2, ensure_ascii=False))

if __name__ == "__main__":
    main()
