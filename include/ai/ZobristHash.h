#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <random>

/**
 * @file ZobristHash.h
 * @brief Zobrist Hash Generator
 *
 * Uses Egaroucid-style Zobrist hashing scheme:
 * - 4 player hash arrays (by position and disc count)
 * - Supports 64x4 position encoding
 * - High-quality random number generation
 *
 * Reference: Egaroucid/src/engine/hash.hpp
 *            edax-reversi/src/hash.h
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @brief Zobrist Hash Generator class
 *
 * Generates unique hash values for board states, used for:
 * - Transposition Table
 * - Network sync verification
 * - Position caching
 *
 * Implementation features:
 * - Uses 4 arrays of 65536 entries each (Egaroucid style)
 * - Each entry is a 32-bit hash value
 * - Configurable hash precision level
 * - Supports loading precomputed hashes from file
 */
class ZobristHash {
public:
    /**
     * @brief Hash precision level constants
     *
     * Level determines table entry count: 2^level
     * 25 = 33,554,432 entries (~32MB for 32-bit hash)
     */
    static constexpr int DEFAULT_HASH_LEVEL = 25;
    static constexpr int MIN_HASH_LEVEL = 20;
    static constexpr int MAX_HASH_LEVEL = 30;

    /**
     * @brief Hash array size
     */
    static constexpr int HASH_RAND_SIZE = 65536;
    static constexpr int N_PLAYER_HASH = 4;

    /**
     * @brief Initialize hash table
     *
     * Initializes hash arrays using high-quality RNG.
     *
     * @param hash_level Hash precision level (default: 25)
     * @param seed Random seed (default: hardware-based)
     */
    static void init(int hash_level = DEFAULT_HASH_LEVEL, uint64_t seed = 0);

    /**
     * @brief Check if initialized
     * @return true if initialized
     */
    static bool isInitialized();

    /**
     * @brief Compute board hash (full hash)
     *
     * Computes full board hash including both players.
     *
     * @param player_bits Black player bitboard
     * @param opponent_bits White player bitboard
     * @return 32-bit hash value
     *
     * @complexity O(1) - optimized bit operations
     */
    static uint32_t computeHash(uint64_t player_bits, uint64_t opponent_bits);

    /**
     * @brief Compute player hash
     *
     * Computes hash for current player pieces only.
     * Used for fast position identification.
     *
     * @param player_bits Current player bitboard
     * @param player_index Player index (0=black, 1=white, based on disc count)
     * @return 32-bit hash value
     */
    static uint32_t computePlayerHash(uint64_t player_bits, int player_index);

    /**
     * @brief Get single square hash
     *
     * @param square Square index (0-63)
     * @param is_player true=player piece, false=opponent piece
     * @return Hash value
     */
    static uint32_t getSquareHash(int square, bool is_player);

    /**
     * @brief Get player index
     *
     * Determines player index in hash array based on disc count.
     *
     * @param stone_count Disc count
     * @return Index (0-3)
     */
    static int getPlayerIndex(int stone_count);

    /**
     * @brief Get current hash level
     * @return Hash level
     */
    static int getHashLevel();

    /**
     * @brief Shutdown and cleanup memory
     */
    static void shutdown();

    /**
     * @brief Hash table size (number of entries)
     */
    static size_t getHashSize();

    /**
     * @brief Hash table size (bytes)
     */
    static size_t getHashMemory();

private:
    // Static member variables
    static std::array<std::array<uint32_t, HASH_RAND_SIZE>, N_PLAYER_HASH> hash_player_;
    static std::array<std::array<uint32_t, HASH_RAND_SIZE>, N_PLAYER_HASH> hash_opponent_;
    static bool initialized_;
    static int hash_level_;

    // Mersenne Twister RNG
    static std::mt19937_64 rng_;

    /**
     * @brief Generate high-quality random number
     *
     * Uses Mersenne Twister for uniformly distributed random numbers.
     *
     * @return 32-bit random number
     */
    static uint32_t generateRandom();

    /**
     * @brief Validate random number quality
     *
     * Checks if random number bit distribution meets requirements.
     *
     * @param value Value to check
     * @param hash_level Hash level
     * @return true if validation passes
     */
    static bool validateRandom(uint32_t value, int hash_level);
};

// Convenience functions

/**
 * @brief Convert board position to index
 *
 * Converts (row, col) to 0-63 index.
 *
 * @param row Row (0-7)
 * @param col Column (0-7)
 * @return Index (0-63)
 */
inline constexpr int positionToIndex(int row, int col) {
    return row * 8 + col;
}

/**
 * @brief Get row from index
 * @param index Index (0-63)
 * @return Row (0-7)
 */
inline constexpr int indexToRow(int index) {
    return index / 8;
}

/**
 * @brief Get column from index
 * @param index Index (0-63)
 * @return Column (0-7)
 */
inline constexpr int indexToCol(int index) {
    return index % 8;
}

} // namespace Reversi
