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
#include <QIcon>

NetworkLobbyWindow::NetworkLobbyWindow(QWidget* parent)
    : QMainWindow(parent)
    , discovery_(nullptr)
    , roomManager_(nullptr)
    , networkClient_(nullptr)
    , localPlayerName_("Player")
    , localRoomName_("")
    , isHosting_(false)
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
    setWindowIcon(QIcon(":/rsc/black.png"));
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
    
    // Status label
    statusLabel_ = new QLabel(tr("Ready"), this);
    statusLabel_->setStyleSheet("color: gray;");
    statusLabel_->setAlignment(Qt::AlignCenter);
    mainLayout_->addWidget(statusLabel_);
    
    // Room table
    roomTable_ = new QTableWidget(this);
    roomTable_->setColumnCount(4);
    roomTable_->setHorizontalHeaderLabels(QStringList() 
        << tr("Room Name") << tr("Host") << tr("Players") << tr("Status"));
    roomTable_->horizontalHeader()->setStretchLastSection(true);
    roomTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    roomTable_->setSelectionMode(QAbstractItemView::SingleSelection);
    roomTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    roomTable_->setMinimumHeight(200);
    mainLayout_->addWidget(roomTable_);
    
    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    refreshButton_ = new QPushButton(tr("Refresh"), this);
    buttonLayout->addWidget(refreshButton_);
    
    joinRoomButton_ = new QPushButton(tr("Join Room"), this);
    joinRoomButton_->setEnabled(false);
    buttonLayout->addWidget(joinRoomButton_);
    
    mainLayout_->addLayout(buttonLayout);
    
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

    createButton_ = new QPushButton(tr("Create Room"), this);
    formLayout->addRow("", createButton_);
    
    createGroup_->setLayout(formLayout);
    mainLayout_->addWidget(createGroup_);
    
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

    // Sync discovered host into RoomManager so it appears in the room table
    if (roomManager_) {
        roomManager_->addDiscoveredHost(host);
    }

    updateRoomTable();
    showStatus(tr("Found host: %1").arg(host.playerName));
}

void NetworkLobbyWindow::onHostLost(const Network::DiscoveredHost& host)
{
    qDebug() << "Host lost:" << host.playerName;

    // Remove from RoomManager
    if (roomManager_) {
        Network::GameRoom* room = roomManager_->getRoomByHost(host.address, host.port);
        if (room) {
            roomManager_->deleteRoom(room->roomId);
        }
    }

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
    settings["isRanked"] = false;

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
        qWarning() << "No room selected for join";
        QMessageBox::warning(this, tr("Error"), tr("Please select a room to join"));
        return;
    }
    
    qDebug() << "Attempting to join room:" << roomId;
    
    Network::GameRoom* room = roomManager_->getRoom(roomId);
    if (!room) {
        qWarning() << "Room not found in manager:" << roomId;
        QMessageBox::warning(this, tr("Error"), tr("Room not found"));
        return;
    }
    
    qDebug() << "Room found, host:" << room->hostName << "address:" << room->hostAddress.toString() << "port:" << room->hostPort;
    
    // Update player name
    localPlayerName_ = playerNameEdit_->text();
    qDebug() << "Joining as player:" << localPlayerName_;
    
    // Join room
    if (roomManager_->joinRoom(roomId, localPlayerName_)) {
        qDebug() << "Successfully joined room, emitting joinGame signal";
        isHosting_ = false;
        emit joinGame(room->hostAddress, room->hostPort, localPlayerName_);
    } else {
        qWarning() << "Failed to join room:" << roomId;
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
        
        // Players (discovered rooms count host as player 1)
        int displayCount = room.players.size();
        if (room.roomId.startsWith("disc_")) {
            displayCount += 1;  // Host is already in the room
        }
        QString playerCount = QString("%1/2").arg(displayCount);
        roomTable_->setItem(row, 2, new QTableWidgetItem(playerCount));
        
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
        roomTable_->setItem(row, 3, new QTableWidgetItem(status));
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

