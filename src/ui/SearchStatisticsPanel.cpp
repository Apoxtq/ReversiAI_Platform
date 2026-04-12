/**
 * @file SearchStatisticsPanel.cpp
 * @brief Real-time statistics panel implementation - v0.9.0 visualization enhancement
 */

#include "ui/SearchStatisticsPanel.h"
#include <QDebug>
#include <sstream>
#include <iomanip>

namespace Reversi {

// Helper function: format number
static QString formatNumber(int64_t num) {
    if (num >= 1000000) {
        return QString::number(num / 1000000.0, 'f', 1) + "M";
    } else if (num >= 1000) {
        return QString::number(num / 1000.0, 'f', 1) + "K";
    }
    return QString::number(num);
}

SearchStatisticsPanel::SearchStatisticsPanel(QWidget* parent)
    : QWidget(parent)
    , groupBox_(nullptr)
    , mainLayout_(nullptr)
    , depthLabel_(nullptr)
    , maxDepthLabel_(nullptr)
    , nodesLabel_(nullptr)
    , npsLabel_(nullptr)
    , timeLabel_(nullptr)
    , cutoffsLabel_(nullptr)
    , ttHitRateLabel_(nullptr)
    , killerHitRateLabel_(nullptr)
    , historyHitRateLabel_(nullptr)
    , ttEntriesLabel_(nullptr)
    , bestMoveLabel_(nullptr)
    , bestValueLabel_(nullptr)
    , simulationsLabel_(nullptr)
    , winRateLabel_(nullptr)
    , searchProgress_(nullptr)
    , showDetails_(true)
    , isSearching_(false)
{
    setupUI();
    clear();
}

SearchStatisticsPanel::~SearchStatisticsPanel() = default;

void SearchStatisticsPanel::setupUI() {
    // Main layout
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(0, 0, 0, 0);

    // Group box
    groupBox_ = new QGroupBox(tr("Search Statistics"), this);
    QVBoxLayout* boxLayout = new QVBoxLayout();

    // Grid layout for statistics items
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(8);

    int row = 0;

    // Basic statistics
    createStatRow(tr("Depth:"), depthLabel_);
    gridLayout->addWidget(new QLabel(tr("Depth:")), row, 0);
    gridLayout->addWidget(depthLabel_, row++, 1);

    createStatRow(tr("Max Depth:"), maxDepthLabel_);
    gridLayout->addWidget(new QLabel(tr("Max:")), row, 0);
    gridLayout->addWidget(maxDepthLabel_, row++, 1);

    createStatRow(tr("0"), nodesLabel_);
    gridLayout->addWidget(new QLabel(tr("Nodes:")), row, 0);
    gridLayout->addWidget(nodesLabel_, row++, 1);

    createStatRow(tr("0"), npsLabel_);
    gridLayout->addWidget(new QLabel(tr("NPS:")), row, 0);
    gridLayout->addWidget(npsLabel_, row++, 1);

    createStatRow(tr("0.0s"), timeLabel_);
    gridLayout->addWidget(new QLabel(tr("Time:")), row, 0);
    gridLayout->addWidget(timeLabel_, row++, 1);

    // Detailed statistics (collapsible)
    row++; // Empty row

    createStatRow(tr("0"), cutoffsLabel_);
    gridLayout->addWidget(new QLabel(tr("Cutoffs:")), row, 0);
    gridLayout->addWidget(cutoffsLabel_, row++, 1);

    createStatRow(tr("0%"), ttHitRateLabel_);
    gridLayout->addWidget(new QLabel(tr("TT Hit:")), row, 0);
    gridLayout->addWidget(ttHitRateLabel_, row++, 1);

    createStatRow(tr("0%"), killerHitRateLabel_);
    gridLayout->addWidget(new QLabel(tr("Killer:")), row, 0);
    gridLayout->addWidget(killerHitRateLabel_, row++, 1);

    createStatRow(tr("0%"), historyHitRateLabel_);
    gridLayout->addWidget(new QLabel(tr("History:")), row, 0);
    gridLayout->addWidget(historyHitRateLabel_, row++, 1);

    createStatRow(tr("0"), ttEntriesLabel_);
    gridLayout->addWidget(new QLabel(tr("TT Size:")), row, 0);
    gridLayout->addWidget(ttEntriesLabel_, row++, 1);

    row++; // Empty row

    // Best move
    createStatRow(tr("--"), bestMoveLabel_);
    gridLayout->addWidget(new QLabel(tr("Best Move:")), row, 0);
    gridLayout->addWidget(bestMoveLabel_, row++, 1);

    createStatRow(tr("0"), bestValueLabel_);
    gridLayout->addWidget(new QLabel(tr("Value:")), row, 0);
    gridLayout->addWidget(bestValueLabel_, row++, 1);

    row++; // Empty row

    // MCTS statistics
    createStatRow(tr("0"), simulationsLabel_);
    gridLayout->addWidget(new QLabel(tr("Simulations:")), row, 0);
    gridLayout->addWidget(simulationsLabel_, row++, 1);

    createStatRow(tr("0%"), winRateLabel_);
    gridLayout->addWidget(new QLabel(tr("Win Rate:")), row, 0);
    gridLayout->addWidget(winRateLabel_, row++, 1);

    boxLayout->addLayout(gridLayout);

    // Progress bar
    searchProgress_ = new QProgressBar(this);
    searchProgress_->setRange(0, 100);
    searchProgress_->setValue(0);
    searchProgress_->setTextVisible(false);
    searchProgress_->setFixedHeight(4);
    boxLayout->addWidget(searchProgress_);

    groupBox_->setLayout(boxLayout);
    mainLayout_->addWidget(groupBox_);

    // Set stylesheet
    setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            border: 1px solid #555;
            border-radius: 4px;
            margin-top: 8px;
            padding-top: 8px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
        QLabel {
            color: #ccc;
        }
        QProgressBar {
            border: none;
            background-color: #333;
        }
        QProgressBar::chunk {
            background-color: #4a90e2;
        }
    )");
}

void SearchStatisticsPanel::createStatRow(const QString& initialValue, QLabel*& label) {
    label = new QLabel(initialValue, this);
    label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QFont monoFont("Consolas", 9);
    monoFont.setStyleHint(QFont::Monospace);
    label->setFont(monoFont);
}

void SearchStatisticsPanel::updateStatistics(const SearchStats& stats) {
    currentStats_ = stats;
    isSearching_ = stats.isSearching;

    // Update display
    updateDisplay();
}

void SearchStatisticsPanel::updateDisplay() {
    // Depth
    updateLabel(depthLabel_, QString::number(currentStats_.currentDepth));
    updateLabel(maxDepthLabel_, QString::number(currentStats_.maxDepth));

    // Nodes
    updateLabel(nodesLabel_, formatNumber(currentStats_.nodesExplored));

    // NPS
    updateLabel(npsLabel_, QString::fromStdString(currentStats_.getNPSString()) + "/s");

    // Time
    updateLabel(timeLabel_, QString::fromStdString(currentStats_.getTimeString()) + "s");

    // Detailed statistics
    if (showDetails_) {
        updateLabel(cutoffsLabel_, formatNumber(currentStats_.cutoffs));
        updateLabel(ttHitRateLabel_, QString::number(currentStats_.ttHitRate * 100, 'f', 1) + "%");
        updateLabel(killerHitRateLabel_, QString::number(currentStats_.killerHitRate * 100, 'f', 1) + "%");
        updateLabel(historyHitRateLabel_, QString::number(currentStats_.historyHitRate * 100, 'f', 1) + "%");
        updateLabel(ttEntriesLabel_, formatNumber(currentStats_.ttEntries));
    }

    // Best move
    if (currentStats_.bestMove >= 0) {
        int col = currentStats_.bestMove % 8;
        int row = currentStats_.bestMove / 8;
        QString coord = QString(QChar('A' + col)) + QString::number(row + 1);
        updateLabel(bestMoveLabel_, coord);
    } else {
        updateLabel(bestMoveLabel_, "--");
    }
    updateLabel(bestValueLabel_, QString::number(currentStats_.bestValue));

    // MCTS statistics
    if (showDetails_) {
        updateLabel(simulationsLabel_, formatNumber(currentStats_.simulations));
        updateLabel(winRateLabel_, QString::number(currentStats_.winRate * 100, 'f', 1) + "%");
    }

    // Progress bar
    if (currentStats_.isSearching) {
        searchProgress_->setRange(0, 0); // Indeterminate
        searchProgress_->setValue(0);
    } else if (currentStats_.isComplete) {
        searchProgress_->setRange(0, 100);
        searchProgress_->setValue(100);
    } else {
        searchProgress_->setRange(0, 100);
        searchProgress_->setValue(0);
    }
}

void SearchStatisticsPanel::updateLabel(QLabel* label, const QString& value, const QString& color) {
    if (label) {
        label->setText(value);
        if (!color.isEmpty()) {
            label->setStyleSheet(QString("color: %1;").arg(color));
        }
    }
}

void SearchStatisticsPanel::clear() {
    currentStats_.reset();
    isSearching_ = false;

    updateLabel(depthLabel_, "0");
    updateLabel(maxDepthLabel_, "0");
    updateLabel(nodesLabel_, "0");
    updateLabel(npsLabel_, "0/s");
    updateLabel(timeLabel_, "0.0s");
    updateLabel(cutoffsLabel_, "0");
    updateLabel(ttHitRateLabel_, "0%");
    updateLabel(killerHitRateLabel_, "0%");
    updateLabel(historyHitRateLabel_, "0%");
    updateLabel(ttEntriesLabel_, "0");
    updateLabel(bestMoveLabel_, "--");
    updateLabel(bestValueLabel_, "0");
    updateLabel(simulationsLabel_, "0");
    updateLabel(winRateLabel_, "0%");

    searchProgress_->setRange(0, 100);
    searchProgress_->setValue(0);
}

void SearchStatisticsPanel::setTitle(const QString& title) {
    if (groupBox_) {
        groupBox_->setTitle(title);
    }
}

void SearchStatisticsPanel::setShowDetails(bool show) {
    showDetails_ = show;

    // Show/hide detailed statistics labels
    if (cutoffsLabel_) cutoffsLabel_->setVisible(show);
    if (ttHitRateLabel_) ttHitRateLabel_->setVisible(show);
    if (killerHitRateLabel_) killerHitRateLabel_->setVisible(show);
    if (historyHitRateLabel_) historyHitRateLabel_->setVisible(show);
    if (ttEntriesLabel_) ttEntriesLabel_->setVisible(show);
    if (simulationsLabel_) simulationsLabel_->setVisible(show);
    if (winRateLabel_) winRateLabel_->setVisible(show);
}

} // namespace Reversi
