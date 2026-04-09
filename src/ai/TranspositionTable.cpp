#include "ai/TranspositionTable.h"
#include <iostream>
#include <algorithm>
#include <cstring>

#if defined(_MSC_VER)
    #include <emmintrin.h>
    #define PREFETCH(addr) _mm_prefetch(reinterpret_cast<const char*>(addr), _MM_HINT_T0)
#else
    #define PREFETCH(addr) __builtin_prefetch(addr)
#endif

namespace Reversi {

constexpr size_t ENTRY_SIZE = sizeof(TTEntry);
constexpr size_t MIN_TABLE_SIZE = 1024;
constexpr uint8_t MAX_DATE = 127;

TranspositionTable::TranspositionTable(size_t size_mb) {
    init(size_mb);
}

TranspositionTable::~TranspositionTable() {
    table_.clear();
    table_.shrink_to_fit();
}

void TranspositionTable::init(size_t size_mb) {
    if (initialized_) {
        clear();
    }

    config_.size_mb = size_mb;

    size_t num_entries = (size_mb * 1024 * 1024) / ENTRY_SIZE;

    size_t size = MIN_TABLE_SIZE;
    while (size < num_entries) {
        size *= 2;
    }

    table_.resize(size);
    mask_ = size - 1;

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

    TTEntry& entry = table_[index];

    if (entry.isValid() && entry.depth_ > depth && entry.hash_ != hash) {
        return;
    }

    entry.hash_ = hash;
    entry.depth_ = static_cast<int8_t>(depth);
    entry.score_ = static_cast<int16_t>(score);
    entry.alpha_ = static_cast<int16_t>(alpha);
    entry.beta_ = static_cast<int16_t>(beta);
    entry.move_ = move;
    entry.used_ = true;

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

    const TTEntry* entry = findInBucket(index, hash, depth);

    if (entry == nullptr) {
        return false;
    }

    hits_++;

    score = entry->score_;
    move = entry->move_;

    return true;
}

bool TranspositionTable::probe(uint32_t hash, int min_depth, int& score, Move& move) const {
    if (!initialized_ || table_.empty()) {
        return false;
    }

    size_t index = getIndex(hash);

    return findInBucket(index, hash, min_depth, score, move);
}

const TTEntry* TranspositionTable::probe(uint32_t hash, int min_depth) const {
    if (!initialized_ || table_.empty()) {
        return nullptr;
    }

    size_t index = getIndex(hash);

    return findInBucket(index, hash, min_depth);
}

TTEntryType TranspositionTable::getEntryType(uint32_t hash, int min_depth) const {
    if (!initialized_ || table_.empty()) {
        return TTEntryType::EXACT;
    }

    size_t index = getIndex(hash);
    const TTEntry* entry = findInBucket(index, hash, min_depth);

    if (entry == nullptr) {
        return TTEntryType::EXACT;
    }

    return static_cast<TTEntryType>(entry->type_);
}

void TranspositionTable::prefetch(uint32_t hash) const {
    if (!initialized_ || table_.empty()) {
        return;
    }

    size_t index = getIndex(hash);
    PREFETCH(&table_[index]);
}

size_t TranspositionTable::getMemoryUsage() const {
    return table_.size() * ENTRY_SIZE;
}

void TranspositionTable::incrementDate() {
    if (date_ >= MAX_DATE) {
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

bool TranspositionTable::findInBucket(size_t index,
                                       uint32_t hash,
                                       int min_depth,
                                       int& score, Move& move) const {
    if (!table_.empty()) {
        const TTEntry& entry = table_[index];
        if (entry.match(hash, min_depth)) {
            score = entry.score_;
            move = entry.move_;
            return true;
        }
    }
    return false;
}

} // namespace Reversi
