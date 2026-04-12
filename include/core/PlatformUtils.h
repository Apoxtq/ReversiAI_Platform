#pragma once

/**
 * @file PlatformUtils.h
 * @brief Cross-platform utility functions and macros
 *
 * Provides portable utility functions across different compilers (MSVC, GCC, Clang)
 */

#include <cstdint>

#if defined(_MSC_VER)
    #include <intrin.h>
#endif

namespace PlatformUtils {

/**
 * @brief Cross-platform population count (Popcount)
 *
 * Counts the number of bits set to 1 in a 64-bit integer
 *
 * @param x 64-bit integer to count
 * @return Number of bits set to 1
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
 * @brief Cross-platform count trailing zeros (CTZ)
 *
 * Counts the number of consecutive zeros starting from the least significant bit
 *
 * @param x 64-bit integer to calculate
 * @return Number of trailing zeros
 * @note If x is 0, behavior is undefined
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
 * @brief Cross-platform population count (32-bit version)
 *
 * @param x 32-bit integer to count
 * @return Number of bits set to 1
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

// Convenience macro definitions
#if defined(_MSC_VER)
    #define POPCOUNT64(x) PlatformUtils::popcount64(x)
    #define CTZ64(x) PlatformUtils::ctz64(x)
    #define POPCOUNT32(x) PlatformUtils::popcount32(x)
#else
    #define POPCOUNT64(x) __builtin_popcountll(x)
    #define CTZ64(x) __builtin_ctzll(x)
    #define POPCOUNT32(x) __builtin_popcount(x)
#endif

