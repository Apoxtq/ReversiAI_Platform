#include "ui/MenuWindow.h"
#include "ui/PvEWindow.h"
#include "ui/PvPWindow.h"
#include "ui/NetworkLobbyWindow.h"
#include "ui/NetworkGameWindow.h"
#include "ui_menuwindow.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>

MenuWindow::MenuWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MenuWindow) {
    ui->setupUi(this);
    setFixedSize(400, 400);  // 增加高度以容纳新按钮

    // 设置 PvE 按钮样式
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

    // 设置 PvP 按钮样式
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
    connect(ui->networkButton, &QPushButton::clicked,
            this, &MenuWindow::onNetworkButtonClicked);
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

void MenuWindow::onNetworkButtonClicked() {
    qDebug() << "MenuWindow: Starting Network game";
    emit startPvNGame();

    // 创建并显示网络大厅
    NetworkLobbyWindow* lobbyWindow = new NetworkLobbyWindow(this);
    lobbyWindow->setAttribute(Qt::WA_DeleteOnClose);
    lobbyWindow->initNetwork();

    // 连接大厅信号到槽
    connect(lobbyWindow, &NetworkLobbyWindow::backToMenu, this, [this, lobbyWindow]() {
        lobbyWindow->close();
        this->show();
    });

    // 当加入游戏时，创建网络对战窗口
    connect(lobbyWindow, &NetworkLobbyWindow::joinGame,
            this, [this, lobbyWindow](const QHostAddress& hostAddress, quint16 port, const QString& playerName) {
        qDebug() << "MenuWindow: Joining network game at" << hostAddress.toString() << ":" << port;

        // 隐藏大厅，显示游戏窗口
        lobbyWindow->hide();

        // 创建网络对战窗口
        NetworkGameWindow* gameWindow = new NetworkGameWindow(this);
        gameWindow->setAttribute(Qt::WA_DeleteOnClose);
        gameWindow->initNetwork(hostAddress, port, playerName);

        // 连接返回菜单信号
        connect(gameWindow, &NetworkGameWindow::backToMenu, this, [this, gameWindow, lobbyWindow]() {
            gameWindow->close();
            lobbyWindow->show();  // 重新显示大厅
        });

        gameWindow->show();
    });

    // 当创建游戏时
    connect(lobbyWindow, &NetworkLobbyWindow::createGame,
            this, [this, lobbyWindow](const QString& roomName, const QString& playerName, const QJsonObject& settings) {
        Q_UNUSED(roomName)
        Q_UNUSED(settings)
        qDebug() << "MenuWindow: Creating network game as" << playerName;

        // 隐藏大厅，显示游戏窗口
        lobbyWindow->hide();

        // 创建网络对战窗口（作为主机）
        NetworkGameWindow* gameWindow = new NetworkGameWindow(this);
        gameWindow->setAttribute(Qt::WA_DeleteOnClose);
        gameWindow->startHosting(playerName, 45455);  // 默认端口

        // 连接返回菜单信号
        connect(gameWindow, &NetworkGameWindow::backToMenu, this, [this, gameWindow, lobbyWindow]() {
            gameWindow->close();
            lobbyWindow->show();  // 重新显示大厅
        });

        gameWindow->show();
    });

    lobbyWindow->show();
    this->hide();
}

