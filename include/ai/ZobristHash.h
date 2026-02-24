#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <random>

/**
 * @file ZobristHash.h
 * @brief Zobrist哈希生成器
 *
 * 使用Egaroucid风格的Zobrist哈希方案：
 * - 4个player hash数组 (按位置和棋子数)
 * - 支持64x4位置编码
 * - 高质量随机数生成
 *
 * 参考: Egaroucid/src/engine/hash.hpp
 *       edax-reversi/src/hash.h
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @brief Zobrist哈希生成器类
 *
 * 为棋盘状态生成唯一哈希值，用于：
 * - 转置表 (Transposition Table)
 * - 网络同步验证
 * - 位置缓存
 *
 * 实现特点:
 * - 使用4个数组，每个65536个条目 (Egaroucid风格)
 * - 每个条目32位哈希值
 * - 可配置的哈希精度级别
 * - 支持从文件加载预计算的哈希值
 */
class ZobristHash {
public:
    /**
     * @brief 哈希精度级别常量
     *
     * level决定哈希表的条目数量: 2^level
     * 25 = 33,554,432 条目 (约32MB for 32-bit hash)
     */
    static constexpr int DEFAULT_HASH_LEVEL = 25;
    static constexpr int MIN_HASH_LEVEL = 20;
    static constexpr int MAX_HASH_LEVEL = 30;

    /**
     * @brief 哈希数组大小
     */
    static constexpr int HASH_RAND_SIZE = 65536;
    static constexpr int N_PLAYER_HASH = 4;

    /**
     * @brief 初始化哈希表
     *
     * 使用高质量随机数生成器初始化哈希数组
     *
     * @param hash_level 哈希精度级别 (default: 25)
     * @param seed 随机种子 (default: 基于硬件)
     */
    static void init(int hash_level = DEFAULT_HASH_LEVEL, uint64_t seed = 0);

    /**
     * @brief 检查是否已初始化
     * @return true 如果已初始化
     */
    static bool isInitialized();

    /**
     * @brief 生成棋盘哈希值 (完整哈希)
     *
     * 计算包含双方棋子的完整棋盘哈希
     *
     * @param player_bits 黑棋位图
     * @param opponent_bits 白棋位图
     * @return 32位哈希值
     *
     * @complexity O(1) - 位运算优化
     */
    static uint32_t computeHash(uint64_t player_bits, uint64_t opponent_bits);

    /**
     * @brief 生成玩家哈希值
     *
     * 仅计算当前玩家棋子的哈希
     * 用于快速位置识别
     *
     * @param player_bits 当前玩家的位图
     * @param player_index 玩家索引 (0=黑, 1=白, 基于棋子数)
     * @return 32位哈希值
     */
    static uint32_t computePlayerHash(uint64_t player_bits, int player_index);

    /**
     * @brief 获取单格哈希值
     *
     * @param square 格子索引 (0-63)
     * @param is_player true=玩家棋子, false=对手棋子
     * @return 哈希值
     */
    static uint32_t getSquareHash(int square, bool is_player);

    /**
     * @brief 获取玩家索引
     *
     * 根据棋子数量确定玩家在哈希数组中的索引
     *
     * @param stone_count 棋子数量
     * @return 索引 (0-3)
     */
    static int getPlayerIndex(int stone_count);

    /**
     * @brief 获取当前哈希级别
     * @return 哈希级别
     */
    static int getHashLevel();

    /**
     * @brief 关闭并清理内存
     */
    static void shutdown();

    /**
     * @brief 哈希表大小 (条目数)
     */
    static size_t getHashSize();

    /**
     * @brief 哈希表大小 (字节)
     */
    static size_t getHashMemory();

private:
    // 静态成员变量
    static std::array<std::array<uint32_t, HASH_RAND_SIZE>, N_PLAYER_HASH> hash_player_;
    static std::array<std::array<uint32_t, HASH_RAND_SIZE>, N_PLAYER_HASH> hash_opponent_;
    static bool initialized_;
    static int hash_level_;

    // Mersenne Twister随机数生成器
    static std::mt19937_64 rng_;

    /**
     * @brief 生成高质量随机数
     *
     * 使用Mersenne Twister生成均匀分布的随机数
     *
     * @return 32位随机数
     */
    static uint32_t generateRandom();

    /**
     * @brief 验证随机数质量
     *
     * 检查随机数的位分布是否符合要求
     *
     * @param value 要检查的值
     * @param hash_level 哈希级别
     * @return true 如果通过验证
     */
    static bool validateRandom(uint32_t value, int hash_level);
};

// 便捷函数

/**
 * @brief 计算棋盘位置的索引
 *
 * 将(row, col)转换为0-63的索引
 *
 * @param row 行 (0-7)
 * @param col 列 (0-7)
 * @return 索引 (0-63)
 */
inline constexpr int positionToIndex(int row, int col) {
    return row * 8 + col;
}

/**
 * @brief 从索引获取行
 * @param index 索引 (0-63)
 * @return 行 (0-7)
 */
inline constexpr int indexToRow(int index) {
    return index / 8;
}

/**
 * @brief 从索引获取列
 * @param index 索引 (0-63)
 * @return 列 (0-7)
 */
inline constexpr int indexToCol(int index) {
    return index % 8;
}

} // namespace Reversi

