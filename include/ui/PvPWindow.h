/**
 * @file PvPWindow.h
 * @brief 双人对战窗口
 *
 * 功能:
 * - 双人同机对战
 * - 轮流执黑/白棋
 * - 悔棋功能
 * - 返回菜单功能
 */

#ifndef PVPWINDOW_H
#define PVPWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QPixmap>
#include "ui/GameController.h"

// 前向声明，避免循环引用
namespace Ui {
class PvPWindow;
}

namespace Reversi {
class GameController;
}

class PvPWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PvPWindow(QWidget* parent = nullptr);
    ~PvPWindow() override;

signals:
    void backToMenu();  // 返回菜单

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* e) override;

private slots:
    void onStartGameClicked();
    void onUndoClicked();
    void onGameStarted(Reversi::GameMode mode, Reversi::PlayerColor humanColor);
    void onPhaseChanged(Reversi::GamePhase phase);
    void onTurnChanged(Reversi::PlayerColor player);
    void onMoveMade(int row, int col, Reversi::PlayerColor player);
    void onGameEnded(Reversi::GameResult result);
    void onErrorOccurred(const QString& message);
    void onBackToMenuClicked();

private:
    void initUI();
    void setupConnections();
    void updateScoreDisplay();

    Ui::PvPWindow* ui;
    std::unique_ptr<Reversi::GameController> gameController_;

    QPixmap background;
    QPixmap white, black;
    QPixmap hintwhite, hintblack, hintred;
};

#endif // PVPWINDOW_H

