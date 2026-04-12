/**
 * @file AIWatchWindow.h
 * @brief Single AI vs AI battle watch window
 *
 * Provides a visual interface for watching a single AI vs AI battle:
 * - Select AI types for both players (Minimax, MCTS, Random)
 * - Configure AI difficulty and search depth
 * - Set move delay for viewing
 * - Pause/Resume/Restart functionality
 *
 * @date 2026
 * @author Project Team
 * @license GPL-3.0
 */

#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QPixmap>
#include <QCloseEvent>
#include <QMessageBox>
#include <QApplication>
#include <memory>
#include <stack>

#include "Board.h"
#include "ai/AIStrategy.h"
#include "ui/GameController.h"

/**
 * @namespace Reversi
 * @brief Main namespace for Reversi game
 */
namespace Reversi {

/**
 * @enum GameWatchState
 * @brief Game state for watch mode
 */
enum class GameWatchState {
    IDLE,      ///< Game not started
    PLAYING,   ///< Game in progress
    PAUSED,    ///< Game paused
    FINISHED   ///< Game ended
};

/**
 * @struct AIWatchConfig
 * @brief Configuration for AI in watch mode
 */
struct AIWatchConfig {
    enum class AIType { MINIMAX, MCTS, RANDOM };

    AIType type = AIType::MINIMAX;
    Difficulty difficulty = Difficulty::MEDIUM;
    int depth = 4;
};

/**
 * @class AIWatchWindow
 * @brief Window for watching single AI vs AI battle
 */
class AIWatchWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit AIWatchWindow(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~AIWatchWindow() override;

signals:
    /**
     * @brief Signal emitted when user wants to return to menu
     */
    void backToMenu();

protected:
    /**
     * @brief Override paint event to draw the board
     */
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief Handle window close event
     */
    void closeEvent(QCloseEvent* event) override;

private slots:
    /**
     * @brief Handle Start button click
     */
    void onStartClicked();

    /**
     * @brief Handle Pause/Resume button click
     */
    void onPauseResumeClicked();

    /**
     * @brief Handle New Game button click
     */
    void onNewGameClicked();

    /**
     * @brief Handle Back button click
     */
    void onBackClicked();

    /**
     * @brief Handle AI configuration change
     */
    void onAIConfigChanged();

    /**
     * @brief Execute the next move in the game
     */
    void playNextMove();

private:
    // ==================== UI Setup ====================

    /**
     * @brief Setup the main UI components
     */
    void setupUI();

    /**
     * @brief Setup AI configuration widgets
     */
    void setupAIConfig();

    /**
     * @brief Setup control buttons
     */
    void setupControls();

    /**
     * @brief Connect signals and slots
     */
    void setupConnections();

    /**
     * @brief Load board images
     */
    void loadResources();

    // ==================== Game Logic ====================

    /**
     * @brief Initialize a new game
     */
    void initGame();

    /**
     * @brief Create AI instances based on configuration
     */
    void createAIs();

    /**
     * @brief Check if current player has valid moves
     * @return true if valid moves exist
     */
    bool hasValidMoves();

    /**
     * @brief Execute a move
     * @param move The move to execute
     */
    void executeMove(const Move& move);

    /**
     * @brief Check if game is over
     * @return true if game ended
     */
    bool isGameOver();

    /**
     * @brief Determine game result
     */
    void determineResult();

    /**
     * @brief Update the board display
     */
    void updateBoardDisplay();

    /**
     * @brief Update turn indicator
     */
    void updateTurnIndicator();

    /**
     * @brief Update score display
     */
    void updateScoreDisplay();

    /**
     * @brief Update button states based on game state
     */
    void updateButtonStates();

    /**
     * @brief Create AI from configuration
     * @param config AI configuration
     * @return Unique pointer to AI strategy
     */
    std::unique_ptr<AIStrategy> createAI(const AIWatchConfig& config);

    // ==================== Constants ====================

    // Board dimensions
    static constexpr int CELL_SIZE = 60;
    static constexpr int BOARD_SIZE = CELL_SIZE * 8;
    static constexpr int BOARD_OFFSET_X = 50;  // Board left margin for centering
    static constexpr int BOARD_OFFSET_Y = 30;

    // Delay options in milliseconds
    static constexpr int DELAY_500MS = 500;
    static constexpr int DELAY_1S = 1000;
    static constexpr int DELAY_2S = 2000;
    static constexpr int DELAY_5S = 5000;

    // ==================== UI Components ====================

    // Central widget
    QWidget* centralWidget_;

    // Layout pointers for internal use
    QVBoxLayout* leftLayout_;
    QVBoxLayout* rightLayout_;

    // Board display
    QLabel* boardLabel_;
    QPixmap background_;     // Board background image
    QPixmap black_;          // Black piece image
    QPixmap white_;          // White piece image
    QPixmap hintwhite_;      // Valid move hint image

    // Info display
    QLabel* turnIndicator_;
    QLabel* blackScoreLabel_;
    QLabel* whiteScoreLabel_;
    QLabel* resultLabel_;

    // AI Configuration - Black
    QGroupBox* blackConfigGroup_;
    QComboBox* blackAlgorithmCombo_;
    QComboBox* blackDifficultyCombo_;
    QComboBox* blackDepthCombo_;

    // AI Configuration - White
    QGroupBox* whiteConfigGroup_;
    QComboBox* whiteAlgorithmCombo_;
    QComboBox* whiteDifficultyCombo_;
    QComboBox* whiteDepthCombo_;

    // Delay configuration
    QComboBox* delayCombo_;

    // Control buttons
    QPushButton* startButton_;
    QPushButton* pauseResumeButton_;  // Combined pause/resume button
    QPushButton* newGameButton_;
    QPushButton* backButton_;

    // ==================== Game State ====================

    GameWatchState gameState_;
    Board board_;
    PlayerColor currentPlayer_;

    // AI instances
    std::unique_ptr<AIStrategy> blackAI_;
    std::unique_ptr<AIStrategy> whiteAI_;
    SearchLimits searchLimits_;

    // AI Configuration
    AIWatchConfig blackConfig_;
    AIWatchConfig whiteConfig_;

    // Move delay in milliseconds
    int moveDelayMs_;

    // Game result
    GameResult gameResult_;

    // History for potential undo feature
    std::stack<Board> boardHistory_;

    // ==================== Timers ====================

    QTimer* moveTimer_;
};

} // namespace Reversi
