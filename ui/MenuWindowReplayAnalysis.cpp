/**
 * @file MenuWindowReplayAnalysis.cpp
 * @brief 复盘分析按钮槽函数实现 - v0.9.0
 *
 * 单独实现以避免头文件循环依赖
 */

#include "ui/MenuWindow.h"
#include "ui/ReplayAnalysisWindow.h"
#include <QDebug>

namespace Ui { class ReplayAnalysisWindow; }

void MenuWindow::onReplayAnalysisButtonClicked() {
    qDebug() << "MenuWindow: Starting Replay Analysis Mode";
    emit startReplayAnalysis();

    // 创建并显示复盘分析窗口
    Reversi::ReplayAnalysisWindow* replayWindow = new Reversi::ReplayAnalysisWindow(this);
    replayWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(replayWindow, &Reversi::ReplayAnalysisWindow::backToMenu, this, [this, replayWindow]() {
        replayWindow->close();
        this->show();
    });

    replayWindow->show();
    this->hide();
}
