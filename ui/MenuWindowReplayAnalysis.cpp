/**
 * @file MenuWindowReplayAnalysis.cpp
 * @brief Replay analysis button slot implementation - v0.9.0
 *
 * Implemented separately to avoid header file circular dependencies
 */

#include "ui/MenuWindow.h"
#include "ui/ReplayAnalysisWindow.h"
#include <QDebug>

namespace Ui { class ReplayAnalysisWindow; }

void MenuWindow::onReplayAnalysisButtonClicked() {
    qDebug() << "MenuWindow: Starting Replay Analysis Mode";
    emit startReplayAnalysis();

    // Create and show replay analysis window
    Reversi::ReplayAnalysisWindow* replayWindow = new Reversi::ReplayAnalysisWindow(this);
    replayWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(replayWindow, &Reversi::ReplayAnalysisWindow::backToMenu, this, [this, replayWindow]() {
        replayWindow->close();
        this->show();
    });

    replayWindow->show();
    this->hide();
}
