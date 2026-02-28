/**
 * @file BoardHeatmap.h
 * @brief 棋盘热度图 - v0.9.0可视化增强版
 *
 * 在棋盘上叠加显示位置价值、访问次数、胜率等热力图
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
 * @brief 棋盘热度图组件
 *
 * 在棋盘上叠加显示各种热力图数据
 */
class BoardHeatmap : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit BoardHeatmap(QWidget* parent = nullptr);

    /**
     * @brief 设置热度数据
     * @param values 64个格子的值 (0.0 - 1.0)
     * @param type 热度图类型
     */
    void setHeatmapData(const QVector<double>& values, HeatmapType type);

    /**
     * @brief 设置热度数据类型
     * @param type 数据类型
     */
    void setHeatmapType(HeatmapType type);

    /**
     * @brief 清除热度数据
     */
    void clearHeatmap();

    /**
     * @brief 设置透明度
     * @param opacity 透明度 (0.0 - 1.0)
     */
    void setHeatmapOpacity(double opacity);

    /**
     * @brief 获取当前热度类型
     */
    HeatmapType getHeatmapType() const { return currentType_; }

    /**
     * @brief 是否显示热度图
     */
    bool isVisible() const { return isVisible_; }

    /**
     * @brief 显示/隐藏热度图
     */
    void setVisible(bool visible) { isVisible_ = visible; update(); }

signals:
    /**
     * @brief 热度类型变化信号
     */
    void heatmapTypeChanged(HeatmapType type);

protected:
    /**
     * @brief 绘制事件
     */
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief 获取热力颜色
     * @param value 数值 (0.0 - 1.0)
     * @return 颜色
     */
    QColor getHeatColor(double value) const;

private:
    /**
     * @brief 初始化颜色映射
     */
    void initColorMaps();

    QVector<double> heatmapData_;      // 热度数据
    HeatmapType currentType_;          // 当前类型
    double opacity_;                    // 透明度
    bool isVisible_;                   // 是否显示
    bool hasData_;                     // 是否有数据
};

} // namespace Reversi
