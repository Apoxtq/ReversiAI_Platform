/**
 * @file mainwindow.h
 * @brief 主窗口 - V0.4.0重构版本
 *
 * 重构内容:
 * - 移除对MCTS.h的引用（已删除）
 * - 使用GameController管理游戏状态
 * - 移除重复的Board类定义
 * - 移除全局变量
 * - 基于QtReversi/widget.h的信号槽设计
 *
 * @reference QtReversi/代码/chess/widget.h - 游戏状态定义和信号槽设计
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
 * @brief 主窗口类 - 负责UI显示和用户交互
 *
 * 职责:
 * - 棋盘绘制 (paintEvent)
 * - 鼠标事件处理 (mousePressEvent)
 * - 连接GameController信号槽
 *
 * 状态管理委托给GameController处理
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    /**
     * @brief 绘制棋盘和棋子
     * 保留原有实现逻辑，只修改数据来源
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief 处理鼠标点击事件
     * 将落子操作委托给GameController处理
     */
    void mousePressEvent(QMouseEvent *e) override;

private slots:
    // ============ UI信号槽 ============

    /**
     * @brief 开始/重置游戏按钮点击
     */
    void onStartGameClicked();

    /**
     * @brief 游戏开始信号处理
     */
    void onGameStarted(Reversi::GameMode mode, Reversi::PlayerColor humanColor);

    /**
     * @brief 游戏阶段变化信号处理
     */
    void onPhaseChanged(Reversi::GamePhase phase);

    /**
     * @brief 回合变化信号处理
     */
    void onTurnChanged(Reversi::PlayerColor player);

    /**
     * @brief 落子信号处理
     */
    void onMoveMade(int row, int col, Reversi::PlayerColor player);

    /**
     * @brief 游戏结束信号处理
     */
    void onGameEnded(Reversi::GameResult result);

    /**
     * @brief AI开始思考信号处理
     */
    void onAIThinkingStarted(const QString& aiName);

    /**
     * @brief AI思考完成信号处理
     */
    void onAIThinkingFinished(int row, int col);

    /**
     * @brief AI统计更新信号处理
     */
    void onAIStatsUpdated(const Reversi::AIStats& stats);

    /**
     * @brief 错误信号处理
     */
    void onErrorOccurred(const QString& message);

private:
    // ============ 私有方法 ============

    /**
     * @brief 初始化UI组件
     * 保留原有init()调用
     */
    void initUI();

    /**
     * @brief 连接GameController信号槽
     */
    void setupConnections();

    /**
     * @brief 更新分数显示
     */
    void updateScoreDisplay();

    // ============ 成员变量 ============

    Ui::MainWindow *ui;                                     // Qt Designer生成的UI
    std::unique_ptr<Reversi::GameController> gameController_; // 游戏控制器

    // ============ 保留的资源文件 ============
    // 这些资源文件从原有代码保留
    QPixmap background;
    QPixmap white, black;
    QPixmap hintwhite, hintblack, hintred;

    // ============ 移除的全局变量 ============
    // 以下全局变量已移除，改为由GameController管理
    // - bool mousedown ❌ 删除
    // - int mousex, mousey ❌ 删除
    // - Board* mainBoard ❌ 由GameController管理
    // - MCTS* root ❌ 使用AIStrategy
    // - std::string turn ❌ 使用GamePhase枚举
    // - bool gameOver ❌ 由GameController管理
    // - int playerTile, computerTile ❌ 由GameController管理
};

#endif // MAINWINDOW_H
