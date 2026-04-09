/**
 * @file AIvsAISelectWindow.h
 * @brief AI vs AI mode selection window
 *
 * Provides a selection interface when user clicks "AI vs AI" button:
 * - Watch Single Battle: Single game with visual display
 *
 * @date 2026
 * @author Project Team
 * @license GPL-3.0
 */

#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

namespace Reversi {

/**
 * @class AIvsAISelectWindow
 * @brief Selection window for AI vs AI modes
 *
 * This window is displayed when user clicks "AI vs AI" button in main menu.
 * It offers:
 * - Watch Single Battle - single game with visual board
 */
class AIvsAISelectWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit AIvsAISelectWindow(QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~AIvsAISelectWindow() override;

signals:
    /**
     * @brief Signal emitted when user wants to return to main menu
     */
    void backToMenu();

private slots:
    /**
     * @brief Handler for Watch Single Battle button click
     */
    void onWatchModeClicked();

    /**
     * @brief Handler for Back button click
     */
    void onBackClicked();

private:
    /**
     * @brief Setup the UI components
     */
    void setupUI();

    /**
     * @brief Create a styled button
     * @param text Button text
     * @param color Background color
     * @return Styled QPushButton
     */
    QPushButton* createButton(const QString& text, const QString& color);

    // UI Components
    QWidget* centralWidget_;
    QVBoxLayout* mainLayout_;

    // Mode buttons
    QPushButton* watchModeButton_;
    QPushButton* backButton_;

    // Parent reference for window management
    QWidget* parentWindow_;
};

} // namespace Reversi
