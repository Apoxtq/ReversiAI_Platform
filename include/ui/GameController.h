/**
 * @file GameController.h
 * @brief Game State Controller
 *
 * Encapsulates game state management, turn control, and AI integration
 * Based on QtReversi/widget.h state design pattern
 *
 * @reference QtReversi/代码/chess/widget.h - Game state definition and signal-slot design
 * @reference Reversi(Java)/src/game/GamePanel.java - manageTurn() turn management logic
 */

#pragma once

#include <QObject>
#include <memory>
#include <vector>
#include <chrono>

// Include core type definitions (within Reversi namespace)
#include "Board.h"
#include "ai/AIStrategy.h"

// Forward declaration to avoid circular reference
namespace Ui {
class MainWindow;
}

namespace Reversi {

// 前向声明Board
class Board;

// 前向声明AIStrategy
class AIStrategy;

/**
 * @enum GameMode
 * @brief Game mode
 */
enum class GameMode {
    PvP,      // Two-player (same computer)
    PvE,      // Human vs AI
    AIvAI     // AI vs AI
};

/**
 * @enum GamePhase
 * @brief Game phase
 */
enum class GamePhase {
    Waiting,    // Waiting to start
    HumanTurn,  // Human player turn
    AITurn,     // AI turn
    GameOver    // Game over
};

/**
 * @enum GameResult
 * @brief Game result
 */
enum class GameResult {
    BlackWins,
    WhiteWins,
    Draw,
    Unknown
};

/**
 * @class GameController
 * @brief Game Controller - Encapsulates game state management
 *
 * Core responsibilities:
 * - Manage game state (board, turn, phase)
 * - Handle player operations (human moves)
 * - Coordinate AI execution (AI moves, state display)
 * - Turn switching logic
 *
 * Signal-slot design reference QtReversi/widget.cpp:
 * - Uses Qt signal-slot mechanism to decouple UI and logic
 * - Emits signals on state changes, UI updates display accordingly
 */
class GameController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent object
     */
    explicit GameController(QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~GameController() override;

    // ============ Game Control ============

    /**
     * @brief Start a new game
     * @param mode Game mode
     * @param humanColor Human player color
     * @param difficulty AI difficulty (effective for PvE/AIvAI mode)
     * @param algorithm AI algorithm type (0=Minimax, 1=MCTS, 2=Random)
     * @param depth Search depth (2-6)
     *
     * @reference QtReversi/widget.cpp:changeRole() - Role switching logic
     */
    void startNewGame(GameMode mode, PlayerColor humanColor,
                      Difficulty difficulty = Difficulty::MEDIUM,
                      int algorithm = 0,
                      int depth = 4);

    /**
     * @brief Reset game
     */
    void resetGame();

    /**
     * @brief Undo move
     */
    void undoMove();

    /**
     * @brief Check if undo is possible
     */
    bool canUndo() const;

    // ============ State Query ============

    const Board& getBoard() const { return *board_; }
    Board& getBoard() { return *board_; }  ///< Non-const getter for state sync
    GamePhase getCurrentPhase() const { return currentPhase_; }
    GameMode getGameMode() const { return gameMode_; }
    PlayerColor getCurrentPlayer() const { return currentPlayer_; }
    PlayerColor getHumanColor() const { return humanColor_; }

    /**
     * @brief Get valid moves list
     */
    std::vector<Move> getValidMoves() const;

    // ============ Player Operations ============

    /**
     * @brief Human player makes a move
     * @param row Row (0-7)
     * @param col Column (0-7)
     * @return Whether move was successful
     *
     * @reference QtReversi/widget.cpp:mousePressEvent() - Mouse click handling
     * @reference Reversi(Java)/GamePanel.java:makeHumanMove() - Human move logic
     */
    bool makeHumanMove(int row, int col);

    /**
     * @brief Check if position is valid for move
     */
    bool isValidMove(int row, int col) const;

    /**
     * @brief Set current player (for network sync)
     * @param player Player color to set
     */
    void setCurrentPlayer(PlayerColor player) { currentPlayer_ = player; }

    /**
     * @brief Set game phase (for network sync)
     * @param phase Phase to set
     */
    void setGamePhase(GamePhase phase) { currentPhase_ = phase; }

signals:
    // ============ State Change Signals ============

    /**
     * @brief Game start signal
     */
    void gameStarted(GameMode mode, PlayerColor humanColor);

    /**
     * @brief Game phase change signal
     */
    void phaseChanged(GamePhase phase);

    /**
     * @brief Turn change signal
     */
    void turnChanged(PlayerColor player);

    /**
     * @brief Move made signal
     * @param row Row
     * @param col Column
     * @param player Player who made the move
     */
    void moveMade(int row, int col, PlayerColor player);

    /**
     * @brief Game end signal
     */
    void gameEnded(GameResult result);

    // ============ AI-Related Signals ============

    /**
     * @brief AI started thinking signal
     */
    void aiThinkingStarted(const QString& aiName);

    /**
     * @brief AI thinking completed signal
     */
    void aiThinkingFinished(int row, int col);

    /**
     * @brief AI stats update signal
     */
    void aiStatsUpdated(const AIStats& stats);

    // ============ Error Signals ============

    /**
     * @brief Error occurred signal
     */
    void errorOccurred(const QString& message);

private slots:
    /**
     * @brief Execute AI move
     */
    void executeAIMove();

private:
    // ============ Private Methods ============

    /**
     * @brief Switch turn
     *
     * @reference Reversi(Java)/GamePanel.java:manageTurn() - Turn management logic
     */
    void switchTurn();

    /**
     * @brief Check if game is over
     */
    void checkGameOver();

    /**
     * @brief Save move history (for undo)
     */
    void saveMoveForUndo();

    /**
     * @brief Emit state change signals
     */
    void emitStatusSignals();

    // ============ Member Variables ============

    std::unique_ptr<Board> board_;           // Board state
    std::unique_ptr<AIStrategy> ai_;         // AI strategy (PvE/AIvAI mode)
    GameMode gameMode_;                      // Game mode
    GamePhase currentPhase_;                 // Current phase
    PlayerColor currentPlayer_;              // Current player
    PlayerColor humanColor_;                 // Human player color

    // Undo support (up to 10 moves)
    std::vector<std::unique_ptr<Board>> moveHistory_;

    // AI search depth
    int aiDepth_ = 4;

    // Disable copy
    Q_DISABLE_COPY(GameController)
};

} // namespace Reversi

