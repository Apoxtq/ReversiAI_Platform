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
 * @brief 转置表 (Transposition Table)
 *
 * 缓存搜索结果以避免重复计算，提高搜索效率。
 *
 * 实现特点:
 * - 简单覆盖策略 (可后续升级为替换策略)
 * - 支持Alpha/Beta/Exact三种类型
 * - 可配置大小
 * - 线程安全设计
 *
 * 参考: edax-reversi/src/hash.h
 *       Egaroucid/src/engine/hash.hpp
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @brief 转置表条目类型
 */
enum class TTEntryType {
    EXACT,   ///< 精确值 (score在[alpha, beta]范围内)
    LOWER,   ///< 下界 (score >= beta)
    UPPER    ///< 上界 (score <= alpha)
};

/**
 * @brief 转置表条目
 *
 * 存储搜索结果以避免重复计算
 */
struct TTEntry {
    uint32_t hash_;           ///< 棋盘哈希值
    int8_t depth_;            ///< 搜索深度
    int16_t score_;           ///< 评估分数
    int16_t alpha_;           ///< Alpha边界
    int16_t beta_;            ///< Beta边界
    Move move_;               ///< 最佳着法
    uint8_t type_;            ///< 条目类型 (EXACT/LOWER/UPPER)
    bool used_;               ///< 标记是否被使用过

    /**
     * @brief 默认构造函数
     */
    TTEntry() : hash_(0), depth_(0), score_(0), alpha_(0), beta_(0),
                move_(), type_(0), used_(false) {}

    /**
     * @brief 检查哈希是否匹配
     *
     * @param hash 要检查的哈希值
     * @param min_depth 最小期望深度
     * @return true 如果匹配
     */
    bool match(uint32_t hash, int min_depth) const {
        return used_ && hash_ == hash && depth_ >= min_depth;
    }

    /**
     * @brief 检查是否有效
     */
    bool isValid() const {
        return used_;
    }

    /**
     * @brief 重置条目
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
 * @brief 转置表配置
 */
struct TTConfig {
    size_t size_mb;           ///< 转置表大小 (MB)
    bool use_clustering;      ///< 是否使用聚类 (多路查找)
    int n_way;                ///< 每个桶的条目数

    TTConfig() : size_mb(64), use_clustering(true), n_way(4) {}
};

/**
 * @brief 转置表管理器
 *
 * 提供线程安全的转置表操作
 */
class TranspositionTable {
public:
    /**
     * @brief 构造函数
     *
     * @param size_mb 转置表大小 (MB), default: 64
     */
    explicit TranspositionTable(size_t size_mb = 64);

    /**
     * @brief 析构函数
     */
    ~TranspositionTable();

    /**
     * @brief 初始化转置表
     *
     * @param size_mb 大小 (MB)
     */
    void init(size_t size_mb);

    /**
     * @brief 清空转置表
     */
    void clear();

    /**
     * @brief 清空并重置日期
     */
    void cleanup();

    /**
     * @brief 存储搜索结果
     *
     * @param hash 棋盘哈希
     * @param depth 搜索深度
     * @param score 评估分数
     * @param alpha Alpha值
     * @param beta Beta值
     * @param move 最佳着法
     */
    void store(uint32_t hash, int depth, int score, int alpha, int beta, const Move& move);

    /**
     * @brief 查找条目
     *
     * @param hash 棋盘哈希
     * @param depth 期望深度
     * @param alpha Alpha值 (输入/输出)
     * @param beta Beta值 (输入/输出)
     * @return 找到返回true
     */
    bool probe(uint32_t hash, int depth, int& alpha, int& beta, int& score, Move& move);

    /**
     * @brief 查找条目 (简化版)
     *
     * @param hash 棋盘哈希
     * @param min_depth 最小期望深度
     * @return 找到返回TTEntry指针，否则返回nullptr
     */
    const TTEntry* probe(uint32_t hash, int min_depth) const;

    /**
     * @brief 预取条目到缓存
     *
     * @param hash 棋盘哈希
     */
    void prefetch(uint32_t hash) const;

    /**
     * @brief 获取命中次数
     */
    uint64_t getHits() const { return hits_; }

    /**
     * @brief 获取查找次数
     */
    uint64_t getLookups() const { return lookups_; }

    /**
     * @brief 获取命中率
     */
    double getHitRate() const {
        return lookups_ > 0 ? static_cast<double>(hits_) / lookups_ : 0.0;
    }

    /**
     * @brief 获取存储次数
     */
    uint64_t getStores() const { return stores_; }

    /**
     * @brief 获取转置表大小 (条目数)
     */
    size_t getSize() const { return table_.size(); }

    /**
     * @brief 获取实际使用内存 (字节)
     */
    size_t getMemoryUsage() const;

    /**
     * @brief 获取配置
     */
    const TTConfig& getConfig() const { return config_; }

    /**
     * @brief 获取当前日期
     */
    uint8_t getDate() const { return date_; }

    /**
     * @brief 增加日期 (用于清理旧条目)
     */
    void incrementDate();

    /**
     * @brief 设置哈希级别
     */
    void setHashLevel(int level);

    /**
     * @brief 获取使用率
     */
    double getUsageRate() const;

private:
    std::vector<TTEntry> table_;
    size_t mask_;              ///< 索引掩码
    uint64_t hits_ = 0;
    uint64_t lookups_ = 0;
    uint64_t stores_ = 0;
    uint8_t date_ = 0;         ///< 日期 (用于替换策略)
    TTConfig config_;
    bool initialized_ = false;

    /**
     * @brief 获取索引
     */
    size_t getIndex(uint32_t hash) const {
        return hash & mask_;
    }

    /**
     * @brief 存储条目到桶
     */
    void storeToBucket(size_t index, uint32_t hash, int depth,
                       int score, int alpha, int beta, const Move& move);

    /**
     * @brief 从桶中查找
     */
    const TTEntry* findInBucket(size_t index, uint32_t hash, int min_depth) const;
};

/**
 * @brief 搜索结果存储器
 *
 * 辅助类，用于存储搜索结果数据
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
     * @brief 从搜索参数创建
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

