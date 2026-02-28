/**
 * @file SearchStatisticsPanel.h
 * @brief 实时统计面板 - v0.9.0可视化增强版
 *
 * 显示AI搜索的实时统计数据，包括深度、节点数、NPS等
 */

#pragma once

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QProgressBar>
#include <QFont>

#include "research/SearchStats.h"

namespace Reversi {

/**
 * @class SearchStatisticsPanel
 * @brief 实时搜索统计面板
 *
 * 显示AI搜索过程中的各项统计指标
 */
class SearchStatisticsPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit SearchStatisticsPanel(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~SearchStatisticsPanel() override;

    /**
     * @brief 更新统计数据
     * @param stats 搜索统计数据
     */
    void updateStatistics(const SearchStats& stats);

    /**
     * @brief 清空显示
     */
    void clear();

    /**
     * @brief 设置面板标题
     * @param title 标题
     */
    void setTitle(const QString& title);

    /**
     * @brief 设置是否显示详细统计
     * @param show 是否显示
     */
    void setShowDetails(bool show);

signals:
    /**
     * @brief 面板点击信号（可扩展）
     */
    void panelClicked();

private slots:
    /**
     * @brief 更新显示（定时器触发）
     */
    void updateDisplay();

private:
    /**
     * @brief 初始化UI组件
     */
    void setupUI();

    /**
     * @brief 创建统计行
     * @param label 标签
     * @param valueLabel 值标签（输出）
     */
    void createStatRow(const QString& label, QLabel*& valueLabel);

    /**
     * @brief 更新单个标签
     */
    void updateLabel(QLabel* label, const QString& value, const QString& color = QString());

    // 标题
    QGroupBox* groupBox_;
    QVBoxLayout* mainLayout_;

    // 基础统计
    QLabel* depthLabel_;
    QLabel* maxDepthLabel_;
    QLabel* nodesLabel_;
    QLabel* npsLabel_;              // Nodes per second
    QLabel* timeLabel_;

    // 高级统计
    QLabel* cutoffsLabel_;
    QLabel* ttHitRateLabel_;        // 转置表命中率
    QLabel* killerHitRateLabel_;    // Killer走法命中
    QLabel* historyHitRateLabel_;   // History启发命中
    QLabel* ttEntriesLabel_;        // 转置表条目数

    // 最佳走法
    QLabel* bestMoveLabel_;
    QLabel* bestValueLabel_;

    // MCTS特定
    QLabel* simulationsLabel_;
    QLabel* winRateLabel_;

    // 进度显示
    QProgressBar* searchProgress_;

    // 当前数据
    SearchStats currentStats_;

    // 显示设置
    bool showDetails_;
    bool isSearching_;
};

} // namespace Reversi
