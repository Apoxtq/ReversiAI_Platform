/**
 * @file SearchTreeWidget.cpp
 * @brief 搜索树可视化组件实现 - v0.9.0可视化增强版
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

    // 背景
    painter.fillRect(rect(), QColor(43, 43, 43));

    if (!hasData_) {
        // 显示提示文字
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "No search data");
        return;
    }

    drawSearchTree(painter);
}

void SearchTreeWidget::drawSearchTree(QPainter& painter) {
    int width = this->width();
    int height = this->height();

    // 绘制标题
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));
    QString title = QString("Search Tree - Depth: %1, Nodes: %2")
                        .arg(currentStats_.maxDepth)
                        .arg(currentStats_.nodesExplored);
    painter.drawText(10, 20, title);

    // 简化显示：显示搜索统计信息作为树的替代
    // 完整实现需要遍历实际的搜索树节点
    int y = 50;
    int lineHeight = 20;

    // 显示PV路线信息
    painter.setPen(QColor(255, 215, 0)); // 金色
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
        // 如果没有PV信息，显示基本信息
        painter.setPen(QColor(200, 200, 200));
        painter.drawText(30, y, "  (searching...)");
        y += lineHeight;
    }

    y += lineHeight;

    // 显示统计摘要
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
        // 简化实现：点击时发出节点点击信号
        // 完整实现需要检测点击位置对应的节点
        emit nodeClicked(-1);
    }
    QWidget::mousePressEvent(event);
}

} // namespace Reversi
