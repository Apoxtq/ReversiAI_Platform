/*
    ReversiAI_Platform - Network Module

    @file networklobbywindow.cpp
    @brief Network multiplayer lobby implementation
    @date 2026
    @author Project Team
    @license GPL-3.0

    Reference: QtReversi widget.cpp structure
*/

#include "ui/NetworkLobbyWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>

NetworkLobbyWindow::NetworkLobbyWindow(QWidget* parent)
    : QMainWindow(parent)
    , discovery_(nullptr)
    , roomManager_(nullptr)
    , networkClient_(nullptr)
    , localPlayerName_("Player")
    , localRoomName_("")
    , isHosting_(false)
    , modeTabWidget_(nullptr)
    , onlineModeWidget_(nullptr)
    , ggsStatusLabel_(nullptr)
    , ggsConnectButton_(nullptr)
    , ggsDisconnectButton_(nullptr)
    , opponentEdit_(nullptr)
    , timeLimitCombo_(nullptr)
    , ratedCheck_(nullptr)
    , sendChallengeButton_(nullptr)
    , challengeList_(nullptr)
    , ggsChatDisplay_(nullptr)
    , ggsChatInput_(nullptr)
    , ggsSendChatButton_(nullptr)
    , ggsClient_(nullptr)
    , ggsUsername_("")
    , isGGSSConnected_(false)
{
    setupUI();
    setupConnections();
}

NetworkLobbyWindow::~NetworkLobbyWindow()
{
    stopDiscovery();
    // No ui member to delete
}

void NetworkLobbyWindow::setupUI()
{
    // Reference: QtReversi widget.cpp init pattern
    
    // Set window properties
    setWindowTitle(tr("Network Lobby - ReversiAI"));
    resize(800, 600);
    
    // Central widget
    centralWidget_ = new QWidget(this);
    setCentralWidget(centralWidget_);
    
    // Main layout
    mainLayout_ = new QVBoxLayout(centralWidget_);
    mainLayout_->setSpacing(10);
    mainLayout_->setContentsMargins(20, 20, 20, 20);
    
    // Title
    QLabel* titleLabel = new QLabel(tr("Network Multiplayer Lobby"), this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout_->addWidget(titleLabel);
    
    // Create Tab Widget for LAN / Online mode selection
    modeTabWidget_ = new QTabWidget(this);
    
    // ==================== LAN Mode Tab ====================
    QWidget* lanModeWidget = new QWidget(this);
    QVBoxLayout* lanLayout = new QVBoxLayout(lanModeWidget);
    
    // Status label
    statusLabel_ = new QLabel(tr("Ready"), this);
    statusLabel_->setStyleSheet("color: gray;");
    statusLabel_->setAlignment(Qt::AlignCenter);
    lanLayout->addWidget(statusLabel_);
    
    // Room table
    roomTable_ = new QTableWidget(this);
    roomTable_->setColumnCount(5);
    roomTable_->setHorizontalHeaderLabels(QStringList() 
        << tr("Room Name") << tr("Host") << tr("Players") << tr("Time Limit") << tr("Status"));
    roomTable_->horizontalHeader()->setStretchLastSection(true);
    roomTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    roomTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    roomTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    roomTable_->setMinimumHeight(200);
    lanLayout->addWidget(roomTable_);
    
    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    refreshButton_ = new QPushButton(tr("Refresh"), this);
    buttonLayout->addWidget(refreshButton_);
    
    joinRoomButton_ = new QPushButton(tr("Join Room"), this);
    joinRoomButton_->setEnabled(false);
    buttonLayout->addWidget(joinRoomButton_);
    
    lanLayout->addLayout(buttonLayout);
    
    // Create room group
    createGroup_ = new QGroupBox(tr("Create New Room"), this);
    QFormLayout* formLayout = new QFormLayout();
    
    roomNameEdit_ = new QLineEdit(this);
    roomNameEdit_->setPlaceholderText(tr("Enter room name"));
    formLayout->addRow(tr("Room Name:"), roomNameEdit_);
    
    playerNameEdit_ = new QLineEdit(this);
    playerNameEdit_->setPlaceholderText(tr("Enter your name"));
    playerNameEdit_->setText(localPlayerName_);
    formLayout->addRow(tr("Your Name:"), playerNameEdit_);
    
    timeLimitSpin_ = new QSpinBox(this);
    timeLimitSpin_->setRange(0, 60);
    timeLimitSpin_->setValue(0);
    timeLimitSpin_->setSuffix(tr(" min (0 = unlimited)"));
    formLayout->addRow(tr("Time Limit:"), timeLimitSpin_);
    
    rankedCheck_ = new QCheckBox(tr("Ranked Game"), this);
    formLayout->addRow(tr("Game Type:"), rankedCheck_);
    
    createButton_ = new QPushButton(tr("Create Room"), this);
    formLayout->addRow("", createButton_);
    
    createGroup_->setLayout(formLayout);
    lanLayout->addWidget(createGroup_);
    
    modeTabWidget_->addTab(lanModeWidget, tr("LAN Mode"));
    
    // ==================== Online Mode Tab ====================
    setupOnlineModeUI();
    modeTabWidget_->addTab(onlineModeWidget_, tr("Online Mode (GGS)"));
    
    mainLayout_->addWidget(modeTabWidget_);
    
    // Bottom buttons
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();
    
    backButton_ = new QPushButton(tr("Back to Menu"), this);
    bottomLayout->addWidget(backButton_);
    
    mainLayout_->addLayout(bottomLayout);
}

void NetworkLobbyWindow::setupConnections()
{
    // Reference: QtReversi widget.cpp connect pattern
    
    // Button connections
    connect(refreshButton_, &QPushButton::clicked, this, &NetworkLobbyWindow::onRefreshClicked);
    connect(joinRoomButton_, &QPushButton::clicked, this, &NetworkLobbyWindow::onJoinRoomClicked);
    connect(createButton_, &QPushButton::clicked, this, &NetworkLobbyWindow::onCreateRoomClicked);
    connect(backButton_, &QPushButton::clicked, this, &NetworkLobbyWindow::onBackClicked);
    
    // Table connections
    connect(roomTable_, &QTableWidget::itemClicked, 
            this, &NetworkLobbyWindow::onRoomTableItemClicked);
    connect(roomTable_, &QTableWidget::itemDoubleClicked, 
            this, &NetworkLobbyWindow::onRoomTableItemDoubleClicked);
}

void NetworkLobbyWindow::initNetwork()
{
    // Create network modules
    discovery_ = new Network::NetworkDiscovery(this);
    roomManager_ = new Network::RoomManager(this);
    networkClient_ = new Network::NetworkClient(this);
    
    // Connect discovery signals
    connect(discovery_, &Network::NetworkDiscovery::hostFound,
            this, &NetworkLobbyWindow::onHostFound);
    connect(discovery_, &Network::NetworkDiscovery::hostLost,
            this, &NetworkLobbyWindow::onHostLost);
    connect(discovery_, &Network::NetworkDiscovery::discoveryError,
            this, &NetworkLobbyWindow::onDiscoveryError);
    
    // Connect room signals
    connect(roomManager_, &Network::RoomManager::roomCreated,
            this, &NetworkLobbyWindow::onRoomCreated);
    connect(roomManager_, &Network::RoomManager::roomListChanged,
            this, &NetworkLobbyWindow::onRoomListChanged);
    connect(roomManager_, &Network::RoomManager::playerJoined,
            this, &NetworkLobbyWindow::onPlayerJoined);
    connect(roomManager_, &Network::RoomManager::playerLeft,
            this, &NetworkLobbyWindow::onPlayerLeft);
    
    qDebug() << "Network modules initialized";
}

void NetworkLobbyWindow::startDiscovery()
{
    if (!discovery_) {
        initNetwork();
    }
    
    discovery_->startDiscovery();
    statusLabel_->setText(tr("Searching for games..."));
    statusLabel_->setStyleSheet("color: blue;");
    
    qDebug() << "Network discovery started";
}

void NetworkLobbyWindow::stopDiscovery()
{
    if (discovery_) {
        discovery_->stopDiscovery();
    }
    
    statusLabel_->setText(tr("Discovery stopped"));
    statusLabel_->setStyleSheet("color: gray;");
}

// ==================== Discovery Slots ====================

void NetworkLobbyWindow::onHostFound(const Network::DiscoveredHost& host)
{
    qDebug() << "Host found:" << host.playerName << "@" << host.address.toString();
    
    updateRoomTable();
    showStatus(tr("Found host: %1").arg(host.playerName));
}

void NetworkLobbyWindow::onHostLost(const Network::DiscoveredHost& host)
{
    qDebug() << "Host lost:" << host.playerName;
    
    updateRoomTable();
    showStatus(tr("Host lost: %1").arg(host.playerName));
}

void NetworkLobbyWindow::onDiscoveryError(const QString& error)
{
    showStatus(tr("Discovery error: %1").arg(error), true);
}

// ==================== Room Slots ====================

void NetworkLobbyWindow::onRoomCreated(const Network::GameRoom& room)
{
    qDebug() << "Room created:" << room.roomName << "(" << room.roomId << ")";
    
    localRoomName_ = room.roomId;
    isHosting_ = true;
    
    updateRoomTable();
    showStatus(tr("Room created: %1").arg(room.roomName));
    
    // Disable create group while hosting
    createGroup_->setEnabled(false);
}

void NetworkLobbyWindow::onRoomListChanged()
{
    updateRoomTable();
}

void NetworkLobbyWindow::onPlayerJoined(const QString& roomId, const QString& playerName)
{
    Q_UNUSED(roomId)
    qDebug() << "Player joined:" << playerName;
    
    showStatus(tr("%1 joined the room").arg(playerName));
    updateRoomTable();
}

void NetworkLobbyWindow::onPlayerLeft(const QString& roomId, const QString& playerName)
{
    Q_UNUSED(roomId)
    qDebug() << "Player left:" << playerName;
    
    showStatus(tr("%1 left the room").arg(playerName));
    updateRoomTable();
}

// ==================== Button Slots ====================

void NetworkLobbyWindow::onCreateRoomClicked()
{
    if (!validateCreateRoomInput()) {
        return;
    }
    
    // Update player name
    localPlayerName_ = playerNameEdit_->text();
    
    // Get room settings
    QJsonObject settings;
    settings["timeLimit"] = timeLimitSpin_->value();
    settings["isRanked"] = rankedCheck_->isChecked();
    
    // Create room
    Network::GameRoom room = roomManager_->createRoom(
        roomNameEdit_->text(),
        localPlayerName_,
        settings
    );
    
    localRoomName_ = room.roomId;
    isHosting_ = true;
    
    emit createGame(roomNameEdit_->text(), localPlayerName_, settings);
}

void NetworkLobbyWindow::onJoinRoomClicked()
{
    QString roomId = getSelectedRoomId();
    if (roomId.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please select a room to join"));
        return;
    }
    
    Network::GameRoom* room = roomManager_->getRoom(roomId);
    if (!room) {
        QMessageBox::warning(this, tr("Error"), tr("Room not found"));
        return;
    }
    
    // Update player name
    localPlayerName_ = playerNameEdit_->text();
    
    // Join room
    if (roomManager_->joinRoom(roomId, localPlayerName_)) {
        isHosting_ = false;
        emit joinGame(room->hostAddress, room->hostPort, localPlayerName_);
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to join room"));
    }
}

void NetworkLobbyWindow::onRefreshClicked()
{
    startDiscovery();
    showStatus(tr("Refreshing room list..."));
}

void NetworkLobbyWindow::onBackClicked()
{
    stopDiscovery();
    emit backToMenu();
}

// ==================== Selection Slots ====================

void NetworkLobbyWindow::onRoomTableItemClicked(QTableWidgetItem* item)
{
    if (item) {
        joinRoomButton_->setEnabled(true);
    }
}

void NetworkLobbyWindow::onRoomTableItemDoubleClicked(QTableWidgetItem* item)
{
    if (item) {
        // Select the row and trigger join
        joinRoomButton_->setEnabled(true);
        onJoinRoomClicked();
    }
}

// ==================== Private Methods ====================

void NetworkLobbyWindow::updateRoomTable()
{
    // Reference: Egaroucid game_information.hpp display pattern
    
    roomTable_->setRowCount(0);
    
    QList<Network::GameRoom> rooms = roomManager_->getAvailableRooms();
    
    for (const Network::GameRoom& room : rooms) {
        int row = roomTable_->rowCount();
        roomTable_->insertRow(row);
        
        // Room name
        QTableWidgetItem* nameItem = new QTableWidgetItem(room.roomName);
        nameItem->setData(Qt::UserRole, room.roomId);
        roomTable_->setItem(row, 0, nameItem);
        
        // Host name
        roomTable_->setItem(row, 1, new QTableWidgetItem(room.hostName));
        
        // Players
        QString playerCount = QString("%1/2").arg(room.players.size());
        roomTable_->setItem(row, 2, new QTableWidgetItem(playerCount));
        
        // Time limit
        QString timeLimit = room.timeLimit > 0 
            ? QString("%1 min").arg(room.timeLimit)
            : tr("Unlimited");
        roomTable_->setItem(row, 3, new QTableWidgetItem(timeLimit));
        
        // Status
        QString status;
        switch (room.state) {
            case Network::RoomState::WAITING:
                status = tr("Waiting");
                break;
            case Network::RoomState::READY:
                status = tr("Ready");
                break;
            case Network::RoomState::PLAYING:
                status = tr("In Game");
                break;
            default:
                status = tr("Unknown");
        }
        roomTable_->setItem(row, 4, new QTableWidgetItem(status));
    }
    
    // Update status
    QString statusText = QString(tr("%1 rooms available")).arg(rooms.size());
    statusLabel_->setText(statusText);
}

void NetworkLobbyWindow::showStatus(const QString& message, bool isError)
{
    statusLabel_->setText(message);
    
    if (isError) {
        statusLabel_->setStyleSheet("color: red;");
    } else {
        statusLabel_->setStyleSheet("color: green;");
    }
}

QString NetworkLobbyWindow::getSelectedRoomId() const
{
    int row = roomTable_->currentRow();
    if (row < 0) {
        return "";
    }
    
    QTableWidgetItem* item = roomTable_->item(row, 0);
    if (!item) {
        return "";
    }
    
    return item->data(Qt::UserRole).toString();
}

bool NetworkLobbyWindow::validateCreateRoomInput()
{
    QString roomName = roomNameEdit_->text().trimmed();
    QString playerName = playerNameEdit_->text().trimmed();
    
    if (roomName.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please enter a room name"));
        return false;
    }
    
    if (playerName.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please enter your name"));
        return false;
    }
    
    if (roomName.length() > 30) {
        QMessageBox::warning(this, tr("Error"), tr("Room name is too long"));
        return false;
    }
    
    return true;
}

void NetworkLobbyWindow::updateDiscoveryStatus()
{
    if (discovery_ && discovery_->isDiscovering()) {
        statusLabel_->setText(tr("Searching for games..."));
        statusLabel_->setStyleSheet("color: blue;");
    } else {
        statusLabel_->setText(tr("Ready"));
        statusLabel_->setStyleSheet("color: gray;");
    }
}

// ==================== GGS Online Mode Implementation ====================

void NetworkLobbyWindow::setupOnlineModeUI()
{
    onlineModeWidget_ = new QWidget(this);
    QVBoxLayout* onlineLayout = new QVBoxLayout(onlineModeWidget_);
    onlineLayout->setSpacing(10);
    
    // Connection status
    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLayout->addWidget(new QLabel(tr("Status:"), this));
    ggsStatusLabel_ = new QLabel(tr("Disconnected"), this);
    ggsStatusLabel_->setStyleSheet("font-weight: bold; color: gray;");
    statusLayout->addWidget(ggsStatusLabel_);
    statusLayout->addStretch();
    
    ggsConnectButton_ = new QPushButton(tr("Connect to GGS"), this);
    ggsDisconnectButton_ = new QPushButton(tr("Disconnect"), this);
    ggsDisconnectButton_->setEnabled(false);
    statusLayout->addWidget(ggsConnectButton_);
    statusLayout->addWidget(ggsDisconnectButton_);
    onlineLayout->addLayout(statusLayout);
    
    // Challenge section
    QGroupBox* challengeGroup = new QGroupBox(tr("Challenge Opponent"), this);
    QFormLayout* challengeLayout = new QFormLayout();
    
    opponentEdit_ = new QLineEdit(this);
    opponentEdit_->setPlaceholderText(tr("Enter opponent's username"));
    challengeLayout->addRow(tr("Opponent:"), opponentEdit_);
    
    timeLimitCombo_ = new QComboBox(this);
    timeLimitCombo_->addItems(QStringList() << "1 min" << "3 min" << "5 min" << "10 min" << "15 min" << "30 min");
    timeLimitCombo_->setCurrentIndex(2); // Default 5 min
    challengeLayout->addRow(tr("Time:"), timeLimitCombo_);
    
    ratedCheck_ = new QCheckBox(tr("Rated Match"), this);
    challengeLayout->addRow(tr("Type:"), ratedCheck_);
    
    sendChallengeButton_ = new QPushButton(tr("Send Challenge"), this);
    sendChallengeButton_->setEnabled(false);
    challengeLayout->addRow("", sendChallengeButton_);
    
    challengeGroup->setLayout(challengeLayout);
    onlineLayout->addWidget(challengeGroup);
    
    // Incoming challenge section
    QGroupBox* incomingGroup = new QGroupBox(tr("Incoming Challenges"), this);
    QVBoxLayout* incomingLayout = new QVBoxLayout();
    
    challengeList_ = new QListWidget(this);
    challengeList_->setMaximumHeight(100);
    incomingLayout->addWidget(challengeList_);
    
    // We'll add Accept/Decline buttons when challenge arrives
    incomingGroup->setLayout(incomingLayout);
    onlineLayout->addWidget(incomingGroup);
    
    // Chat section
    QGroupBox* chatGroup = new QGroupBox(tr("Chat"), this);
    QVBoxLayout* chatLayout = new QVBoxLayout();
    
    ggsChatDisplay_ = new QTextEdit(this);
    ggsChatDisplay_->setReadOnly(true);
    ggsChatDisplay_->setMaximumHeight(150);
    chatLayout->addWidget(ggsChatDisplay_);
    
    QHBoxLayout* chatInputLayout = new QHBoxLayout();
    ggsChatInput_ = new QLineEdit(this);
    ggsChatInput_->setPlaceholderText(tr("Type a message..."));
    ggsChatInput_->setEnabled(false);
    chatInputLayout->addWidget(ggsChatInput_);
    
    ggsSendChatButton_ = new QPushButton(tr("Send"), this);
    ggsSendChatButton_->setEnabled(false);
    chatInputLayout->addWidget(ggsSendChatButton_);
    
    chatLayout->addLayout(chatInputLayout);
    chatGroup->setLayout(chatLayout);
    onlineLayout->addWidget(chatGroup);
    
    onlineLayout->addStretch();
}

void NetworkLobbyWindow::updateGGSStatus()
{
    if (isGGSSConnected_) {
        ggsStatusLabel_->setText(tr("Connected as %1").arg(ggsUsername_));
        ggsStatusLabel_->setStyleSheet("font-weight: bold; color: green;");
        ggsConnectButton_->setEnabled(false);
        ggsDisconnectButton_->setEnabled(true);
        sendChallengeButton_->setEnabled(true);
        ggsChatInput_->setEnabled(true);
        ggsSendChatButton_->setEnabled(true);
    } else {
        ggsStatusLabel_->setText(tr("Disconnected"));
        ggsStatusLabel_->setStyleSheet("font-weight: bold; color: gray;");
        ggsConnectButton_->setEnabled(true);
        ggsDisconnectButton_->setEnabled(false);
        sendChallengeButton_->setEnabled(false);
        ggsChatInput_->setEnabled(false);
        ggsSendChatButton_->setEnabled(false);
    }
}

void NetworkLobbyWindow::addChallengeToList(const Network::GGSMatchRequest& request)
{
    QString itemText = QString("%1 wants to play! (Time: %2 min)")
                      .arg(request.player[0].name)
                      .arg(request.clock[0].ini_time / 60000);
    QListWidgetItem* item = new QListWidgetItem(itemText, challengeList_);
    item->setData(Qt::UserRole, request.requestId);
    
    // Add accept/decline buttons
    QWidget* buttonWidget = new QWidget();
    QHBoxLayout* buttonLayout = new QHBoxLayout(buttonWidget);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    
    QPushButton* acceptBtn = new QPushButton(tr("Accept"), buttonWidget);
    QPushButton* declineBtn = new QPushButton(tr("Decline"), buttonWidget);
    
    connect(acceptBtn, &QPushButton::clicked, this, [this, request]() {
        onGGSAcceptChallengeClicked(request.requestId);
    });
    connect(declineBtn, &QPushButton::clicked, this, [this, request]() {
        onGGSDeclineChallengeClicked(request.requestId);
    });
    
    buttonLayout->addWidget(acceptBtn);
    buttonLayout->addWidget(declineBtn);
    
    challengeList_->setItemWidget(item, buttonWidget);
}

// ==================== GGS Slot Implementations ====================

void NetworkLobbyWindow::onGGSConnectClicked()
{
    // Show login dialog or use default credentials
    // For now, use guest login with a generated username
    QString username = "Player" + QString::number(QRandomGenerator::global()->bounded(1000, 9999));
    
    // Create GGS client if not exists
    if (!ggsClient_) {
        ggsClient_ = new Network::GGSGameClient(this);
        setupGGSConnections();
    }
    
    // Connect to GGS server
    ggsClient_->connectToServer(Network::GGSProtocol::GGS_URL, Network::GGSProtocol::GGS_PORT);
}

void NetworkLobbyWindow::onGGSDisconnectClicked()
{
    if (ggsClient_) {
        ggsClient_->disconnectFromServer();
    }
    isGGSSConnected_ = false;
    ggsUsername_ = "";
    updateGGSStatus();
}

void NetworkLobbyWindow::onGGSSendChallengeClicked()
{
    QString opponent = opponentEdit_->text().trimmed();
    if (opponent.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please enter an opponent's username"));
        return;
    }
    
    if (!ggsClient_) return;
    
    // Get time limit from combo
    int timeMinutes[] = {1, 3, 5, 10, 15, 30};
    int timeLimitMs = timeMinutes[timeLimitCombo_->currentIndex()] * 60 * 1000;
    
    // Send challenge
    bool isRated = ratedCheck_->isChecked();
    if (ggsClient_->sendMatchRequest(opponent, isRated, timeLimitMs)) {
        ggsChatDisplay_->append(QString("[System] Challenge sent to %1").arg(opponent));
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to send challenge"));
    }
}

void NetworkLobbyWindow::onGGSAcceptChallengeClicked(const QString& requestId)
{
    if (ggsClient_) {
        ggsClient_->acceptMatch(requestId);
    }
}

void NetworkLobbyWindow::onGGSDeclineChallengeClicked(const QString& requestId)
{
    if (ggsClient_) {
        ggsClient_->declineMatch(requestId);
    }
    
    // Remove from list
    for (int i = 0; i < challengeList_->count(); ++i) {
        QListWidgetItem* item = challengeList_->item(i);
        if (item->data(Qt::UserRole).toString() == requestId) {
            delete challengeList_->takeItem(i);
            break;
        }
    }
}

void NetworkLobbyWindow::onGGSConnected()
{
    // Login as guest
    QString username = "Player" + QString::number(QRandomGenerator::global()->bounded(1000, 9999));
    ggsClient_->login(username, "");
}

void NetworkLobbyWindow::onGGSDisconnected()
{
    isGGSSConnected_ = false;
    ggsUsername_ = "";
    updateGGSStatus();
    ggsChatDisplay_->append("[System] Disconnected from server");
}

void NetworkLobbyWindow::onGGSConnectionError(const QString& error)
{
    QMessageBox::critical(this, tr("Connection Error"), error);
    isGGSSConnected_ = false;
    updateGGSStatus();
}

void NetworkLobbyWindow::onGGSLoginSuccessful(const QString& username)
{
    isGGSSConnected_ = true;
    ggsUsername_ = username;
    updateGGSStatus();
    ggsChatDisplay_->append(QString("[System] Logged in as %1").arg(username));
}

void NetworkLobbyWindow::onGGSLoginFailed(const QString& error)
{
    QMessageBox::warning(this, tr("Login Failed"), error);
    isGGSSConnected_ = false;
    updateGGSStatus();
}

void NetworkLobbyWindow::onGGSMatchRequestReceived(const Network::GGSMatchRequest& request)
{
    addChallengeToList(request);
    ggsChatDisplay_->append(QString("[System] %1 wants to play!").arg(request.player[0].name));
}

void NetworkLobbyWindow::onGGSGameStarted(const QString& gameId, const QString& playerBlack,
                                          const QString& playerWhite, bool isPlayerBlack)
{
    // Emit signal to start GGS game window
    emit ggsGameStarted(ggsClient_, gameId, playerBlack, playerWhite, isPlayerBlack);
}

void NetworkLobbyWindow::onGGSChatReceived(const QString& sender, const QString& message)
{
    ggsChatDisplay_->append(QString("<%1>: %2").arg(sender, message));
}

void NetworkLobbyWindow::onGGSSendChatClicked()
{
    QString message = ggsChatInput_->text().trimmed();
    if (message.isEmpty() || !ggsClient_) return;
    
    // Send to global channel (0)
    if (ggsClient_->sendChat("0", message)) {
        ggsChatInput_->clear();
    }
}

void NetworkLobbyWindow::onTabChanged(int index)
{
    // Stop LAN discovery when switching to Online tab
    if (index == 1) { // Online Mode
        stopDiscovery();
    }
}

void NetworkLobbyWindow::setupGGSConnections()
{
    if (!ggsClient_) return;
    
    // Connect GGS client signals to slots
    connect(ggsClient_, &Network::GGSGameClient::connected,
            this, &NetworkLobbyWindow::onGGSConnected);
    connect(ggsClient_, &Network::GGSGameClient::disconnected,
            this, &NetworkLobbyWindow::onGGSDisconnected);
    connect(ggsClient_, &Network::GGSGameClient::connectionError,
            this, &NetworkLobbyWindow::onGGSConnectionError);
    connect(ggsClient_, &Network::GGSGameClient::loginSuccessful,
            this, &NetworkLobbyWindow::onGGSLoginSuccessful);
    connect(ggsClient_, &Network::GGSGameClient::loginFailed,
            this, &NetworkLobbyWindow::onGGSLoginFailed);
    connect(ggsClient_, &Network::GGSGameClient::matchRequestReceived,
            this, &NetworkLobbyWindow::onGGSMatchRequestReceived);
    connect(ggsClient_, &Network::GGSGameClient::gameStarted,
            this, &NetworkLobbyWindow::onGGSGameStarted);
    connect(ggsClient_, &Network::GGSGameClient::chatMessageReceived,
            this, &NetworkLobbyWindow::onGGSChatReceived);
}


