/**
 * @file MenuWindowAIResearch.cpp
 * @brief AI研究模式按钮槽函数实现
 *
 * 单独实现以避免头文件循环依赖
 */

#include "ui/MenuWindow.h"
#include "ui/AIvsAIWindow.h"
#include <QDebug>

namespace Ui { class AIvsAIWindow; }

void MenuWindow::onAIResearchButtonClicked() {
    qDebug() << "MenuWindow: Starting AI Research Mode";
    emit startAIResearch();

    // 创建并显示AI vs AI对战窗口
    Reversi::AIvsAIWindow* aiWindow = new Reversi::AIvsAIWindow(this);
    aiWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(aiWindow, &Reversi::AIvsAIWindow::backToMenu, this, [this, aiWindow]() {
        aiWindow->close();
        this->show();
    });

    aiWindow->show();
    this->hide();
}

