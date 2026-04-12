/**
 * @file BoardHeatmap.h
 * @brief Board Heatmap - v0.9.0 Visualization Enhanced
 *
 * Overlay display of position value, visit count, win rate heatmap on board
 */

#pragma once

#include <QWidget>
#include <QPainter>
#include <QVector>
#include <QColor>
#include <QLabel>

#include "research/SearchStats.h"

namespace Reversi {

/**
 * @class BoardHeatmap
 * @brief Board heatmap component
 *
 * Overlay display of various heatmap data on board
 */
class BoardHeatmap : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent window pointer
     */
    explicit BoardHeatmap(QWidget* parent = nullptr);

    /**
     * @brief Set heatmap data
     * @param values Values for 64 cells (0.0 - 1.0)
     * @param type Heatmap type
     */
    void setHeatmapData(const QVector<double>& values, HeatmapType type);

    /**
     * @brief Set heatmap data type
     * @param type Data type
     */
    void setHeatmapType(HeatmapType type);

    /**
     * @brief Clear heatmap data
     */
    void clearHeatmap();

    /**
     * @brief Set opacity
     * @param opacity Opacity (0.0 - 1.0)
     */
    void setHeatmapOpacity(double opacity);

    /**
     * @brief Get current heatmap type
     */
    HeatmapType getHeatmapType() const { return currentType_; }

    /**
     * @brief Whether to show heatmap
     */
    bool isVisible() const { return isVisible_; }

    /**
     * @brief Show/hide heatmap
     */
    void setVisible(bool visible) { isVisible_ = visible; update(); }

signals:
    /**
     * @brief Heatmap type change signal
     */
    void heatmapTypeChanged(HeatmapType type);

protected:
    /**
     * @brief Paint event
     */
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief Get heat color
     * @param value Value (0.0 - 1.0)
     * @return Color
     */
    QColor getHeatColor(double value) const;

private:
    /**
     * @brief Initialize color maps
     */
    void initColorMaps();

    QVector<double> heatmapData_;      // Heatmap data
    HeatmapType currentType_;          // Current type
    double opacity_;                    // Opacity
    bool isVisible_;                   // Whether to display
    bool hasData_;                     // Has data
};

} // namespace Reversi
