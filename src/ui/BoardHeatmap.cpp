/**
 * @file BoardHeatmap.cpp
 * @brief 棋盘热度图实现 - v0.9.0可视化增强版
 */

#include "ui/BoardHeatmap.h"
#include <QDebug>
#include <cmath>

namespace Reversi {

BoardHeatmap::BoardHeatmap(QWidget* parent)
    : QWidget(parent)
    , currentType_(HeatmapType::PositionValue)
    , opacity_(0.5)
    , isVisible_(true)
    , hasData_(false)
{
    heatmapData_.resize(64, 0.0);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    initColorMaps();
}

void BoardHeatmap::initColorMaps() {
    // 颜色映射在getHeatColor中动态生成
}

void BoardHeatmap::setHeatmapData(const QVector<double>& values, HeatmapType type) {
    if (values.size() != 64) {
        qWarning() << "BoardHeatmap: Invalid data size" << values.size();
        return;
    }

    heatmapData_ = values;
    currentType_ = type;
    hasData_ = true;
    update();
}

void BoardHeatmap::setHeatmapType(HeatmapType type) {
    currentType_ = type;
    emit heatmapTypeChanged(type);
    update();
}

void BoardHeatmap::clearHeatmap() {
    heatmapData_.fill(0.0);
    hasData_ = false;
    update();
}

void BoardHeatmap::setHeatmapOpacity(double opacity) {
    opacity_ = qBound(0.0, opacity, 1.0);
    update();
}

QColor BoardHeatmap::getHeatColor(double value) const {
    // 将值限制在0-1之间
    double v = qBound(0.0, value, 1.0);

    switch (currentType_) {
        case HeatmapType::PositionValue:
        case HeatmapType::ActionValue:
            // 蓝色(低) -> 红色(高)
            return QColor::fromRgb(
                static_cast<int>(255 * v),
                0,
                static_cast<int>(255 * (1.0 - v)),
                static_cast<int>(255 * opacity_)
            );

        case HeatmapType::VisitCount:
            // 白色(低) -> 深蓝(高)
            return QColor::fromRgb(
                static_cast<int>(255 * (1.0 - v)),
                static_cast<int>(255 * (1.0 - v)),
                255,
                static_cast<int>(255 * opacity_)
            );

        case HeatmapType::WinRate:
            // 红色(0%) -> 黄色(50%) -> 绿色(100%)
            if (v < 0.5) {
                // 红色 -> 黄色
                double t = v * 2.0;
                return QColor::fromRgb(
                    255,
                    static_cast<int>(255 * t),
                    0,
                    static_cast<int>(255 * opacity_)
                );
            } else {
                // 黄色 -> 绿色
                double t = (v - 0.5) * 2.0;
                return QColor::fromRgb(
                    static_cast<int>(255 * (1.0 - t)),
                    255,
                    0,
                    static_cast<int>(255 * opacity_)
                );
            }
    }

    return QColor(0, 0, 0, 0);
}

void BoardHeatmap::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    if (!isVisible_ || !hasData_) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 获取棋盘大小
    int width = this->width();
    int height = this->height();

    // 计算格子大小
    int cellWidth = width / 8;
    int cellHeight = height / 8;

    // 绘制每个格子的热度
    for (int i = 0; i < 64; ++i) {
        double value = heatmapData_[i];
        if (value <= 0.0) continue; // 跳过无数据的格子

        int row = i / 8;
        int col = i % 8;

        QRect cellRect(col * cellWidth, row * cellHeight, cellWidth, cellHeight);

        // 缩小一点以便显示格子边框
        cellRect.adjust(1, 1, -1, -1);

        // 绘制热度
        painter.fillRect(cellRect, getHeatColor(value));
    }
}

} // namespace Reversi
