#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include "core/BitBoard.h"
#include "ZobristHash.h"

/**
 * @file TranspositionTable.h
 * @brief Transposition Table
 *
 * Caches search results to avoid redundant computation and improve search efficiency.
 *
 * Implementation features:
 * - Simple overwrite strategy (upgradeable to replacement strategy)
 * - Supports EXACT/LOWER/UPPER entry types
 * - Configurable size
 * - Thread-safe design
 *
 * Reference: edax-reversi/src/hash.h
 *           Egaroucid/src/engine/hash.hpp
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @brief Transposition table entry type
 */
enum class TTEntryType {
    EXACT,   ///< Exact value (score within [alpha, beta])
    LOWER,   ///< Lower bound (score >= beta)
    UPPER    ///< Upper bound (score <= alpha)
};

/**
 * @brief Transposition table entry
 *
 * Stores search results to avoid redundant computation.
 */
struct TTEntry {
    uint32_t hash_;           ///< Board hash value
    int8_t depth_;            ///< Search depth
    int16_t score_;           ///< Evaluation score
    int16_t alpha_;           ///< Alpha bound
    int16_t beta_;            ///< Beta bound
    Move move_;               ///< Best move
    uint8_t type_;            ///< Entry type (EXACT/LOWER/UPPER)
    bool used_;               ///< Whether entry has been used

    /**
     * @brief Default constructor
     */
    TTEntry() : hash_(0), depth_(0), score_(0), alpha_(0), beta_(0),
                move_(), type_(0), used_(false) {}

    /**
     * @brief Check if hash matches
     *
     * @param hash Hash value to check
     * @param min_depth Minimum expected depth
     * @return true if matches
     */
    bool match(uint32_t hash, int min_depth) const {
        return used_ && hash_ == hash && depth_ >= min_depth;
    }

    /**
     * @brief Check if valid
     */
    bool isValid() const {
        return used_;
    }

    /**
     * @brief Reset entry
     */
    void clear() {
        hash_ = 0;
        depth_ = 0;
        score_ = 0;
        alpha_ = 0;
        beta_ = 0;
        move_ = Move();
        type_ = 0;
        used_ = false;
    }
};

/**
 * @brief Transposition table configuration
 */
struct TTConfig {
    size_t size_mb;           ///< Table size (MB)
    bool use_clustering;      ///< Whether to use clustering (multi-way lookup)
    int n_way;                ///< Number of entries per bucket

    TTConfig() : size_mb(64), use_clustering(true), n_way(4) {}
};

/**
 * @brief Transposition table manager
 *
 * Provides thread-safe transposition table operations.
 */
class TranspositionTable {
public:
    /**
     * @brief Constructor
     *
     * @param size_mb Table size (MB), default: 64
     */
    explicit TranspositionTable(size_t size_mb = 64);

    /**
     * @brief Destructor
     */
    ~TranspositionTable();

    /**
     * @brief Initialize transposition table
     *
     * @param size_mb Size (MB)
     */
    void init(size_t size_mb);

    /**
     * @brief Clear transposition table
     */
    void clear();

    /**
     * @brief Cleanup and reset date
     */
    void cleanup();

    /**
     * @brief Store search result
     *
     * @param hash Board hash
     * @param depth Search depth
     * @param score Evaluation score
     * @param alpha Alpha value
     * @param beta Beta value
     * @param move Best move
     */
    void store(uint32_t hash, int depth, int score, int alpha, int beta, const Move& move);

    /**
     * @brief Probe table entry
     *
     * @param hash Board hash
     * @param depth Expected depth
     * @param alpha Alpha value (in/out)
     * @param beta Beta value (in/out)
     * @return true if found
     */
    bool probe(uint32_t hash, int depth, int& alpha, int& beta, int& score, Move& move);

    /**
     * @brief Probe table entry (simplified - no alpha/beta)
     *
     * @param hash Board hash
     * @param min_depth Minimum expected depth
     * @param score Output: evaluation score
     * @param move Output: best move
     * @return true if found
     */
    bool probe(uint32_t hash, int min_depth, int& score, Move& move) const;

    /**
     * @brief Probe table entry (simplified - returns pointer)
     *
     * @param hash Board hash
     * @param min_depth Minimum expected depth
     * @return TTEntry pointer if found, nullptr otherwise
     */
    const TTEntry* probe(uint32_t hash, int min_depth) const;

    /**
     * @brief Get entry type
     * @param hash Hash value
     * @param min_depth Minimum expected depth
     * @return Entry type, EXACT if not found
     */
    TTEntryType getEntryType(uint32_t hash, int min_depth) const;

    /**
     * @brief Prefetch entry to cache
     *
     * @param hash Board hash
     */
    void prefetch(uint32_t hash) const;

    /**
     * @brief Get hit count
     */
    uint64_t getHits() const { return hits_; }

    /**
     * @brief Get lookup count
     */
    uint64_t getLookups() const { return lookups_; }

    /**
     * @brief Get hit rate
     */
    double getHitRate() const {
        return lookups_ > 0 ? static_cast<double>(hits_) / lookups_ : 0.0;
    }

    /**
     * @brief Get store count
     */
    uint64_t getStores() const { return stores_; }

    /**
     * @brief Get table size (number of entries)
     */
    size_t getSize() const { return table_.size(); }

    /**
     * @brief Get actual memory usage (bytes)
     */
    size_t getMemoryUsage() const;

    /**
     * @brief Get configuration
     */
    const TTConfig& getConfig() const { return config_; }

    /**
     * @brief Get current date
     */
    uint8_t getDate() const { return date_; }

    /**
     * @brief Increment date (for cleaning old entries)
     */
    void incrementDate();

    /**
     * @brief Set hash level
     */
    void setHashLevel(int level);

    /**
     * @brief Get usage rate
     */
    double getUsageRate() const;

private:
    std::vector<TTEntry> table_;
    size_t mask_;              ///< Index mask
    uint64_t hits_ = 0;
    uint64_t lookups_ = 0;
    uint64_t stores_ = 0;
    uint8_t date_ = 0;         ///< Date (for replacement strategy)
    TTConfig config_;
    bool initialized_ = false;

    /**
     * @brief Get index
     */
    size_t getIndex(uint32_t hash) const {
        return hash & mask_;
    }

    /**
     * @brief Store entry to bucket
     */
    void storeToBucket(size_t index, uint32_t hash, int depth,
                       int score, int alpha, int beta, const Move& move);

    /**
     * @brief Find in bucket
     */
    const TTEntry* findInBucket(size_t index, uint32_t hash, int min_depth) const;
    bool findInBucket(size_t index, uint32_t hash, int min_depth, int& score, Move& move) const;
};

/**
 * @brief Search result storage helper
 *
 * Helper class for storing search result data.
 */
struct SearchStore {
    int depth;
    int selectivity;
    int cost;
    int alpha;
    int beta;
    int score;
    Move move;
    uint8_t date;

    SearchStore() : depth(0), selectivity(0), cost(0),
                    alpha(0), beta(0), score(0), move(), date(0) {}

    /**
     * @brief Create from search parameters
     */
    static SearchStore create(int depth, int alpha, int beta, int score, const Move& move) {
        SearchStore store;
        store.depth = depth;
        store.alpha = alpha;
        store.beta = beta;
        store.score = score;
        store.move = move;
        store.selectivity = 0;
        store.cost = 0;
        store.date = 0;
        return store;
    }
};

} // namespace Reversi
