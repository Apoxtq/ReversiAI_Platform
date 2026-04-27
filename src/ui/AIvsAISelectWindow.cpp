/**
 * @file AIvsAISelectWindow.cpp
 * @brief AI vs AI mode selection window implementation
 *
 * @date 2026
 * @author Project Team
 * @license GPL-3.0
 */

#include "ui/AIvsAISelectWindow.h"
#include "ui/AIWatchWindow.h"
#include "ui/MenuWindow.h"
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QMessageBox>
#include <QDebug>
#include <QIcon>

namespace Reversi {

AIvsAISelectWindow::AIvsAISelectWindow(QWidget* parent)
    : QMainWindow(parent)
    , centralWidget_(nullptr)
    , mainLayout_(nullptr)
    , watchModeButton_(nullptr)
    , backButton_(nullptr)
    , parentWindow_(parent)
{
    setupUI();
}

AIvsAISelectWindow::~AIvsAISelectWindow()
{
    qDebug() << "AIvsAISelectWindow destroyed";
}

void AIvsAISelectWindow::setupUI()
{
    setWindowTitle(tr("AI vs AI"));
    setWindowIcon(QIcon(":/rsc/black.png"));
    setFixedSize(450, 400);

    // Central widget
    centralWidget_ = new QWidget(this);
    setCentralWidget(centralWidget_);

    // Main vertical layout
    mainLayout_ = new QVBoxLayout(centralWidget_);
    mainLayout_->setSpacing(20);
    mainLayout_->setContentsMargins(50, 60, 50, 40);

    // Title label
    QLabel* titleLabel = new QLabel(tr("Watch AI Battle"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 28px;"
        "    font-weight: bold;"
        "    color: #2c3e50;"
        "    padding: 20px;"
        "}"
    );
    mainLayout_->addWidget(titleLabel);

    // Subtitle
    QLabel* subtitleLabel = new QLabel(tr("Watch two AIs battle each other"), this);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 14px;"
        "    color: #7f8c8d;"
        "    padding-bottom: 20px;"
        "}"
    );
    mainLayout_->addWidget(subtitleLabel);

    // Spacer
    mainLayout_->addSpacing(20);

    // Watch Single Battle button
    watchModeButton_ = createButton(tr("Watch Single Battle"), "#FF9800");
    mainLayout_->addWidget(watchModeButton_);

    // Spacer
    mainLayout_->addSpacing(20);

    // Back button
    backButton_ = createButton(tr("Back to Menu"), "#95a5a6");
    mainLayout_->addWidget(backButton_);

    // Add stretch to push everything to center
    mainLayout_->addStretch();

    // Connect signals
    connect(watchModeButton_, &QPushButton::clicked,
            this, &AIvsAISelectWindow::onWatchModeClicked);
    connect(backButton_, &QPushButton::clicked,
            this, &AIvsAISelectWindow::onBackClicked);
}

QPushButton* AIvsAISelectWindow::createButton(const QString& text, const QString& color)
{
    QPushButton* button = new QPushButton(text, this);
    button->setMinimumHeight(50);
    button->setCursor(Qt::PointingHandCursor);

    QString styleSheet = QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: white;"
        "    border-radius: 8px;"
        "    padding: 12px 24px;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    opacity: 0.85;"
        "}"
        "QPushButton:pressed {"
        "    opacity: 0.7;"
        "}"
    ).arg(color);

    button->setStyleSheet(styleSheet);
    return button;
}

void AIvsAISelectWindow::onWatchModeClicked()
{
    qDebug() << "AIvsAISelectWindow: Opening Watch Single Battle";

    // Create and show AIWatchWindow
    AIWatchWindow* watchWindow = new AIWatchWindow(this);
    watchWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(watchWindow, &AIWatchWindow::backToMenu, this, [this, watchWindow]() {
        watchWindow->close();
        this->show();
    });

    watchWindow->show();
    this->hide();
}

void AIvsAISelectWindow::onBackClicked()
{
    qDebug() << "AIvsAISelectWindow: Returning to menu";

    // Find and show the main menu
    QWidget* mainMenu = parentWindow_;
    if (mainMenu) {
        mainMenu->show();
    } else {
        // Fallback: try to find MenuWindow in application
        foreach (QWidget* widget, QApplication::topLevelWidgets()) {
            if (qobject_cast<MenuWindow*>(widget)) {
                widget->show();
                mainMenu = widget;
                break;
            }
        }
    }

    if (mainMenu) {
        mainMenu->show();
    }

    this->close();
}

} // namespace Reversi
