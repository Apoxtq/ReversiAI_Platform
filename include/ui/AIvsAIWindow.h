/**
 * @file AIvsAIWindow.h
 * @brief AI vs AI 对战窗口
 *
 * 用于展示AI自动对战功能，支持：
 * - 选择两个AI进行对战
 * - 实时显示对局进度
 * - 统计对战结果
 *
 * @reference 参考Egaroucid和edax-reversi的界面设计
 */

#pragma once

#include <QMainWindow>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <chrono>
#include "Board.h"
#include "ai/AIStrategy.h"
#include "ai/AIBattle.h"
#include "research/BattleEngine.h"
#include "research/PositionSuite.h"

namespace Ui {
class AIvsAIWindow;
}

namespace Reversi {

/**
 * @class AIvsAIWindow
 * @brief AI vs AI 对战窗口类
 *
 * 提供AI自动对战的可视化界面，可以：
 * - 选择对战双方AI
 * - 设置对局数量
 * - 实时显示对局进度
 * - 展示详细统计结果
 */
class AIvsAIWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit AIvsAIWindow(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~AIvsAIWindow() override;

signals:
    /**
     * @brief 返回主菜单信号
     */
    void backToMenu();

private slots:
    /**
     * @brief 开始对战按钮点击
     */
    void onStartBattleClicked();

    /**
     * @brief 停止对战按钮点击
     */
    void onStopBattleClicked();

    /**
     * @brief 导出报告按钮点击
     */
    void onExportReportClicked();

    /**
     * @brief 返回主菜单
     */
    void onBackClicked();

    /**
     * @brief 更新对局进度
     */
    void updateProgress();

    /**
     * @brief 对局完成更新结果
     * @param stats 对战统计结果
     */
    void onBattleCompleted(const BattleStats& stats);

private:
    /**
     * @brief 初始化UI组件
     */
    void setupUI();

    /**
     * @brief 连接信号槽
     */
    void setupConnections();

    /**
     * @brief 获取当前选中的AI类型名称
     */
    QString getAIString(AIStrategy* ai);

    /**
     * @brief 更新统计表格
     */
    void updateStatsTable();

    /**
     * @brief 清除统计
     */
    void clearStats();

    /**
     * @brief 启用/禁用控制按钮
     */
    void setControlsEnabled(bool enabled);

    // UI组件
    QWidget* centralWidget_;
    QVBoxLayout* mainLayout_;

    // 对战配置区域
    QGroupBox* configGroup_;
    QGridLayout* configLayout_;
    QComboBox* ai1Combo_;
    QComboBox* ai2Combo_;
    QSpinBox* gamesSpinBox_;
    QComboBox* depth1Combo_;
    QComboBox* depth2Combo_;

    // 控制按钮
    QPushButton* startButton_;
    QPushButton* stopButton_;
    QPushButton* exportButton_;
    QPushButton* backButton_;

    // 进度显示
    QProgressBar* progressBar_;
    QLabel* progressLabel_;

    // 棋盘显示
    QLabel* boardLabel_;
    QLabel* currentMoveLabel_;

    // 统计结果区域
    QGroupBox* statsGroup_;
    QVBoxLayout* statsLayout_;
    QTableWidget* statsTable_;
    QTextEdit* resultText_;

    // 对战状态
    bool isRunning_;
    BattleConfig battleConfig_;
    QTimer* updateTimer_;

    // 统计数据
    int gamesPlayed_;
    int gamesWon1_;
    int gamesWon2_;
    int draws_;
    double avgMoves_;
    std::chrono::milliseconds totalTime_;
};

} // namespace Reversi

