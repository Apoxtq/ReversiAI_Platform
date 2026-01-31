#pragma once

#include "ai/AIStrategy.h"
#include "Board.h"
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include <functional>

/**
 * @file AIBattle.h
 * @brief AI对战系统
 *
 * 实现AI算法之间的自动对战，用于性能比较和基准测试
 * 支持GUI环境下的异步对战和进度回调
 */

namespace Reversi {

/**
 * @struct BattleResult
 * @brief 单局对战结果
 */
struct BattleResult {
    PlayerColor winner = PlayerColor::Black;  ///< 获胜者
    bool isDraw = false;                       ///< 是否平局
    int totalMoves = 0;                        ///< 总移动数
    std::chrono::milliseconds duration;        ///< 对战持续时间

    // AI统计信息
    AIStats blackAIStats;                      ///< 黑方AI统计
    AIStats whiteAIStats;                      ///< 白方AI统计

    // 移动历史
    std::vector<Move> moveHistory;             ///< 移动历史记录
};

/**
 * @struct TournamentResult
 * @brief 锦标赛结果
 */
struct TournamentResult {
    std::string blackAIName;                   ///< 黑方AI名称
    std::string whiteAIName;                   ///< 白方AI名称

    int totalGames = 0;                        ///< 总对战局数
    int blackWins = 0;                         ///< 黑方获胜局数
    int whiteWins = 0;                         ///< 白方获胜局数
    int draws = 0;                             ///< 平局局数

    double blackWinRate = 0.0;                 ///< 黑方胜率
    double whiteWinRate = 0.0;                 ///< 白方胜率
    double drawRate = 0.0;                     ///< 平局率

    std::chrono::milliseconds totalDuration;   ///< 总耗时

    // 平均统计
    AIStats avgBlackStats;                     ///< 黑方平均统计
    AIStats avgWhiteStats;                     ///< 白方平均统计

    // 所有单局结果
    std::vector<BattleResult> gameResults;     ///< 所有对战结果
};

/**
 * @class AIBattle
 * @brief AI对战管理器
 *
 * 管理AI算法之间的自动对战，支持：
 * - 单局对战
 * - 批量锦标赛
 * - 进度回调（适合GUI环境）
 * - 线程安全
 */
class AIBattle {
public:
    /**
     * @brief 对战进度回调函数类型
     * @param current 当前完成的局数
     * @param total 总局数
     * @param result 当前局的结果
     */
    using ProgressCallback = std::function<void(int current, int total, const BattleResult& result)>;

    /**
     * @brief 构造函数
     * @param blackAI 黑方AI
     * @param whiteAI 白方AI
     */
    AIBattle(std::unique_ptr<AIStrategy> blackAI, std::unique_ptr<AIStrategy> whiteAI);

    /**
     * @brief 执行单局对战
     * @param searchLimits 搜索限制
     * @return 对战结果
     */
    BattleResult playSingleGame(const SearchLimits& searchLimits = SearchLimits::createDefault());

    /**
     * @brief 执行锦标赛（多局对战）
     * @param numGames 对战局数
     * @param searchLimits 搜索限制
     * @param progressCallback 进度回调函数（可选，用于GUI更新）
     * @return 锦标赛结果
     */
    TournamentResult playTournament(int numGames,
                                   const SearchLimits& searchLimits = SearchLimits::createDefault(),
                                   ProgressCallback progressCallback = nullptr);

    /**
     * @brief 获取黑方AI名称
     */
    std::string getBlackAIName() const;

    /**
     * @brief 获取白方AI名称
     */
    std::string getWhiteAIName() const;

    /**
     * @brief 获取黑方AI描述
     */
    std::string getBlackAIDescription() const;

    /**
     * @brief 获取白方AI描述
     */
    std::string getWhiteAIDescription() const;

    /**
     * @brief 设置随机种子（用于重现实验）
     * @param seed 随机种子
     */
    void setRandomSeed(unsigned int seed);

private:
    /**
     * @brief 执行一局完整的对战
     * @param searchLimits 搜索限制
     * @return 对战结果
     */
    BattleResult executeGame(const SearchLimits& searchLimits);

    /**
     * @brief 处理单个移动
     * @param board 当前棋盘
     * @param currentPlayer 当前玩家
     * @param ai AI算法
     * @param searchLimits 搜索限制
     * @param moveHistory 移动历史（用于记录）
     * @param aiStats AI统计信息（用于累积）
     * @return 是否成功执行移动
     */
    bool executeMove(Board& board, PlayerColor currentPlayer, AIStrategy& ai,
                    const SearchLimits& searchLimits, std::vector<Move>& moveHistory,
                    AIStats& aiStats);

    // AI算法
    std::unique_ptr<AIStrategy> blackAI_;
    std::unique_ptr<AIStrategy> whiteAI_;

    // 随机数生成器（用于可能的随机性控制）
    unsigned int randomSeed_ = 42;
};

/**
 * @class AIBenchmark
 * @brief AI性能基准测试
 *
 * 提供标准化的性能测试和比较功能
 * 参考: Egaroucid的benchmark系统
 */
class AIBenchmark {
public:
    /**
     * @brief 基准测试结果
     */
    struct BenchmarkResult {
        std::string aiName;
        int testGames = 0;
        std::chrono::milliseconds avgTimePerMove;
        double avgNodesExplored = 0.0;
        double avgBranchingFactor = 0.0;
        double winRate = 0.0;  // 对抗随机AI的胜率
    };

    /**
     * @brief 执行AI性能基准测试
     * @param ai 要测试的AI
     * @param testGames 测试局数
     * @return 基准测试结果
     */
    static BenchmarkResult runBenchmark(std::unique_ptr<AIStrategy> ai, int testGames = 10);

    /**
     * @brief 比较两个AI算法
     * @param ai1 第一个AI
     * @param ai2 第二个AI
     * @param numGames 比较局数
     * @return 比较结果
     */
    static TournamentResult compareAIs(std::unique_ptr<AIStrategy> ai1,
                                      std::unique_ptr<AIStrategy> ai2,
                                      int numGames = 20);
};

} // namespace Reversi
