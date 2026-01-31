#include "ui/MenuWindow.h"
#include "ui/PvEWindow.h"
#include "ui/PvPWindow.h"
#include "ui_menuwindow.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>

MenuWindow::MenuWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MenuWindow) {
    ui->setupUi(this);
    setFixedSize(400, 300);

    // 设置样式
    ui->pveButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "    border-radius: 8px;"
        "    padding: 10px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #45a049;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3d8b40;"
        "}"
    );

    ui->pvpButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border-radius: 8px;"
        "    padding: 10px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1565C0;"
        "}"
    );

    setupConnections();
}

MenuWindow::~MenuWindow() {
    delete ui;
}

void MenuWindow::setupConnections() {
    connect(ui->pveButton, &QPushButton::clicked,
            this, &MenuWindow::onPvEButtonClicked);
    connect(ui->pvpButton, &QPushButton::clicked,
            this, &MenuWindow::onPvPButtonClicked);
}

void MenuWindow::onPvEButtonClicked() {
    qDebug() << "MenuWindow: Starting PvE game";
    emit startPvEGame();

    // 创建并显示 PvE 窗口
    PvEWindow* pveWindow = new PvEWindow(this);
    pveWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(pveWindow, &PvEWindow::backToMenu, this, [this, pveWindow]() {
        pveWindow->close();
        this->show();
    });

    pveWindow->show();
    this->hide();
}

void MenuWindow::onPvPButtonClicked() {
    qDebug() << "MenuWindow: Starting PvP game";
    emit startPvPGame();

    // 创建并显示 PvP 窗口
    PvPWindow* pvpWindow = new PvPWindow(this);
    pvpWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(pvpWindow, &PvPWindow::backToMenu, this, [this, pvpWindow]() {
        pvpWindow->close();
        this->show();
    });

    pvpWindow->show();
    this->hide();
}

