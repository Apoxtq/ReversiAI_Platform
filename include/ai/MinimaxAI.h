#pragma once

#include "ai/AIStrategy.h"
#include "ai/Evaluator.h"
#include "ai/TranspositionTable.h"
#include "ai/MoveOrderer.h"
#include <memory>
#include <chrono>

namespace Reversi {

struct MinimaxConfig {
    int maxDepth = 4;
    bool useAlphaBeta = true;
    bool useIterativeDeepening = false;
    bool useTranspositionTable = true;
    bool useKillerMoves = true;
    bool useHistoryHeuristic = true;
    bool useMoveOrdering = true;

    std::chrono::milliseconds timeLimit = std::chrono::milliseconds(3000);
    size_t transpositionTableSizeMB = 64;

    mutable int nodesExplored = 0;
    mutable int cutoffs = 0;
    mutable int ttHits = 0;
    mutable int killerHits = 0;
    mutable int historyHits = 0;
};

class MinimaxAI : public AIStrategy {
public:
    explicit MinimaxAI(MinimaxConfig config = MinimaxConfig(),
                       std::unique_ptr<Evaluator> evaluator = nullptr);

    Move findBestMove(const Board& board, const SearchLimits& limits) override;
    std::string getName() const override { return "MinimaxAI"; }
    Difficulty getDifficulty() const override;
    AIStats getStats() const override;
    std::string getDescription() const override;
    std::string getConfigDescription() const override;
    void reset() override;
    bool supportsFeature(const std::string& feature) const override;

    const MinimaxConfig& getConfig() const { return config_; }
    void setConfig(const MinimaxConfig& config) { config_ = config; }
    void setColor(PlayerColor color) { color_ = color; }
    PlayerColor getColor() const { return color_; }

    double getTranspositionTableHitRate() const {
        return tt_ ? tt_->getHitRate() : 0.0;
    }

    int getLastScore() const { return bestScore_; }

    void clearTranspositionTable() {
        if (tt_) tt_->clear();
    }

    double getMoveOrdererHitRate() const {
        if (!config_.useHistoryHeuristic || !moveOrderer_) return 0.0;
        const auto& stats = moveOrderer_->getStatistics();
        return stats.totalMoves > 0 ? static_cast<double>(stats.historyHits) / stats.totalMoves : 0.0;
    }

    void clearMoveOrderer() {
        if (moveOrderer_) moveOrderer_->clear();
    }

private:
    void initTranspositionTable();

    int minimaxAlphaBeta(const Board& board, int depth, int maxSearchDepth,
                          int alpha, int beta, const SearchLimits& limits,
                          PlayerColor originalPlayer);

    bool shouldStop(const SearchLimits& limits,
                     std::chrono::steady_clock::time_point startTime) const;

    Move getBestMoveFromLastDepth() const;

    MinimaxConfig config_;
    std::unique_ptr<Evaluator> evaluator_;

    std::unique_ptr<TranspositionTable> tt_;

    std::unique_ptr<MoveOrderer> moveOrderer_;

    mutable AIStats stats_;
    mutable Move bestMove_;
    mutable int bestScore_;
    mutable int lastCompletedDepth_;
    mutable PlayerColor color_;
    mutable std::chrono::steady_clock::time_point searchStartTime_;
};

} // namespace Reversi