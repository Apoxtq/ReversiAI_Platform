/**
 * @file GameController.cpp
 * @brief Game controller implementation
 *
 * Game flow management logic based on QtReversi/widget.cpp
 * Reference Reversi(Java)/GamePanel.java's manageTurn() design
 *
 * @reference QtReversi/code/chess/widget.cpp - Game rules and UI interaction
 * @reference Reversi(Java)/src/game/GamePanel.java - manageTurn() turn management logic
 */

#include "ui/GameController.h"
#include "ai/AIStrategy.h"
#include "Board.h"
#include <QTimer>
#include <QDebug>

namespace Reversi {

// Maximum undo steps (Reversi has max 60 moves)
constexpr int MAX_UNDO_STEPS = 60;

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
                                  Difficulty difficulty,
                                  int algorithm,
                                  int depth) {
    qDebug() << "GameController: Starting new game"
             << "mode:" << (int)mode
             << "humanColor:" << (int)humanColor
             << "difficulty:" << (int)difficulty
             << "algorithm:" << algorithm
             << "depth:" << depth;

    gameMode_ = mode;
    humanColor_ = humanColor;
    currentPlayer_ = PlayerColor::Black;
    moveHistory_.clear();

    // Create new board (Board doesn't have reset() method)
    board_ = std::make_unique<Board>();

    // Create AI (if not PvP mode)
    if (mode != GameMode::PvP) {
        // Select AI type based on algorithm
        qDebug() << "GameController: Creating AI, algorithm type:" << algorithm;
        switch (algorithm) {
            case 0: // Minimax
                ai_ = AIStrategyFactory::createMinimaxAI(difficulty);
                break;
            case 1: // MCTS
                ai_ = AIStrategyFactory::createMCTSAI(difficulty);
                break;
            case 2: // Random
            default:
                ai_ = AIStrategyFactory::createRandomAI();
                break;
        }

        qDebug() << "GameController: AI created," << QString::fromStdString(ai_->getName());
        if (!ai_) {
            emit errorOccurred("Failed to create AI");
            return;
        }

        // Set AI search depth
        aiDepth_ = depth;
    } else {
        ai_.reset();  // PvP mode doesn't need AI
    }

    // Determine initial phase (based on who goes first)
    if (mode == GameMode::PvE && humanColor_ == PlayerColor::White) {
        // AI goes first (human is white), AI should move first
        currentPhase_ = GamePhase::AITurn;
        // Delayed AI move to avoid blocking UI
        QTimer::singleShot(500, this, &GameController::executeAIMove);
    } else {
        // Human goes first
        currentPhase_ = GamePhase::HumanTurn;
    }

    // Emit game start signal
    emit gameStarted(mode, humanColor_);
    emit turnChanged(currentPlayer_);
    emit phaseChanged(currentPhase_);

    qDebug() << "GameController: Game started, current player:" << (int)currentPlayer_;
}

void GameController::resetGame() {
    qDebug() << "GameController: Resetting game";

    // Recreate board
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

    // Check if move is valid
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

    // Execute move
    saveMoveForUndo();
    board_->makeMove(move);

    // Emit move signal
    emit moveMade(row, col, currentPlayer_);

    // Switch turn
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

    // Switch player
    currentPlayer_ = (currentPlayer_ == PlayerColor::Black) ?
                     PlayerColor::White : PlayerColor::Black;

    // Check if current player has valid moves
    bool currentHasMoves = !board_->getValidMoves().empty();

    // Check if opponent has valid moves
    PlayerColor opponent = (currentPlayer_ == PlayerColor::Black) ?
                           PlayerColor::White : PlayerColor::Black;

    // Temporarily switch to opponent to check for moves
    bool opponentHasMoves;
    const auto& originalTurn = board_->getCurrentTurn();
    // Note: Here we directly check valid moves for current player on current board state
    // For complete implementation, need to consider pass scenario

    qDebug() << "GameController: Current player" << (int)currentPlayer_
             << "has" << board_->getValidMoves().size() << "valid moves";

    // Check if game is over
    if (board_->isGameOver()) {
        checkGameOver();
        return;
    }

    // If current player has no valid moves, pass
    if (!currentHasMoves) {
        qDebug() << "GameController: Player" << (int)currentPlayer_ << "passes";
        // Switch back to opponent
        currentPlayer_ = opponent;

        // Check game over again
        if (board_->getValidMoves().empty()) {
            checkGameOver();
            return;
        }

        // Note: Don't send turnChanged here, wait until phase is updated
        // to avoid UI using old phase value to draw board
    }

    // Determine next phase
    // In PvP mode, both players are human, all turns are HumanTurn
    // In PvE mode, only humanColor_ player's turns are HumanTurn, AI turns are AITurn
    bool isHumanTurn = (gameMode_ == GameMode::PvP) || (currentPlayer_ == humanColor_);

    currentPhase_ = isHumanTurn ? GamePhase::HumanTurn : GamePhase::AITurn;

    qDebug() << "GameController: Turn switched to" << (int)currentPlayer_
             << ", phase:" << (int)currentPhase_;

    emit turnChanged(currentPlayer_);
    emit phaseChanged(currentPhase_);

    // If AI turn, schedule AI move
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

    // Set search limits
    SearchLimits limits = SearchLimits::createDefault();
    limits.maxDepth = aiDepth_;  // Use configured search depth

    // Find best move
    Move bestMove = ai_->findBestMove(*board_, limits);

    qDebug() << "GameController: AI selected move"
             << bestMove.row << "," << bestMove.col;

    // Update statistics
    AIStats stats = ai_->getStats();
    emit aiStatsUpdated(stats);

    // Emit AI thinking finished signal
    emit aiThinkingFinished(bestMove.row, bestMove.col);

    // Execute move
    saveMoveForUndo();
    board_->makeMove(bestMove);

    // Emit move signal
    emit moveMade(bestMove.row, bestMove.col, currentPlayer_);

    // Switch turn
    switchTurn();
}

void GameController::saveMoveForUndo() {
    // Only keep recent N steps
    if (moveHistory_.size() >= MAX_UNDO_STEPS) {
        moveHistory_.erase(moveHistory_.begin());
    }
    // Save copy of current board
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

    // Restore previous state
    board_ = std::make_unique<Board>(*moveHistory_.back());
    moveHistory_.pop_back();

    // Restore current player - get from restored board directly
    currentPlayer_ = board_->getCurrentTurn();

    // Update phase
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
