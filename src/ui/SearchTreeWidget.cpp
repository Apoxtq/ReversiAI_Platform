/**
 * @file SearchTreeWidget.cpp
 * @brief Search tree visualization component implementation - v0.9.0 visualization enhancement
 */

#include "ui/SearchTreeWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>

namespace Reversi {

SearchTreeWidget::SearchTreeWidget(QWidget* parent)
    : QWidget(parent)
    , maxDisplayDepth_(10)
    , autoScroll_(true)
    , hasData_(false)
{
    setMinimumHeight(200);
    setAttribute(Qt::WA_TranslucentBackground);
}

void SearchTreeWidget::updateSearchTree(const SearchStats& stats) {
    currentStats_ = stats;
    hasData_ = true;
    update();
}

void SearchTreeWidget::clear() {
    currentStats_.reset();
    hasData_ = false;
    update();
}

void SearchTreeWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background
    painter.fillRect(rect(), QColor(43, 43, 43));

    if (!hasData_) {
        // Show placeholder text
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "No search data");
        return;
    }

    drawSearchTree(painter);
}

void SearchTreeWidget::drawSearchTree(QPainter& painter) {
    int width = this->width();
    int height = this->height();

    // Draw title
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));
    QString title = QString("Search Tree - Depth: %1, Nodes: %2")
                        .arg(currentStats_.maxDepth)
                        .arg(currentStats_.nodesExplored);
    painter.drawText(10, 20, title);

    // Simplified display: show search statistics as alternative to tree
    // Full implementation requires traversing actual search tree nodes
    int y = 50;
    int lineHeight = 20;

    // Show PV line information
    painter.setPen(QColor(255, 215, 0)); // Gold
    painter.drawText(20, y, "PV Line:");
    y += lineHeight;

    if (!currentStats_.pvLine.empty()) {
        for (size_t i = 0; i < currentStats_.pvLine.size() && i < 10; ++i) {
            const auto& node = currentStats_.pvLine[i];
            int col = node.move % 8;
            int row = node.move / 8;
            QString coord = QString(QChar('A' + col)) + QString::number(row + 1);
            QString nodeText = QString("  %1. %2 (value: %3)")
                                   .arg(i + 1)
                                   .arg(coord)
                                   .arg(node.value);
            painter.drawText(30, y, nodeText);
            y += lineHeight;
        }
    } else {
        // If no PV info, show basic info
        painter.setPen(QColor(200, 200, 200));
        painter.drawText(30, y, "  (searching...)");
        y += lineHeight;
    }

    y += lineHeight;

    // Show statistics summary
    painter.setPen(Qt::white);
    painter.drawText(20, y, "Statistics:");
    y += lineHeight;

    painter.setPen(QColor(180, 180, 180));
    painter.drawText(30, y, QString("  Nodes: %1").arg(currentStats_.nodesExplored));
    y += lineHeight;

    painter.drawText(30, y, QString("  NPS: %1/s").arg(currentStats_.getNPSString()));
    y += lineHeight;

    painter.drawText(30, y, QString("  Time: %1s").arg(currentStats_.getTimeString()));
    y += lineHeight;

    painter.drawText(30, y, QString("  TT Hit Rate: %1%").arg(currentStats_.ttHitRate * 100, 0, 'f', 1));
    y += lineHeight;

    painter.drawText(30, y, QString("  Cutoffs: %1").arg(currentStats_.cutoffs));
}

void SearchTreeWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // Simplified implementation: emit node clicked signal on click
        // Full implementation requires detecting which node was clicked
        emit nodeClicked(-1);
    }
    QWidget::mousePressEvent(event);
}

} // namespace Reversi
