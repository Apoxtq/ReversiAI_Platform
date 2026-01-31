#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MenuWindow;
}
QT_END_NAMESPACE

/**
 * @class MenuWindow
 * @brief 目录界面 - 游戏模式选择
 *
 * 功能:
 * - 显示游戏标题
 * - 提供人机对战 (PvE) 和双人对战 (PvP) 入口
 * - 点击按钮后打开对应游戏窗口
 */
class MenuWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MenuWindow(QWidget* parent = nullptr);
    ~MenuWindow() override;

signals:
    void startPvEGame();    // 开始人机对战
    void startPvPGame();    // 开始双人对战

private slots:
    void onPvEButtonClicked();
    void onPvPButtonClicked();

private:
    Ui::MenuWindow* ui;
    void setupConnections();
};

