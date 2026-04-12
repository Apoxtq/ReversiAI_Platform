/**
 * @file mainwindow.h
 * @brief Main Window - V0.4.0 Refactored Version
 *
 * Refactoring content:
 * - Removed reference to MCTS.h (deleted)
 * - Use GameController to manage game state
 * - Removed duplicate Board class definition
 * - Removed global variables
 * - Signal-slot design based on QtReversi/widget.h
 *
 * @reference QtReversi/代码/chess/widget.h - Game state definition and signal-slot design
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QPixmap>
#include "ui/GameController.h"

// 前向声明，避免循环引用
namespace Ui {
class MainWindow;
}

namespace Reversi {
class GameController;
}

/**
 * @class MainWindow
 * @brief Main Window class - responsible for UI display and user interaction
 *
 * Responsibilities:
 * - Board drawing (paintEvent)
 * - Mouse event handling (mousePressEvent)
 * - Connect GameController signal-slot
 *
 * State management delegated to GameController
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    /**
     * @brief Draw board and pieces
     * Keep original implementation, only modify data source
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief Handle mouse click event
     * Delegate move operation to GameController
     */
    void mousePressEvent(QMouseEvent *e) override;

private slots:
    // ============ UI Signal-Slot ============

    /**
     * @brief Start/Reset game button click
     */
    void onStartGameClicked();

    /**
     * @brief Game start signal handling
     */
    void onGameStarted(Reversi::GameMode mode, Reversi::PlayerColor humanColor);

    /**
     * @brief Game phase change signal handling
     */
    void onPhaseChanged(Reversi::GamePhase phase);

    /**
     * @brief Turn change signal handling
     */
    void onTurnChanged(Reversi::PlayerColor player);

    /**
     * @brief Move made signal handling
     */
    void onMoveMade(int row, int col, Reversi::PlayerColor player);

    /**
     * @brief Game end signal handling
     */
    void onGameEnded(Reversi::GameResult result);

    /**
     * @brief AI started thinking signal handling
     */
    void onAIThinkingStarted(const QString& aiName);

    /**
     * @brief AI thinking completed signal handling
     */
    void onAIThinkingFinished(int row, int col);

    /**
     * @brief AI stats update signal handling
     */
    void onAIStatsUpdated(const Reversi::AIStats& stats);

    /**
     * @brief Error signal handling
     */
    void onErrorOccurred(const QString& message);

private:
    // ============ Private Methods ============

    /**
     * @brief Initialize UI components
     * Keep original init() call
     */
    void initUI();

    /**
     * @brief Connect GameController signal-slot
     */
    void setupConnections();

    /**
     * @brief Update score display
     */
    void updateScoreDisplay();

    // ============ Member Variables ============

    Ui::MainWindow *ui;                                     // Qt Designer generated UI
    std::unique_ptr<Reversi::GameController> gameController_; // Game controller

    // ============ Retained Resource Files ============
    // These resource files retained from original code
    QPixmap background;
    QPixmap white, black;
    QPixmap hintwhite, hintblack, hintred;

    // ============ Removed Global Variables ============
    // The following global variables have been removed, now managed by GameController
    // - bool mousedown (removed)
    // - int mousex, mousey (removed)
    // - Board* mainBoard (managed by GameController)
    // - MCTS* root (use AIStrategy)
    // - std::string turn (use GamePhase enum)
    // - bool gameOver (managed by GameController)
    // - int playerTile, computerTile (managed by GameController)
};

#endif // MAINWINDOW_H
