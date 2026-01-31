/**
 * @file PvEWindow.h
 * @brief 人机对战窗口
 *
 * 功能:
 * - 人机对战模式
 * - AI难度选择
 * - 先手/后手选择
 * - 返回菜单功能
 */

#ifndef PVEWINDOW_H
#define PVEWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QPixmap>
#include "ui/GameController.h"

// 前向声明，避免循环引用
namespace Ui {
class PvEWindow;
}

namespace Reversi {
class GameController;
}

class PvEWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PvEWindow(QWidget* parent = nullptr);
    ~PvEWindow() override;

signals:
    void backToMenu();  // 返回菜单

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* e) override;

private slots:
    void onStartGameClicked();
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

private:
    void initUI();
    void setupConnections();
    void updateScoreDisplay();

    Ui::PvEWindow* ui;
    std::unique_ptr<Reversi::GameController> gameController_;

    QPixmap background;
    QPixmap white, black;
    QPixmap hintwhite, hintblack, hintred;
};

#endif // PVEWINDOW_H

