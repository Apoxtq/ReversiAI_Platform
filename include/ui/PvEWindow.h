/**
 * @file PvEWindow.h
 * @brief PvE (Player vs AI) game window
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
#include <QRadioButton>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QPixmap>
#include <memory>
#include "Board.h"
#include "ai/AIStrategy.h"
#include "ui/GameController.h"

class PvEWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PvEWindow(QWidget* parent = nullptr);
    ~PvEWindow() override;

signals:
    void backToMenu();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* e) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onStartGameClicked();
    void onNewGameClicked();
    void onGameStarted(Reversi::GameMode mode, Reversi::PlayerColor humanColor);
    void onPhaseChanged(Reversi::GamePhase phase);
    void onTurnChanged(Reversi::PlayerColor player);
    void onMoveMade(int row, int col, Reversi::PlayerColor player);
    void onGameEnded(Reversi::GameResult result);
    void onAIThinkingStarted(const QString& aiName);
    void onAIThinkingFinished(int row, int col);
    void onAIStatsUpdated(const Reversi::AIStats& stats);
    void onErrorOccurred(const QString& message);
    void onBackToMenuClicked();
    void onDelayTimerTimeout();

private:
    void setupUI();
    void setupAIConfig();
    void setupControls();
    void setupConnections();
    void updateScoreDisplay();
    void updateButtonStates();
    void loadResources();
    void initGame();

    // Board dimensions
    static constexpr int CELL_SIZE = 60;
    static constexpr int BOARD_SIZE = CELL_SIZE * 8;
    static constexpr int BOARD_OFFSET_X = 50;
    static constexpr int BOARD_OFFSET_Y = 30;

    // UI Components
    QWidget* centralWidget_;
    QVBoxLayout* leftLayout_;
    QVBoxLayout* rightLayout_;

    // Board display
    QLabel* boardLabel_;
    QPixmap pixmapBackground_, pixmapWhite_, pixmapBlack_;
    QPixmap pixmapHintWhite_, pixmapHintBlack_, pixmapHintRed_;

    // Scale factors
    double boardScale_;

    // Info display
    QLabel* resultLabel_;           // Victory announcement
    QLabel* turnIndicator_;
    QLabel* blackScoreLabel_;
    QLabel* whiteScoreLabel_;

    // AI Configuration
    QGroupBox* aiConfigGroup_;
    QRadioButton* aiFirstRadio_;
    QRadioButton* playerFirstRadio_;
    QComboBox* algorithmCombo_;
    QComboBox* difficultyCombo_;
    QComboBox* depthCombo_;
    QComboBox* delayCombo_;

    // Control buttons
    QGroupBox* controlsGroup_;
    QPushButton* startButton_;
    QPushButton* newGameButton_;
    QPushButton* backButton_;

    // Game state
    enum class GameWatchState { IDLE, PLAYING, PAUSED, GAME_OVER };
    GameWatchState gameState_;
    Reversi::Board board_;
    Reversi::PlayerColor currentPlayer_;
    Reversi::GameResult gameResult_;

    // Game controller
    std::unique_ptr<Reversi::GameController> gameController_;

    // Move delay
    int moveDelayMs_;
    QTimer* moveDelayTimer_;
};