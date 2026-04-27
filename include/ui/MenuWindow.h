#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MenuWindow;
}
QT_END_NAMESPACE

/**
 * @class MenuWindow
 * @brief Main menu window - game mode selection
 *
 * Features:
 * - Display game title
 * - Provide entries for Player vs AI (PvE), Local 2P (PvP), and Network (PvN)
 * - Provide Watch AI Battle entry
 * - Provide Benchmark Suite entry
 * - Open corresponding game window when button is clicked
 */
class MenuWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MenuWindow(QWidget* parent = nullptr);
    ~MenuWindow() override;

signals:
    void startPvEGame();          // Start Player vs AI game
    void startPvPGame();          // Start Local 2P game
    void startPvNGame();          // Start Network game

private slots:
    void onPvEButtonClicked();
    void onPvPButtonClicked();
    void onAiVsAiButtonClicked();        // Watch AI Battle button
    void onNetworkButtonClicked();        // Network button
    void onBenchmarkButtonClicked();     // Benchmark Suite button

private:
    Ui::MenuWindow* ui;
    void setupConnections();
protected:
    void closeEvent(QCloseEvent* event) override;
};

