/**
 * @file GameController.cpp
 * @brief 游戏控制器实现
 *
 * 基于QtReversi/widget.cpp的游戏流程管理逻辑
 * 参考Reversi(Java)/GamePanel.java的manageTurn()设计
 *
 * @reference QtReversi/代码/chess/widget.cpp - 游戏规则和界面交互
 * @reference Reversi(Java)/src/game/GamePanel.java - manageTurn()回合管理逻辑
 */

#include "ui/GameController.h"
#include "ai/AIStrategy.h"
#include "Board.h"
#include <QTimer>
#include <QDebug>

namespace Reversi {

// 悔棋最大步数
constexpr int MAX_UNDO_STEPS = 10;

GameController::GameController(QObject* parent)
    : QObject(parent)
    , board_(std::make_unique<Board>())
    , gameMode_(GameMode::PvE)
    , currentPhase_(GamePhase::Waiting)
    , currentPlayer_(PlayerColor::Black)
    , humanColor_(PlayerColor::Black) {
    qDebug() << "GameController: Initialized";
}

GameController::~GameController() = default;

void GameController::startNewGame(GameMode mode, PlayerColor humanColor,
                                  Difficulty difficulty) {
    qDebug() << "GameController: Starting new game"
             << "mode:" << (int)mode
             << "humanColor:" << (int)humanColor
             << "difficulty:" << (int)difficulty;

    gameMode_ = mode;
    humanColor_ = humanColor;
    currentPlayer_ = PlayerColor::Black;  // 黑棋先手
    currentPlayer_ = PlayerColor::Black;  // 黑棋先手
    moveHistory_.clear();

    // 创建新的棋盘（因为Board没有reset()方法）
    board_ = std::make_unique<Board>();

    // 创建AI（如果不是PvP模式）
    if (mode != GameMode::PvP) {
        // 暂时使用MinimaxAI，MCTS AI正在开发中
        qDebug() << "GameController: Creating MinimaxAI...";
        ai_ = AIStrategyFactory::createMinimaxAI(difficulty);
        qDebug() << "GameController: AI pointer:" << ai_.get();
        if (!ai_) {
            emit errorOccurred("创建AI失败");
            return;
        }
        qDebug() << "GameController: AI created," << QString::fromStdString(ai_->getName());
    } else {
        ai_.reset();  // PvP模式不需要AI
    }

    // 确定初始阶段（根据谁先手）
    if (mode == GameMode::PvE && humanColor_ == PlayerColor::White) {
        // AI先手（人类是白棋），AI应该先走
        currentPhase_ = GamePhase::AITurn;
        // 延迟执行AI移动，避免阻塞UI
        QTimer::singleShot(500, this, &GameController::executeAIMove);
    } else {
        // 人类先手
        currentPhase_ = GamePhase::HumanTurn;
    }

    // 发射游戏开始信号
    emit gameStarted(mode, humanColor_);
    emit turnChanged(currentPlayer_);
    emit phaseChanged(currentPhase_);

    qDebug() << "GameController: Game started, current player:" << (int)currentPlayer_;
}

void GameController::resetGame() {
    qDebug() << "GameController: Resetting game";

    // 重新创建棋盘
    board_ = std::make_unique<Board>();
    currentPlayer_ = PlayerColor::Black;
    currentPhase_ = GamePhase::HumanTurn;
    moveHistory_.clear();

    emitStatusSignals();
}

bool GameController::makeHumanMove(int row, int col) {
    if (currentPhase_ != GamePhase::HumanTurn) {
        qDebug() << "GameController: Human move rejected - not human turn";
        return false;
    }

    Move move(row, col);

    // 检查是否是有效移动
    bool isValid = false;
    for (const auto& validMove : board_->getValidMoves()) {
        if (validMove.row == row && validMove.col == col) {
            isValid = true;
            break;
        }
    }

    if (!isValid) {
        qDebug() << "GameController: Invalid move at" << row << "," << col;
        return false;
    }

    qDebug() << "GameController: Human move at" << row << "," << col;

    // 执行移动
    saveMoveForUndo();
    board_->makeMove(move);

    // 发射移动信号
    emit moveMade(row, col, currentPlayer_);

    // 切换回合
    switchTurn();

    return true;
}

bool GameController::isValidMove(int row, int col) const {
    for (const auto& move : board_->getValidMoves()) {
        if (move.row == row && move.col == col) {
            return true;
        }
    }
    return false;
}

void GameController::switchTurn() {
    qDebug() << "GameController: Switching turn from" << (int)currentPlayer_;

    // 切换玩家
    currentPlayer_ = (currentPlayer_ == PlayerColor::Black) ?
                     PlayerColor::White : PlayerColor::Black;

    // 检查当前玩家是否有合法移动
    bool currentHasMoves = !board_->getValidMoves().empty();

    // 检查对手是否有合法移动
    PlayerColor opponent = (currentPlayer_ == PlayerColor::Black) ?
                           PlayerColor::White : PlayerColor::Black;

    // 临时切换到对手检查是否有移动
    bool opponentHasMoves;
    const auto& originalTurn = board_->getCurrentTurn();
    // 注意：这里我们直接检查当前棋盘状态下当前玩家的合法移动
    // 对于完整实现，需要考虑pass的情况

    qDebug() << "GameController: Current player" << (int)currentPlayer_
             << "has" << board_->getValidMoves().size() << "valid moves";

    // 检查游戏是否结束
    if (board_->isGameOver()) {
        checkGameOver();
        return;
    }

    // 如果当前玩家没有合法移动，则跳过
    if (!currentHasMoves) {
        qDebug() << "GameController: Player" << (int)currentPlayer_ << "passes";
        // 切换回对手
        currentPlayer_ = opponent;

        // 再次检查游戏是否结束
        if (board_->getValidMoves().empty()) {
            checkGameOver();
            return;
        }

        // 注意：不要在这里发送 turnChanged，等 phase 更新后再发送
        // 避免 UI 使用旧 phase 值绘制棋盘
    }

    // 决定下一阶段
    // PvP模式下，两个玩家都是人类，所有回合都是HumanTurn
    // PvE模式下，只有humanColor_玩家的回合是HumanTurn，AI回合是AITurn
    bool isHumanTurn = (gameMode_ == GameMode::PvP) || (currentPlayer_ == humanColor_);

    currentPhase_ = isHumanTurn ? GamePhase::HumanTurn : GamePhase::AITurn;

    qDebug() << "GameController: Turn switched to" << (int)currentPlayer_
             << ", phase:" << (int)currentPhase_;

    emit turnChanged(currentPlayer_);
    emit phaseChanged(currentPhase_);

    // 如果是AI回合，安排AI移动
    if (currentPhase_ == GamePhase::AITurn) {
        QTimer::singleShot(500, this, &GameController::executeAIMove);
    }
}

void GameController::executeAIMove() {
    qDebug() << "GameController::executeAIMove called";
    qDebug() << "GameController: currentPhase_ =" << (int)currentPhase_;
    qDebug() << "GameController: ai_ pointer =" << ai_.get();

    if (currentPhase_ != GamePhase::AITurn) {
        qDebug() << "GameController: Not AITurn, returning";
        return;
    }

    if (!ai_) {
        qDebug() << "GameController: No AI available";
        return;
    }

    emit aiThinkingStarted(QString::fromStdString(ai_->getName()));

    // 设置搜索限制
    SearchLimits limits = SearchLimits::createDefault();
    limits.maxDepth = 4;  // 默认搜索深度

    // 找到最佳移动
    Move bestMove = ai_->findBestMove(*board_, limits);

    qDebug() << "GameController: AI selected move"
             << bestMove.row << "," << bestMove.col;

    // 更新统计信息
    AIStats stats = ai_->getStats();
    emit aiStatsUpdated(stats);

    // 发射AI思考完成信号
    emit aiThinkingFinished(bestMove.row, bestMove.col);

    // 执行移动
    saveMoveForUndo();
    board_->makeMove(bestMove);

    // 发射移动信号
    emit moveMade(bestMove.row, bestMove.col, currentPlayer_);

    // 切换回合
    switchTurn();
}

void GameController::saveMoveForUndo() {
    // 只保存最近N步
    if (moveHistory_.size() >= MAX_UNDO_STEPS) {
        moveHistory_.erase(moveHistory_.begin());
    }
    // 保存当前棋盘的副本
    moveHistory_.push_back(std::make_unique<Board>(*board_));
}

bool GameController::canUndo() const {
    return !moveHistory_.empty();
}

void GameController::undoMove() {
    if (moveHistory_.empty()) {
        qDebug() << "GameController: No moves to undo";
        return;
    }

    // 恢复上一个状态
    board_ = std::make_unique<Board>(*moveHistory_.back());
    moveHistory_.pop_back();

    // 恢复当前玩家
    if (!moveHistory_.empty()) {
        currentPlayer_ = moveHistory_.back()->getCurrentTurn();
    } else {
        currentPlayer_ = PlayerColor::Black;
    }

    // 更新阶段
    bool isHumanTurn = (currentPlayer_ == humanColor_) &&
                       (gameMode_ == GameMode::PvE || gameMode_ == GameMode::PvP);
    currentPhase_ = isHumanTurn ? GamePhase::HumanTurn : GamePhase::AITurn;

    qDebug() << "GameController: Undo successful, player:" << (int)currentPlayer_;

    emitStatusSignals();
}

void GameController::checkGameOver() {
    auto winnerOpt = board_->getWinner();

    GameResult result;
    if (!winnerOpt.has_value()) {
        result = GameResult::Draw;
    } else if (winnerOpt.value() == PlayerColor::Black) {
        result = GameResult::BlackWins;
    } else {
        result = GameResult::WhiteWins;
    }

    currentPhase_ = GamePhase::GameOver;

    qDebug() << "GameController: Game ended, result:" << (int)result;

    emit gameEnded(result);
}

void GameController::emitStatusSignals() {
    emit turnChanged(currentPlayer_);
}

} // namespace Reversi
