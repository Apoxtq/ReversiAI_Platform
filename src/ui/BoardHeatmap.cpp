/**
 * @file BoardHeatmap.cpp
 * @brief Board heatmap implementation - v0.9.0 visualization enhancement
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
    // Color mapping is generated dynamically in getHeatColor
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
    // Clamp value to 0-1 range
    double v = qBound(0.0, value, 1.0);

    switch (currentType_) {
        case HeatmapType::PositionValue:
        case HeatmapType::ActionValue:
            // Blue (low) -> Red (high)
            return QColor::fromRgb(
                static_cast<int>(255 * v),
                0,
                static_cast<int>(255 * (1.0 - v)),
                static_cast<int>(255 * opacity_)
            );

        case HeatmapType::VisitCount:
            // White (low) -> Dark blue (high)
            return QColor::fromRgb(
                static_cast<int>(255 * (1.0 - v)),
                static_cast<int>(255 * (1.0 - v)),
                255,
                static_cast<int>(255 * opacity_)
            );

        case HeatmapType::WinRate:
            // Red (0%) -> Yellow (50%) -> Green (100%)
            if (v < 0.5) {
                // Red -> Yellow
                double t = v * 2.0;
                return QColor::fromRgb(
                    255,
                    static_cast<int>(255 * t),
                    0,
                    static_cast<int>(255 * opacity_)
                );
            } else {
                // Yellow -> Green
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

    // Get board dimensions
    int width = this->width();
    int height = this->height();

    // Calculate cell size
    int cellWidth = width / 8;
    int cellHeight = height / 8;

    // Draw heatmap for each cell
    for (int i = 0; i < 64; ++i) {
        double value = heatmapData_[i];
        if (value <= 0.0) continue; // Skip cells with no data

        int row = i / 8;
        int col = i % 8;

        QRect cellRect(col * cellWidth, row * cellHeight, cellWidth, cellHeight);

        // Shrink slightly to show cell border
        cellRect.adjust(1, 1, -1, -1);

        // Draw heatmap
        painter.fillRect(cellRect, getHeatColor(value));
    }
}

} // namespace Reversi
