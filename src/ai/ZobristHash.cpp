#include "ai/ZobristHash.h"
#include "core/PlatformUtils.h"
#include <iostream>
#include <algorithm>
#include <bit>
#include <random>

namespace Reversi {

// 静态成员变量定义
std::array<std::array<uint32_t, ZobristHash::HASH_RAND_SIZE>, ZobristHash::N_PLAYER_HASH> ZobristHash::hash_player_;
std::array<std::array<uint32_t, ZobristHash::HASH_RAND_SIZE>, ZobristHash::N_PLAYER_HASH> ZobristHash::hash_opponent_;
bool ZobristHash::initialized_ = false;
int ZobristHash::hash_level_ = ZobristHash::DEFAULT_HASH_LEVEL;
std::mt19937_64 ZobristHash::rng_;

// 预计算的位掩码表 (2^level)
static uint64_t hash_size_mask = 0;

void ZobristHash::init(int hash_level, uint64_t seed) {
    if (initialized_) {
        shutdown();
    }

    // 验证哈希级别
    hash_level = std::max(MIN_HASH_LEVEL, std::min(MAX_HASH_LEVEL, hash_level));
    hash_level_ = hash_level;

    // 计算哈希大小掩码
    hash_size_mask = (1ULL << hash_level) - 1;

    // 初始化随机数生成器
    if (seed == 0) {
        std::random_device rd;
        rng_.seed(rd());
    } else {
        rng_.seed(seed);
    }

    std::cout << "[ZobristHash] Initializing with level " << hash_level
              << " (size: " << (1ULL << hash_level) << " entries)" << std::endl;

    // 生成玩家哈希数组
    for (int i = 0; i < N_PLAYER_HASH; ++i) {
        for (int j = 0; j < HASH_RAND_SIZE; ++j) {
            uint32_t value = 0;
            // 确保每个值有足够的位数
            while (!validateRandom(value, hash_level)) {
                value = generateRandom() & hash_size_mask;
            }
            hash_player_[i][j] = value;
        }
    }

    // 生成对手哈希数组
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

    // 获取玩家棋子数量并计算索引
    int player_count = POPCOUNT64(player_bits);
    int opponent_count = POPCOUNT64(opponent_bits);
    int player_index = getPlayerIndex(player_count);
    int opponent_index = getPlayerIndex(opponent_count);

    // 遍历玩家棋子的每个位置
    while (player_bits) {
        int square = CTZ64(player_bits);
        player_bits &= player_bits - 1;  // 清除最低位
        hash ^= hash_player_[player_index][square];
    }

    // 遍历对手棋子的每个位置
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

    // 根据棋子数量调整索引
    int stone_count = POPCOUNT64(player_bits);
    int adjusted_index = getPlayerIndex(stone_count);

    // 遍历玩家棋子的每个位置
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

    int player_index = 2;  // 默认使用中间范围

    if (is_player) {
        return hash_player_[player_index][square];
    } else {
        return hash_opponent_[player_index][square];
    }
}

int ZobristHash::getPlayerIndex(int stone_count) {
    // 根据棋子数量选择哈希数组索引
    // 0-16: 索引0, 17-32: 索引1, 33-48: 索引2, 49-64: 索引3
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
        // 清理数组
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
    // 每个数组: 4 * 65536 * 4 bytes = 1MB
    // 两个数组: 2MB
    return N_PLAYER_HASH * HASH_RAND_SIZE * sizeof(uint32_t) * 2;
}

uint32_t ZobristHash::generateRandom() {
    return static_cast<uint32_t>(rng_());
}

bool ZobristHash::validateRandom(uint32_t value, int hash_level) {
    // 确保值有足够的位数
    // 哈希级别除以6得到每个值需要的最小位数
    int min_bits = hash_level / 6;
    if (min_bits < 1) min_bits = 1;

    // 使用popcount检查位数
    int bit_count = POPCOUNT32(value);
    return bit_count >= min_bits;
}

} // namespace Reversi

