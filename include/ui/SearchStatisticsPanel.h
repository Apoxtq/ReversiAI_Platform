/**
 * @file SearchStatisticsPanel.h
 * @brief Real-time Statistics Panel - v0.9.0 Visualization Enhanced
 *
 * Displays real-time AI search statistics including depth, node count, NPS, etc.
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
 * @brief Real-time search statistics panel
 *
 * Displays various statistics during AI search process
 */
class SearchStatisticsPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent window pointer
     */
    explicit SearchStatisticsPanel(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~SearchStatisticsPanel() override;

    /**
     * @brief Update statistics data
     * @param stats Search statistics data
     */
    void updateStatistics(const SearchStats& stats);

    /**
     * @brief Clear display
     */
    void clear();

    /**
     * @brief Set panel title
     * @param title Title
     */
    void setTitle(const QString& title);

    /**
     * @brief Set whether to show detailed statistics
     * @param show Whether to show
     */
    void setShowDetails(bool show);

signals:
    /**
     * @brief Panel click signal (expandable)
     */
    void panelClicked();

private slots:
    /**
     * @brief Update display (timer triggered)
     */
    void updateDisplay();

private:
    /**
     * @brief Initialize UI components
     */
    void setupUI();

    /**
     * @brief Create statistics row
     * @param label Label
     * @param valueLabel Value label (output)
     */
    void createStatRow(const QString& label, QLabel*& valueLabel);

    /**
     * @brief Update single label
     */
    void updateLabel(QLabel* label, const QString& value, const QString& color = QString());

    // Title
    QGroupBox* groupBox_;
    QVBoxLayout* mainLayout_;

    // Basic statistics
    QLabel* depthLabel_;
    QLabel* maxDepthLabel_;
    QLabel* nodesLabel_;
    QLabel* npsLabel_;              // Nodes per second
    QLabel* timeLabel_;

    // Advanced statistics
    QLabel* cutoffsLabel_;
    QLabel* ttHitRateLabel_;        // Transposition table hit rate
    QLabel* killerHitRateLabel_;    // Killer move hit
    QLabel* historyHitRateLabel_;   // History heuristic hit
    QLabel* ttEntriesLabel_;        // Transposition table entries

    // Best move
    QLabel* bestMoveLabel_;
    QLabel* bestValueLabel_;

    // MCTS specific
    QLabel* simulationsLabel_;
    QLabel* winRateLabel_;

    // Progress display
    QProgressBar* searchProgress_;

    // Current data
    SearchStats currentStats_;

    // Display settings
    bool showDetails_;
    bool isSearching_;
};

} // namespace Reversi
