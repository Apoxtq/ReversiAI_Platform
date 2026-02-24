#pragma once

/**
 * @file PlatformUtils.h
 * @brief 跨平台工具函数和宏
 *
 * 提供在不同编译器（MSVC, GCC, Clang）之间可移植的工具函数
 */

#include <cstdint>

#if defined(_MSC_VER)
    #include <intrin.h>
#endif

namespace PlatformUtils {

/**
 * @brief 跨平台人口计数 (Population Count)
 *
 * 计算64位整数中设置为1的位数
 *
 * @param x 要计数的64位整数
 * @return 设置为1的位数
 */
#if defined(_MSC_VER)
    inline int popcount64(uint64_t x) {
        return __popcnt64(x);
    }
#else
    inline int popcount64(uint64_t x) {
        return __builtin_popcountll(x);
    }
#endif

/**
 * @brief 跨平台计算尾随零 (Count Trailing Zeros)
 *
 * 计算从最低位开始连续0的个数
 *
 * @param x 要计算的64位整数
 * @return 尾随零的个数
 * @note 如果 x 为0，则行为未定义
 */
#if defined(_MSC_VER)
    inline int ctz64(uint64_t x) {
        unsigned long index;
        _BitScanForward64(&index, x);
        return static_cast<int>(index);
    }
#else
    inline int ctz64(uint64_t x) {
        return __builtin_ctzll(x);
    }
#endif

/**
 * @brief 跨平台人口计数 (32位版本)
 *
 * @param x 要计数的32位整数
 * @return 设置为1的位数
 */
#if defined(_MSC_VER)
    inline int popcount32(uint32_t x) {
        return __popcnt(x);
    }
#else
    inline int popcount32(uint32_t x) {
        return __builtin_popcount(x);
    }
#endif

} // namespace PlatformUtils

// 便捷宏定义
#if defined(_MSC_VER)
    #define POPCOUNT64(x) PlatformUtils::popcount64(x)
    #define CTZ64(x) PlatformUtils::ctz64(x)
    #define POPCOUNT32(x) PlatformUtils::popcount32(x)
#else
    #define POPCOUNT64(x) __builtin_popcountll(x)
    #define CTZ64(x) __builtin_ctzll(x)
    #define POPCOUNT32(x) __builtin_popcount(x)
#endif

