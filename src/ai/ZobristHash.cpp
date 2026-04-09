#include "ai/ZobristHash.h"
#include "core/PlatformUtils.h"
#include <iostream>
#include <algorithm>
#include <bit>
#include <random>

namespace Reversi {

std::array<std::array<uint32_t, ZobristHash::HASH_RAND_SIZE>, ZobristHash::N_PLAYER_HASH> ZobristHash::hash_player_;
std::array<std::array<uint32_t, ZobristHash::HASH_RAND_SIZE>, ZobristHash::N_PLAYER_HASH> ZobristHash::hash_opponent_;
bool ZobristHash::initialized_ = false;
int ZobristHash::hash_level_ = ZobristHash::DEFAULT_HASH_LEVEL;
std::mt19937_64 ZobristHash::rng_;

static uint64_t hash_size_mask = 0;

void ZobristHash::init(int hash_level, uint64_t seed) {
    if (initialized_) {
        shutdown();
    }

    hash_level = std::max(MIN_HASH_LEVEL, std::min(MAX_HASH_LEVEL, hash_level));
    hash_level_ = hash_level;

    hash_size_mask = (1ULL << hash_level) - 1;

    if (seed == 0) {
        std::random_device rd;
        rng_.seed(rd());
    } else {
        rng_.seed(seed);
    }

    std::cout << "[ZobristHash] Initializing with level " << hash_level
              << " (size: " << (1ULL << hash_level) << " entries)" << std::endl;

    for (int i = 0; i < N_PLAYER_HASH; ++i) {
        for (int j = 0; j < HASH_RAND_SIZE; ++j) {
            uint32_t value = 0;
            while (!validateRandom(value, hash_level)) {
                value = generateRandom() & hash_size_mask;
            }
            hash_player_[i][j] = value;
        }
    }

    for (int i = 0; i < N_PLAYER_HASH; ++i) {
        for (int j = 0; j < HASH_RAND_SIZE; ++j) {
            uint32_t value = 0;
            while (!validateRandom(value, hash_level)) {
                value = generateRandom() & hash_size_mask;
            }
            hash_opponent_[i][j] = value;
        }
    }

    initialized_ = true;
    std::cout << "[ZobristHash] Initialization complete" << std::endl;
}

bool ZobristHash::isInitialized() {
    return initialized_;
}

uint32_t ZobristHash::computeHash(uint64_t player_bits, uint64_t opponent_bits) {
    if (!initialized_) {
        init(DEFAULT_HASH_LEVEL);
    }

    uint32_t hash = 0;

    int player_count = POPCOUNT64(player_bits);
    int opponent_count = POPCOUNT64(opponent_bits);
    int player_index = getPlayerIndex(player_count);
    int opponent_index = getPlayerIndex(opponent_count);

    while (player_bits) {
        int square = CTZ64(player_bits);
        player_bits &= player_bits - 1;
        hash ^= hash_player_[player_index][square];
    }

    while (opponent_bits) {
        int square = CTZ64(opponent_bits);
        opponent_bits &= opponent_bits - 1;
        hash ^= hash_opponent_[opponent_index][square];
    }

    return hash;
}

uint32_t ZobristHash::computePlayerHash(uint64_t player_bits, int player_index) {
    if (!initialized_) {
        init(DEFAULT_HASH_LEVEL);
    }

    uint32_t hash = 0;

    int stone_count = POPCOUNT64(player_bits);
    int adjusted_index = getPlayerIndex(stone_count);

    while (player_bits) {
        int square = CTZ64(player_bits);
        player_bits &= player_bits - 1;
        hash ^= hash_player_[adjusted_index][square];
    }

    return hash;
}

uint32_t ZobristHash::getSquareHash(int square, bool is_player) {
    if (!initialized_) {
        init(DEFAULT_HASH_LEVEL);
    }

    int player_index = 2;

    if (is_player) {
        return hash_player_[player_index][square];
    } else {
        return hash_opponent_[player_index][square];
    }
}

int ZobristHash::getPlayerIndex(int stone_count) {
    if (stone_count <= 16) return 0;
    if (stone_count <= 32) return 1;
    if (stone_count <= 48) return 2;
    return 3;
}

int ZobristHash::getHashLevel() {
    return hash_level_;
}

void ZobristHash::shutdown() {
    if (initialized_) {
        for (int i = 0; i < N_PLAYER_HASH; ++i) {
            hash_player_[i].fill(0);
            hash_opponent_[i].fill(0);
        }
        initialized_ = false;
        std::cout << "[ZobristHash] Shutdown complete" << std::endl;
    }
}

size_t ZobristHash::getHashSize() {
    return initialized_ ? (1ULL << hash_level_) : 0;
}

size_t ZobristHash::getHashMemory() {
    return N_PLAYER_HASH * HASH_RAND_SIZE * sizeof(uint32_t) * 2;
}

uint32_t ZobristHash::generateRandom() {
    return static_cast<uint32_t>(rng_());
}

bool ZobristHash::validateRandom(uint32_t value, int hash_level) {
    int min_bits = hash_level / 6;
    if (min_bits < 1) min_bits = 1;

    int bit_count = POPCOUNT32(value);
    return bit_count >= min_bits;
}

} // namespace Reversi
