/*
    ReversiAI_Platform - Network Module

    @file networkgamewindow.cpp
    @brief Network multiplayer game window implementation
    @date 2026
    @author Project Team
    @license GPL-3.0

    Reference: PvPWindow structure and QtReversi widget.cpp
*/

#include "ui/NetworkGameWindow.h"
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDebug>

NetworkGameWindow::NetworkGameWindow(QWidget* parent)
    : QMainWindow(parent)
    , networkClient_(nullptr)
    , synchronizer_(nullptr)
    , reconnector_(nullptr)
    , localPlayerName_("Player")
    , opponentPlayerName_("Opponent")
    , isHost_(false)
    , isReconnecting_(false)
    , currentLatency_(0)
{
    // 初始化UI
    initGameUI();
    setupNetworkUI();
    
    // 创建GameController（网络PvP模式）
    gameController_ = std::make_unique<Reversi::GameController>(this);
    
    // 加载资源
    loadResources();
    
    // 连接游戏信号
    setupGameConnections();
}

NetworkGameWindow::~NetworkGameWindow()
{
    disconnectFromGame();
}

// ==================== UI Initialization ====================

void NetworkGameWindow::initGameUI()
{
    // Reference: PvPWindow initUI pattern
    
    setWindowTitle(tr("Network Game - ReversiAI"));
    resize(800, 600);
    
    // 创建主布局
    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // 左侧：棋盘区域
    QVBoxLayout* boardLayout = new QVBoxLayout();
    boardLayout->setContentsMargins(0, 0, 0, 0);
    
    // 棋盘标签
    QLabel* boardLabel = new QLabel(this);
    boardLabel->setFixedSize(400, 400);
    boardLabel->setStyleSheet("background-color: #000000;");
    boardLayout->addWidget(boardLabel);
    
    // 游戏控制按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    startButton_ = new QPushButton(tr("Start Game"), this);
    startButton_->setStyleSheet(
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "    border-radius: 5px;"
        "    padding: 8px;"
        "}"
    );
    buttonLayout->addWidget(startButton_);
    
    undoButton_ = new QPushButton(tr("Undo"), this);
    undoButton_->setEnabled(false);
    undoButton_->setStyleSheet(
        "QPushButton {"
        "    background-color: #FF9800;"
        "    color: white;"
        "    border-radius: 5px;"
        "    padding: 8px;"
        "}"
    );
    buttonLayout->addWidget(undoButton_);
    
    backButton_ = new QPushButton(tr("Back"), this);
    backButton_->setStyleSheet(
        "QPushButton {"
        "    background-color: #f44336;"
        "    color: white;"
        "    border-radius: 5px;"
        "    padding: 8px;"
        "}"
    );
    buttonLayout->addWidget(backButton_);
    
    boardLayout->addLayout(buttonLayout);
    mainLayout->addLayout(boardLayout, 1);
    
    // 右侧：网络信息和聊天
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->setContentsMargins(10, 0, 0, 0);
    
    // 连接状态组
    connectionGroup_ = new QGroupBox(tr("Network Status"), this);
    QVBoxLayout* connectionLayout = new QVBoxLayout();
    
    // 连接状态
    QHBoxLayout* statusLayout = new QHBoxLayout();
    QLabel* statusText = new QLabel(tr("Status:"), this);
    statusLayout->addWidget(statusText);
    connectionStatusLabel_ = new QLabel(tr("Disconnected"), this);
    connectionStatusLabel_->setStyleSheet("color: gray;");
    statusLayout->addWidget(connectionStatusLabel_);
    statusLayout->addStretch();
    connectionLayout->addLayout(statusLayout);
    
    // 对手名称
    QHBoxLayout* opponentLayout = new QHBoxLayout();
    QLabel* opponentText = new QLabel(tr("Opponent:"), this);
    opponentLayout->addWidget(opponentText);
    opponentNameLabel_ = new QLabel(tr("Waiting..."), this);
    opponentNameLabel_->setStyleSheet("font-weight: bold; color: blue;");
    opponentLayout->addWidget(opponentNameLabel_);
    opponentLayout->addStretch();
    connectionLayout->addLayout(opponentLayout);
    
    // 延迟显示
    QHBoxLayout* latencyLayout = new QHBoxLayout();
    QLabel* latencyText = new QLabel(tr("Latency:"), this);
    latencyLayout->addWidget(latencyText);
    latencyLCD_ = new QLCDNumber(this);
    latencyLCD_->setDigitCount(4);
    latencyLCD_->setSegmentStyle(QLCDNumber::Flat);
    latencyLCD_->setStyleSheet("color: gray;");
    latencyLayout->addWidget(latencyLCD_);
    latencyLayout->addStretch();
    connectionLayout->addLayout(latencyLayout);
    
    connectionGroup_->setLayout(connectionLayout);
    rightLayout->addWidget(connectionGroup_);
    
    // 聊天区域
    QGroupBox* chatGroup = new QGroupBox(tr("Chat"), this);
    QVBoxLayout* chatLayout = new QVBoxLayout();
    
    chatDisplay_ = new QTextEdit(this);
    chatDisplay_->setReadOnly(true);
    chatDisplay_->setMaximumHeight(150);
    chatLayout->addWidget(chatDisplay_);
    
    QHBoxLayout* chatInputLayout = new QHBoxLayout();
    chatInput_ = new QLineEdit(this);
    chatInput_->setPlaceholderText(tr("Type a message..."));
    chatInputLayout->addWidget(chatInput_);
    
    sendChatButton_ = new QPushButton(tr("Send"), this);
    sendChatButton_->setStyleSheet(
        "QPushButton {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border-radius: 5px;"
        "    padding: 5px;"
        "}"
    );
    chatInputLayout->addWidget(sendChatButton_);
    
    chatLayout->addLayout(chatInputLayout);
    chatGroup->setLayout(chatLayout);
    rightLayout->addWidget(chatGroup);
    
    // 添加到主布局
    mainLayout->addLayout(rightLayout, 0);
    
    // 设置中央部件
    QWidget* central = new QWidget(this);
    central->setLayout(mainLayout);
    setCentralWidget(central);
}

void NetworkGameWindow::setupNetworkUI()
{
    // 创建延迟更新定时器
    latencyUpdateTimer_ = new QTimer(this);
    latencyUpdateTimer_->setInterval(5000);  // 5 seconds
    
    // 初始状态
    connectionStatusLabel_->setText(tr("Disconnected"));
    connectionStatusLabel_->setStyleSheet("color: gray;");
    
    // 禁用聊天直到连接
    chatInput_->setEnabled(false);
    sendChatButton_->setEnabled(false);
}

void NetworkGameWindow::loadResources()
{
    // 加载棋盘和棋子图片
    // 实际项目中应该使用资源文件
    background.load(":/rsc/board.png");
    black.load(":/rsc/black.png");
    white.load(":/rsc/white.png");
    hintwhite.load(":/rsc/whitepotential.png");
    hintblack.load(":/rsc/blackpotential.png");
    hintred.load(":/rsc/redpotential.png");
}

void NetworkGameWindow::setupGameConnections()
{
    // Reference: PvPWindow setupConnections pattern
    
    // 按钮连接
    connect(startButton_, &QPushButton::clicked, this, &NetworkGameWindow::onStartGameClicked);
    connect(undoButton_, &QPushButton::clicked, this, &NetworkGameWindow::onUndoClicked);
    connect(backButton_, &QPushButton::clicked, this, &NetworkGameWindow::onBackToMenuClicked);
    
    // GameController 信号槽连接
    connect(gameController_.get(), &Reversi::GameController::gameStarted,
            this, &NetworkGameWindow::onGameStarted);
    connect(gameController_.get(), &Reversi::GameController::phaseChanged,
            this, &NetworkGameWindow::onPhaseChanged);
    connect(gameController_.get(), &Reversi::GameController::turnChanged,
            this, &NetworkGameWindow::onTurnChanged);
    connect(gameController_.get(), &Reversi::GameController::moveMade,
            this, &NetworkGameWindow::onMoveMade);
    connect(gameController_.get(), &Reversi::GameController::gameEnded,
            this, &NetworkGameWindow::onGameEnded);
    connect(gameController_.get(), &Reversi::GameController::errorOccurred,
            this, &NetworkGameWindow::onErrorOccurred);
}

void NetworkGameWindow::setupNetworkConnections()
{
    // Reference: QtReversi widget.cpp signal pattern
    
    // 连接信号
    connect(networkClient_, &Network::NetworkClient::connected,
            this, &NetworkGameWindow::onConnected);
    connect(networkClient_, &Network::NetworkClient::disconnected,
            this, &NetworkGameWindow::onDisconnected);
    connect(networkClient_, &Network::NetworkClient::errorOccurred,
            this, &NetworkGameWindow::onNetworkError);
    connect(networkClient_, &Network::NetworkClient::connectionTimeout,
            this, &NetworkGameWindow::onConnectionTimeout);
    
    // 重连信号
    connect(reconnector_, &Network::ReconnectionManager::reconnectingChanged,
            this, &NetworkGameWindow::onReconnectingChanged);
    connect(reconnector_, &Network::ReconnectionManager::reconnected,
            this, &NetworkGameWindow::onReconnected);
    connect(reconnector_, &Network::ReconnectionManager::reconnectFailed,
            this, &NetworkGameWindow::onReconnectFailed);
    connect(reconnector_, &Network::ReconnectionManager::maxAttemptsReached,
            this, &NetworkGameWindow::onMaxAttemptsReached);
    
    // 游戏信号
    connect(networkClient_, &Network::NetworkClient::moveReceived,
            this, &NetworkGameWindow::onMoveReceived);
    connect(networkClient_, &Network::NetworkClient::gameStateReceived,
            this, &NetworkGameWindow::onGameStateReceived);
    
    // 延迟信号
    connect(networkClient_, &Network::NetworkClient::pongReceived,
            this, &NetworkGameWindow::onPongReceived);
    
    // 聊天信号
    connect(sendChatButton_, &QPushButton::clicked, this, &NetworkGameWindow::onSendChatMessage);
    connect(chatInput_, &QLineEdit::returnPressed, this, &NetworkGameWindow::onSendChatMessage);
}

// ==================== Initialization ====================

void NetworkGameWindow::initNetwork(const QHostAddress& hostAddress, quint16 port, const QString& playerName)
{
    qDebug() << "Initializing network for game with" << hostAddress.toString() << ":" << port;
    
    localPlayerName_ = playerName;
    isHost_ = false;
    
    // 创建网络模块
    networkClient_ = new Network::NetworkClient(this);
    synchronizer_ = new Network::GameSynchronizer(this);
    reconnector_ = new Network::ReconnectionManager(this);
    
    // 连接网络信号
    setupNetworkConnections();
    
    // 连接
    connectToHost(hostAddress, port, playerName);
}

void NetworkGameWindow::startHosting(const QString& playerName, quint16 port)
{
    Q_UNUSED(port)
    qDebug() << "Starting hosted game as" << playerName;
    
    localPlayerName_ = playerName;
    isHost_ = true;
    
    // 创建网络模块
    networkClient_ = new Network::NetworkClient(this);
    synchronizer_ = new Network::GameSynchronizer(this);
    reconnector_ = new Network::ReconnectionManager(this);
    
    // 连接网络信号
    setupNetworkConnections();
    
    // 主机等待连接
    connectionStatusLabel_->setText(tr("Waiting for opponent..."));
    connectionStatusLabel_->setStyleSheet("color: blue;");
    
    qDebug() << "Ready to host as" << playerName;
}

void NetworkGameWindow::connectToHost(const QHostAddress& address, quint16 port, const QString& playerName)
{
    qDebug() << "Connecting to" << address.toString() << ":" << port;
    
    localPlayerName_ = playerName;
    connectionStatusLabel_->setText(tr("Connecting..."));
    connectionStatusLabel_->setStyleSheet("color: blue;");
    
    networkClient_->connectToHost(address, port);
}

void NetworkGameWindow::disconnectFromGame()
{
    qDebug() << "Disconnecting from game";
    
    // 停止重连
    if (reconnector_) {
        reconnector_->cancelReconnection();
    }
    
    // 停止延迟更新
    if (latencyUpdateTimer_) {
        latencyUpdateTimer_->stop();
    }
    
    // 断开网络
    if (networkClient_) {
        networkClient_->disconnectFromHost();
    }
    
    updateConnectionStatus();
}

// ==================== Paint Events ====================

void NetworkGameWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    // 绘制棋盘
    if (!background.isNull()) {
        painter.drawPixmap(0, 0, 400, 400, background);
    }
    
    // 获取当前状态
    const Reversi::Board& board = gameController_->getBoard();
    Reversi::PlayerColor currentPlayer = gameController_->getCurrentPlayer();
    
    // 确定当前显示的棋子颜色
    int tile = (currentPlayer == Reversi::PlayerColor::Black) ? 2 : 1;
    
    // 获取合法移动并绘制高亮
    int markHaveDraw[8][8] = {0};
    auto validMoves = board.getValidMoves();
    for (const auto& move : validMoves) {
        markHaveDraw[move.row][move.col] = tile;
    }
    
    // 绘制棋子和高亮
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int cellValue = board.at(j, i);
            
            if (cellValue == 1) {  // 白子
                if (!white.isNull()) {
                    painter.drawPixmap(i * 50, j * 50, 50, 50, white);
                }
            } else if (cellValue == 2) {  // 黑子
                if (!black.isNull()) {
                    painter.drawPixmap(i * 50, j * 50, 50, 50, black);
                }
            }
            
            // 绘制合法移动高亮
            if (markHaveDraw[j][i] == 2 && cellValue == 0) {
                if (!hintblack.isNull()) {
                    painter.drawPixmap(i * 50, j * 50, 50, 50, hintblack);
                }
            } else if (markHaveDraw[j][i] == 1 && cellValue == 0) {
                if (!hintwhite.isNull()) {
                    painter.drawPixmap(i * 50, j * 50, 50, 50, hintwhite);
                }
            }
        }
    }
}

void NetworkGameWindow::mousePressEvent(QMouseEvent* e)
{
    // 处理点击事件
    int x = e->pos().x();
    int y = e->pos().y();
    
    if (x >= 0 && x <= 400 && y >= 0 && y <= 400) {
        int col = x / 50;
        int row = y / 50;
        
        // 检查是否是当前玩家的回合
        Reversi::GamePhase phase = gameController_->getCurrentPhase();
        if (phase == Reversi::GamePhase::HumanTurn || phase == Reversi::GamePhase::AITurn) {
            // 发送移动到网络
            sendLocalMove(row, col);
        }
    }
}

void NetworkGameWindow::closeEvent(QCloseEvent* event)
{
    // 询问用户
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("Close Game"),
        tr("Are you sure you want to leave the game?"),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        disconnectFromGame();
        event->accept();
    } else {
        event->ignore();
    }
}

// ==================== Game Slots ====================

void NetworkGameWindow::onStartGameClicked()
{
    qDebug() << "NetworkGameWindow: Starting game";
    gameController_->startNewGame(Reversi::GameMode::PvP, Reversi::PlayerColor::Black);
    
    addChatMessage(tr("System"), tr("Game started!"));
}

void NetworkGameWindow::onUndoClicked()
{
    qDebug() << "NetworkGameWindow: Undo requested";
    // 网络模式下，悔棋需要对方同意
    addChatMessage(tr("System"), tr("Undo requested - waiting for opponent..."));
}

void NetworkGameWindow::onBackToMenuClicked()
{
    emit backToMenu();
}

void NetworkGameWindow::onGameStarted(Reversi::GameMode mode, Reversi::PlayerColor humanColor)
{
    qDebug() << "Game started in network mode";
    undoButton_->setEnabled(true);
    
    addChatMessage(tr("System"), tr("Game started!"));
    update();
}

void NetworkGameWindow::onPhaseChanged(Reversi::GamePhase phase)
{
    qDebug() << "Phase changed:" << static_cast<int>(phase);
    update();
}

void NetworkGameWindow::onTurnChanged(Reversi::PlayerColor player)
{
    qDebug() << "Turn changed:" << (player == Reversi::PlayerColor::Black ? "Black" : "White");
    update();
}

void NetworkGameWindow::onMoveMade(int row, int col, Reversi::PlayerColor player)
{
    QString playerStr = (player == Reversi::PlayerColor::Black) ? "Black" : "White";
    qDebug() << "Move made:" << playerStr << "@" << row << "," << col;
    update();
}

void NetworkGameWindow::onGameEnded(Reversi::GameResult result)
{
    qDebug() << "Game ended:" << static_cast<int>(result);
    
    QString winner;
    switch (result) {
        case Reversi::GameResult::BlackWins:
            winner = "Black";
            break;
        case Reversi::GameResult::WhiteWins:
            winner = "White";
            break;
        case Reversi::GameResult::Draw:
            winner = "Draw";
            break;
        default:
            winner = "Unknown";
    }
    
    addChatMessage(tr("System"), tr("Game over! Winner: %1").arg(winner));
    undoButton_->setEnabled(false);
}

void NetworkGameWindow::onErrorOccurred(const QString& message)
{
    qDebug() << "Game error:" << message;
    addChatMessage(tr("Error"), message);
}

// ==================== Network Slots ====================

void NetworkGameWindow::onConnected()
{
    qDebug() << "Connected to network game";
    
    connectionStatusLabel_->setText(tr("Connected"));
    connectionStatusLabel_->setStyleSheet("color: green;");
    
    // 启用聊天
    chatInput_->setEnabled(true);
    sendChatButton_->setEnabled(true);
    
    // 保存状态用于重连
    if (networkClient_) {
        reconnector_->saveState(QJsonObject());
    }
    
    // 开始延迟更新
    latencyUpdateTimer_->start();
    networkClient_->sendPing();
    
    addChatMessage(tr("System"), tr("Connected to game"));
}

void NetworkGameWindow::onDisconnected()
{
    qDebug() << "Disconnected from network game";
    
    if (!isReconnecting_) {
        handleConnectionLoss();
    }
}

void NetworkGameWindow::onNetworkError(Network::NetworkError error, const QString& message)
{
    qDebug() << "Network error:" << static_cast<int>(error) << "-" << message;
    
    QString errorMsg = tr("Network error: %1").arg(message);
    addChatMessage(tr("System"), errorMsg);
    
    // 检查是否需要重连
    if (error == Network::NetworkError::ConnectionReset ||
        error == Network::NetworkError::ConnectionTimeout ||
        error == Network::NetworkError::ConnectionRefused) {
        handleConnectionLoss();
    }
}

void NetworkGameWindow::onConnectionTimeout()
{
    qDebug() << "Connection timeout";
    handleConnectionLoss();
}

void NetworkGameWindow::onReconnectingChanged(bool state)
{
    isReconnecting_ = state;
    
    if (state) {
        connectionStatusLabel_->setText(tr("Reconnecting..."));
        connectionStatusLabel_->setStyleSheet("color: orange;");
    } else {
        connectionStatusLabel_->setText(tr("Disconnected"));
        connectionStatusLabel_->setStyleSheet("color: red;");
    }
}

void NetworkGameWindow::onReconnected()
{
    qDebug() << "Reconnected successfully";
    
    connectionStatusLabel_->setText(tr("Reconnected"));
    connectionStatusLabel_->setStyleSheet("color: green;");
    
    addChatMessage(tr("System"), tr("Reconnected to game"));
    
    emit reconnected();
}

void NetworkGameWindow::onReconnectFailed(int attempt, const QString& reason)
{
    qDebug() << "Reconnect attempt" << attempt << "failed:" << reason;
    
    emit reconnecting(attempt, reconnector_->getMaxAttempts());
}

void NetworkGameWindow::onMaxAttemptsReached()
{
    qDebug() << "Max reconnection attempts reached";
    
    connectionStatusLabel_->setText(tr("Connection Lost"));
    connectionStatusLabel_->setStyleSheet("color: red;");
    
    addChatMessage(tr("System"), tr("Failed to reconnect. Connection lost."));
    
    // 禁用聊天
    chatInput_->setEnabled(false);
    sendChatButton_->setEnabled(false);
    
    emit connectionLost();
}

void NetworkGameWindow::onMoveReceived(int row, int col, const QString& player)
{
    qDebug() << "Remote move received:" << player << "@" << row << "," << col;
    
    // 验证并应用移动
    synchronizer_->verifyState(Network::GameStateMessage());
    
    // 应用到游戏
    applyRemoteMove(row, col, player);
}

void NetworkGameWindow::onGameStateReceived(const Network::GameStateMessage& state)
{
    qDebug() << "Game state received";
    
    // 验证状态一致性
    synchronizer_->verifyState(state);
}

// ==================== Chat Slots ====================

void NetworkGameWindow::onSendChatMessage()
{
    QString message = chatInput_->text().trimmed();
    if (message.isEmpty()) {
        return;
    }
    
    // 发送到网络
    if (networkClient_ && networkClient_->isConnected()) {
        Network::ChatMessage chat(localPlayerName_, message);
        QJsonObject json = chat.toJson();
        
        Network::Message msg;
        msg.type = Network::MessageType::CHAT_MESSAGE;
        msg.payload = json;
        networkClient_->sendMessage(msg);
        
        // 显示自己的消息
        addChatMessage(localPlayerName_ + " (you)", message);
    }
    
    chatInput_->clear();
}

void NetworkGameWindow::onChatMessageReceived(const QString& sender, const QString& message)
{
    addChatMessage(sender, message);
}

// ==================== Latency Slots ====================

void NetworkGameWindow::onPongReceived(qint64 latency)
{
    currentLatency_ = static_cast<int>(latency);
    latencyLCD_->display(currentLatency_);
    
    // 更新延迟颜色
    if (latency < 50) {
        latencyLCD_->setStyleSheet("color: green;");
    } else if (latency < 150) {
        latencyLCD_->setStyleSheet("color: orange;");
    } else {
        latencyLCD_->setStyleSheet("color: red;");
    }
}

void NetworkGameWindow::updateLatencyDisplay()
{
    if (networkClient_ && networkClient_->isConnected()) {
        networkClient_->sendPing();
    }
}

// ==================== Private Methods ====================

void NetworkGameWindow::updateConnectionStatus()
{
    if (networkClient_) {
        switch (networkClient_->getState()) {
            case Network::ConnectionState::Connected:
                connectionStatusLabel_->setText(tr("Connected"));
                connectionStatusLabel_->setStyleSheet("color: green;");
                break;
            case Network::ConnectionState::Connecting:
                connectionStatusLabel_->setText(tr("Connecting..."));
                connectionStatusLabel_->setStyleSheet("color: blue;");
                break;
            case Network::ConnectionState::Error:
                connectionStatusLabel_->setText(tr("Error"));
                connectionStatusLabel_->setStyleSheet("color: red;");
                break;
            default:
                connectionStatusLabel_->setText(tr("Disconnected"));
                connectionStatusLabel_->setStyleSheet("color: gray;");
                break;
        }
    }
}

void NetworkGameWindow::updateOpponentDisplay()
{
    opponentNameLabel_->setText(opponentPlayerName_);
}

void NetworkGameWindow::addChatMessage(const QString& sender, const QString& message)
{
    QString formatted = QString("[%1] %2: %3")
                           .arg(QTime::currentTime().toString("HH:mm:ss"))
                           .arg(sender)
                           .arg(message);
    
    chatDisplay_->append(formatted);
}

void NetworkGameWindow::handleConnectionLoss()
{
    qDebug() << "Handling connection loss";
    
    connectionStatusLabel_->setText(tr("Disconnected"));
    connectionStatusLabel_->setStyleSheet("color: red;");
    
    latencyUpdateTimer_->stop();
    
    // 禁用聊天
    chatInput_->setEnabled(false);
    sendChatButton_->setEnabled(false);
    
    // 开始重连
    if (networkClient_ && reconnector_) {
        QString address = networkClient_->getPeerAddress().toString();
        quint16 port = networkClient_->getPeerPort();
        
        reconnector_->startReconnection(address, port);
    }
    
    addChatMessage(tr("System"), tr("Connection lost. Attempting to reconnect..."));
    
    emit connectionLost();
}

void NetworkGameWindow::sendLocalMove(int row, int col)
{
    if (networkClient_ && networkClient_->isConnected()) {
        Reversi::PlayerColor color = gameController_->getCurrentPlayer();
        QString player = (color == Reversi::PlayerColor::Black) ? "black" : "white";
        
        // 获取当前移动编号
        int moveNumber = gameController_->getBoard().getMoveCount();
        networkClient_->sendMove(row, col, player, moveNumber);
        
        qDebug() << "Sent local move:" << player << "@" << row << "," << col;
    }
}

void NetworkGameWindow::applyRemoteMove(int row, int col, const QString& player)
{
    // Reference: PvPWindow handleLocalMove pattern
    Reversi::PlayerColor color = (player == "black") 
        ? Reversi::PlayerColor::Black 
        : Reversi::PlayerColor::White;
    
    // 获取合法移动列表
    const Reversi::Board& board = gameController_->getBoard();
    auto validMoves = board.getValidMoves();
    
    // 检查移动是否合法
    bool isValid = false;
    for (const auto& move : validMoves) {
        if (move.row == row && move.col == col) {
            isValid = true;
            break;
        }
    }
    
    if (isValid) {
        gameController_->makeHumanMove(row, col);
        qDebug() << "Applied remote move:" << player << "@" << row << "," << col;
    } else {
        qWarning() << "Invalid remote move received:" << player << "@" << row << "," << col;
    }
}
