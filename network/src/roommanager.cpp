/*
    ReversiAI_Platform - Network Module

    @file roommanager.cpp
    @brief Room management implementation
    @date 2026
    @author Project Team
    @license GPL-3.0

    Reference: Egaroucid ggs.hpp room management (line 539-551)
*/

#include "network/roommanager.hpp"
#include "network/message.hpp"
#include <QDebug>
#include <QRandomGenerator>

namespace Network {

// ==================== RoomSettings Implementation ====================

QJsonObject RoomSettings::toJson() const
{
    QJsonObject json;
    json["timeLimit"] = timeLimit;
    json["isRanked"] = isRanked;
    json["allowSpectators"] = allowSpectators;
    json["maxPlayers"] = maxPlayers;
    json["gameType"] = gameType;
    json["aiDifficulty"] = aiDifficulty;
    return json;
}

RoomSettings RoomSettings::fromJson(const QJsonObject& json)
{
    RoomSettings settings;
    settings.timeLimit = json["timeLimit"].toInt(0);
    settings.isRanked = json["isRanked"].toBool(false);
    settings.allowSpectators = json["allowSpectators"].toBool(true);
    settings.maxPlayers = json["maxPlayers"].toInt(2);
    settings.gameType = json["gameType"].toString("pvp");
    settings.aiDifficulty = json["aiDifficulty"].toInt(5);
    return settings;
}

// ==================== RoomManager Implementation ====================

RoomManager::RoomManager(QObject* parent)
    : QObject(parent)
    , cleanupTimer_(nullptr)
{
    // Create cleanup timer
    cleanupTimer_ = new QTimer(this);
    cleanupTimer_->setInterval(CLEANUP_INTERVAL);
    cleanupTimer_->setSingleShot(false);
    
    connect(cleanupTimer_, &QTimer::timeout, this, &RoomManager::cleanupExpiredRooms);
}

RoomManager::~RoomManager()
{
    cleanupTimer_->stop();
}

// ==================== Room Creation ====================

GameRoom RoomManager::createRoom(const QString& roomName, const QString& hostName, 
                                 const QJsonObject& settings)
{
    GameRoom room;
    room.roomId = generateRoomId();
    room.roomName = roomName;
    room.hostName = hostName;
    room.state = RoomState::WAITING;
    room.timeLimit = 0;
    room.createTime = QDateTime::currentMSecsSinceEpoch();
    room.lastActivityTime = room.createTime;
    
    // Add host as player
    room.players.append(hostName);
    
    rooms_[room.roomId] = room;
    
    emit roomCreated(room);
    emit roomListChanged();
    
    qInfo() << "Room created:" << room.roomName << "(" << room.roomId << ")";
    
    return room;
}

GameRoom RoomManager::createRoom(const QString& roomName, const QString& hostName,
                                 int timeLimit, bool isRanked)
{
    QJsonObject settings;
    settings["timeLimit"] = timeLimit;
    settings["isRanked"] = isRanked;
    return createRoom(roomName, hostName, settings);
}

bool RoomManager::deleteRoom(const QString& roomId)
{
    if (!rooms_.contains(roomId)) {
        emit error("Room not found:" + roomId);
        return false;
    }
    
    GameRoom room = rooms_[roomId];
    rooms_.remove(roomId);
    
    emit roomDeleted(roomId);
    emit roomListChanged();
    
    qInfo() << "Room deleted:" << room.roomName;
    
    return true;
}

// ==================== Room Joining ====================

bool RoomManager::joinRoom(const QString& roomId, const QString& playerName,
                           const QString& playerColor)
{
    Q_UNUSED(playerColor)
    
    qDebug() << "RoomManager::joinRoom called with roomId:" << roomId << "playerName:" << playerName;
    
    if (!rooms_.contains(roomId)) {
        qWarning() << "joinRoom failed: Room not found" << roomId;
        emit error("Room not found:" + roomId);
        return false;
    }
    
    GameRoom& room = rooms_[roomId];
    qDebug() << "Room found, current state:" << static_cast<int>(room.state) << "players:" << room.players.size();
    
    // Check room state
    if (room.state != RoomState::WAITING && room.state != RoomState::READY) {
        qWarning() << "joinRoom failed: Room state is" << static_cast<int>(room.state) << "(not WAITING or READY)";
        emit error("Cannot join room in state:" + QString::number(static_cast<int>(room.state)));
        return false;
    }
    
    // Check if room is full
    if (isRoomFull(room)) {
        qWarning() << "joinRoom failed: Room is full" << room.players.size() << "players";
        emit error("Room is full");
        return false;
    }
    
    // Check if player already in room
    if (room.players.contains(playerName)) {
        qWarning() << "joinRoom failed: Player" << playerName << "already in room";
        emit error("Player already in room");
        return false;
    }
    
    // Add player
    room.players.append(playerName);
    room.lastActivityTime = QDateTime::currentMSecsSinceEpoch();
    
    // If room has 2 players, mark as ready
    if (room.players.size() >= 2) {
        room.state = RoomState::READY;
    }
    
    emit playerJoined(roomId, playerName);
    emit roomStateChanged(roomId, room.state);
    emit roomListChanged();
    
    qInfo() << playerName << "joined room" << room.roomName;
    
    return true;
}

bool RoomManager::leaveRoom(const QString& playerName)
{
    // Find room containing player
    QString roomId;
    for (auto it = rooms_.begin(); it != rooms_.end(); ++it) {
        if (it.value().players.contains(playerName)) {
            roomId = it.key();
            break;
        }
    }
    
    if (roomId.isEmpty()) {
        emit error("Player not in any room:" + playerName);
        return false;
    }
    
    return removePlayer(roomId, playerName);
}

GameRoom* RoomManager::getRoom(const QString& roomId)
{
    if (rooms_.contains(roomId)) {
        return &rooms_[roomId];
    }
    return nullptr;
}

GameRoom* RoomManager::getRoomByHost(const QHostAddress& address, quint16 port)
{
    for (auto it = rooms_.begin(); it != rooms_.end(); ++it) {
        if (it.value().hostAddress == address && it.value().hostPort == port) {
            return &it.value();
        }
    }
    return nullptr;
}

bool RoomManager::addDiscoveredHost(const DiscoveredHost& host)
{
    // Check if already exists (by host address + port)
    if (getRoomByHost(host.address, host.port)) {
        qDebug() << "Discovered host already tracked:" << host.playerName << "@" << host.address.toString() << ":" << host.port;
        return true; // Already tracked
    }

    GameRoom room;
    room.roomId = QString("disc_%1_%2").arg(host.address.toString()).arg(host.port);
    room.roomName = host.roomName.isEmpty() ? QString("%1's Room").arg(host.playerName) : host.roomName;
    room.hostName = host.playerName;
    room.state = RoomState::WAITING;
    room.gameVersion = host.gameVersion;
    room.createTime = QDateTime::currentMSecsSinceEpoch();
    room.lastActivityTime = room.createTime;
    room.hostAddress = host.address;
    room.hostPort = host.port;

    qDebug() << "Adding new discovered host as room:";
    qDebug() << "  roomId:" << room.roomId;
    qDebug() << "  roomName:" << room.roomName;
    qDebug() << "  hostName:" << room.hostName;
    qDebug() << "  hostAddress:" << room.hostAddress.toString();
    qDebug() << "  hostPort:" << room.hostPort;
    qDebug() << "  isJoinable:" << room.isJoinable();

    rooms_[room.roomId] = room;
    emit roomCreated(room);
    emit roomListChanged();

    qInfo() << "Added discovered host as room:" << room.roomName
            << "@" << host.address.toString() << ":" << host.port;
    return true;
}

// ==================== Room Query ====================

QList<GameRoom> RoomManager::getAvailableRooms() const
{
    QList<GameRoom> available;
    
    for (const GameRoom& room : rooms_) {
        if (room.isJoinable()) {
            available.append(room);
        }
    }
    
    return available;
}

int RoomManager::getPlayerCount() const
{
    int count = 0;
    for (const GameRoom& room : rooms_) {
        count += room.players.size();
    }
    return count;
}

bool RoomManager::roomExists(const QString& roomId) const
{
    return rooms_.contains(roomId);
}

// ==================== Room State ====================

bool RoomManager::startGame(const QString& roomId)
{
    if (!rooms_.contains(roomId)) {
        emit error("Room not found:" + roomId);
        return false;
    }
    
    GameRoom& room = rooms_[roomId];
    
    // Check if room has enough players
    if (room.players.size() < 2) {
        emit error("Not enough players to start");
        return false;
    }
    
    room.state = RoomState::PLAYING;
    room.lastActivityTime = QDateTime::currentMSecsSinceEpoch();
    
    emit roomStateChanged(roomId, room.state);
    emit gameStarted(roomId);
    
    qInfo() << "Game started in room" << room.roomName;
    
    return true;
}

bool RoomManager::endGame(const QString& roomId, const QString& winner,
                          const QString& reason)
{
    Q_UNUSED(reason)
    
    if (!rooms_.contains(roomId)) {
        emit error("Room not found:" + roomId);
        return false;
    }
    
    GameRoom& room = rooms_[roomId];
    
    room.state = RoomState::FINISHED;
    room.winner = winner;
    room.lastActivityTime = QDateTime::currentMSecsSinceEpoch();
    
    emit roomStateChanged(roomId, room.state);
    emit gameEnded(roomId, winner);
    
    qInfo() << "Game ended in room" << room.roomName << "winner:" << winner;
    
    return true;
}

bool RoomManager::updateRoomState(const QString& roomId, RoomState state)
{
    if (!rooms_.contains(roomId)) {
        return false;
    }
    
    GameRoom& room = rooms_[roomId];
    room.state = state;
    room.lastActivityTime = QDateTime::currentMSecsSinceEpoch();
    
    emit roomStateChanged(roomId, state);
    emit roomListChanged();
    
    return true;
}

bool RoomManager::updateRoomSettings(const QString& roomId, const QJsonObject& settings)
{
    if (!rooms_.contains(roomId)) {
        return false;
    }
    
    if (!validateSettings(settings)) {
        emit error("Invalid room settings");
        return false;
    }
    
    GameRoom& room = rooms_[roomId];
    room.settings = settings;
    room.lastActivityTime = QDateTime::currentMSecsSinceEpoch();
    
    notifyRoomUpdate(roomId);
    
    return true;
}

// ==================== Player Management ====================

bool RoomManager::addPlayer(const QString& roomId, const QString& playerName)
{
    if (!rooms_.contains(roomId)) {
        return false;
    }
    
    GameRoom& room = rooms_[roomId];
    
    if (!room.players.contains(playerName)) {
        room.players.append(playerName);
        room.lastActivityTime = QDateTime::currentMSecsSinceEpoch();
        
        emit playerJoined(roomId, playerName);
        emit roomListChanged();
    }
    
    return true;
}

bool RoomManager::removePlayer(const QString& roomId, const QString& playerName)
{
    if (!rooms_.contains(roomId)) {
        return false;
    }
    
    GameRoom& room = rooms_[roomId];
    
    if (room.players.removeOne(playerName)) {
        // If host leaves, abandon room
        if (playerName == room.hostName) {
            room.state = RoomState::ABANDONED;
            emit roomStateChanged(roomId, room.state);
        }
        
        room.lastActivityTime = QDateTime::currentMSecsSinceEpoch();
        
        emit playerLeft(roomId, playerName);
        emit roomListChanged();
        
        qInfo() << playerName << "left room" << room.roomName;
        
        return true;
    }
    
    return false;
}

QStringList RoomManager::getPlayers(const QString& roomId) const
{
    if (rooms_.contains(roomId)) {
        return rooms_[roomId].players;
    }
    return QStringList();
}

bool RoomManager::isPlayerInRoom(const QString& roomId, const QString& playerName) const
{
    if (rooms_.contains(roomId)) {
        return rooms_[roomId].players.contains(playerName);
    }
    return false;
}

// ==================== Private Methods ====================

QString RoomManager::generateRoomId()
{
    // Generate 6-character room ID
    const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QString id;
    QRandomGenerator* rng = QRandomGenerator::global();
    
    for (int i = 0; i < 6; ++i) {
        id += chars[rng->bounded(0, 36)];
    }
    
    // Ensure uniqueness
    while (rooms_.contains(id)) {
        for (int i = 0; i < 6; ++i) {
            id[i] = chars[rng->bounded(0, 36)];
        }
    }
    
    return id;
}

bool RoomManager::validateSettings(const QJsonObject& settings)
{
    // Basic validation
    if (settings.contains("timeLimit") && settings["timeLimit"].toInt() < 0) {
        return false;
    }
    
    if (settings.contains("maxPlayers") && settings["maxPlayers"].toInt() < 2) {
        return false;
    }
    
    if (settings.contains("maxPlayers") && settings["maxPlayers"].toInt() > 8) {
        return false;
    }
    
    return true;
}

bool RoomManager::isRoomFull(const GameRoom& room) const
{
    int maxPlayers = 2;
    
    if (room.settings.contains("maxPlayers")) {
        maxPlayers = room.settings["maxPlayers"].toInt(2);
    }
    
    return room.players.size() >= maxPlayers;
}

void RoomManager::notifyRoomUpdate(const QString& roomId)
{
    if (rooms_.contains(roomId)) {
        emit roomListChanged();
    }
}

void RoomManager::cleanupExpiredRooms()
{
    // Reference: Egaroucid ggs.hpp timeout handling
    
    uint64_t now = QDateTime::currentMSecsSinceEpoch();
    QList<QString> expired;
    
    for (const QString& roomId : rooms_.keys()) {
        const GameRoom& room = rooms_[roomId];
        
        // Remove abandoned rooms immediately
        if (room.state == RoomState::ABANDONED) {
            expired.append(roomId);
            continue;
        }
        
        // Remove rooms idle for too long
        if (room.state == RoomState::WAITING && 
            now - room.lastActivityTime > ROOM_TIMEOUT) {
            expired.append(roomId);
        }
    }
    
    for (const QString& roomId : expired) {
        deleteRoom(roomId);
    }
    
    if (!expired.isEmpty()) {
        qDebug() << "Cleaned up" << expired.size() << "expired rooms";
    }
}

} // namespace Network

