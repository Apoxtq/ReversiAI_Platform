/*
    ReversiAI_Platform - Network Module

    @file roommanager.hpp
    @brief Room management for multiplayer games
    @date 2026
    @author Project Team
    @license GPL-3.0

    Reference: Egaroucid ggs.hpp room management (line 539-551)
*/

#ifndef ROOM_MANAGER_HPP
#define ROOM_MANAGER_HPP

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QTimer>
#include <QDateTime>
#include "network/message.hpp"

namespace Network {

/**
 * @brief Room Manager
 * 
 * Manages game rooms for LAN multiplayer.
 * 
 * Reference: Egaroucid ggs.hpp room handling (line 539-551)
 */
class RoomManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a room manager
     * @param parent Parent QObject
     */
    explicit RoomManager(QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~RoomManager() override;

    // ==================== Room Creation ====================

    /**
     * @brief Create a new room
     * @param roomName Room name
     * @param hostName Host player name
     * @param settings Room settings (optional)
     * @return Created room
     * 
     * Reference: Egaroucid ggs.hpp room creation pattern
     */
    GameRoom createRoom(const QString& roomName, const QString& hostName, 
                        const QJsonObject& settings = QJsonObject());

    /**
     * @brief Create room with custom settings
     * @param roomName Room name
     * @param hostName Host player name
     * @param timeLimit Time limit in seconds (0 = unlimited)
     * @param isRanked Whether room is ranked
     * @return Created room
     */
    GameRoom createRoom(const QString& roomName, const QString& hostName,
                        int timeLimit, bool isRanked);

    /**
     * @brief Delete a room
     * @param roomId Room ID
     * @return true if room was deleted
     */
    bool deleteRoom(const QString& roomId);

    // ==================== Room Joining ====================

    /**
     * @brief Join a room
     * @param roomId Room ID
     * @param playerName Player name
     * @param playerColor Preferred color (optional)
     * @return true if joined successfully
     * 
     * Reference: Egaroucid ggs.hpp room joining pattern
     */
    bool joinRoom(const QString& roomId, const QString& playerName, 
                  const QString& playerColor = "");

    /**
     * @brief Leave current room
     * @param playerName Player name
     * @return true if left successfully
     */
    bool leaveRoom(const QString& playerName);

    /**
     * @brief Get room by ID
     * @param roomId Room ID
     * @return Room pointer or nullptr
     */
    GameRoom* getRoom(const QString& roomId);

    // ==================== Room Query ====================

    /**
     * @brief Get all available rooms
     * @return List of joinable rooms
     * 
     * Reference: Egaroucid ggs.hpp room listing (line 539-551)
     */
    QList<GameRoom> getAvailableRooms() const;

    /**
     * @brief Get room count
     * @return Number of rooms
     */
    int getRoomCount() const { return rooms_.size(); }

    /**
     * @brief Get player count
     * @return Total players in all rooms
     */
    int getPlayerCount() const;

    /**
     * @brief Check if room exists
     * @param roomId Room ID
     * @return true if exists
     */
    bool roomExists(const QString& roomId) const;

    // ==================== Room State ====================

    /**
     * @brief Start game in room
     * @param roomId Room ID
     * @return true if started
     */
    bool startGame(const QString& roomId);

    /**
     * @brief End game in room
     * @param roomId Room ID
     * @param winner Winner player name (optional)
     * @param reason End reason
     * @return true if ended
     */
    bool endGame(const QString& roomId, const QString& winner = "", 
                 const QString& reason = "normal");

    /**
     * @brief Update room state
     * @param roomId Room ID
     * @param state New state
     * @return true if updated
     */
    bool updateRoomState(const QString& roomId, RoomState state);

    /**
     * @brief Update room settings
     * @param roomId Room ID
     * @param settings New settings
     * @return true if updated
     */
    bool updateRoomSettings(const QString& roomId, const QJsonObject& settings);

    // ==================== Player Management ====================

    /**
     * @brief Add player to room
     * @param roomId Room ID
     * @param player Player to add
     * @return true if added
     */
    bool addPlayer(const QString& roomId, const QString& playerName);

    /**
     * @brief Remove player from room
     * @param roomId Room ID
     * @param playerName Player to remove
     * @return true if removed
     */
    bool removePlayer(const QString& roomId, const QString& playerName);

    /**
     * @brief Get players in room
     * @param roomId Room ID
     * @return List of player names
     */
    QStringList getPlayers(const QString& roomId) const;

    /**
     * @brief Check if player is in room
     * @param roomId Room ID
     * @param playerName Player name
     * @return true if in room
     */
    bool isPlayerInRoom(const QString& roomId, const QString& playerName) const;

signals:
    /**
     * @brief Emitted when room is created
     * @param room Created room
     */
    void roomCreated(const GameRoom& room);

    /**
     * @brief Emitted when room is deleted
     * @param roomId Deleted room ID
     */
    void roomDeleted(const QString& roomId);

    /**
     * @brief Emitted when player joins room
     * @param roomId Room ID
     * @param playerName Player name
     */
    void playerJoined(const QString& roomId, const QString& playerName);

    /**
     * @brief Emitted when player leaves room
     * @param roomId Room ID
     * @param playerName Player name
     */
    void playerLeft(const QString& roomId, const QString& playerName);

    /**
     * @brief Emitted when room state changes
     * @param roomId Room ID
     * @param state New state
     */
    void roomStateChanged(const QString& roomId, RoomState state);

    /**
     * @brief Emitted when game starts
     * @param roomId Room ID
     */
    void gameStarted(const QString& roomId);

    /**
     * @brief Emitted when game ends
     * @param roomId Room ID
     * @param winner Winner player name
     */
    void gameEnded(const QString& roomId, const QString& winner);

    /**
     * @brief Emitted when room list changes
     */
    void roomListChanged();

    /**
     * @brief Emitted on error
     * @param error Error message
     */
    void error(const QString& error);

private slots:
    /**
     * @brief Clean up expired rooms
     */
    void cleanupExpiredRooms();

private:
    // ==================== Data ====================
    QMap<QString, GameRoom> rooms_;     ///< Room storage by ID
    
    // ==================== Timers ====================
    QTimer* cleanupTimer_;             ///< Room cleanup timer
    
    // ==================== Constants ====================
    static constexpr int CLEANUP_INTERVAL = 60000;  ///< 1 minute
    static constexpr int ROOM_TIMEOUT = 300000;      ///< 5 minutes idle timeout
    
    // ==================== Private Methods ====================
    
    /**
     * @brief Generate unique room ID
     * @return New room ID
     */
    QString generateRoomId();

    /**
     * @brief Validate room settings
     * @param settings Settings to validate
     * @return true if valid
     */
    bool validateSettings(const QJsonObject& settings);

    /**
     * @brief Check if room is full
     * @param room Room to check
     * @return true if full
     */
    bool isRoomFull(const GameRoom& room) const;

    /**
     * @brief Notify room update
     * @param roomId Room ID
     */
    void notifyRoomUpdate(const QString& roomId);
};

/**
 * @brief Room settings helper
 */
struct RoomSettings {
    int timeLimit;          ///< Time limit in seconds (0 = unlimited)
    bool isRanked;          ///< Whether game is ranked
    bool allowSpectators;   ///< Whether spectators are allowed
    int maxPlayers;         ///< Maximum players (2 = PvP)
    QString gameType;       ///< Game type ("pvp", "pve", "ai_battle")
    int aiDifficulty;      ///< AI difficulty (if PvE)
    
    // Default constructor
    RoomSettings() : timeLimit(0), isRanked(false), allowSpectators(true),
                     maxPlayers(2), gameType("pvp"), aiDifficulty(5) {}
    
    // To JSON
    QJsonObject toJson() const;
    
    // From JSON
    static RoomSettings fromJson(const QJsonObject& json);
};

} // namespace Network

#endif // ROOM_MANAGER_HPP

