/**
 * @file AIvsAIWindow.cpp
 * @brief AI vs AI battle window implementation
 *
 * Implements visual interface for AI auto-battle
 */

#include "ui/AIvsAIWindow.h"
#include <QPainter>
#include <QFont>
#include <QTextStream>
#include <QDate>
#include <QtConcurrent/QtConcurrent>
#include <random>

namespace Reversi {

AIvsAIWindow::AIvsAIWindow(QWidget* parent)
    : QMainWindow(parent)
    , isRunning_(false)
    , updateTimer_(nullptr)
    , gamesPlayed_(0)
    , gamesWon1_(0)
    , gamesWon2_(0)
    , draws_(0)
    , avgMoves_(0.0)
{
    setupUI();
    setupConnections();
    clearStats();
}

AIvsAIWindow::~AIvsAIWindow() {
    if (isRunning_) {
        onStopBattleClicked();
    }
    if (updateTimer_) {
        delete updateTimer_;
    }
}

void AIvsAIWindow::setupUI() {
    // Main window settings
    centralWidget_ = new QWidget(this);
    setCentralWidget(centralWidget_);
    mainLayout_ = new QVBoxLayout(centralWidget_);

    // Battle configuration area
    configGroup_ = new QGroupBox(tr("Battle Configuration"), this);
    configLayout_ = new QGridLayout(configGroup_);

    // AI1 Selection
    configLayout_->addWidget(new QLabel(tr("Player 1 (Black):"), this), 0, 0);
    ai1TypeCombo_ = new QComboBox(this);
    ai1TypeCombo_->addItem("Minimax", 0);
    ai1TypeCombo_->addItem("MCTS", 1);
    ai1TypeCombo_->addItem("Random", 2);
    configLayout_->addWidget(ai1TypeCombo_, 0, 1);

    ai1Combo_ = new QComboBox(this);
    ai1Combo_->addItem("Easy", QVariant::fromValue(Difficulty::EASY));
    ai1Combo_->addItem("Medium", QVariant::fromValue(Difficulty::MEDIUM));
    ai1Combo_->addItem("Hard", QVariant::fromValue(Difficulty::HARD));
    configLayout_->addWidget(ai1Combo_, 0, 2);

    configLayout_->addWidget(new QLabel(tr("Depth:"), this), 0, 3);
    depth1Combo_ = new QComboBox(this);
    depth1Combo_->addItem("2");
    depth1Combo_->addItem("3");
    depth1Combo_->addItem("4");
    depth1Combo_->addItem("5");
    depth1Combo_->addItem("6");
    depth1Combo_->setCurrentIndex(2);  // Default: 4
    configLayout_->addWidget(depth1Combo_, 0, 4);

    // AI2 Selection
    configLayout_->addWidget(new QLabel(tr("Player 2 (White):"), this), 1, 0);
    ai2TypeCombo_ = new QComboBox(this);
    ai2TypeCombo_->addItem("Minimax", 0);
    ai2TypeCombo_->addItem("MCTS", 1);
    ai2TypeCombo_->addItem("Random", 2);
    ai2TypeCombo_->setCurrentIndex(2);  // Default: Random
    configLayout_->addWidget(ai2TypeCombo_, 1, 1);

    ai2Combo_ = new QComboBox(this);
    ai2Combo_->addItem("Easy", QVariant::fromValue(Difficulty::EASY));
    ai2Combo_->addItem("Medium", QVariant::fromValue(Difficulty::MEDIUM));
    ai2Combo_->addItem("Hard", QVariant::fromValue(Difficulty::HARD));
    ai2Combo_->setCurrentIndex(2);  // Default: Hard
    configLayout_->addWidget(ai2Combo_, 1, 2);

    configLayout_->addWidget(new QLabel(tr("Depth:"), this), 1, 3);
    depth2Combo_ = new QComboBox(this);
    depth2Combo_->addItem("2");
    depth2Combo_->addItem("3");
    depth2Combo_->addItem("4");
    depth2Combo_->addItem("5");
    depth2Combo_->addItem("6");
    depth2Combo_->setCurrentIndex(1);  // Default: 3
    configLayout_->addWidget(depth2Combo_, 1, 4);

    // Number of games
    configLayout_->addWidget(new QLabel(tr("Games:"), this), 2, 0);
    gamesSpinBox_ = new QSpinBox(this);
    gamesSpinBox_->setRange(1, 1000);
    gamesSpinBox_->setValue(10);
    configLayout_->addWidget(gamesSpinBox_, 2, 1);

    // Random Seed Configuration
    configLayout_->addWidget(new QLabel(tr("Seed:"), this), 2, 2);
    seedSpinBox_ = new QSpinBox(this);
    seedSpinBox_->setRange(0, 999999999);
    seedSpinBox_->setValue(42);
    seedSpinBox_->setToolTip(tr("Random seed for reproducibility"));
    configLayout_->addWidget(seedSpinBox_, 2, 3);

    randomSeedButton_ = new QPushButton(tr("Random"), this);
    randomSeedButton_->setMaximumWidth(80);
    randomSeedButton_->setToolTip(tr("Generate random seed"));
    configLayout_->addWidget(randomSeedButton_, 2, 4);

    // Verification Mode
    verificationModeCheckBox_ = new QCheckBox(tr("Verification Mode"), this);
    verificationModeCheckBox_->setToolTip(tr("Run same config twice to verify determinism"));
    configLayout_->addWidget(verificationModeCheckBox_, 3, 0, 1, 2);

    // Position Suite Selection
    suiteCombo_ = new QComboBox(this);
    suiteCombo_->addItem("Standard-64 (Full)", QVariant::fromValue(static_cast<int>(0)));
    suiteCombo_->addItem("Opening (16 positions)", QVariant::fromValue(static_cast<int>(1)));
    suiteCombo_->addItem("Midgame (24 positions)", QVariant::fromValue(static_cast<int>(2)));
    suiteCombo_->addItem("Endgame (24 positions)", QVariant::fromValue(static_cast<int>(3)));
    suiteCombo_->setToolTip(tr("Select position suite for testing"));
    configLayout_->addWidget(new QLabel(tr("Position Suite:"), this), 4, 0);
    configLayout_->addWidget(suiteCombo_, 4, 1, 1, 2);

    mainLayout_->addWidget(configGroup_);

    // Advanced Configuration (Collapsible)
    QGroupBox* advancedGroup = new QGroupBox(tr("Advanced Configuration"), this);
    QGridLayout* advancedLayout = new QGridLayout(advancedGroup);

    // Parallel Processing
    parallelCheckBox_ = new QCheckBox(tr("Enable Parallel Processing"), this);
    parallelCheckBox_->setToolTip(tr("Use multiple threads for faster battles"));
    advancedLayout->addWidget(parallelCheckBox_, 0, 0, 1, 2);

    advancedLayout->addWidget(new QLabel(tr("Threads:"), this), 1, 0);
    threadsSpinBox_ = new QSpinBox(this);
    threadsSpinBox_->setRange(1, 16);
    threadsSpinBox_->setValue(4);
    threadsSpinBox_->setToolTip(tr("Number of parallel threads (1-16)"));
    advancedLayout->addWidget(threadsSpinBox_, 1, 1);

    // Warning label
    QLabel* warningLabel = new QLabel(
        tr("Warning: Parallel mode may cause inconsistent results between runs"), this);
    warningLabel->setStyleSheet("color: #e74c3c; font-size: 11px;");
    advancedLayout->addWidget(warningLabel, 2, 0, 1, 2);

    mainLayout_->addWidget(advancedGroup);

    // Progress display
    QHBoxLayout* progressLayout = new QHBoxLayout();
    progressBar_ = new QProgressBar(this);
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(true);
    progressLayout->addWidget(progressBar_);

    progressLabel_ = new QLabel(tr("Ready"), this);
    progressLayout->addWidget(progressLabel_);
    mainLayout_->addLayout(progressLayout);

    // Current state display
    currentMoveLabel_ = new QLabel(tr("Ready to start"), this);
    currentMoveLabel_->setAlignment(Qt::AlignCenter);
    currentMoveLabel_->setStyleSheet("font-size: 16px; color: #7f8c8d; padding: 10px;");
    mainLayout_->addWidget(currentMoveLabel_);

    // Control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    startButton_ = new QPushButton(tr("Start Battle"), this);
    startButton_->setStyleSheet("background-color: #27ae60; color: white; padding: 10px 30px; font-size: 16px; border-radius: 5px;");
    buttonLayout->addWidget(startButton_);

    stopButton_ = new QPushButton(tr("Stop"), this);
    stopButton_->setStyleSheet("background-color: #e74c3c; color: white; padding: 10px 30px; font-size: 16px; border-radius: 5px;");
    stopButton_->setEnabled(false);
    buttonLayout->addWidget(stopButton_);

    exportButton_ = new QPushButton(tr("Export Report"), this);
    exportButton_->setStyleSheet("background-color: #3498db; color: white; padding: 10px 30px; font-size: 16px; border-radius: 5px;");
    exportButton_->setEnabled(false);
    buttonLayout->addWidget(exportButton_);

    mainLayout_->addLayout(buttonLayout);

    // Benchmark buttons area
    QLabel* benchmarkLabel = new QLabel(tr("Benchmarks:"), this);
    benchmarkLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50;");
    mainLayout_->addWidget(benchmarkLabel);

    QHBoxLayout* benchmarkLayout = new QHBoxLayout();

    bitboardBenchmarkButton_ = new QPushButton(tr("Bitboard Benchmark"), this);
    bitboardBenchmarkButton_->setStyleSheet("background-color: #9b59b6; color: white; padding: 8px 20px; font-size: 14px; border-radius: 5px;");
    benchmarkLayout->addWidget(bitboardBenchmarkButton_);

    aiBenchmarkButton_ = new QPushButton(tr("AI Benchmark"), this);
    aiBenchmarkButton_->setStyleSheet("background-color: #e67e22; color: white; padding: 8px 20px; font-size: 14px; border-radius: 5px;");
    benchmarkLayout->addWidget(aiBenchmarkButton_);

    mainLayout_->addLayout(benchmarkLayout);

    // Quick Validation Section
    QGroupBox* validationGroup = new QGroupBox(tr("Quick Validation"), this);
    QVBoxLayout* validationLayout = new QVBoxLayout(validationGroup);

    QLabel* validationInfoLabel = new QLabel(
        tr("Run academic standard tests: Minimax-6 vs Random (>=90%), MCTS-10k vs Minimax-4 (>=70%)"), this);
    validationInfoLabel->setStyleSheet("color: #666; font-size: 12px;");
    validationLayout->addWidget(validationInfoLabel);

    QHBoxLayout* validationBtnLayout = new QHBoxLayout();
    runAllValidationButton_ = new QPushButton(tr("Run All Validations"), this);
    runAllValidationButton_->setStyleSheet(
        "background-color: #16a085; color: white; padding: 10px 20px; font-size: 14px; border-radius: 5px;");
    runAllValidationButton_->setToolTip(tr("Run all preset validation tests"));
    validationBtnLayout->addWidget(runAllValidationButton_);

    exportValidationButton_ = new QPushButton(tr("Export"), this);
    exportValidationButton_->setStyleSheet(
        "background-color: #3498db; color: white; padding: 10px 20px; font-size: 14px; border-radius: 5px;");
    exportValidationButton_->setEnabled(false);
    validationBtnLayout->addWidget(exportValidationButton_);
    validationBtnLayout->addStretch();

    validationLayout->addLayout(validationBtnLayout);
    mainLayout_->addWidget(validationGroup);

    // Runtime Estimate Display
    runtimeEstimateLabel_ = new QLabel(tr("Estimated time: --"), this);
    runtimeEstimateLabel_->setStyleSheet(
        "background-color: #ecf0f1; padding: 8px; border-radius: 4px; font-size: 13px; color: #2c3e50;");
    runtimeEstimateLabel_->setAlignment(Qt::AlignCenter);
    mainLayout_->addWidget(runtimeEstimateLabel_);

    // Statistics results area
    statsGroup_ = new QGroupBox(tr("Results"), this);
    statsLayout_ = new QVBoxLayout(statsGroup_);

    statsTable_ = new QTableWidget(this);
    statsTable_->setRowCount(8);
    statsTable_->setColumnCount(4);
    statsTable_->setHorizontalHeaderLabels(QStringList()
        << tr("Metric") << tr("Value") << tr("Target") << tr("Status"));
    statsTable_->horizontalHeader()->setStretchLastSection(true);
    statsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable_->setStyleSheet("font-size: 14px;");
    statsTable_->setColumnWidth(0, 120);
    statsTable_->setColumnWidth(1, 100);
    statsTable_->setColumnWidth(2, 100);
    statsTable_->setColumnWidth(3, 80);
    statsLayout_->addWidget(statsTable_);

    resultText_ = new QTextEdit(this);
    resultText_->setMaximumHeight(150);
    resultText_->setReadOnly(true);
    statsLayout_->addWidget(resultText_);

    mainLayout_->addWidget(statsGroup_);

    // Bottom buttons
    backButton_ = new QPushButton(tr("Back to Menu"), this);
    backButton_->setStyleSheet("background-color: #95a5a6; color: white; padding: 8px 20px; font-size: 14px; border-radius: 5px;");
    QHBoxLayout* backLayout = new QHBoxLayout();
    backLayout->addStretch();
    backLayout->addWidget(backButton_);
    backLayout->addStretch();
    mainLayout_->addLayout(backLayout);

    // Initialize statistics table
    clearStats();
}

void AIvsAIWindow::setupConnections() {
    connect(startButton_, &QPushButton::clicked, this, &AIvsAIWindow::onStartBattleClicked);
    connect(stopButton_, &QPushButton::clicked, this, &AIvsAIWindow::onStopBattleClicked);
    connect(exportButton_, &QPushButton::clicked, this, &AIvsAIWindow::onExportReportClicked);
    connect(backButton_, &QPushButton::clicked, this, &AIvsAIWindow::onBackClicked);
    connect(bitboardBenchmarkButton_, &QPushButton::clicked, this, &AIvsAIWindow::onRunBitboardBenchmarkClicked);
    connect(aiBenchmarkButton_, &QPushButton::clicked, this, &AIvsAIWindow::onRunAIBenchmarkClicked);
    connect(randomSeedButton_, &QPushButton::clicked, this, &AIvsAIWindow::onRandomSeedClicked);
    connect(runAllValidationButton_, &QPushButton::clicked, this, &AIvsAIWindow::onRunAllValidationClicked);
    connect(exportValidationButton_, &QPushButton::clicked, this, &AIvsAIWindow::onExportValidationClicked);

    // AI type selection changes update UI
    connect(ai1TypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                depth1Combo_->setEnabled(index != 1);  // MCTS doesn't need depth
                ai1Combo_->setEnabled(index != 2);     // Random doesn't need difficulty
                updateRuntimeEstimate();
            });
    connect(ai2TypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                depth2Combo_->setEnabled(index != 1);  // MCTS doesn't need depth
                ai2Combo_->setEnabled(index != 2);     // Random doesn't need difficulty
                updateRuntimeEstimate();
            });

    // Games count changes update estimate
    connect(gamesSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int) { updateRuntimeEstimate(); });

    // Parallel processing checkbox
    connect(parallelCheckBox_, &QCheckBox::toggled,
            this, [this](bool checked) {
                threadsSpinBox_->setEnabled(checked);
                updateRuntimeEstimate();
            });

    // Threads spinbox changes update estimate
    connect(threadsSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int) { updateRuntimeEstimate(); });

    // Initialize UI state
    depth1Combo_->setEnabled(true);   // Minimax enabled by default
    depth2Combo_->setEnabled(true);
    ai1Combo_->setEnabled(true);
    ai2Combo_->setEnabled(true);
    threadsSpinBox_->setEnabled(parallelCheckBox_->isChecked());
    updateRuntimeEstimate();
}

void AIvsAIWindow::onStartBattleClicked() {
    if (isRunning_) return;

    clearStats();

    // Configure battle
    battleConfig_.num_games = gamesSpinBox_->value();
    battleConfig_.verbose = false;
    battleConfig_.random_seed = seedSpinBox_->value();  // Use configured seed

    // Parallel processing configuration
    battleConfig_.parallel = parallelCheckBox_->isChecked();
    battleConfig_.max_threads = threadsSpinBox_->value();

    // Player 1 - Create based on selected algorithm type
    int ai1Type = ai1TypeCombo_->currentData().toInt();
    Difficulty diff1 = ai1Combo_->currentData().value<Difficulty>();

    if (ai1Type == 2) {
        // Random
        battleConfig_.player1 = AIStrategyFactory::createRandomAI();
    } else if (ai1Type == 1) {
        // MCTS
        battleConfig_.player1 = AIStrategyFactory::createMCTSAI(diff1);
    } else {
        // Minimax
        battleConfig_.player1 = AIStrategyFactory::createMinimaxAI(diff1);
    }
    battleConfig_.player1_name = battleConfig_.player1->getName();
    battleConfig_.limits1.maxDepth = depth1Combo_->currentText().toInt();

    // Player 2 - Create based on selected algorithm type
    int ai2Type = ai2TypeCombo_->currentData().toInt();
    Difficulty diff2 = ai2Combo_->currentData().value<Difficulty>();

    if (ai2Type == 2) {
        // Random
        battleConfig_.player2 = AIStrategyFactory::createRandomAI();
    } else if (ai2Type == 1) {
        // MCTS
        battleConfig_.player2 = AIStrategyFactory::createMCTSAI(diff2);
    } else {
        // Minimax
        battleConfig_.player2 = AIStrategyFactory::createMinimaxAI(diff2);
    }
    battleConfig_.player2_name = battleConfig_.player2->getName();
    battleConfig_.limits2.maxDepth = depth2Combo_->currentText().toInt();

    // Create timer
    updateTimer_ = new QTimer(this);
    connect(updateTimer_, &QTimer::timeout, this, &AIvsAIWindow::updateProgress);

    // Start battle
    isRunning_ = true;
    setControlsEnabled(false);

    // Use thread-based execution
    progressBar_->setRange(0, battleConfig_.num_games);

    // Show initial state
    currentMoveLabel_->setText(tr("Battle in progress: %1 vs %2")
        .arg(QString::fromStdString(battleConfig_.player1_name))
        .arg(QString::fromStdString(battleConfig_.player2_name)));

    // Launch timer for UI updates
    updateTimer_->start(100);

    // Run battle in background thread
    (void)QtConcurrent::run([this]() {
        BattleStats stats = BattleEngine::runBattle(battleConfig_);

        // Update UI when complete
        QMetaObject::invokeMethod(this, [this, stats]() {
            onBattleCompleted(stats);
        });
    });
}

void AIvsAIWindow::onStopBattleClicked() {
    if (!isRunning_) return;

    isRunning_ = false;

    if (updateTimer_) {
        updateTimer_->stop();
    }

    setControlsEnabled(true);
    currentMoveLabel_->setText(tr("Battle stopped by user"));
}

void AIvsAIWindow::onExportReportClicked() {
    QString filename = QFileDialog::getSaveFileName(this,
        tr("Export Battle Report"),
        QString("battle_report_%1").arg(QDate::currentDate().toString("yyyyMMdd")),
        tr("Text Files (*.txt);;CSV Files (*.csv)"));

    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file for writing"));
        return;
    }

    QTextStream out(&file);

    // Write report
    out << "========================================\n";
    out << "       AI Battle Benchmark Report\n";
    out << "========================================\n\n";
    out << "Date: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n\n";

    out << "Configuration:\n";
    out << "----------------------------------------\n";
    out << "Player 1: " << QString::fromStdString(battleConfig_.player1_name) << "\n";
    out << "Player 2: " << QString::fromStdString(battleConfig_.player2_name) << "\n";
    out << "Games: " << gamesPlayed_ << "\n\n";

    out << "Results:\n";
    out << "----------------------------------------\n";
    out << QString("%1 wins: %2 (%3%)\n")
        .arg(QString::fromStdString(battleConfig_.player1_name))
        .arg(gamesWon1_)
        .arg(gamesPlayed_ > 0 ? QString::number(gamesWon1_ * 100.0 / gamesPlayed_, 'f', 1) : "0");
    out << QString("%1 wins: %2 (%3%)\n")
        .arg(QString::fromStdString(battleConfig_.player2_name))
        .arg(gamesWon2_)
        .arg(gamesPlayed_ > 0 ? QString::number(gamesWon2_ * 100.0 / gamesPlayed_, 'f', 1) : "0");
    out << QString("Draws: %1 (%2%)\n")
        .arg(draws_)
        .arg(gamesPlayed_ > 0 ? QString::number(draws_ * 100.0 / gamesPlayed_, 'f', 1) : "0");
    out << QString("Average moves: %1\n").arg(avgMoves_, 0, 'f', 1);
    out << QString("Total time: %1 ms\n").arg(totalTime_.count());

    file.close();

    QMessageBox::information(this, tr("Success"), tr("Report exported successfully"));
}

void AIvsAIWindow::onBackClicked() {
    if (isRunning_) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            tr("Battle in Progress"),
            tr("A battle is currently running. Do you want to stop it and return to menu?"),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) return;
        onStopBattleClicked();
    }
    emit backToMenu();
}

void AIvsAIWindow::updateProgress() {
    if (!isRunning_) return;

    // Get current progress - simplified to just update label
    progressLabel_->setText(tr("Battle in progress..."));
}

void AIvsAIWindow::onBattleCompleted(const BattleStats& stats) {
    isRunning_ = false;

    if (updateTimer_) {
        updateTimer_->stop();
    }

    // Update statistics
    gamesPlayed_ = stats.total_games;
    gamesWon1_ = stats.player1_wins;
    gamesWon2_ = stats.player2_wins;
    draws_ = stats.draws;
    avgMoves_ = stats.avg_moves;
    totalTime_ = std::chrono::milliseconds(static_cast<long long>(stats.avg_duration_ms * gamesPlayed_));

    // Update UI
    updateStatsTable();
    setControlsEnabled(true);
    exportButton_->setEnabled(true);

    // Update progress bar
    progressBar_->setValue(gamesPlayed_);
    progressLabel_->setText(tr("Completed: %1 games").arg(gamesPlayed_));

    // Update status label
    QString result = QString(tr("Battle Complete! %1 vs %2 - %1 won %3 games (%4%)"))
        .arg(QString::fromStdString(battleConfig_.player1_name))
        .arg(QString::fromStdString(battleConfig_.player2_name))
        .arg(gamesWon1_)
        .arg(gamesPlayed_ > 0 ? QString::number(gamesWon1_ * 100.0 / gamesPlayed_, 'f', 1) : "0");
    currentMoveLabel_->setText(result);

    // Show detailed results
    QString detail;
    QTextStream ss(&detail);
    ss << "========================================\n";
    ss << "         Battle Results\n";
    ss << "========================================\n\n";
    ss << QString("Winner: %1\n").arg(stats.player1_wins > stats.player2_wins ?
        QString::fromStdString(battleConfig_.player1_name) : QString::fromStdString(battleConfig_.player2_name));
    ss << QString("Score: %1 - %2 - %3\n").arg(stats.player1_wins).arg(stats.player2_wins).arg(stats.draws);

    double winRate = (stats.player1_wins + stats.player2_wins) > 0 ?
        stats.player1_wins * 100.0 / (stats.player1_wins + stats.player2_wins) : 0;
    ss << QString("Win Rate: %1%\n").arg(winRate, 0, 'f', 1);
    ss << QString("Average Moves: %1\n").arg(stats.avg_moves, 0, 'f', 1);
    ss << QString("Average Time: %1 ms\n").arg(stats.avg_duration_ms, 0, 'f', 1);

    if (gamesPlayed_ >= 10 && stats.significant) {
        ss << "\n[Statistical Significance]\n";
        ss << QString("p-value: %1\n").arg(stats.p_value, 0, 'e', 3);
        ss << "Result: SIGNIFICANT (p < 0.05)";
    }

    resultText_->setText(detail);

    // Update p-value and significance rows in stats table
    statsTable_->setItem(6, 1, new QTableWidgetItem(QString::number(stats.p_value, 'e', 3)));
    statsTable_->setItem(7, 1, new QTableWidgetItem(stats.significant ? "Yes" : "No"));
}

void AIvsAIWindow::updateStatsTable() {
    // Row 0: Player 1 Wins
    statsTable_->setItem(0, 0, new QTableWidgetItem(tr("P1 Wins")));
    statsTable_->setItem(0, 1, new QTableWidgetItem(QString::number(gamesWon1_)));
    statsTable_->setItem(0, 2, new QTableWidgetItem("-"));
    statsTable_->setItem(0, 3, new QTableWidgetItem("N/A"));

    // Row 1: Player 2 Wins
    statsTable_->setItem(1, 0, new QTableWidgetItem(tr("P2 Wins")));
    statsTable_->setItem(1, 1, new QTableWidgetItem(QString::number(gamesWon2_)));
    statsTable_->setItem(1, 2, new QTableWidgetItem("-"));
    statsTable_->setItem(1, 3, new QTableWidgetItem("N/A"));

    // Row 2: Draws
    statsTable_->setItem(2, 0, new QTableWidgetItem(tr("Draws")));
    statsTable_->setItem(2, 1, new QTableWidgetItem(QString::number(draws_)));
    statsTable_->setItem(2, 2, new QTableWidgetItem("-"));
    statsTable_->setItem(2, 3, new QTableWidgetItem("N/A"));

    // Row 3: Win Rate 1 (with Pass/Fail)
    double winRate1 = gamesPlayed_ > 0 ? gamesWon1_ * 100.0 / gamesPlayed_ : 0;
    statsTable_->setItem(3, 0, new QTableWidgetItem(tr("Win Rate P1")));
    statsTable_->setItem(3, 1, new QTableWidgetItem(QString("%1%").arg(winRate1, 0, 'f', 1)));
    statsTable_->setItem(3, 2, new QTableWidgetItem(tr(">50%")));
    statsTable_->setItem(3, 3, new QTableWidgetItem(winRate1 > 50 ? "PASS" : "FAIL"));

    // Row 4: Win Rate 2 (with Pass/Fail)
    double winRate2 = gamesPlayed_ > 0 ? gamesWon2_ * 100.0 / gamesPlayed_ : 0;
    statsTable_->setItem(4, 0, new QTableWidgetItem(tr("Win Rate P2")));
    statsTable_->setItem(4, 1, new QTableWidgetItem(QString("%1%").arg(winRate2, 0, 'f', 1)));
    statsTable_->setItem(4, 2, new QTableWidgetItem(tr("-")));
    statsTable_->setItem(4, 3, new QTableWidgetItem("N/A"));

    // Row 5: Average Moves
    statsTable_->setItem(5, 0, new QTableWidgetItem(tr("Avg Moves")));
    statsTable_->setItem(5, 1, new QTableWidgetItem(QString::number(avgMoves_, 'f', 1)));
    statsTable_->setItem(5, 2, new QTableWidgetItem("-"));
    statsTable_->setItem(5, 3, new QTableWidgetItem("N/A"));

    // Row 6: p-value (with Pass/Fail)
    statsTable_->setItem(6, 0, new QTableWidgetItem(tr("p-value")));
    statsTable_->setItem(6, 1, new QTableWidgetItem("-"));  // Will be updated in onBattleCompleted
    statsTable_->setItem(6, 2, new QTableWidgetItem(tr("<0.05")));
    statsTable_->setItem(6, 3, new QTableWidgetItem("N/A"));

    // Row 7: Statistical Significance (with Pass/Fail)
    statsTable_->setItem(7, 0, new QTableWidgetItem(tr("Significant")));
    statsTable_->setItem(7, 1, new QTableWidgetItem("-"));  // Will be updated in onBattleCompleted
    statsTable_->setItem(7, 2, new QTableWidgetItem(tr("Yes")));
    statsTable_->setItem(7, 3, new QTableWidgetItem("N/A"));
}

void AIvsAIWindow::clearStats() {
    gamesPlayed_ = 0;
    gamesWon1_ = 0;
    gamesWon2_ = 0;
    draws_ = 0;
    avgMoves_ = 0;
    totalTime_ = std::chrono::milliseconds(0);

    progressBar_->setValue(0);
    progressLabel_->setText(tr("Ready"));
    currentMoveLabel_->setText(tr("Ready to start"));
    resultText_->clear();

    // Clear stats table with 4 columns: Metric, Value, Target, Status
    QStringList metrics = {
        tr("P1 Wins"), tr("P2 Wins"), tr("Draws"),
        tr("Win Rate P1"), tr("Win Rate P2"), tr("Avg Moves"),
        tr("p-value"), tr("Significant")
    };
    QStringList values = {"-", "-", "-", "-", "-", "-", "-", "-"};
    QStringList targets = {"-", "-", "-", ">50%", "-", "-", "<0.05", "Yes"};
    QStringList statuses = {"N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"};

    for (int i = 0; i < 8; ++i) {
        statsTable_->setItem(i, 0, new QTableWidgetItem(metrics[i]));
        statsTable_->setItem(i, 1, new QTableWidgetItem(values[i]));
        statsTable_->setItem(i, 2, new QTableWidgetItem(targets[i]));
        statsTable_->setItem(i, 3, new QTableWidgetItem(statuses[i]));
    }
}

void AIvsAIWindow::setControlsEnabled(bool enabled) {
    startButton_->setEnabled(enabled);
    stopButton_->setEnabled(!enabled);
    ai1TypeCombo_->setEnabled(enabled);
    ai2TypeCombo_->setEnabled(enabled);
    ai1Combo_->setEnabled(enabled);
    ai2Combo_->setEnabled(enabled);
    gamesSpinBox_->setEnabled(enabled);
    depth1Combo_->setEnabled(enabled);
    depth2Combo_->setEnabled(enabled);
    seedSpinBox_->setEnabled(enabled);
    randomSeedButton_->setEnabled(enabled);
    verificationModeCheckBox_->setEnabled(enabled);
    suiteCombo_->setEnabled(enabled);
    parallelCheckBox_->setEnabled(enabled);
    threadsSpinBox_->setEnabled(enabled && parallelCheckBox_->isChecked());
}

void AIvsAIWindow::onRandomSeedClicked() {
    // Generate random seed using current time
    std::random_device rd;
    std::mt19937_64 rng(rd());
    uint64_t randomSeed = rng();
    seedSpinBox_->setValue(static_cast<int>(randomSeed % 1000000000));
}

void AIvsAIWindow::updateRuntimeEstimate() {
    // Get AI types
    QString ai1TypeName = ai1TypeCombo_->currentText();
    QString ai2TypeName = ai2TypeCombo_->currentText();
    int depth1 = depth1Combo_->currentText().toInt();
    int depth2 = depth2Combo_->currentText().toInt();
    int games = gamesSpinBox_->value();

    // Use larger depth for estimation
    int maxDepth = qMax(depth1, depth2);

    // Get thread count for parallel processing
    int threads = parallelCheckBox_->isChecked() ? threadsSpinBox_->value() : 1;

    // Estimate runtime
    std::string estimate = SimpleRuntimeEstimator::getEstimateString(
        games,
        ai1TypeName.toStdString(),
        ai2TypeName.toStdString(),
        maxDepth,
        threads
    );

    runtimeEstimateLabel_->setText(tr("Estimated time: %1").arg(QString::fromStdString(estimate)));
}

void AIvsAIWindow::onRunAllValidationClicked() {
    // Disable all buttons during validation
    runAllValidationButton_->setEnabled(false);
    exportValidationButton_->setEnabled(false);
    startButton_->setEnabled(false);
    bitboardBenchmarkButton_->setEnabled(false);
    aiBenchmarkButton_->setEnabled(false);

    progressLabel_->setText(tr("Running validation tests..."));
    currentMoveLabel_->setText(tr("This may take several minutes..."));

    // Run validation in background thread
    QThread* thread = QThread::create([this]() {
        auto results = ValidationSuite::runAll();

        // Convert to QVariant for signal
        QVariantList variantResults;
        for (const auto& r : results) {
            QVariantMap map;
            map["name"] = QString::fromStdString(r.name);
            map["passed"] = r.passed;
            map["actual_value"] = r.actual_value;
            map["target_value"] = r.target_value;
            map["games_played"] = r.games_played;
            map["win_rate"] = r.win_rate;
            map["p_value"] = r.p_value;
            map["statistically_significant"] = r.statistically_significant;
            map["details"] = QString::fromStdString(r.details);
            map["duration_ms"] = static_cast<qint64>(r.duration.count());
            variantResults.append(map);
        }

        QMetaObject::invokeMethod(this, "onValidationComplete",
            Qt::QueuedConnection, Q_ARG(QVariant, QVariant(variantResults)));
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void AIvsAIWindow::onValidationComplete(const QVariant& results) {
    // Re-enable buttons
    runAllValidationButton_->setEnabled(true);
    startButton_->setEnabled(true);
    bitboardBenchmarkButton_->setEnabled(true);
    aiBenchmarkButton_->setEnabled(true);

    QVariantList variantResults = results.toList();
    validationResults_.clear();

    // Build report text
    QString report;
    QTextStream ss(&report);
    ss << "========================================\n";
    ss << "     Validation Report\n";
    ss << "========================================\n\n";

    int passCount = 0;
    for (const QVariant& v : variantResults) {
        QVariantMap map = v.toMap();
        QString name = map["name"].toString();
        bool passed = map["passed"].toBool();
        double winRate = map["win_rate"].toDouble();
        double target = map["target_value"].toDouble();
        int games = map["games_played"].toInt();
        qint64 duration = map["duration_ms"].toLongLong();

        if (passed) passCount++;

        ss << "Test: " << name << "\n";
        ss << "  Games: " << games << "\n";
        if (target >= 0) {
            ss << "  Win Rate: " << static_cast<int>(winRate * 100) << "%\n";
            ss << "  Target: " << static_cast<int>(target * 100) << "%\n";
        }
        ss << "  Status: " << (passed ? "PASS" : "FAIL") << "\n";
        ss << "  Duration: " << (duration / 1000) << "s\n";
        ss << "  Details: " << map["details"].toString() << "\n\n";

        // Store for export
        ValidationResult vr;
        vr.name = name.toStdString();
        vr.passed = passed;
        vr.actual_value = winRate;
        vr.target_value = target;
        vr.games_played = games;
        vr.win_rate = winRate;
        vr.duration = std::chrono::milliseconds(duration);
        vr.details = map["details"].toString().toStdString();
        validationResults_.push_back(vr);
    }

    ss << "========================================\n";
    ss << "Overall: " << passCount << "/" << variantResults.size() << " tests passed\n";
    ss << "========================================\n";

    resultText_->setText(report);
    progressLabel_->setText(tr("Validation complete: %1/%2 passed").arg(passCount).arg(variantResults.size()));
    currentMoveLabel_->setText(passCount == variantResults.size() ?
        tr("All validations PASSED") : tr("Some validations FAILED"));

    exportValidationButton_->setEnabled(true);
}

void AIvsAIWindow::onExportValidationClicked() {
    QString filename = QFileDialog::getSaveFileName(this,
        tr("Export Validation Report"),
        QString("validation_report_%1").arg(QDate::currentDate().toString("yyyyMMdd")),
        tr("Text Files (*.txt);;CSV Files (*.csv)"));

    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file for writing"));
        return;
    }

    QTextStream out(&file);
    out << QString::fromStdString(ValidationSuite::generateReport(validationResults_));
    file.close();

    QMessageBox::information(this, tr("Success"), tr("Report exported successfully"));
}

void AIvsAIWindow::onRunBitboardBenchmarkClicked() {
    // Disable buttons
    bitboardBenchmarkButton_->setEnabled(false);
    aiBenchmarkButton_->setEnabled(false);
    startButton_->setEnabled(false);

    progressLabel_->setText(tr("Running Bitboard Benchmark..."));
    currentMoveLabel_->setText(tr("Please wait..."));

    // Run benchmark in background
    QThread* thread = QThread::create([this]() {
        BitboardBenchmark benchmark;
        BitboardBenchmark::Config config;
        config.verbose = true;
        config.warmup = true;
        config.flip_iterations = 1000000;  // Reduced iterations for faster testing
        config.move_iterations = 100000;
        config.legal_iterations = 100000;
        config.copy_iterations = 100000;
        benchmark.setConfig(config);

        auto results = benchmark.runAllBenchmarks();

        QMetaObject::invokeMethod(this, "onBitboardBenchmarkComplete",
            Qt::QueuedConnection, Q_ARG(QVariant, QVariant::fromValue(results)));
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void AIvsAIWindow::onRunAIBenchmarkClicked() {
    // Disable buttons
    bitboardBenchmarkButton_->setEnabled(false);
    aiBenchmarkButton_->setEnabled(false);
    startButton_->setEnabled(false);

    progressLabel_->setText(tr("Running AI Benchmark..."));
    currentMoveLabel_->setText(tr("Please wait (this may take a while)..."));

    // Run benchmark in background
    QThread* thread = QThread::create([this]() {
        AISearchBenchmark benchmark;
        AISearchBenchmark::Config config;
        config.verbose = true;
        config.warmup = true;
        config.time_limit_ms = 2000;  // Reduced time for faster testing
        benchmark.setConfig(config);

        auto results = benchmark.runFullBenchmark();

        QMetaObject::invokeMethod(this, "onAIBenchmarkComplete",
            Qt::QueuedConnection, Q_ARG(QVariant, QVariant::fromValue(results)));
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

// Qt meta-object invocation to handle benchmark completion
void AIvsAIWindow::onBitboardBenchmarkComplete(const QVariant& results) {
    Q_UNUSED(results);

    // Re-enable buttons
    bitboardBenchmarkButton_->setEnabled(true);
    aiBenchmarkButton_->setEnabled(true);
    startButton_->setEnabled(true);

    progressLabel_->setText(tr("Bitboard Benchmark Complete"));
    currentMoveLabel_->setText(tr("Check console for detailed results"));

    // Show completion message
    QMessageBox::information(this, tr("Benchmark Complete"),
        tr("Bitboard benchmark completed!\n\nCheck the console output for detailed results."));
}

void AIvsAIWindow::onAIBenchmarkComplete(const QVariant& results) {
    Q_UNUSED(results);

    // Re-enable buttons
    bitboardBenchmarkButton_->setEnabled(true);
    aiBenchmarkButton_->setEnabled(true);
    startButton_->setEnabled(true);

    progressLabel_->setText(tr("AI Benchmark Complete"));
    currentMoveLabel_->setText(tr("Check console for detailed results"));

    // Show completion message
    QMessageBox::information(this, tr("Benchmark Complete"),
        tr("AI benchmark completed!\n\nCheck the console output for detailed results."));
}

} // namespace Reversi

