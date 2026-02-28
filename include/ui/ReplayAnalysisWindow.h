/**
 * @file ReplayAnalysisWindow.h
 * @brief 复盘分析窗口 - v0.9.0可视化增强版
 *
 * 提供对局复盘和分析功能，支持棋谱导入导出
 */

#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTimer>

#include "research/GameRecord.h"

namespace Reversi {

/**
 * @class ReplayAnalysisWindow
 * @brief 复盘分析窗口
 *
 * 提供对局回放、分析和导出功能
 */
class ReplayAnalysisWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit ReplayAnalysisWindow(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ReplayAnalysisWindow() override;

    /**
     * @brief 加载对局文件
     * @param filepath 文件路径
     * @return 是否成功
     */
    bool loadGameFile(const QString& filepath);

    /**
     * @brief 加载对局记录
     * @param record 对局记录
     */
    void loadRecord(const GameRecord& record);

    /**
     * @brief 获取当前对局记录
     */
    const GameRecord& getCurrentRecord() const { return currentRecord_; }

signals:
    /**
     * @brief 返回主菜单信号
     */
    void backToMenu();

private slots:
    /**
     * @brief 打开文件
     */
    void onOpenFile();

    /**
     * @brief 导出PGN
     */
    void onExportPGN();

    /**
     * @brief 导出SGF
     */
    void onExportSGF();

    /**
     * @brief 播放速度变化
     */
    void onPlaybackSpeedChanged(int index);

    /**
     * @brief 播放/暂停按钮
     */
    void onPlayPauseClicked();

    /**
     * @brief 停止按钮
     */
    void onStopClicked();

    /**
     * @brief 上一步
     */
    void onStepBackwardClicked();

    /**
     * @brief 下一步
     */
    void onStepForwardClicked();

    /**
     * @brief 滑块值变化
     */
    void onSliderValueChanged(int value);

    /**
     * @brief 返回主菜单
     */
    void onBackClicked();

    /**
     * @brief 棋盘更新
     */
    void onBoardUpdated(class Board* board);

    /**
     * @brief 走法变化
     */
    void onMoveChanged(int moveIndex, const MoveRecord& move);

    /**
     * @brief 播放完毕
     */
    void onPlaybackFinished();

    /**
     * @brief 定时器触发
     */
    void onTimerTimeout();

private:
    /**
     * @brief 初始化UI
     */
    void setupUI();

    /**
     * @brief 初始化菜单
     */
    void setupMenu();

    /**
     * @brief 初始化工具栏
     */
    void setupToolbar();

    /**
     * @brief 连接信号槽
     */
    void setupConnections();

    /**
     * @brief 更新显示
     */
    void updateDisplay();

    /**
     * @brief 更新控制按钮状态
     */
    void updateControlButtons();

    /**
     * @brief 加载JSON文件
     */
    bool loadJsonFile(const QString& filepath);

    // 中心部件
    QWidget* centralWidget_;
    QVBoxLayout* mainLayout_;

    // 棋盘显示区域
    QLabel* boardLabel_;
    QSize boardDisplaySize_;

    // 播放控制区域
    QGroupBox* controlGroup_;
    QHBoxLayout* controlLayout_;

    // 滑块
    QSlider* moveSlider_;
    QLabel* moveIndexLabel_;
    QLabel* progressLabel_;

    // 播放按钮
    QPushButton* playPauseButton_;
    QPushButton* stopButton_;
    QPushButton* stepBackwardButton_;
    QPushButton* stepForwardButton_;

    // 速度选择
    QComboBox* speedComboBox_;

    // 信息显示
    QLabel* playerInfoLabel_;
    QLabel* moveInfoLabel_;

    // 记录和回放
    GameRecord currentRecord_;
    GameReplay replay_;

    // 播放定时器
    QTimer* playbackTimer_;

    // 当前状态
    bool isLoaded_;
};

} // namespace Reversi
