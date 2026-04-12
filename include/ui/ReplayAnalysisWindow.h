/**
 * @file ReplayAnalysisWindow.h
 * @brief Replay Analysis Window - v0.9.0 Visualization Enhanced
 *
 * Provides game replay and analysis features, supports game record import/export
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
 * @brief Replay analysis window
 *
 * Provides game replay, analysis, and export functionality
 */
class ReplayAnalysisWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent window pointer
     */
    explicit ReplayAnalysisWindow(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~ReplayAnalysisWindow() override;

    /**
     * @brief Load game file
     * @param filepath File path
     * @return Whether successful
     */
    bool loadGameFile(const QString& filepath);

    /**
     * @brief Load game record
     * @param record Game record
     */
    void loadRecord(const GameRecord& record);

    /**
     * @brief Get current game record
     */
    const GameRecord& getCurrentRecord() const { return currentRecord_; }

signals:
    /**
     * @brief Return to main menu signal
     */
    void backToMenu();

private slots:
    /**
     * @brief Open file
     */
    void onOpenFile();

    /**
     * @brief Export PGN
     */
    void onExportPGN();

    /**
     * @brief Export SGF
     */
    void onExportSGF();

    /**
     * @brief Playback speed changed
     */
    void onPlaybackSpeedChanged(int index);

    /**
     * @brief Play/pause button
     */
    void onPlayPauseClicked();

    /**
     * @brief Stop button
     */
    void onStopClicked();

    /**
     * @brief Previous step
     */
    void onStepBackwardClicked();

    /**
     * @brief Next step
     */
    void onStepForwardClicked();

    /**
     * @brief Slider value changed
     */
    void onSliderValueChanged(int value);

    /**
     * @brief Return to main menu
     */
    void onBackClicked();

    /**
     * @brief Board update
     */
    void onBoardUpdated(class Board* board);

    /**
     * @brief Move changed
     */
    void onMoveChanged(int moveIndex, const MoveRecord& move);

    /**
     * @brief Playback finished
     */
    void onPlaybackFinished();

    /**
     * @brief Timer triggered
     */
    void onTimerTimeout();

private:
    /**
     * @brief Initialize UI
     */
    void setupUI();

    /**
     * @brief Initialize menu
     */
    void setupMenu();

    /**
     * @brief Initialize toolbar
     */
    void setupToolbar();

    /**
     * @brief Connect signals and slots
     */
    void setupConnections();

    /**
     * @brief Update display
     */
    void updateDisplay();

    /**
     * @brief Update control button states
     */
    void updateControlButtons();

    /**
     * @brief Load JSON file
     */
    bool loadJsonFile(const QString& filepath);

    // Central widget
    QWidget* centralWidget_;
    QVBoxLayout* mainLayout_;

    // Board display area
    QLabel* boardLabel_;
    QSize boardDisplaySize_;

    // Playback control area
    QGroupBox* controlGroup_;
    QHBoxLayout* controlLayout_;

    // Slider
    QSlider* moveSlider_;
    QLabel* moveIndexLabel_;
    QLabel* progressLabel_;

    // Playback buttons
    QPushButton* playPauseButton_;
    QPushButton* stopButton_;
    QPushButton* stepBackwardButton_;
    QPushButton* stepForwardButton_;

    // Speed selection
    QComboBox* speedComboBox_;

    // Information display
    QLabel* playerInfoLabel_;
    QLabel* moveInfoLabel_;

    // Record and replay
    GameRecord currentRecord_;
    GameReplay replay_;

    // Playback timer
    QTimer* playbackTimer_;

    // Current state
    bool isLoaded_;
};

} // namespace Reversi
