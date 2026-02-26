/**
 * @file AIvsAIWindow.cpp
 * @brief AI vs AI 对战窗口实现
 *
 * 实现AI自动对战的可视化界面
 */

#include "ui/AIvsAIWindow.h"
#include <QPainter>
#include <QFont>
#include <QTextStream>
#include <QtConcurrent/QtConcurrent>  // 使用完整路径

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
    // 主窗口设置
    centralWidget_ = new QWidget(this);
    setCentralWidget(centralWidget_);
    mainLayout_ = new QVBoxLayout(centralWidget_);

    // 标题
    QLabel* titleLabel = new QLabel(tr("AI vs AI Benchmark"), this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #2c3e50;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout_->addWidget(titleLabel);

    // 对战配置区域
    configGroup_ = new QGroupBox(tr("Battle Configuration"), this);
    configLayout_ = new QGridLayout(configGroup_);

    // AI1选择
    configLayout_->addWidget(new QLabel(tr("Player 1 (Black):"), this), 0, 0);
    ai1Combo_ = new QComboBox(this);
    ai1Combo_->addItem("MinimaxAI - Easy", QVariant::fromValue(Difficulty::EASY));
    ai1Combo_->addItem("MinimaxAI - Medium", QVariant::fromValue(Difficulty::MEDIUM));
    ai1Combo_->addItem("MinimaxAI - Hard", QVariant::fromValue(Difficulty::HARD));
    ai1Combo_->addItem("RandomAI", QVariant::fromValue(0));
    configLayout_->addWidget(ai1Combo_, 0, 1);

    configLayout_->addWidget(new QLabel(tr("Depth:"), this), 0, 2);
    depth1Combo_ = new QComboBox(this);
    depth1Combo_->addItem("2");
    depth1Combo_->addItem("3");
    depth1Combo_->addItem("4");
    depth1Combo_->addItem("5");
    depth1Combo_->setCurrentIndex(1);  // 默认3
    configLayout_->addWidget(depth1Combo_, 0, 3);

    // AI2选择
    configLayout_->addWidget(new QLabel(tr("Player 2 (White):"), this), 1, 0);
    ai2Combo_ = new QComboBox(this);
    ai2Combo_->addItem("MinimaxAI - Easy", QVariant::fromValue(Difficulty::EASY));
    ai2Combo_->addItem("MinimaxAI - Medium", QVariant::fromValue(Difficulty::MEDIUM));
    ai2Combo_->addItem("MinimaxAI - Hard", QVariant::fromValue(Difficulty::HARD));
    ai2Combo_->addItem("RandomAI", QVariant::fromValue(0));
    ai2Combo_->setCurrentIndex(3);  // 默认RandomAI
    configLayout_->addWidget(ai2Combo_, 1, 1);

    configLayout_->addWidget(new QLabel(tr("Depth:"), this), 1, 2);
    depth2Combo_ = new QComboBox(this);
    depth2Combo_->addItem("2");
    depth2Combo_->addItem("3");
    depth2Combo_->addItem("4");
    depth2Combo_->addItem("5");
    depth2Combo_->setCurrentIndex(1);  // 默认3
    configLayout_->addWidget(depth2Combo_, 1, 3);

    // 对局数量
    configLayout_->addWidget(new QLabel(tr("Games:"), this), 2, 0);
    gamesSpinBox_ = new QSpinBox(this);
    gamesSpinBox_->setRange(1, 1000);
    gamesSpinBox_->setValue(10);
    configLayout_->addWidget(gamesSpinBox_, 2, 1);

    mainLayout_->addWidget(configGroup_);

    // 进度显示
    QHBoxLayout* progressLayout = new QHBoxLayout();
    progressBar_ = new QProgressBar(this);
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(true);
    progressLayout->addWidget(progressBar_);

    progressLabel_ = new QLabel(tr("Ready"), this);
    progressLayout->addWidget(progressLabel_);
    mainLayout_->addLayout(progressLayout);

    // 当前状态显示
    currentMoveLabel_ = new QLabel(tr("Ready to start"), this);
    currentMoveLabel_->setAlignment(Qt::AlignCenter);
    currentMoveLabel_->setStyleSheet("font-size: 16px; color: #7f8c8d; padding: 10px;");
    mainLayout_->addWidget(currentMoveLabel_);

    // 控制按钮
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

    // 基准测试按钮区域
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

    // 统计结果区域
    statsGroup_ = new QGroupBox(tr("Results"), this);
    statsLayout_ = new QVBoxLayout(statsGroup_);

    statsTable_ = new QTableWidget(this);
    statsTable_->setRowCount(6);
    statsTable_->setColumnCount(2);
    statsTable_->setHorizontalHeaderLabels(QStringList() << tr("Metric") << tr("Value"));
    statsTable_->horizontalHeader()->setStretchLastSection(true);
    statsTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable_->setStyleSheet("font-size: 14px;");
    statsLayout_->addWidget(statsTable_);

    resultText_ = new QTextEdit(this);
    resultText_->setMaximumHeight(150);
    resultText_->setReadOnly(true);
    statsLayout_->addWidget(resultText_);

    mainLayout_->addWidget(statsGroup_);

    // 底部按钮
    backButton_ = new QPushButton(tr("Back to Menu"), this);
    backButton_->setStyleSheet("background-color: #95a5a6; color: white; padding: 8px 20px; font-size: 14px; border-radius: 5px;");
    QHBoxLayout* backLayout = new QHBoxLayout();
    backLayout->addStretch();
    backLayout->addWidget(backButton_);
    backLayout->addStretch();
    mainLayout_->addLayout(backLayout);

    // 初始化统计表
    clearStats();
}

void AIvsAIWindow::setupConnections() {
    connect(startButton_, &QPushButton::clicked, this, &AIvsAIWindow::onStartBattleClicked);
    connect(stopButton_, &QPushButton::clicked, this, &AIvsAIWindow::onStopBattleClicked);
    connect(exportButton_, &QPushButton::clicked, this, &AIvsAIWindow::onExportReportClicked);
    connect(backButton_, &QPushButton::clicked, this, &AIvsAIWindow::onBackClicked);
    connect(bitboardBenchmarkButton_, &QPushButton::clicked, this, &AIvsAIWindow::onRunBitboardBenchmarkClicked);
    connect(aiBenchmarkButton_, &QPushButton::clicked, this, &AIvsAIWindow::onRunAIBenchmarkClicked);
}

void AIvsAIWindow::onStartBattleClicked() {
    if (isRunning_) return;

    clearStats();

    // 配置对战
    battleConfig_.num_games = gamesSpinBox_->value();
    battleConfig_.verbose = false;
    battleConfig_.random_seed = 42;  // 可重现

    // Player 1
    Difficulty diff1 = ai1Combo_->currentData().value<Difficulty>();
    battleConfig_.player1 = AIStrategyFactory::createMinimaxAI(diff1);
    battleConfig_.player1_name = battleConfig_.player1->getName();
    battleConfig_.limits1.maxDepth = depth1Combo_->currentText().toInt();

    // Player 2
    int ai2Type = ai2Combo_->currentData().toInt();
    if (ai2Type == 0) {
        battleConfig_.player2 = AIStrategyFactory::createRandomAI();
    } else {
        Difficulty diff2 = ai2Combo_->currentData().value<Difficulty>();
        battleConfig_.player2 = AIStrategyFactory::createMinimaxAI(diff2);
    }
    battleConfig_.player2_name = battleConfig_.player2->getName();
    battleConfig_.limits2.maxDepth = depth2Combo_->currentText().toInt();

    // 创建定时器
    updateTimer_ = new QTimer(this);
    connect(updateTimer_, &QTimer::timeout, this, &AIvsAIWindow::updateProgress);

    // 开始对战
    isRunning_ = true;
    setControlsEnabled(false);

    // 使用线程方式运行
    progressBar_->setRange(0, battleConfig_.num_games);

    // 显示初始状态
    currentMoveLabel_->setText(tr("Battle in progress: %1 vs %2")
        .arg(QString::fromStdString(battleConfig_.player1_name))
        .arg(QString::fromStdString(battleConfig_.player2_name)));

    // 启动定时器（用于UI更新）
    updateTimer_->start(100);

    // 在后台线程运行对战
    QtConcurrent::run([this]() {
        BattleStats stats = BattleEngine::runBattle(battleConfig_);

        // 完成后更新UI
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

    // 写入报告
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

    // 获取当前进度 - 由于BattleEngine没有getCurrentGame，通过文件或信号获取
    // 这里简化为只更新标签
    progressLabel_->setText(tr("Battle in progress..."));
}

void AIvsAIWindow::onBattleCompleted(const BattleStats& stats) {
    isRunning_ = false;

    if (updateTimer_) {
        updateTimer_->stop();
    }

    // 更新统计
    gamesPlayed_ = stats.total_games;
    gamesWon1_ = stats.player1_wins;
    gamesWon2_ = stats.player2_wins;
    draws_ = stats.draws;
    avgMoves_ = stats.avg_moves;
    totalTime_ = std::chrono::milliseconds(static_cast<long long>(stats.avg_duration_ms * gamesPlayed_));

    // 更新UI
    updateStatsTable();
    setControlsEnabled(true);
    exportButton_->setEnabled(true);

    // 更新进度条
    progressBar_->setValue(gamesPlayed_);
    progressLabel_->setText(tr("Completed: %1 games").arg(gamesPlayed_));

    // 更新状态标签
    QString result = QString(tr("Battle Complete! %1 vs %2 - %1 won %3 games (%4%)"))
        .arg(QString::fromStdString(battleConfig_.player1_name))
        .arg(QString::fromStdString(battleConfig_.player2_name))
        .arg(gamesWon1_)
        .arg(gamesPlayed_ > 0 ? QString::number(gamesWon1_ * 100.0 / gamesPlayed_, 'f', 1) : "0");
    currentMoveLabel_->setText(result);

    // 显示详细结果
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
}

void AIvsAIWindow::updateStatsTable() {
    QStringList metrics = {
        tr("Player 1"),
        tr("Player 2"),
        tr("Draws"),
        tr("Win Rate 1"),
        tr("Win Rate 2"),
        tr("Avg Moves")
    };

    double winRate1 = gamesPlayed_ > 0 ? gamesWon1_ * 100.0 / gamesPlayed_ : 0;
    double winRate2 = gamesPlayed_ > 0 ? gamesWon2_ * 100.0 / gamesPlayed_ : 0;

    QStringList values = {
        QString::number(gamesWon1_),
        QString::number(gamesWon2_),
        QString::number(draws_),
        QString("%1%").arg(winRate1, 0, 'f', 1),
        QString("%1%").arg(winRate2, 0, 'f', 1),
        QString::number(avgMoves_, 'f', 1)
    };

    for (int i = 0; i < 6; ++i) {
        statsTable_->setItem(i, 0, new QTableWidgetItem(metrics[i]));
        statsTable_->setItem(i, 1, new QTableWidgetItem(values[i]));
    }
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

    QStringList metrics = {
        tr("Player 1"),
        tr("Player 2"),
        tr("Draws"),
        tr("Win Rate 1"),
        tr("Win Rate 2"),
        tr("Avg Moves")
    };

    QStringList values = {"-", "-", "-", "-", "-", "-"};

    for (int i = 0; i < 6; ++i) {
        statsTable_->setItem(i, 0, new QTableWidgetItem(metrics[i]));
        statsTable_->setItem(i, 1, new QTableWidgetItem(values[i]));
    }
}

void AIvsAIWindow::setControlsEnabled(bool enabled) {
    startButton_->setEnabled(enabled);
    stopButton_->setEnabled(!enabled);
    ai1Combo_->setEnabled(enabled);
    ai2Combo_->setEnabled(enabled);
    gamesSpinBox_->setEnabled(enabled);
    depth1Combo_->setEnabled(enabled);
    depth2Combo_->setEnabled(enabled);
}

void AIvsAIWindow::onRunBitboardBenchmarkClicked() {
    // 禁用按钮
    bitboardBenchmarkButton_->setEnabled(false);
    aiBenchmarkButton_->setEnabled(false);
    startButton_->setEnabled(false);

    progressLabel_->setText(tr("Running Bitboard Benchmark..."));
    currentMoveLabel_->setText(tr("Please wait..."));

    // 在后台运行基准测试
    QThread* thread = QThread::create([this]() {
        BitboardBenchmark benchmark;
        BitboardBenchmark::Config config;
        config.verbose = true;
        config.warmup = true;
        config.flip_iterations = 1000000;  // 减少迭代次数加快测试
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
    // 禁用按钮
    bitboardBenchmarkButton_->setEnabled(false);
    aiBenchmarkButton_->setEnabled(false);
    startButton_->setEnabled(false);

    progressLabel_->setText(tr("Running AI Benchmark..."));
    currentMoveLabel_->setText(tr("Please wait (this may take a while)..."));

    // 在后台运行基准测试
    QThread* thread = QThread::create([this]() {
        AISearchBenchmark benchmark;
        AISearchBenchmark::Config config;
        config.verbose = true;
        config.warmup = true;
        config.time_limit_ms = 2000;  // 减少时间加快测试
        benchmark.setConfig(config);

        auto results = benchmark.runFullBenchmark();

        QMetaObject::invokeMethod(this, "onAIBenchmarkComplete",
            Qt::QueuedConnection, Q_ARG(QVariant, QVariant::fromValue(results)));
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

// Qt元对象调用处理基准测试完成
void AIvsAIWindow::onBitboardBenchmarkComplete(const QVariant& results) {
    Q_UNUSED(results);

    // 重新启用按钮
    bitboardBenchmarkButton_->setEnabled(true);
    aiBenchmarkButton_->setEnabled(true);
    startButton_->setEnabled(true);

    progressLabel_->setText(tr("Bitboard Benchmark Complete"));
    currentMoveLabel_->setText(tr("Check console for detailed results"));

    // 显示完成消息
    QMessageBox::information(this, tr("Benchmark Complete"),
        tr("Bitboard benchmark completed!\n\nCheck the console output for detailed results."));
}

void AIvsAIWindow::onAIBenchmarkComplete(const QVariant& results) {
    Q_UNUSED(results);

    // 重新启用按钮
    bitboardBenchmarkButton_->setEnabled(true);
    aiBenchmarkButton_->setEnabled(true);
    startButton_->setEnabled(true);

    progressLabel_->setText(tr("AI Benchmark Complete"));
    currentMoveLabel_->setText(tr("Check console for detailed results"));

    // 显示完成消息
    QMessageBox::information(this, tr("Benchmark Complete"),
        tr("AI benchmark completed!\n\nCheck the console output for detailed results."));
}

} // namespace Reversi

