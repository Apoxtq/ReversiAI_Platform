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

// 棋盘参数：窗口 800x600，右侧面板约 300px，左侧棋盘区域约 500x600
const int BOARD_OFFSET_X = 0;
const int BOARD_OFFSET_Y = 0;
const int CELL_SIZE = 60;  // 每格 60 像素，8x8 = 480x480
const int BOARD_SIZE = CELL_SIZE * 8;

NetworkGameWindow::NetworkGameWindow(QWidget* parent)
    : QMainWindow(parent)
    , networkClient_(nullptr)
    , networkHost_(nullptr)
    , discovery_(nullptr)
    , synchronizer_(nullptr)
    , reconnector_(nullptr)
    , localPlayerName_("Player")
    , opponentPlayerName_("Opponent")
    , isHost_(false)
    , isReconnecting_(false)
    , currentLatency_(0)
    , localPlayerColor_(Reversi::PlayerColor::Black)
    , localPlayerReady_(false)
    , opponentReady_(false)
    , gameStarted_(false)
    , isClosing_(false)
    , lastSentTimestamp_(0)
    , lastReceivedTimestamp_(0)
{
    qDebug() << "===========================================";
    qDebug() << "NetworkGameWindow::NetworkGameWindow() - Constructor START";
    qDebug() << "===========================================";
    
    // 初始化UI
    initGameUI();
    setupNetworkUI();
    
    // 创建GameController（网络PvP模式）
    gameController_ = std::make_unique<Reversi::GameController>(this);
    qDebug() << "GameController created";
    
    // 加载资源
    loadResources();
    
    // 连接游戏信号
    setupGameConnections();
    
    qDebug() << "NetworkGameWindow::NetworkGameWindow() - Constructor END";
    qDebug() << "===========================================";
}

NetworkGameWindow::~NetworkGameWindow()
{
    if (isClosing_) {
        // 清理已在 closeEvent/onBackToMenuClicked 中完成，只需安全删除对象
        if (discovery_) {
            discovery_->stopBroadcasting();
            discovery_->deleteLater();
        }
        if (networkHost_) {
            networkHost_->deleteLater();
        }
        return;
    }
    disconnectFromGame();
    if (discovery_) {
        discovery_->stopBroadcasting();
        discovery_->deleteLater();
    }
    if (networkHost_) {
        networkHost_->stopHosting();
        networkHost_->deleteLater();
    }
}

// ==================== UI Initialization ====================

void NetworkGameWindow::initGameUI()
{
    // Reference: PvPWindow initUI pattern
    
    setWindowTitle(tr("Network Game - ReversiAI"));
    resize(800, 600);
    
    // 创建主布局（水平，左侧棋盘+分数，右侧控制面板）
    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // === 左侧区域：棋盘 + 分数 + 当前回合 ===
    QWidget* leftWidget = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(5);
    
    // 占位空白，将棋盘向下推移到居中偏上位置
    leftLayout->addSpacing(30);
    
    // 棋盘面板（占位，由 paintEvent 绘制）
    QWidget* boardPanel = new QWidget(this);
    boardPanel->setFixedSize(BOARD_SIZE, BOARD_SIZE);
    leftLayout->addWidget(boardPanel);
    
    // 下方信息区（分数 + 当前回合），居中显示
    QWidget* infoPanel = new QWidget(this);
    infoPanel->setFixedHeight(90);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoPanel);
    infoLayout->setContentsMargins(0, 5, 0, 0);
    infoLayout->setSpacing(4);
    
    // 当前回合提示（大字）
    turnIndicator_ = new QLabel(tr("Black's Turn"), this);
    turnIndicator_->setAlignment(Qt::AlignCenter);
    turnIndicator_->setStyleSheet(
        "font-weight: bold; font-size: 18px;"
        "color: #000000;"
        "background-color: rgba(220,220,220,200);"
        "border-radius: 6px;"
        "padding: 4px 16px;"
    );
    infoLayout->addWidget(turnIndicator_, 0, Qt::AlignHCenter);
    
    // 棋子数量标签（放大字体，居中）
    QHBoxLayout* scoreLayout = new QHBoxLayout();
    scoreLayout->setSpacing(30);
    
    blackScoreLabel_ = new QLabel(tr("Black: 2"), this);
    blackScoreLabel_->setAlignment(Qt::AlignCenter);
    blackScoreLabel_->setStyleSheet(
        "font-weight: bold; font-size: 20px; color: #111111;"
        "background-color: rgba(240,240,240,180);"
        "border-radius: 6px; padding: 3px 12px;"
    );
    scoreLayout->addWidget(blackScoreLabel_);
    
    whiteScoreLabel_ = new QLabel(tr("White: 2"), this);
    whiteScoreLabel_->setAlignment(Qt::AlignCenter);
    whiteScoreLabel_->setStyleSheet(
        "font-weight: bold; font-size: 20px; color: #eeeeee;"
        "background-color: rgba(30,30,30,220);"
        "border-radius: 6px; padding: 3px 12px;"
    );
    scoreLayout->addWidget(whiteScoreLabel_);
    
    infoLayout->addLayout(scoreLayout);
    leftLayout->addWidget(infoPanel);
    
    leftWidget->setLayout(leftLayout);
    mainLayout->addWidget(leftWidget, 1);
    
    // 右侧：游戏控制和网络信息
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->setContentsMargins(15, 0, 0, 0);
    
    // 游戏控制按钮组
    QGroupBox* gameControlGroup = new QGroupBox(tr("Game Control"), this);
    QVBoxLayout* buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(8);
    
    startButton_ = new QPushButton(tr("Start Game"), this);
    startButton_->setStyleSheet(
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "    border-radius: 5px;"
        "    padding: 10px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #cccccc;"
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
    
    backButton_ = new QPushButton(tr("Back to Menu"), this);
    backButton_->setStyleSheet(
        "QPushButton {"
        "    background-color: #f44336;"
        "    color: white;"
        "    border-radius: 5px;"
        "    padding: 8px;"
        "}"
    );
    buttonLayout->addWidget(backButton_);
    
    gameControlGroup->setLayout(buttonLayout);
    rightLayout->addWidget(gameControlGroup);
    
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

    // 本玩家颜色
    QHBoxLayout* colorLayout = new QHBoxLayout();
    QLabel* colorText = new QLabel(tr("Your Color:"), this);
    colorLayout->addWidget(colorText);
    playerColorLabel_ = new QLabel(tr("-"), this);
    playerColorLabel_->setStyleSheet("font-weight: bold;");
    colorLayout->addWidget(playerColorLabel_);
    colorLayout->addStretch();
    connectionLayout->addLayout(colorLayout);
    
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

    // 连接网络客户端信号（适用于客户端和主机模式）
    Network::NetworkClient* client = isHost_ ? networkHost_ : networkClient_;
    if (!client) return;

    connect(client, &Network::NetworkClient::connected,
            this, &NetworkGameWindow::onConnected);
    connect(client, &Network::NetworkClient::disconnected,
            this, &NetworkGameWindow::onDisconnected);
    connect(client, &Network::NetworkClient::errorOccurred,
            this, &NetworkGameWindow::onNetworkError);
    connect(client, &Network::NetworkClient::connectionTimeout,
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
    connect(client, &Network::NetworkClient::moveReceived,
            this, &NetworkGameWindow::onMoveReceived);
    connect(client, &Network::NetworkClient::gameStateReceived,
            this, &NetworkGameWindow::onGameStateReceived);
    connect(client, &Network::NetworkClient::playerReadyReceived,
            this, &NetworkGameWindow::onPlayerReadyReceived);

    // 延迟信号
    connect(client, &Network::NetworkClient::pongReceived,
            this, &NetworkGameWindow::onPongReceived);

    // 聊天信号
    connect(client, &Network::NetworkClient::chatMessageReceived,
            this, &NetworkGameWindow::onChatMessageReceived);
    connect(sendChatButton_, &QPushButton::clicked, this, &NetworkGameWindow::onSendChatMessage);
    connect(chatInput_, &QLineEdit::returnPressed, this, &NetworkGameWindow::onSendChatMessage);
}

// ==================== Initialization ====================

void NetworkGameWindow::initNetwork(const QHostAddress& hostAddress, quint16 port, const QString& playerName)
{
    qDebug() << "Initializing network for game with" << hostAddress.toString() << ":" << port;
    
    localPlayerName_ = playerName;
    isHost_ = false;
    
    // 客户端执白棋
    localPlayerColor_ = Reversi::PlayerColor::White;
    
    // 创建网络模块
    networkClient_ = new Network::NetworkClient(this);
    synchronizer_ = new Network::GameSynchronizer(this);
    reconnector_ = new Network::ReconnectionManager(this);
    
    // 连接网络信号
    setupNetworkConnections();
    
    // 连接
    connectToHost(hostAddress, port, playerName);
}

void NetworkGameWindow::startHosting(const QString& playerName, quint16 /* port */)
{
    qDebug() << "===========================================";
    qDebug() << "NetworkGameWindow::startHosting() - START";
    qDebug() << "===========================================";
    
    localPlayerName_ = playerName;
    isHost_ = true;
    
    // 主机执黑棋
    localPlayerColor_ = Reversi::PlayerColor::Black;

    // 创建 NetworkHost（TCP 服务器）
    qDebug() << "Creating NetworkHost...";
    networkHost_ = new Network::NetworkHost(this);
    qDebug() << "NetworkHost created";
    
    qDebug() << "Creating GameSynchronizer...";
    synchronizer_ = new Network::GameSynchronizer(this);
    qDebug() << "GameSynchronizer created";
    
    qDebug() << "Creating ReconnectionManager...";
    reconnector_ = new Network::ReconnectionManager(this);
    qDebug() << "ReconnectionManager created";
    
    // 启动 TCP 服务器，使用 port=0 让系统自动分配可用端口
    qDebug() << "Starting hosting server...";
    if (!networkHost_->startHosting(0)) {
        qWarning() << "Failed to start hosting server";
        connectionStatusLabel_->setText(tr("Failed to start server"));
        connectionStatusLabel_->setStyleSheet("color: red;");
        return;
    }

    // 获取实际分配的端口
    quint16 actualPort = networkHost_->getListeningPort();
    qInfo() << "TCP server listening on port" << actualPort;

    // 创建 UDP 广播模块并开始广播房间存在
    qDebug() << "Creating NetworkDiscovery...";
    discovery_ = new Network::NetworkDiscovery(this);
    discovery_->setPlayerName(playerName);
    discovery_->setRoomName("Reversi Room");
    discovery_->setGameVersion("1.0.2");
    qDebug() << "Starting UDP broadcast...";
    discovery_->startBroadcasting(actualPort);
    
    qDebug() << "Setting up network connections...";
    // 连接网络信号
    setupNetworkConnections();
    
    qDebug() << "Connecting clientConnected signal to onOpponentJoined...";
    // 连接主机特有的客户端连接信号
    connect(networkHost_, &Network::NetworkHost::clientConnected,
            this, &NetworkGameWindow::onOpponentJoined,
            Qt::QueuedConnection);
    
    qDebug() << "Host setup complete";
    // 主机等待连接
    connectionStatusLabel_->setText(tr("Waiting for opponent on port %1...").arg(actualPort));
    connectionStatusLabel_->setStyleSheet("color: blue;");

    qDebug() << "===========================================";
    qDebug() << "NetworkGameWindow::startHosting() - END";
    qDebug() << "===========================================";
}

void NetworkGameWindow::connectToHost(const QHostAddress& address, quint16 port, const QString& playerName)
{
    qInfo() << "NetworkGameWindow: Attempting to connect to" << address.toString() << ":" << port;
    qInfo() << "Address protocol:" << (address.protocol() == QAbstractSocket::IPv4Protocol ? "IPv4" : "IPv6");
    qInfo() << "Is loopback:" << address.isLoopback();
    
    localPlayerName_ = playerName;
    connectionStatusLabel_->setText(tr("Connecting to %1:%2...").arg(address.toString()).arg(port));
    connectionStatusLabel_->setStyleSheet("color: blue;");

    bool result = networkClient_->connectToHost(address, port);
    if (!result) {
        qWarning() << "NetworkGameWindow: connectToHost returned false immediately";
        connectionStatusLabel_->setText(tr("Failed to initiate connection"));
        connectionStatusLabel_->setStyleSheet("color: red;");
    }
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

    // 停止 UDP 广播
    if (discovery_) {
        discovery_->stopBroadcasting();
    }

    // 断开网络客户端连接
    if (networkClient_) {
        networkClient_->disconnectFromHost();
    }

    // 停止 TCP 服务器
    if (networkHost_) {
        networkHost_->stopHosting();
    }

    updateConnectionStatus();
}

// ==================== Paint Events ====================

void NetworkGameWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    // 绘制棋盘（棋盘铺满左侧区域，480x480）
    if (!background.isNull()) {
        painter.drawPixmap(BOARD_OFFSET_X, BOARD_OFFSET_Y, BOARD_SIZE, BOARD_SIZE, background);
    } else {
        // 如果没有棋盘图片，绘制灰色背景和网格
        painter.fillRect(BOARD_OFFSET_X, BOARD_OFFSET_Y, BOARD_SIZE, BOARD_SIZE, QColor(200, 200, 180));
        painter.setPen(QPen(Qt::black, 1));
        for (int i = 0; i <= 8; ++i) {
            painter.drawLine(BOARD_OFFSET_X, BOARD_OFFSET_Y + i * CELL_SIZE, BOARD_OFFSET_X + BOARD_SIZE, BOARD_OFFSET_Y + i * CELL_SIZE);
            painter.drawLine(BOARD_OFFSET_X + i * CELL_SIZE, BOARD_OFFSET_Y, BOARD_OFFSET_X + i * CELL_SIZE, BOARD_OFFSET_Y + BOARD_SIZE);
        }
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
                    painter.drawPixmap(BOARD_OFFSET_X + i * CELL_SIZE, BOARD_OFFSET_Y + j * CELL_SIZE, CELL_SIZE, CELL_SIZE, white);
                }
            } else if (cellValue == 2) {  // 黑子
                if (!black.isNull()) {
                    painter.drawPixmap(BOARD_OFFSET_X + i * CELL_SIZE, BOARD_OFFSET_Y + j * CELL_SIZE, CELL_SIZE, CELL_SIZE, black);
                }
            }
            
            // 绘制合法移动高亮
            if (markHaveDraw[j][i] == 2 && cellValue == 0) {
                if (!hintblack.isNull()) {
                    painter.drawPixmap(BOARD_OFFSET_X + i * CELL_SIZE, BOARD_OFFSET_Y + j * CELL_SIZE, CELL_SIZE, CELL_SIZE, hintblack);
                }
            } else if (markHaveDraw[j][i] == 1 && cellValue == 0) {
                if (!hintwhite.isNull()) {
                    painter.drawPixmap(BOARD_OFFSET_X + i * CELL_SIZE, BOARD_OFFSET_Y + j * CELL_SIZE, CELL_SIZE, CELL_SIZE, hintwhite);
                }
            }
        }
    }
}

void NetworkGameWindow::mousePressEvent(QMouseEvent* e)
{
    // 处理点击事件（棋盘范围）
    int x = e->pos().x();
    int y = e->pos().y();
    
    if (x >= BOARD_OFFSET_X && x < BOARD_OFFSET_X + BOARD_SIZE &&
        y >= BOARD_OFFSET_Y && y < BOARD_OFFSET_Y + BOARD_SIZE) {
        int col = (x - BOARD_OFFSET_X) / CELL_SIZE;
        int row = (y - BOARD_OFFSET_Y) / CELL_SIZE;
        
        // 检查游戏是否已开始
        Reversi::GamePhase phase = gameController_->getCurrentPhase();
        if (phase == Reversi::GamePhase::Waiting) {
            addChatMessage(tr("System"), tr("Please wait for the game to start..."));
            return;
        }
        
        // 检查是否是当前玩家的回合，并且是本玩家的回合
        Reversi::PlayerColor currentPlayer = gameController_->getCurrentPlayer();
        if (phase == Reversi::GamePhase::HumanTurn && currentPlayer == localPlayerColor_) {
            // 发送移动到网络
            sendLocalMove(row, col);
        } else if (phase == Reversi::GamePhase::HumanTurn) {
            addChatMessage(tr("System"), tr("It's not your turn yet!"));
        }
    }
}

void NetworkGameWindow::closeEvent(QCloseEvent* event)
{
    if (isClosing_) {
        event->accept();
        return;
    }
    isClosing_ = true;
    disconnectFromGame();
    event->accept();
}

// ==================== Game Slots ====================

void NetworkGameWindow::onStartGameClicked()
{
    qDebug() << "NetworkGameWindow: onStartGameClicked, isHost:" << isHost_;
    
    // 如果游戏已经开始了，忽略
    if (gameStarted_) {
        qDebug() << "Game already started, ignoring start button";
        addChatMessage(tr("System"), tr("Game has already started!"));
        return;
    }
    
    // 如果已经准备，忽略
    if (localPlayerReady_) {
        qDebug() << "Already ready, ignoring start button";
        addChatMessage(tr("System"), tr("You are already ready!"));
        return;
    }
    
    // 标记本地玩家已准备
    localPlayerReady_ = true;
    
    // 发送准备消息给对手
    Network::NetworkClient* client = isHost_ ? networkHost_ : networkClient_;
    qint64 currentTimestamp = QDateTime::currentMSecsSinceEpoch();
    if (client && client->isConnected()) {
        QJsonObject payload;
        payload["player"] = localPlayerName_;
        payload["color"] = (localPlayerColor_ == Reversi::PlayerColor::Black) ? "black" : "white";
        
        Network::Message msg;
        msg.type = Network::MessageType::PLAYER_READY;
        msg.timestamp = currentTimestamp;
        msg.sender = localPlayerName_;
        msg.payload = payload;
        
        // 记录发送时间戳，用于回声检测
        lastSentTimestamp_ = currentTimestamp;
        
        // 客户端发送消息时会使用队列，这里确保消息被发送
        client->sendMessage(msg);
        qDebug() << "Sent PLAYER_READY message, sender:" << msg.sender << "timestamp:" << msg.timestamp;
    }
    
    // 显示准备状态
    addChatMessage(tr("System"), tr("You are ready! Waiting for opponent..."));
    startButton_->setEnabled(false);
    startButton_->setText("Ready");
    
    // 如果是主机，发送初始游戏状态给对手（用于同步）
    if (isHost_) {
        qDebug() << "Sending initial game state to opponent";
        const Reversi::Board& board = gameController_->getBoard();
        
        Network::GameStateMessage state;
        state.board.resize(8);
        for (int i = 0; i < 8; i++) {
            state.board[i].resize(8);
            for (int j = 0; j < 8; j++) {
                state.board[i][j] = board.at(i, j);
            }
        }
        state.currentPlayer = "black";
        state.blackCount = 2;
        state.whiteCount = 2;
        state.moveNumber = 0;
        
        Network::Message stateMsg;
        stateMsg.type = Network::MessageType::GAME_STATE_UPDATE;
        stateMsg.timestamp = QDateTime::currentMSecsSinceEpoch();
        stateMsg.payload = state.toJson();
        
        networkHost_->sendMessage(stateMsg);
    }
    
    // 检查是否双方都准备好了
    checkAndStartGame();
}

void NetworkGameWindow::onPlayerReadyReceived(const QString& playerName, const QString& sender, qint64 timestamp)
{
    qDebug() << "NetworkGameWindow::onPlayerReadyReceived from" << playerName << "sender:" << sender << "isHost:" << isHost_ << "timestamp:" << timestamp;
    
    // 如果游戏已经开始了，忽略
    if (gameStarted_) {
        qDebug() << "Game already started, ignoring ready message";
        return;
    }
    
    // 回声检测：比较时间戳
    if (isHost_) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        qint64 timeDiff = now - timestamp;
        qint64 sentDiff = timestamp - lastSentTimestamp_;
        
        qDebug() << "=== Echo check ===";
        qDebug() << "  now:" << now;
        qDebug() << "  msg.timestamp:" << timestamp;
        qDebug() << "  lastSentTimestamp_:" << lastSentTimestamp_;
        qDebug() << "  time since msg sent (by sender):" << timeDiff << "ms";
        qDebug() << "  msg.ts - lastSent.ts:" << sentDiff << "ms";
        
        // 如果消息时间戳 <= 最后发送时间戳 - 100ms，认为是回声
        // 但如果时间差很大（>1秒），说明是来自客户端的消息
        if (sentDiff < 0 && qAbs(sentDiff) > 100) {
            // 消息比最后发送的还早，说明是对手的消息
            qDebug() << "Host: msg is BEFORE last sent, this is opponent's message";
        } else if (sentDiff > 100) {
            // 消息比最后发送的晚 100ms 以上，说明是对手的消息
            qDebug() << "Host: msg is AFTER last sent by" << sentDiff << "ms, this is opponent's message";
        } else {
            // 时间戳接近，可能是回声
            if (sender == localPlayerName_) {
                qDebug() << "Host ignoring own ready message (timestamp and sender echo)";
                return;
            }
        }
        
        lastReceivedTimestamp_ = timestamp;
    } else {
        // 客户端：收到的任何消息都是来自主机的
        qDebug() << "Client received ready from opponent (host)";
        lastReceivedTimestamp_ = timestamp;
    }
    
    // 这是对手的准备好了消息
    QString opponentName = playerName.isEmpty() ? sender : playerName;
    opponentPlayerName_ = opponentName;
    opponentReady_ = true;
    
    addChatMessage(tr("System"), tr("%1 is ready!").arg(opponentName));
    updateOpponentDisplay();
    
    qDebug() << "Opponent ready, localReady=" << localPlayerReady_ << "opponentReady=" << opponentReady_;
    
    // 检查是否双方都准备好了
    checkAndStartGame();
}

void NetworkGameWindow::checkAndStartGame()
{
    qDebug() << "checkAndStartGame: localReady=" << localPlayerReady_ 
             << "opponentReady=" << opponentReady_;
    
    // 只有当双方都准备好时才正式开始游戏
    if (localPlayerReady_ && opponentReady_) {
        qDebug() << "Both players ready, starting game!";
        
        gameStarted_ = true;
        
        // 主机执黑棋，客户端执白棋
        Reversi::PlayerColor humanColor = isHost_ 
            ? Reversi::PlayerColor::Black 
            : Reversi::PlayerColor::White;
        
        localPlayerColor_ = humanColor;
        
        // 启动游戏
        gameController_->startNewGame(Reversi::GameMode::PvP, humanColor);
        
        addChatMessage(tr("System"), tr("Both players ready! Game starting..."));
        
        qDebug() << "Network game started, localPlayerColor:" 
                 << (humanColor == Reversi::PlayerColor::Black ? "Black" : "White");
    }
}

void NetworkGameWindow::onUndoClicked()
{
    qDebug() << "NetworkGameWindow: Undo requested";
    // 网络模式下，悔棋需要对方同意
    addChatMessage(tr("System"), tr("Undo requested - waiting for opponent..."));
}

void NetworkGameWindow::onBackToMenuClicked()
{
    isClosing_ = true;
    disconnectFromGame();
    emit backToMenu();
    close();
}

void NetworkGameWindow::onGameStarted(Reversi::GameMode mode, Reversi::PlayerColor /* humanColor */)
{
    qDebug() << "Game started in network mode, localPlayerColor:" 
             << (localPlayerColor_ == Reversi::PlayerColor::Black ? "Black" : "White");

    // 使用本地保存的颜色（主机黑棋，客户端白棋）
    QString colorStr = (localPlayerColor_ == Reversi::PlayerColor::Black) 
        ? tr("Black") 
        : tr("White");
    playerColorLabel_->setText(colorStr);
    playerColorLabel_->setStyleSheet(
        localPlayerColor_ == Reversi::PlayerColor::Black
            ? "font-weight: bold; color: #000000;"
            : "font-weight: bold; color: #aaaaaa;"
    );

    undoButton_->setEnabled(true);

    addChatMessage(tr("System"), tr("Game started!"));
    updateScoreDisplay();
    update();
}

void NetworkGameWindow::onOpponentJoined(const QHostAddress& address, quint16 port)
{
    qDebug() << "===========================================";
    qDebug() << "NetworkGameWindow::onOpponentJoined called from" << address.toString() << ":" << port;
    qDebug() << "isHost_:" << isHost_;
    qDebug() << "===========================================";
    
    // 检查必要的组件是否已初始化
    if (!gameController_) {
        qWarning() << "Game controller not initialized, skipping opponent join handling";
        return;
    }
    
    qDebug() << "Game controller is valid, proceeding with join handling";
    
    QString name = isHost_ ? "Player" : opponentNameLabel_->text();
    qDebug() << "Setting opponent name to:" << name;
    
    opponentNameLabel_->setText(name);
    opponentNameLabel_->setStyleSheet("font-weight: bold; color: green;");
    connectionStatusLabel_->setText(tr("Opponent connected"));
    connectionStatusLabel_->setStyleSheet("color: green;");
    addChatMessage(tr("System"), tr("Opponent joined the game!"));
    qDebug() << "UI updated for opponent join";
    
    // 启用聊天（对手已连接，可以开始聊天了）
    chatInput_->setEnabled(true);
    sendChatButton_->setEnabled(true);
    qDebug() << "Chat enabled for opponent join";
    
    // 主机发送完整的游戏状态给新连接的对手
    // 注意：必须使用 NetworkHost::sendMessage() 直接发送，
    // 因为 NetworkHost 继承的 sendGameState() 使用基类的 state_ (永远是 Disconnected)
    if (isHost_) {
        qDebug() << "This is the host, preparing to send game state";
        
        // 检查 NetworkHost 是否有效
        if (!networkHost_) {
            qWarning() << "NetworkHost is null, cannot send game state";
            return;
        }
        
        qDebug() << "NetworkHost is valid, checking connection status";
        
        // 检查是否已连接
        if (!networkHost_->isConnected()) {
            qWarning() << "NetworkHost not connected, cannot send game state";
            return;
        }
        
        qDebug() << "NetworkHost is connected, building game state message";
        
        // 获取当前棋盘状态
        const Reversi::Board& board = gameController_->getBoard();
        
        Network::GameStateMessage state;
        // 初始化 board 数组（GameStateMessage::board 默认是空的）
        state.board.resize(8);
        for (int i = 0; i < 8; i++) {
            state.board[i].resize(8);
            for (int j = 0; j < 8; j++) {
                state.board[i][j] = board.at(i, j);
            }
        }
        state.currentPlayer = (gameController_->getCurrentPlayer() == Reversi::PlayerColor::Black) ? "black" : "white";
        int bc = 0, wc = 0;
        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 8; j++) {
                if (board.at(i, j) == 2) bc++;
                else if (board.at(i, j) == 1) wc++;
            }
        state.blackCount = bc;
        state.whiteCount = wc;
        state.moveNumber = board.getMoveCount();
        
        qDebug() << "Game state built: black=" << bc << "white=" << wc << "currentPlayer=" << state.currentPlayer;
        
        Network::Message msg;
        msg.type = Network::MessageType::GAME_STATE_UPDATE;
        msg.timestamp = QDateTime::currentMSecsSinceEpoch();
        msg.payload = state.toJson();
        
        qDebug() << "Sending game state message";
        bool sent = networkHost_->sendMessage(msg);
        if (sent) {
            qDebug() << "Successfully sent game state via NetworkHost::sendMessage";
        } else {
            qWarning() << "Failed to send game state via NetworkHost::sendMessage";
        }
        
        qDebug() << "===========================================";
        qDebug() << "onOpponentJoined completed";
        qDebug() << "===========================================";
    }
}

void NetworkGameWindow::onPhaseChanged(Reversi::GamePhase phase)
{
    qDebug() << "=== onPhaseChanged START ===";
    qDebug() << "Phase changed:" << static_cast<int>(phase);
    qDebug() << "turnIndicator_:" << (turnIndicator_ ? "valid" : "NULL");
    update();
    qDebug() << "=== onPhaseChanged END ===";
}

void NetworkGameWindow::onTurnChanged(Reversi::PlayerColor player)
{
    qDebug() << "=== onTurnChanged START ===";
    qDebug() << "Turn changed:" << (player == Reversi::PlayerColor::Black ? "Black" : "White");
    qDebug() << "turnIndicator_:" << (turnIndicator_ ? "valid" : "NULL");
    qDebug() << "gameController_:" << (gameController_ ? "valid" : "NULL");

    // 更新回合提示
    QString turnText = (player == Reversi::PlayerColor::Black) ? tr("Black's Turn") : tr("White's Turn");
    turnIndicator_->setText(turnText);

    // 根据当前玩家设置颜色高亮
    if (player == Reversi::PlayerColor::Black) {
        turnIndicator_->setStyleSheet(
            "font-weight: bold; font-size: 18px;"
            "color: #000000;"
            "background-color: rgba(200,200,200,220);"
            "border-radius: 6px; padding: 4px 16px;"
        );
    } else {
        turnIndicator_->setStyleSheet(
            "font-weight: bold; font-size: 18px;"
            "color: #ffffff;"
            "background-color: rgba(50,50,50,220);"
            "border-radius: 6px; padding: 4px 16px;"
        );
    }

    qDebug() << "Calling updateScoreDisplay()...";
    updateScoreDisplay();
    qDebug() << "Calling update()...";
    update();
    qDebug() << "=== onTurnChanged END ===";
}

void NetworkGameWindow::onMoveMade(int row, int col, Reversi::PlayerColor player)
{
    qDebug() << "=== onMoveMade START ===";
    QString playerStr = (player == Reversi::PlayerColor::Black) ? "Black" : "White";
    qDebug() << "Move made:" << playerStr << "@" << row << "," << col;
    qDebug() << "gameController_:" << (gameController_ ? "valid" : "NULL");
    qDebug() << "Calling updateScoreDisplay()...";
    updateScoreDisplay();
    qDebug() << "Calling update()...";
    update();
    qDebug() << "=== onMoveMade END ===";
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
    qWarning() << "NetworkGameWindow: Network error:" << static_cast<int>(error) << "-" << message;
    
    // Provide user-friendly error messages
    QString errorMsg;
    switch (error) {
        case Network::NetworkError::ConnectionRefused:
            errorMsg = tr("Connection refused. The host may not be running or the port is blocked by firewall.");
            break;
        case Network::NetworkError::HostNotFound:
            errorMsg = tr("Host not found. Please refresh and try again.");
            break;
        case Network::NetworkError::ConnectionTimeout:
            errorMsg = tr("Connection timeout. Network may be too slow or host is unreachable.");
            break;
        case Network::NetworkError::ConnectionReset:
            errorMsg = tr("Connection was reset by host.");
            break;
        case Network::NetworkError::ProtocolError:
            errorMsg = tr("Protocol error: %1").arg(message);
            break;
        default:
            errorMsg = tr("Network error: %1").arg(message);
            break;
    }
    
    addChatMessage(tr("System"), tr("[ERROR] %1").arg(errorMsg));
    connectionStatusLabel_->setText(errorMsg);
    connectionStatusLabel_->setStyleSheet("color: red;");
    
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
    qDebug() << "onMoveReceived called:" << player << "@" << row << "," << col;

    // 检查游戏是否已开始
    if (!gameStarted_) {
        qWarning() << "Game not started yet, ignoring move";
        return;
    }

    // 应用远程移动
    qDebug() << "Calling applyRemoteMove...";
    applyRemoteMove(row, col, player);
    qDebug() << "applyRemoteMove completed";
}

void NetworkGameWindow::onGameStateReceived(const Network::GameStateMessage& state)
{
    qDebug() << "NetworkGameWindow::onGameStateReceived called, currentPlayer:" << state.currentPlayer
             << "gameStarted:" << gameStarted_;
    
    // 如果游戏已经开始了，这可能是同步消息，只验证不应用
    if (gameStarted_) {
        qDebug() << "Game already started, verifying received state";
        if (synchronizer_) {
            synchronizer_->verifyState(state);
        }
        return;
    }
    
    // 检查必要的组件是否已初始化
    if (!synchronizer_) {
        qWarning() << "Synchronizer not initialized, cannot verify state";
    } else {
        synchronizer_->verifyState(state);
    }
    
    // 如果我们是客户端（不是主机），应用接收到的初始棋盘状态
    // 主机已有完整状态，只有客户端需要同步初始状态
    if (!isHost_ && gameController_) {
        qDebug() << "Applying initial game state to client board";
        
        // 检查 gameController_ 是否有效
        try {
            // 重置棋盘到标准开局
            gameController_->resetGame();
            
            // 将 QVector 转换为 std::vector（Board 不链接 Qt）
            std::vector<std::vector<int>> stdBoard(8, std::vector<int>(8));
            for (int row = 0; row < 8; ++row) {
                for (int col = 0; col < 8; ++col) {
                    stdBoard[row][col] = state.board[row][col];
                }
            }
            
            // 直接从状态同步棋盘
            Reversi::PlayerColor nextPlayer = (state.currentPlayer == "black")
                ? Reversi::PlayerColor::Black
                : Reversi::PlayerColor::White;
            gameController_->getBoard().syncFrom(stdBoard, nextPlayer, state.moveNumber);
            
            // 设置为等待状态（等双方都准备好才开始）
            gameController_->setGamePhase(Reversi::GamePhase::Waiting);
            
            // 更新 UI
            updateScoreDisplay();
            update();
            
            qDebug() << "Initial game state applied, board synchronized, moveCount:" << state.moveNumber;
        } catch (const std::exception& e) {
            qWarning() << "Exception while applying game state:" << e.what();
        } catch (...) {
            qWarning() << "Unknown exception while applying game state";
        }
    }
}

// ==================== Chat Slots ====================

void NetworkGameWindow::onSendChatMessage()
{
    QString message = chatInput_->text().trimmed();
    if (message.isEmpty()) {
        return;
    }

    // 发送到网络（host 模式和 client 模式都走同一个 client 指针）
    Network::NetworkClient* client = isHost_ ? networkHost_ : networkClient_;
    qDebug() << "onSendChatMessage: isHost=" << isHost_ << "client=" << client << "isConnected=" << (client ? client->isConnected() : false);
    
    if (client && client->isConnected()) {
        Network::ChatMessage chat(localPlayerName_, message);
        QJsonObject json = chat.toJson();

        Network::Message msg;
        msg.type = Network::MessageType::CHAT_MESSAGE;
        msg.payload = json;
        qDebug() << "Sending chat message:" << message;
        bool sent = client->sendMessage(msg);
        qDebug() << "Chat message sent:" << sent;

        // 显示自己的消息
        addChatMessage(localPlayerName_ + " (you)", message);
    } else {
        qWarning() << "Cannot send chat: client not connected";
    }

    chatInput_->clear();
}

void NetworkGameWindow::onChatMessageReceived(const QString& sender, const QString& message)
{
    qDebug() << "onChatMessageReceived: from=" << sender << "message=" << message;
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
    // 获取当前玩家颜色（在 makeHumanMove 之前，因为 makeHumanMove 会切换玩家）
    Reversi::PlayerColor color = gameController_->getCurrentPlayer();
    QString player = (color == Reversi::PlayerColor::Black) ? "black" : "white";
    
    // 应用本地移动
    gameController_->makeHumanMove(row, col);
    
    // 同时发送到网络
    Network::NetworkHost* host = qobject_cast<Network::NetworkHost*>(networkHost_);
    if (host && host->isConnected()) {
        // 获取当前移动编号
        int moveNumber = gameController_->getBoard().getMoveCount();
        bool sent = host->sendMove(row, col, player, moveNumber);
        qDebug() << "Sent local move via NetworkHost::sendMove, result:" << sent << player << "@" << row << "," << col;
    } else {
        // 客户端模式
        Network::NetworkClient* client = networkClient_;
        if (client && client->isConnected()) {
            int moveNumber = gameController_->getBoard().getMoveCount();
            bool sent = client->sendMove(row, col, player, moveNumber);
            qDebug() << "Sent local move via NetworkClient::sendMove, result:" << sent << player << "@" << row << "," << col;
        } else {
            qWarning() << "Cannot send move: no valid connection";
        }
    }
}

void NetworkGameWindow::applyRemoteMove(int row, int col, const QString& player)
{
    qDebug() << "=== applyRemoteMove START ===";
    qDebug() << "applyRemoteMove called:" << player << "@" << row << "," << col;

    if (!gameController_) {
        qCritical() << "applyRemoteMove: gameController_ is null!";
        qDebug() << "=== applyRemoteMove END (gameController_ is null) ===";
        return;
    }

    qDebug() << "Getting board from gameController_...";
    // Reference: PvPWindow handleLocalMove pattern
    Reversi::PlayerColor color = (player == "black")
        ? Reversi::PlayerColor::Black
        : Reversi::PlayerColor::White;

    // 获取合法移动列表
    qDebug() << "Getting valid moves from board...";
    const Reversi::Board& board = gameController_->getBoard();
    auto validMoves = board.getValidMoves();
    qDebug() << "Got" << validMoves.size() << "valid moves";

    // 检查移动是否合法
    bool isValid = false;
    for (const auto& move : validMoves) {
        if (move.row == row && move.col == col) {
            isValid = true;
            break;
        }
    }

    qDebug() << "Move isValid:" << isValid;

    if (isValid) {
        qDebug() << "Calling gameController_->makeHumanMove...";
        gameController_->makeHumanMove(row, col);
        qDebug() << "makeHumanMove completed";
        qDebug() << "Calling updateScoreDisplay()...";
        updateScoreDisplay();
        qDebug() << "Applied remote move:" << player << "@" << row << "," << col;
    } else {
        qWarning() << "Invalid remote move received:" << player << "@" << row << "," << col;
    }

    qDebug() << "=== applyRemoteMove END ===";
}

void NetworkGameWindow::updateScoreDisplay()
{
    const Reversi::Board& board = gameController_->getBoard();
    
    int blackCount = 0;
    int whiteCount = 0;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int cellValue = board.at(i, j);
            if (cellValue == 2) blackCount++;
            else if (cellValue == 1) whiteCount++;
        }
    }
    
    blackScoreLabel_->setText(tr("Black: %1").arg(blackCount));
    whiteScoreLabel_->setText(tr("White: %1").arg(whiteCount));
}
