/**
 * @brief Search tree visualization component - v0.9.0 Visualization Enhanced
 *
 * Displays AI search tree depth, node count, branch information
 * Note: This is a simplified version, full version requires more complex drawing logic
 */

#pragma once

#include <QWidget>
#include <QPainter>
#include <QVector>
#include <QPoint>

#include "research/SearchStats.h"

namespace Reversi {

/**
 * @class SearchTreeWidget
 * @brief Search tree visualization widget
 *
 * Displays graphical representation of search tree
 */
class SearchTreeWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent window pointer
     */
    explicit SearchTreeWidget(QWidget* parent = nullptr);

    /**
     * @brief Update search tree data
     * @param stats Search statistics
     */
    void updateSearchTree(const SearchStats& stats);

    /**
     * @brief Clear display
     */
    void clear();

    /**
     * @brief Set maximum display depth
     * @param depth Maximum depth
     */
    void setMaxDisplayDepth(int depth) { maxDisplayDepth_ = depth; }

    /**
     * @brief Set auto-scroll to latest
     * @param autoScroll Auto scroll
     */
    void setAutoScroll(bool autoScroll) { autoScroll_ = autoScroll; }

signals:
    /**
     * @brief Node click signal
     * @param nodeIndex Node index
     */
    void nodeClicked(int nodeIndex);

protected:
    /**
     * @brief Paint event
     */
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief Mouse press event
     */
    void mousePressEvent(QMouseEvent* event) override;

private:
    /**
     * @brief Draw search tree nodes
     */
    void drawSearchTree(QPainter& painter);

    SearchStats currentStats_;       // Current search statistics
    int maxDisplayDepth_;            // Maximum display depth
    bool autoScroll_;                // Auto scroll
    bool hasData_;                  // Has data
};

} // namespace Reversi
