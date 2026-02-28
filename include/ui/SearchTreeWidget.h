/**
 * @file SearchTreeWidget.h
 * @brief 搜索树可视化组件 - v0.9.0可视化增强版
 *
 * 显示AI搜索树的深度、节点数、分支情况
 * 注意：这是一个简化版本，完整版需要更复杂的绘制逻辑
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
 * @brief 搜索树可视化部件
 *
 * 显示搜索树的图形化表示
 */
class SearchTreeWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit SearchTreeWidget(QWidget* parent = nullptr);

    /**
     * @brief 更新搜索树数据
     * @param stats 搜索统计信息
     */
    void updateSearchTree(const SearchStats& stats);

    /**
     * @brief 清空显示
     */
    void clear();

    /**
     * @brief 设置显示的最大深度
     * @param depth 最大深度
     */
    void setMaxDisplayDepth(int depth) { maxDisplayDepth_ = depth; }

    /**
     * @brief 设置是否自动滚动到最新
     * @param autoScroll 自动滚动
     */
    void setAutoScroll(bool autoScroll) { autoScroll_ = autoScroll; }

signals:
    /**
     * @brief 节点点击信号
     * @param nodeIndex 节点索引
     */
    void nodeClicked(int nodeIndex);

protected:
    /**
     * @brief 绘制事件
     */
    void paintEvent(QPaintEvent* event) override;

    /**
     * @brief 鼠标点击事件
     */
    void mousePressEvent(QMouseEvent* event) override;

private:
    /**
     * @brief 绘制搜索树节点
     */
    void drawSearchTree(QPainter& painter);

    SearchStats currentStats_;       // 当前搜索统计
    int maxDisplayDepth_;            // 最大显示深度
    bool autoScroll_;                // 自动滚动
    bool hasData_;                  // 是否有数据
};

} // namespace Reversi
