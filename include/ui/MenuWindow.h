#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MenuWindow;
}
QT_END_NAMESPACE

// Forward declarations
class NetworkLobbyWindow;
class GGSGameWindow;

/**
 * @class MenuWindow
 * @brief 目录界面 - 游戏模式选择
 *
 * 功能:
 * - 显示游戏标题
 * - 提供人机对战 (PvE)、双人对战 (PvP) 和网络对战 (PvN) 入口
 * - 提供AI研究模式入口
 * - 提供对局复盘分析入口
 * - 点击按钮后打开对应游戏窗口
 */
class MenuWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MenuWindow(QWidget* parent = nullptr);
    ~MenuWindow() override;

signals:
    void startPvEGame();      // 开始人机对战
    void startPvPGame();      // 开始双人对战
    void startPvNGame();      // 开始网络对战
    void startAIResearch();   // 开始AI研究模式
    void startReplayAnalysis(); // v0.9.0: 开始复盘分析

private slots:
    void onPvEButtonClicked();
    void onPvPButtonClicked();
    void onNetworkButtonClicked();  // 网络对战按钮
    void onAIResearchButtonClicked();  // AI研究模式按钮
    void onReplayAnalysisButtonClicked();  // v0.9.0: 复盘分析按钮

private:
    Ui::MenuWindow* ui;
    void setupConnections();
};

