#include "ai/TranspositionTable.h"
#include <iostream>
#include <algorithm>
#include <cstring>

// 跨平台预取支持
#if defined(_MSC_VER)
    #include <emmintrin.h>
    #define PREFETCH(addr) _mm_prefetch(reinterpret_cast<const char*>(addr), _MM_HINT_T0)
#else
    #define PREFETCH(addr) __builtin_prefetch(addr)
#endif

namespace Reversi {

// 常量定义
constexpr size_t ENTRY_SIZE = sizeof(TTEntry);
constexpr size_t MIN_TABLE_SIZE = 1024;  // 最小1K条目
constexpr uint8_t MAX_DATE = 127;

// ============================================================================
// TranspositionTable 实现
// ============================================================================

TranspositionTable::TranspositionTable(size_t size_mb) {
    init(size_mb);
}

TranspositionTable::~TranspositionTable() {
    // 清理内存
    table_.clear();
    table_.shrink_to_fit();
}

void TranspositionTable::init(size_t size_mb) {
    if (initialized_) {
        clear();
    }

    config_.size_mb = size_mb;

    // 计算条目数 (每条目约16字节)
    // 64MB = 64 * 1024 * 1024 / 16 = 4,194,304 条目
    size_t num_entries = (size_mb * 1024 * 1024) / ENTRY_SIZE;

    // 确保大小是2的幂
    size_t size = MIN_TABLE_SIZE;
    while (size < num_entries) {
        size *= 2;
    }

    // 分配内存
    table_.resize(size);
    mask_ = size - 1;

    // 初始化所有条目
    for (auto& entry : table_) {
        entry.clear();
    }

    initialized_ = true;
    date_ = 0;

    std::cout << "[TranspositionTable] Initialized with "
              << size << " entries ("
              << (size * ENTRY_SIZE / 1024 / 1024) << " MB)"
              << std::endl;
}

void TranspositionTable::clear() {
    if (!table_.empty()) {
        std::fill(table_.begin(), table_.end(), TTEntry());
    }
    hits_ = 0;
    lookups_ = 0;
    stores_ = 0;
    date_ = 0;
    std::cout << "[TranspositionTable] Cleared" << std::endl;
}

void TranspositionTable::cleanup() {
    // 清理所有过期条目
    for (auto& entry : table_) {
        if (entry.isValid() && entry.depth_ == date_) {
            entry.clear();
        }
    }
    incrementDate();
}

void TranspositionTable::store(uint32_t hash, int depth, int score,
                                int alpha, int beta, const Move& move) {
    if (!initialized_ || table_.empty()) {
        return;
    }

    stores_++;
    size_t index = getIndex(hash);

    // 简单覆盖策略: 直接替换
    // 可以后续升级为更复杂的替换策略
    TTEntry& entry = table_[index];

    // 如果现有条目深度更大，不覆盖
    if (entry.isValid() && entry.depth_ > depth && entry.hash_ != hash) {
        return;
    }

    // 存储新条目
    entry.hash_ = hash;
    entry.depth_ = static_cast<int8_t>(depth);
    entry.score_ = static_cast<int16_t>(score);
    entry.alpha_ = static_cast<int16_t>(alpha);
    entry.beta_ = static_cast<int16_t>(beta);
    entry.move_ = move;
    entry.used_ = true;

    // 确定类型
    if (score >= beta) {
        entry.type_ = static_cast<uint8_t>(TTEntryType::LOWER);
    } else if (score <= alpha) {
        entry.type_ = static_cast<uint8_t>(TTEntryType::UPPER);
    } else {
        entry.type_ = static_cast<uint8_t>(TTEntryType::EXACT);
    }
}

bool TranspositionTable::probe(uint32_t hash, int depth,
                                int& alpha, int& beta,
                                int& score, Move& move) {
    if (!initialized_ || table_.empty()) {
        return false;
    }

    lookups_++;
    size_t index = getIndex(hash);

    // 查找条目
    const TTEntry* entry = findInBucket(index, hash, depth);

    if (entry == nullptr) {
        return false;
    }

    // 命中
    hits_++;

    // 更新参数
    score = entry->score_;
    move = entry->move_;

    // 根据类型更新alpha/beta
    switch (static_cast<TTEntryType>(entry->type_)) {
        case TTEntryType::EXACT:
            // 精确值，不需要更新alpha/beta
            break;
        case TTEntryType::LOWER:
            // 下界: score >= beta
            if (beta > entry->score_) {
                beta = entry->score_;
            }
            break;
        case TTEntryType::UPPER:
            // 上界: score <= alpha
            if (alpha < entry->score_) {
                alpha = entry->score_;
            }
            break;
    }

    return true;
}

const TTEntry* TranspositionTable::probe(uint32_t hash, int min_depth) const {
    if (!initialized_ || table_.empty()) {
        return nullptr;
    }

    // 不在const函数中修改统计变量
    // lookups_++应该在非const版本中处理
    size_t index = getIndex(hash);

    return findInBucket(index, hash, min_depth);
}

void TranspositionTable::prefetch(uint32_t hash) const {
    if (!initialized_ || table_.empty()) {
        return;
    }

    size_t index = getIndex(hash);
    // 使用编译器内置函数预取
    PREFETCH(&table_[index]);
}

size_t TranspositionTable::getMemoryUsage() const {
    return table_.size() * ENTRY_SIZE;
}

void TranspositionTable::incrementDate() {
    if (date_ >= MAX_DATE) {
        // 日期溢出，清理所有条目
        clear();
    } else {
        date_++;
    }
}

void TranspositionTable::setHashLevel(int level) {
    ZobristHash::init(level);
}

double TranspositionTable::getUsageRate() const {
    if (table_.empty()) return 0.0;

    size_t used = 0;
    for (const auto& entry : table_) {
        if (entry.isValid()) {
            used++;
        }
    }

    return static_cast<double>(used) / table_.size();
}

const TTEntry* TranspositionTable::findInBucket(size_t index,
                                                  uint32_t hash,
                                                  int min_depth) const {
    if (!table_.empty()) {
        const TTEntry& entry = table_[index];
        if (entry.match(hash, min_depth)) {
            return &entry;
        }
    }
    return nullptr;
}

} // namespace Reversi

