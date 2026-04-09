/**
 * @file AIvsAIWindow.h
 * @brief AI vs AI Battle Window
 *
 * Features:
 * - Select two AIs for battle
 * - Real-time battle progress display
 * - Detailed statistics display
 * - Random seed configuration for reproducibility
 * - Verification mode for determinism testing
 *
 * @reference Based on Egaroucid and edax-reversi UI design
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
#include <QCheckBox>
#include <chrono>
#include "Board.h"
#include "ai/AIStrategy.h"
#include "ai/AIBattle.h"
#include "research/BattleEngine.h"
#include "research/PositionSuite.h"
#include "research/BitboardBenchmark.h"
#include "research/AIBenchmark.h"
#include "research/RuntimeEstimator.h"
#include "research/ValidationSuite.h"

namespace Ui {
class AIvsAIWindow;
}

namespace Reversi {

/**
 * @class AIvsAIWindow
 * @brief AI vs AI Battle Window
 *
 * Features:
 * - Select two AIs for battle
 * - Set number of games
 * - Configure random seed for reproducibility
 * - Enable verification mode for determinism testing
 * - Real-time progress display
 * - Detailed statistics with Pass/Fail indicators
 */
class AIvsAIWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent window pointer
     */
    explicit AIvsAIWindow(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~AIvsAIWindow() override;

signals:
    /**
     * @brief Signal to return to main menu
     */
    void backToMenu();

private slots:
    /**
     * @brief Start battle button clicked
     */
    void onStartBattleClicked();

    /**
     * @brief Stop battle button clicked
     */
    void onStopBattleClicked();

    /**
     * @brief Export report button clicked
     */
    void onExportReportClicked();

    /**
     * @brief Random seed button clicked
     */
    void onRandomSeedClicked();

    /**
     * @brief Run Bitboard benchmark
     */
    void onRunBitboardBenchmarkClicked();

    /**
     * @brief Run AI benchmark
     */
    void onRunAIBenchmarkClicked();

    /**
     * @brief Bitboard benchmark completed
     */
    void onBitboardBenchmarkComplete(const QVariant& results);

    /**
     * @brief AI benchmark completed
     */
    void onAIBenchmarkComplete(const QVariant& results);

    /**
     * @brief Run all validation tests
     */
    void onRunAllValidationClicked();

    /**
     * @brief Validation tests completed
     */
    void onValidationComplete(const QVariant& results);

    /**
     * @brief Export validation report
     */
    void onExportValidationClicked();

    /**
     * @brief Update runtime estimate display
     */
    void updateRuntimeEstimate();

    /**
     * @brief Return to main menu
     */
    void onBackClicked();

    /**
     * @brief Update battle progress
     */
    void updateProgress();

    /**
     * @brief Battle completed, update results
     * @param stats Battle statistics
     */
    void onBattleCompleted(const BattleStats& stats);

private:
    /**
     * @brief Initialize UI components
     */
    void setupUI();

    /**
     * @brief Connect signals and slots
     */
    void setupConnections();

    /**
     * @brief Get AI type name
     */
    QString getAIString(AIStrategy* ai);

    /**
     * @brief Update statistics table
     */
    void updateStatsTable();

    /**
     * @brief Clear statistics
     */
    void clearStats();

    /**
     * @brief Enable/disable control buttons
     */
    void setControlsEnabled(bool enabled);

    // UI组件
    QWidget* centralWidget_;
    QVBoxLayout* mainLayout_;

    // 对战配置区域
    QGroupBox* configGroup_;
    QGridLayout* configLayout_;
    QComboBox* ai1TypeCombo_;   // AI1算法类型选择
    QComboBox* ai2TypeCombo_;   // AI2算法类型选择
    QComboBox* ai1Combo_;       // AI1难度选择
    QComboBox* ai2Combo_;       // AI2难度选择
    QSpinBox* gamesSpinBox_;
    QComboBox* depth1Combo_;
    QComboBox* depth2Combo_;

    // Random Seed Configuration
    QSpinBox* seedSpinBox_;
    QPushButton* randomSeedButton_;
    QCheckBox* verificationModeCheckBox_;

    // Position Suite Selection
    QComboBox* suiteCombo_;

    // Parallel Processing Configuration
    QCheckBox* parallelCheckBox_;
    QSpinBox* threadsSpinBox_;

    // Control buttons
    QPushButton* startButton_;
    QPushButton* stopButton_;
    QPushButton* exportButton_;
    QPushButton* backButton_;
    QPushButton* benchmarkButton_;
    QPushButton* bitboardBenchmarkButton_;
    QPushButton* aiBenchmarkButton_;
    QPushButton* runAllValidationButton_;
    QPushButton* exportValidationButton_;

    // Progress display
    QProgressBar* progressBar_;
    QLabel* progressLabel_;
    QLabel* runtimeEstimateLabel_;

    // Board display
    QLabel* boardLabel_;
    QLabel* currentMoveLabel_;

    // Statistics results
    QGroupBox* statsGroup_;
    QVBoxLayout* statsLayout_;
    QTableWidget* statsTable_;
    QTextEdit* resultText_;

    // Battle state
    bool isRunning_;
    BattleConfig battleConfig_;
    QTimer* updateTimer_;

    // Statistics data
    int gamesPlayed_;
    int gamesWon1_;
    int gamesWon2_;
    int draws_;
    double avgMoves_;
    std::chrono::milliseconds totalTime_;

    // Validation results
    std::vector<ValidationResult> validationResults_;
};

} // namespace Reversi

