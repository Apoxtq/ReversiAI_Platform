#pragma once

#include <string>
#include <vector>
#include <memory>
#include "core/BitBoard.h"
#include "core/GamePhase.h"

/**
 * @file PositionSuite.h
 * @brief 标准位置测试套件
 *
 * 提供可重复的测试位置，用于AI算法基准测试。
 *
 * 来源:
 * - WThor 64 Position Suite
 * - FFO (Foundation for Othello) positions
 * - Edax标准测试集
 * - Egaroucid benchmark positions
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @brief 测试位置结构
 */
struct TestPosition {
    std::string name;         ///< 位置名称
    BitBoard board;           ///< 棋盘状态
    PlayerColor player;       ///< 当前玩家
    std::vector<Move> answers; ///< 专家推荐着法 (可选)
    std::string source;       ///< 来源
    int difficulty;           ///< 难度级别 (1-10)
    std::string description;  ///< 描述

    TestPosition() : player(PlayerColor::Black), difficulty(5) {}

    /**
     * @brief 转换为字符串表示
     */
    std::string toString() const;
};

/**
 * @brief 标准位置测试套件
 *
 * 提供标准化的测试位置套件，支持:
 * - 64位置标准测试套件
 * - 开局/中局/残局分类
 * - 自定义测试位置加载
 */
class PositionSuite {
public:
    /**
     * @brief 获取标准64位置测试套件
     *
     * 包含64个精选的标准测试位置
     *
     * @return 测试位置向量
     */
    static std::vector<TestPosition> getStandard64();

    /**
     * @brief 获取开局位置套件
     *
     * 0-20回合的开局位置
     *
     * @return 开局测试位置向量
     */
    static std::vector<TestPosition> getOpening();

    /**
     * @brief 获取中局位置套件
     *
     * 21-40回合的中局位置
     *
     * @return 中局测试位置向量
     */
    static std::vector<TestPosition> getMidgame();

    /**
     * @brief 获取残局位置套件
     *
     * 41-60回合的残局位置
     *
     * @return 残局测试位置向量
     */
    static std::vector<TestPosition> getEndgame();

    /**
     * @brief 获取指定阶段的位置套件
     *
     * @param phase 游戏阶段
     * @return 测试位置向量
     */
    static std::vector<TestPosition> getByPhase(BenchmarkPhase phase);

    /**
     * @brief 获取所有位置套件
     *
     * @return 包含所有位置的综合套件
     */
    static std::vector<TestPosition> getAll();

    /**
     * @brief 从文件加载测试套件
     *
     * 支持简单格式:
     * name;player_bits;opponent_bits;player;color;source
     *
     * @param filepath 文件路径
     * @return 测试位置向量，加载失败返回空向量
     */
    static std::vector<TestPosition> loadFromFile(const std::string& filepath);

    /**
     * @brief 保存测试套件到文件
     *
     * @param positions 要保存的位置
     * @param filepath 文件路径
     * @return true 如果保存成功
     */
    static bool saveToFile(const std::vector<TestPosition>& positions,
                           const std::string& filepath);

    /**
     * @brief 验证测试位置的有效性
     *
     * @param position 要验证的位置
     * @return true 如果位置有效
     */
    static bool validate(const TestPosition& position);

    /**
     * @brief 获取套件统计信息
     *
     * @param positions 位置列表
     * @return 统计字符串
     */
    static std::string getStats(const std::vector<TestPosition>& positions);

    /**
     * @brief 创建随机测试位置
     *
     * 在指定回合数生成随机位置
     *
     * @param move_count 回合数
     * @param seed 随机种子
     * @return 随机测试位置
     */
    static TestPosition createRandom(int move_count, uint64_t seed = 0);

    /**
     * @brief 获取回合数
     */
    static int getMoveCount(const BitBoard& board);

    /**
     * @brief 确定游戏阶段
     */
    static BenchmarkPhase getBenchmarkPhase(const BitBoard& board);

private:
    // 预定义的测试位置数据
    // 这些位置来自标准测试集
    static const TestPosition STANDARD_64[64];

    /**
     * @brief 初始化标准位置
     */
    static void initializeStandardPositions();

    /**
     * @brief 解析位置字符串
     */
    static TestPosition parsePositionString(const std::string& str);
};

/**
 * @brief 位置套件加载器
 *
 * 支持从各种格式加载测试位置
 */
class PositionSuiteLoader {
public:
    /**
     * @brief 加载格式类型
     */
    enum class Format {
        AUTO,       ///< 自动检测
        SIMPLE,     ///< 简单格式 (name;bits;bits;color)
        EPD,        ///< EPD格式
        FEN,        ///< FEN格式
        BINARY      ///< 二进制格式
    };

    /**
     * @brief 从文件加载
     *
     * @param filepath 文件路径
     * @param format 格式 (AUTO自动检测)
     * @return 测试位置列表
     */
    static std::vector<TestPosition> load(const std::string& filepath,
                                           Format format = Format::AUTO);

    /**
     * @brief 检测文件格式
     */
    static Format detectFormat(const std::string& filepath);

private:
    static std::vector<TestPosition> loadSimple(const std::string& filepath);
    static std::vector<TestPosition> loadEPD(const std::string& filepath);
    static std::vector<TestPosition> loadFEN(const std::string& filepath);
};

} // namespace Reversi

