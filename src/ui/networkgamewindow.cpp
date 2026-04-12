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

// Board parameters: window 800x600, right panel ~300px, left board area ~500x600
const int BOARD_OFFSET_X = 0;
const int BOARD_OFFSET_Y = 0;
const int CELL_SIZE = 60;  // 60 pixels per cell, 8x8 = 480x480
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
    , undoPending_(false)
    , undoInProgress_(false)
{
    qDebug() << "===========================================";
    qDebug() << "NetworkGameWindow::NetworkGameWindow() - Constructor START";
    qDebug() << "===========================================";
    
    // Initialize UI
    initGameUI();
    setupNetworkUI();
    
    // Create GameController (network PvP mode)
    gameController_ = std::make_unique<Reversi::GameController>(this);
    qDebug() << "GameController created";
    
    // Load resources
    loadResources();
    
    // Connect game signals
    setupGameConnections();
    
    qDebug() << "NetworkGameWindow::NetworkGameWindow() - Constructor END";
    qDebug() << "===========================================";
}

NetworkGameWindow::~NetworkGameWindow()
{
    if (isClosing_) {
        // Cleanup already done in closeEvent/onBackToMenuClicked, just safely delete objects
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
    
    // Create main layout (horizontal, left board+scores, right control panel)
    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // === Left area: board + scores + current turn ===
    QWidget* leftWidget = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(5);
    
    // Placeholder space to push board down to center-top position
    leftLayout->addSpacing(30);
    
    // Board panel (placeholder, drawn by paintEvent)
    QWidget* boardPanel = new QWidget(this);
    boardPanel->setFixedSize(BOARD_SIZE, BOARD_SIZE);
    leftLayout->addWidget(boardPanel);
    
    // Bottom info area (scores + current turn), centered
    QWidget* infoPanel = new QWidget(this);
    infoPanel->setFixedHeight(90);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoPanel);
    infoLayout->setContentsMargins(0, 5, 0, 0);
    infoLayout->setSpacing(4);
    
    // Current turn indicator (large text)
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
    
    // Piece count labels (larger font, centered)
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
    
    // Right side: game controls and network info
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->setContentsMargins(15, 0, 0, 0);
    
    // Game control button group
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
    
    // Connection status group
    connectionGroup_ = new QGroupBox(tr("Network Status"), this);
    QVBoxLayout* connectionLayout = new QVBoxLayout();
    
    // Connection status
    QHBoxLayout* statusLayout = new QHBoxLayout();
    QLabel* statusText = new QLabel(tr("Status:"), this);
    statusLayout->addWidget(statusText);
    connectionStatusLabel_ = new QLabel(tr("Disconnected"), this);
    connectionStatusLabel_->setStyleSheet("color: gray;");
    statusLayout->addWidget(connectionStatusLabel_);
    statusLayout->addStretch();
    connectionLayout->addLayout(statusLayout);
    
    // Opponent name
    QHBoxLayout* opponentLayout = new QHBoxLayout();
    QLabel* opponentText = new QLabel(tr("Opponent:"), this);
    opponentLayout->addWidget(opponentText);
    opponentNameLabel_ = new QLabel(tr("Waiting..."), this);
    opponentNameLabel_->setStyleSheet("font-weight: bold; color: blue;");
    opponentLayout->addWidget(opponentNameLabel_);
    opponentLayout->addStretch();
    connectionLayout->addLayout(opponentLayout);

    // Local player color
    QHBoxLayout* colorLayout = new QHBoxLayout();
    QLabel* colorText = new QLabel(tr("Your Color:"), this);
    colorLayout->addWidget(colorText);
    playerColorLabel_ = new QLabel(tr("-"), this);
    playerColorLabel_->setStyleSheet("font-weight: bold;");
    colorLayout->addWidget(playerColorLabel_);
    colorLayout->addStretch();
    connectionLayout->addLayout(colorLayout);
    
    // Latency display
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
    
    // Chat area
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
    
    // Add to main layout
    mainLayout->addLayout(rightLayout, 0);
    
    // Set central widget
    QWidget* central = new QWidget(this);
    central->setLayout(mainLayout);
    setCentralWidget(central);
}

void NetworkGameWindow::setupNetworkUI()
{
    // Create latency update timer
    latencyUpdateTimer_ = new QTimer(this);
    latencyUpdateTimer_->setInterval(5000);  // 5 seconds
    
    // Initial state
    connectionStatusLabel_->setText(tr("Disconnected"));
    connectionStatusLabel_->setStyleSheet("color: gray;");
    
    // Disable chat until connected
    chatInput_->setEnabled(false);
    sendChatButton_->setEnabled(false);
}

void NetworkGameWindow::loadResources()
{
    // Load board and piece images
    // In actual project, should use resource files
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
    
    // Button connections
    connect(startButton_, &QPushButton::clicked, this, &NetworkGameWindow::onStartGameClicked);
    connect(undoButton_, &QPushButton::clicked, this, &NetworkGameWindow::onUndoClicked);
    connect(backButton_, &QPushButton::clicked, this, &NetworkGameWindow::onBackToMenuClicked);
    
    // GameController signal/slot connections
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

    // Connect network client signals (applicable to both client and host mode)
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

    // Reconnection signals
    connect(reconnector_, &Network::ReconnectionManager::reconnectingChanged,
            this, &NetworkGameWindow::onReconnectingChanged);
    connect(reconnector_, &Network::ReconnectionManager::reconnected,
            this, &NetworkGameWindow::onReconnected);
    connect(reconnector_, &Network::ReconnectionManager::reconnectFailed,
            this, &NetworkGameWindow::onReconnectFailed);
    connect(reconnector_, &Network::ReconnectionManager::maxAttemptsReached,
            this, &NetworkGameWindow::onMaxAttemptsReached);

    // Game signals
    connect(client, &Network::NetworkClient::moveReceived,
            this, &NetworkGameWindow::onMoveReceived);
    connect(client, &Network::NetworkClient::gameStateReceived,
            this, &NetworkGameWindow::onGameStateReceived);
    connect(client, &Network::NetworkClient::playerReadyReceived,
            this, &NetworkGameWindow::onPlayerReadyReceived);

    // Latency signals
    connect(client, &Network::NetworkClient::pongReceived,
            this, &NetworkGameWindow::onPongReceived);

    // Chat signals
    connect(client, &Network::NetworkClient::chatMessageReceived,
            this, &NetworkGameWindow::onChatMessageReceived);
    connect(sendChatButton_, &QPushButton::clicked, this, &NetworkGameWindow::onSendChatMessage);
    connect(chatInput_, &QLineEdit::returnPressed, this, &NetworkGameWindow::onSendChatMessage);

    // Undo signals
    connect(client, &Network::NetworkClient::undoRequestReceived,
            this, &NetworkGameWindow::onUndoRequestReceived);
    connect(client, &Network::NetworkClient::undoResponseReceived,
            this, &NetworkGameWindow::onUndoResponseReceived);
}

// ==================== Initialization ====================

void NetworkGameWindow::initNetwork(const QHostAddress& hostAddress, quint16 port, const QString& playerName)
{
    qDebug() << "Initializing network for game with" << hostAddress.toString() << ":" << port;
    
    localPlayerName_ = playerName;
    isHost_ = false;
    
    // Client plays white
    localPlayerColor_ = Reversi::PlayerColor::White;
    
    // Create network modules
    networkClient_ = new Network::NetworkClient(this);
    synchronizer_ = new Network::GameSynchronizer(this);
    reconnector_ = new Network::ReconnectionManager(this);
    
    // Connect network signals
    setupNetworkConnections();
    
    // Connect
    connectToHost(hostAddress, port, playerName);
}

void NetworkGameWindow::startHosting(const QString& playerName, const QString& roomName, quint16 /* port */)
{
    qDebug() << "===========================================";
    qDebug() << "NetworkGameWindow::startHosting() - START";
    qDebug() << "===========================================";
    
    localPlayerName_ = playerName;
    isHost_ = true;
    
    // Host plays black
    localPlayerColor_ = Reversi::PlayerColor::Black;

    // Create NetworkHost (TCP server)
    qDebug() << "Creating NetworkHost...";
    networkHost_ = new Network::NetworkHost(this);
    qDebug() << "NetworkHost created";
    
    qDebug() << "Creating GameSynchronizer...";
    synchronizer_ = new Network::GameSynchronizer(this);
    qDebug() << "GameSynchronizer created";
    
    qDebug() << "Creating ReconnectionManager...";
    reconnector_ = new Network::ReconnectionManager(this);
    qDebug() << "ReconnectionManager created";
    
    // Start TCP server with port=0 to let system assign available port
    qDebug() << "Starting hosting server...";
    if (!networkHost_->startHosting(0)) {
        qWarning() << "Failed to start hosting server";
        connectionStatusLabel_->setText(tr("Failed to start server"));
        connectionStatusLabel_->setStyleSheet("color: red;");
        return;
    }

    // Get actual assigned port
    quint16 actualPort = networkHost_->getListeningPort();
    qInfo() << "TCP server listening on port" << actualPort;

    // Create UDP broadcast module and start broadcasting room existence
    qDebug() << "Creating NetworkDiscovery...";
    discovery_ = new Network::NetworkDiscovery(this);
    discovery_->setPlayerName(playerName);
    discovery_->setRoomName(roomName);
    discovery_->setGameVersion("1.0.2");
    qDebug() << "Starting UDP broadcast...";
    discovery_->startBroadcasting(actualPort);
    
    qDebug() << "Setting up network connections...";
    // Connect network signals
    setupNetworkConnections();
    
    qDebug() << "Connecting clientConnected signal to onOpponentJoined...";
    // Connect host-specific client connection signal
    connect(networkHost_, &Network::NetworkHost::clientConnected,
            this, &NetworkGameWindow::onOpponentJoined,
            Qt::QueuedConnection);
    
    qDebug() << "Host setup complete";
    // Host waits for connection
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

    // Stop reconnection
    if (reconnector_) {
        reconnector_->cancelReconnection();
    }

    // Stop latency update
    if (latencyUpdateTimer_) {
        latencyUpdateTimer_->stop();
    }

    // Stop UDP broadcast
    if (discovery_) {
        discovery_->stopBroadcasting();
    }

    // Disconnect network client
    if (networkClient_) {
        networkClient_->disconnectFromHost();
    }

    // Stop TCP server
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
    
    // Draw board (board fills left area, 480x480)
    if (!background.isNull()) {
        painter.drawPixmap(BOARD_OFFSET_X, BOARD_OFFSET_Y, BOARD_SIZE, BOARD_SIZE, background);
    } else {
        // If no board image, draw gray background and grid
        painter.fillRect(BOARD_OFFSET_X, BOARD_OFFSET_Y, BOARD_SIZE, BOARD_SIZE, QColor(200, 200, 180));
        painter.setPen(QPen(Qt::black, 1));
        for (int i = 0; i <= 8; ++i) {
            painter.drawLine(BOARD_OFFSET_X, BOARD_OFFSET_Y + i * CELL_SIZE, BOARD_OFFSET_X + BOARD_SIZE, BOARD_OFFSET_Y + i * CELL_SIZE);
            painter.drawLine(BOARD_OFFSET_X + i * CELL_SIZE, BOARD_OFFSET_Y, BOARD_OFFSET_X + i * CELL_SIZE, BOARD_OFFSET_Y + BOARD_SIZE);
        }
    }
    
    // Get current state
    const Reversi::Board& board = gameController_->getBoard();
    Reversi::PlayerColor currentPlayer = gameController_->getCurrentPlayer();
    
    // Determine current piece color
    int tile = (currentPlayer == Reversi::PlayerColor::Black) ? 2 : 1;
    
    // Get valid moves and draw highlights
    int markHaveDraw[8][8] = {0};
    auto validMoves = board.getValidMoves();
    for (const auto& move : validMoves) {
        markHaveDraw[move.row][move.col] = tile;
    }
    
    // Draw pieces and highlights
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int cellValue = board.at(j, i);
            
            if (cellValue == 1) {  // White piece
                if (!white.isNull()) {
                    painter.drawPixmap(BOARD_OFFSET_X + i * CELL_SIZE, BOARD_OFFSET_Y + j * CELL_SIZE, CELL_SIZE, CELL_SIZE, white);
                }
            } else if (cellValue == 2) {  // Black piece
                if (!black.isNull()) {
                    painter.drawPixmap(BOARD_OFFSET_X + i * CELL_SIZE, BOARD_OFFSET_Y + j * CELL_SIZE, CELL_SIZE, CELL_SIZE, black);
                }
            }
            
            // Draw valid move highlights
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
    // Handle click events (board range)
    int x = e->pos().x();
    int y = e->pos().y();
    
    if (x >= BOARD_OFFSET_X && x < BOARD_OFFSET_X + BOARD_SIZE &&
        y >= BOARD_OFFSET_Y && y < BOARD_OFFSET_Y + BOARD_SIZE) {
        int col = (x - BOARD_OFFSET_X) / CELL_SIZE;
        int row = (y - BOARD_OFFSET_Y) / CELL_SIZE;
        
        // Check if game has started
        Reversi::GamePhase phase = gameController_->getCurrentPhase();
        if (phase == Reversi::GamePhase::Waiting) {
            addChatMessage(tr("System"), tr("Please wait for the game to start..."));
            return;
        }
        
        // Check if it's current player's turn and it's our turn
        Reversi::PlayerColor currentPlayer = gameController_->getCurrentPlayer();
        if (phase == Reversi::GamePhase::HumanTurn && currentPlayer == localPlayerColor_) {
            // Send move to network
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
    
    // If game already started, ignore
    if (gameStarted_) {
        qDebug() << "Game already started, ignoring start button";
        addChatMessage(tr("System"), tr("Game has already started!"));
        return;
    }
    
    // If already ready, ignore
    if (localPlayerReady_) {
        qDebug() << "Already ready, ignoring start button";
        addChatMessage(tr("System"), tr("You are already ready!"));
        return;
    }
    
    // Mark local player as ready
    localPlayerReady_ = true;
    
    // Send ready message to opponent
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
        
        // Record sent timestamp for echo detection
        lastSentTimestamp_ = currentTimestamp;
        
        // Client sends message with queue, ensure message is sent
        client->sendMessage(msg);
        qDebug() << "Sent PLAYER_READY message, sender:" << msg.sender << "timestamp:" << msg.timestamp;
    }
    
    // Show ready status
    addChatMessage(tr("System"), tr("You are ready! Waiting for opponent..."));
    startButton_->setEnabled(false);
    startButton_->setText("Ready");
    
    // If host, send initial game state to opponent (for synchronization)
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
    
    // Check if both players are ready
    checkAndStartGame();
}

void NetworkGameWindow::onPlayerReadyReceived(const QString& playerName, const QString& sender, qint64 timestamp)
{
    qDebug() << "NetworkGameWindow::onPlayerReadyReceived from" << playerName << "sender:" << sender << "isHost:" << isHost_ << "timestamp:" << timestamp;
    
    // If game already started, ignore
    if (gameStarted_) {
        qDebug() << "Game already started, ignoring ready message";
        return;
    }
    
    // Echo detection: compare timestamps
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
        
        // If message timestamp <= last sent timestamp - 100ms, consider it echo
        // But if time difference is large (>1 second), it's from opponent
        if (sentDiff < 0 && qAbs(sentDiff) > 100) {
            // Message is earlier than last sent, it's opponent's message
            qDebug() << "Host: msg is BEFORE last sent, this is opponent's message";
        } else if (sentDiff > 100) {
            // Message is later than last sent by 100ms+, it's opponent's message
            qDebug() << "Host: msg is AFTER last sent by" << sentDiff << "ms, this is opponent's message";
        } else {
            // Timestamps are close, might be echo
            if (sender == localPlayerName_) {
                qDebug() << "Host ignoring own ready message (timestamp and sender echo)";
                return;
            }
        }
        
        lastReceivedTimestamp_ = timestamp;
    } else {
        // Client: any message received is from host
        qDebug() << "Client received ready from opponent (host)";
        lastReceivedTimestamp_ = timestamp;
    }
    
    // This is opponent's ready message
    QString opponentName = playerName.isEmpty() ? sender : playerName;
    opponentPlayerName_ = opponentName;
    opponentReady_ = true;
    
    addChatMessage(tr("System"), tr("%1 is ready!").arg(opponentName));
    updateOpponentDisplay();
    
    qDebug() << "Opponent ready, localReady=" << localPlayerReady_ << "opponentReady=" << opponentReady_;
    
    // Check if both players are ready
    checkAndStartGame();
}

void NetworkGameWindow::checkAndStartGame()
{
    qDebug() << "checkAndStartGame: localReady=" << localPlayerReady_ 
             << "opponentReady=" << opponentReady_;
    
    // Only start game when both players are ready
    if (localPlayerReady_ && opponentReady_) {
        qDebug() << "Both players ready, starting game!";
        
        gameStarted_ = true;
        
        // Host plays black, client plays white
        Reversi::PlayerColor humanColor = isHost_ 
            ? Reversi::PlayerColor::Black 
            : Reversi::PlayerColor::White;
        
        localPlayerColor_ = humanColor;
        
        // Start game
        gameController_->startNewGame(Reversi::GameMode::PvP, humanColor);
        
        addChatMessage(tr("System"), tr("Both players ready! Game starting..."));
        
        qDebug() << "Network game started, localPlayerColor:" 
                 << (humanColor == Reversi::PlayerColor::Black ? "Black" : "White");
    }
}

void NetworkGameWindow::onUndoClicked()
{
    qDebug() << "NetworkGameWindow: Undo clicked";

    // Prevent double-click during pending
    if (undoPending_ || undoInProgress_) {
        qDebug() << "Undo already pending or in progress, ignoring click";
        return;
    }

    // Check if game is running
    if (!gameStarted_) {
        addChatMessage(tr("System"), tr("Game not started yet."));
        return;
    }

    // Get network client
    Network::NetworkClient* client = isHost_ ? networkHost_ : networkClient_;
    if (!client || !client->isConnected()) {
        addChatMessage(tr("System"), tr("Not connected to opponent."));
        return;
    }

    // Check if there are any moves to undo (via GameController)
    if (!gameController_->canUndo()) {
        addChatMessage(tr("System"), tr("No moves to undo."));
        return;
    }

    // Send undo request to opponent
    Network::Message msg = Network::Message::createUndoRequest();
    if (client->sendMessage(msg)) {
        undoPending_ = true;
        undoButton_->setEnabled(false);
        addChatMessage(tr("System"), tr("Undo requested - waiting for opponent..."));
        qDebug() << "Undo request sent, waiting for opponent response";
    } else {
        addChatMessage(tr("System"), tr("Failed to send undo request."));
    }
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

    // Use saved color (host is black, client is white)
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
    
    // Check if necessary components are initialized
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
    
    // Enable chat (opponent connected, can start chatting)
    chatInput_->setEnabled(true);
    sendChatButton_->setEnabled(true);
    qDebug() << "Chat enabled for opponent join";
    
    // Host sends complete game state to newly connected opponent
    // Note: Must use NetworkHost::sendMessage() directly,
    // because NetworkHost::sendGameState() uses base class state_ (always Disconnected)
    if (isHost_) {
        qDebug() << "This is the host, preparing to send game state";
        
        // Check if NetworkHost is valid
        if (!networkHost_) {
            qWarning() << "NetworkHost is null, cannot send game state";
            return;
        }
        
        qDebug() << "NetworkHost is valid, checking connection status";
        
        // Check if connected
        if (!networkHost_->isConnected()) {
            qWarning() << "NetworkHost not connected, cannot send game state";
            return;
        }
        
        qDebug() << "NetworkHost is connected, building game state message";
        
        // Get current board state
        const Reversi::Board& board = gameController_->getBoard();
        
        Network::GameStateMessage state;
        // Initialize board array (GameStateMessage::board is empty by default)
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

    // Update turn indicator
    QString turnText = (player == Reversi::PlayerColor::Black) ? tr("Black's Turn") : tr("White's Turn");
    turnIndicator_->setText(turnText);

    // Set color highlight based on current player
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
    
    // Enable chat
    chatInput_->setEnabled(true);
    sendChatButton_->setEnabled(true);
    
    // Save state for reconnection
    if (networkClient_) {
        reconnector_->saveState(QJsonObject());
    }
    
    // Start latency update
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
    
    // Check if reconnection is needed
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
    
    // Disable chat
    chatInput_->setEnabled(false);
    sendChatButton_->setEnabled(false);
    
    emit connectionLost();
}

void NetworkGameWindow::onMoveReceived(int row, int col, const QString& player)
{
    qDebug() << "onMoveReceived called:" << player << "@" << row << "," << col;

    // Check if game has started
    if (!gameStarted_) {
        qWarning() << "Game not started yet, ignoring move";
        return;
    }

    // Apply remote move
    qDebug() << "Calling applyRemoteMove...";
    applyRemoteMove(row, col, player);
    qDebug() << "applyRemoteMove completed";
}

void NetworkGameWindow::onGameStateReceived(const Network::GameStateMessage& state)
{
    qDebug() << "NetworkGameWindow::onGameStateReceived called, currentPlayer:" << state.currentPlayer
             << "gameStarted:" << gameStarted_;
    
    // If game already started, this may be a sync message, just verify not apply
    if (gameStarted_) {
        qDebug() << "Game already started, verifying received state";
        if (synchronizer_) {
            synchronizer_->verifyState(state);
        }
        // If the received state has a LOWER moveNumber than our current board,
        // this is an undo sync - we MUST apply it to stay in sync
        if (gameController_) {
            int localMoveCount = gameController_->getBoard().getMoveCount();
            if (state.moveNumber < localMoveCount) {
                qDebug() << "Undo detected: received moveNumber=" << state.moveNumber
                         << "is less than local=" << localMoveCount
                         << ", applying synced board state";
                applySyncedBoardState(state);
            }
        }
        return;
    }
    
    // Check if necessary components are initialized
    if (!synchronizer_) {
        qWarning() << "Synchronizer not initialized, cannot verify state";
    } else {
        synchronizer_->verifyState(state);
    }
    
    // If we are the client (not host), apply received initial board state
    // Host already has full state, only client needs to sync initial state
    if (!isHost_ && gameController_) {
        qDebug() << "Applying initial game state to client board";
        
        // Check if gameController_ is valid
        try {
            // Reset board to standard starting position
            gameController_->resetGame();
            
            // Convert QVector to std::vector (Board doesn't link Qt)
            std::vector<std::vector<int>> stdBoard(8, std::vector<int>(8));
            for (int row = 0; row < 8; ++row) {
                for (int col = 0; col < 8; ++col) {
                    stdBoard[row][col] = state.board[row][col];
                }
            }
            
            // Sync board directly from state
            Reversi::PlayerColor nextPlayer = (state.currentPlayer == "black")
                ? Reversi::PlayerColor::Black
                : Reversi::PlayerColor::White;
            gameController_->getBoard().syncFrom(stdBoard, nextPlayer, state.moveNumber);
            
            // Set to waiting state (wait for both players to be ready before starting)
            gameController_->setGamePhase(Reversi::GamePhase::Waiting);
            
            // Update UI
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

    // Send to network (both host and client mode use same client pointer)
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

        // Show own message
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

// ==================== Undo Slots ====================

void NetworkGameWindow::onUndoRequestReceived()
{
    qDebug() << "NetworkGameWindow: Undo request received from opponent";

    // Ignore if undo is already in progress or game hasn't started
    if (undoInProgress_) {
        qDebug() << "Undo already in progress, ignoring request";
        return;
    }
    if (!gameStarted_) {
        qDebug() << "Game not started, ignoring undo request";
        return;
    }

    // Show dialog asking whether to accept the undo
    QString opponentName = isHost_ ? opponentPlayerName_ : opponentPlayerName_;
    QString title = tr("Undo Request");
    QString text = tr("%1 wants to undo the last move.\n\n"
                      "If %1 has already placed a piece this turn, both moves will be undone.\n"
                      "Do you agree?").arg(opponentName);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, title, text,
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    bool accepted = (reply == QMessageBox::Yes);
    qDebug() << "User responded to undo request:" << accepted;

    respondToUndoRequest(accepted);
}

void NetworkGameWindow::respondToUndoRequest(bool accepted)
{
    Network::NetworkClient* client = isHost_ ? networkHost_ : networkClient_;
    if (!client) return;

    // Send response to opponent
    Network::Message response = Network::Message::createUndoResponse(accepted);
    client->sendMessage(response);

    if (accepted) {
        addChatMessage(tr("System"), tr("You accepted the undo request."));
        undoInProgress_ = true;

        // Determine how many steps to undo:
        // If current turn = undo requester's color -> B hasn't moved yet -> undo 1 step
        // If current turn != undo requester's color -> B has moved -> undo 2 steps
        Reversi::PlayerColor currentTurn = gameController_->getCurrentPlayer();
        // Opponent's color: if I am host, opponent is White; if I am client, opponent is Black
        Reversi::PlayerColor requester = isHost_
            ? Reversi::PlayerColor::White
            : Reversi::PlayerColor::Black;

        int steps = (currentTurn == requester) ? 1 : 2;
        qDebug() << "Undo accepted, executing" << steps << "step(s) (current turn="
                 << (int)currentTurn << ", requester=" << (int)requester << ")";

        executeLocalUndo(steps);

        undoInProgress_ = false;
        undoPending_ = false;

        // Re-enable undo button if there are still moves to undo
        if (gameController_->canUndo() && client->isConnected()) {
            undoButton_->setEnabled(true);
        }
    } else {
        addChatMessage(tr("System"), tr("You rejected the undo request."));
        undoPending_ = false;
        undoButton_->setEnabled(true);
    }
}

void NetworkGameWindow::onUndoResponseReceived(bool accepted)
{
    qDebug() << "NetworkGameWindow: Undo response received, accepted=" << accepted;

    if (!undoPending_) {
        qDebug() << "No pending undo request, ignoring response";
        return;
    }

    undoPending_ = false;

    if (!accepted) {
        addChatMessage(tr("System"), tr("Opponent rejected the undo request."));
        undoButton_->setEnabled(true);
        return;
    }

    // Opponent accepted: determine how many steps to undo locally
    addChatMessage(tr("System"), tr("Opponent accepted the undo request."));
    undoInProgress_ = true;

    Reversi::PlayerColor currentTurn = gameController_->getCurrentPlayer();
    // Opponent's color: if I am host, opponent is White; if I am client, opponent is Black
    Reversi::PlayerColor requester = isHost_
        ? Reversi::PlayerColor::White
        : Reversi::PlayerColor::Black;

    int steps = (currentTurn == requester) ? 1 : 2;
    qDebug() << "Undo accepted remotely, executing" << steps << "step(s) locally";

    executeLocalUndo(steps);

    undoInProgress_ = false;

    // Re-enable undo button if there are still moves to undo
    Network::NetworkClient* client = isHost_ ? networkHost_ : networkClient_;
    if (gameController_->canUndo() && client && client->isConnected()) {
        undoButton_->setEnabled(true);
    }
}

void NetworkGameWindow::executeLocalUndo(int undoSteps)
{
    qDebug() << "Executing local undo, steps=" << undoSteps;

    for (int i = 0; i < undoSteps; ++i) {
        if (gameController_->canUndo()) {
            gameController_->undoMove();
            qDebug() << "Undo step" << (i + 1) << "completed";
        } else {
            qDebug() << "No more moves to undo at step" << (i + 1);
            break;
        }
    }

    // Update UI
    updateScoreDisplay();
    update();
    addChatMessage(tr("System"),
        tr("Board has been restored by %1 move(s).").arg(undoSteps));

    // Sync board state to opponent so both sides agree on the restored board
    syncBoardState();
}

void NetworkGameWindow::onPongReceived(qint64 latency)
{
    currentLatency_ = static_cast<int>(latency);
    latencyLCD_->display(currentLatency_);
    
    // Update latency color
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
    
    // Disable chat
    chatInput_->setEnabled(false);
    sendChatButton_->setEnabled(false);
    
    // Start reconnection
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
    // Get current player color (before makeHumanMove because it switches player)
    Reversi::PlayerColor color = gameController_->getCurrentPlayer();
    QString player = (color == Reversi::PlayerColor::Black) ? "black" : "white";
    
    // Apply local move
    gameController_->makeHumanMove(row, col);
    
    // Also send to network
    Network::NetworkHost* host = qobject_cast<Network::NetworkHost*>(networkHost_);
    if (host && host->isConnected()) {
        // Get current move number
        int moveNumber = gameController_->getBoard().getMoveCount();
        bool sent = host->sendMove(row, col, player, moveNumber);
        qDebug() << "Sent local move via NetworkHost::sendMove, result:" << sent << player << "@" << row << "," << col;
    } else {
        // Client mode
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

    // Get valid moves list
    qDebug() << "Getting valid moves from board...";
    const Reversi::Board& board = gameController_->getBoard();
    auto validMoves = board.getValidMoves();
    qDebug() << "Got" << validMoves.size() << "valid moves";

    // Check if move is valid
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

void NetworkGameWindow::applySyncedBoardState(const Network::GameStateMessage& state)
{
    qDebug() << "=== applySyncedBoardState START ===";

    if (!gameController_) {
        qCritical() << "applySyncedBoardState: gameController_ is null!";
        return;
    }

    // Convert QVector to std::vector
    std::vector<std::vector<int>> stdBoard(8, std::vector<int>(8));
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            stdBoard[row][col] = state.board[row][col];
        }
    }

    // Sync board from state
    Reversi::PlayerColor nextPlayer = (state.currentPlayer == "black")
        ? Reversi::PlayerColor::Black
        : Reversi::PlayerColor::White;
    gameController_->getBoard().syncFrom(stdBoard, nextPlayer, state.moveNumber);
    // Also sync GameController's currentPlayer_ (Board::current_turn_ is synced,
    // but GameController::currentPlayer_ is separate and stays stale without this)
    gameController_->setCurrentPlayer(nextPlayer);

    // Update UI
    updateScoreDisplay();
    update();

    qDebug() << "Applied synced board state, moveNumber=" << state.moveNumber
             << "currentPlayer=" << state.currentPlayer;
    qDebug() << "=== applySyncedBoardState END ===";
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

void NetworkGameWindow::syncBoardState()
{
    Network::NetworkClient* client = isHost_ ? networkHost_ : networkClient_;
    if (!client || !client->isConnected()) {
        qDebug() << "syncBoardState: no active connection, skipping";
        return;
    }

    const Reversi::Board& board = gameController_->getBoard();
    Network::GameStateMessage state;
    state.board.resize(8);
    for (int i = 0; i < 8; ++i) {
        state.board[i].resize(8);
        for (int j = 0; j < 8; ++j) {
            state.board[i][j] = board.at(i, j);
        }
    }
    state.currentPlayer = (gameController_->getCurrentPlayer() == Reversi::PlayerColor::Black)
        ? "black" : "white";

    int bc = 0, wc = 0;
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j) {
            if (state.board[i][j] == 2) ++bc;
            else if (state.board[i][j] == 1) ++wc;
        }
    state.blackCount = bc;
    state.whiteCount = wc;
    state.moveNumber = board.getMoveCount();

    Network::Message msg;
    msg.type = Network::MessageType::GAME_STATE_UPDATE;
    msg.timestamp = QDateTime::currentMSecsSinceEpoch();
    msg.payload = state.toJson();

    bool sent = client->sendMessage(msg);
    qDebug() << "syncBoardState: sent=" << sent << "moveCount=" << state.moveNumber;
}
