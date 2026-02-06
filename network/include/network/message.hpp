/*
    ReversiAI_Platform - Network Module

    @file message.hpp
    @brief Network message types and serialization
    @date 2026
    @author Project Team
    @license GPL-3.0

    Message types and serialization system for LAN multiplayer.
    Based on Egaroucid GGS protocol design.
*/

#ifndef NETWORK_MESSAGE_HPP
#define NETWORK_MESSAGE_HPP

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QVector>
#include <QHostAddress>
#include <QRandomGenerator>
#include <cstdint>

namespace Network {

/**
 * @brief Message types for network communication
 * 
 * Reference: Egaroucid ggs.hpp MessageType design
 */
enum class MessageType {
    // Connection Management (P0)
    HANDSHAKE,           ///< Initial connection handshake
    HANDSHAKE_RESPONSE,  ///< Handshake response
    HEARTBEAT,           ///< Keep-alive heartbeat
    DISCONNECT,          ///< Graceful disconnect
    PING,                ///< Latency measurement ping
    PONG,                ///< Latency measurement pong

    // Game State (P0)
    GAME_START,          ///< Game session started
    GAME_END,            ///< Game session ended
    MOVE_MADE,           ///< Player made a move
    GAME_STATE_UPDATE,   ///< Full board state sync

    // Room System (P2)
    CREATE_ROOM,         ///< Create a game room
    JOIN_ROOM,           ///< Join an existing room
    LEAVE_ROOM,          ///< Leave current room
    ROOM_LIST,           ///< Room list request/response
    ROOM_UPDATE,         ///< Room state update

    // Game Settings
    GAME_SETTINGS,       ///< Game configuration
    SYNC_REQUEST,        ///< Request full state sync
    SYNC_RESPONSE,       ///< Response to sync request

    // Chat (P3)
    CHAT_MESSAGE,        ///< Player chat message

    // Error/ACK
    ERROR,               ///< Error message
    ACKNOWLEDGMENT       ///< Message acknowledgment
};

/**
 * @brief Connection state for NetworkClient
 * 
 * Reference: Egaroucid ggs.hpp connection state machine
 */
enum class ConnectionState {
    Disconnected,    ///< Not connected
    Connecting,     ///< Connection in progress
    Connected,      ///< Fully connected
    Disconnecting,  ///< Disconnection in progress
    Error           ///< Error state
};

/**
 * @brief Room state for RoomManager
 * 
 * Reference: Egaroucid ggs.hpp match state management
 */
enum class RoomState {
    WAITING,      ///< Waiting for opponent
    READY,        ///< Both players ready
    PLAYING,      ///< Game in progress
    FINISHED,     ///< Game finished
    ABANDONED     ///< Room abandoned
};

/**
 * @brief Network error types
 * 
 * Reference: Egaroucid ggs.hpp error handling
 */
enum class NetworkError {
    ConnectionRefused,    ///< Connection refused
    HostNotFound,         ///< Cannot resolve host
    ConnectionTimeout,    ///< Connection timeout
    ConnectionReset,      ///< Connection reset by peer
    SendFailed,           ///< Send operation failed
    ReceiveFailed,        ///< Receive operation failed
    ProtocolError,        ///< Protocol violation
    VersionMismatch,      ///< Version incompatibility
    RoomFull,             ///< Room is full
    RoomNotFound,         ///< Room does not exist
    Timeout,              ///< Operation timeout
    Unknown               ///< Unknown error
};

/**
 * @brief Core network message structure
 * 
 * Reference: Egaroucid ggs.hpp GGS_Board serialization
 * Uses Qt JSON API for serialization.
 */
struct Message {
    MessageType type;           ///< Message type
    QJsonObject payload;       ///< Message payload data
    uint32_t sequence;         ///< Sequence number for ordering
    uint64_t timestamp;        ///< Timestamp in milliseconds
    QString sender;            ///< Message sender ID
    QString receiver;          ///< Message receiver ID

    /**
     * @brief Default constructor
     */
    Message() 
        : type(MessageType::HANDSHAKE)
        , sequence(0)
        , timestamp(0)
        , sender()
        , receiver()
    {}

    /**
     * @brief Serialize message to JSON bytes
     * @return Compact JSON byte array
     * 
     * Reference: Egaroucid ggs.hpp board.to_str() pattern
     */
    QByteArray serialize() const {
        QJsonObject msg;
        msg["type"] = static_cast<int>(type);
        msg["sequence"] = static_cast<int>(sequence);
        msg["timestamp"] = static_cast<qint64>(timestamp);
        msg["sender"] = sender;
        msg["receiver"] = receiver;
        msg["payload"] = payload;
        
        QJsonDocument doc(msg);
        return doc.toJson(QJsonDocument::Compact);
    }

    /**
     * @brief Deserialize message from JSON bytes
     * @param data JSON byte array
     * @param ok Output: true if deserialization successful
     * @return Parsed message
     */
    static Message deserialize(const QByteArray& data, bool* ok = nullptr) {
        Message msg;
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error != QJsonParseError::NoError) {
            if (ok) *ok = false;
            return msg;
        }
        
        QJsonObject obj = doc.object();
        msg.type = static_cast<MessageType>(obj["type"].toInt(0));
        msg.sequence = obj["sequence"].toInt(0);
        msg.timestamp = obj["timestamp"].toVariant().toULongLong();
        msg.sender = obj["sender"].toString();
        msg.receiver = obj["receiver"].toString();
        msg.payload = obj["payload"].toObject();
        
        if (ok) *ok = true;
        return msg;
    }

    /**
     * @brief Create a heartbeat message
     * @return Heartbeat message
     * 
     * Reference: Egaroucid ggs.hpp GGS_SEND_EMPTY_INTERVAL pattern
     */
    static Message createHeartbeat() {
        Message msg;
        msg.type = MessageType::HEARTBEAT;
        msg.timestamp = QDateTime::currentMSecsSinceEpoch();
        return msg;
    }

    /**
     * @brief Create a ping message for latency measurement
     * @return Ping message
     */
    static Message createPing() {
        Message msg;
        msg.type = MessageType::PING;
        msg.timestamp = QDateTime::currentMSecsSinceEpoch();
        return msg;
    }

    /**
     * @brief Create a pong response
     * @param pingTimestamp Original ping timestamp
     * @return Pong message
     */
    static Message createPong(uint64_t pingTimestamp) {
        Message msg;
        msg.type = MessageType::PONG;
        msg.timestamp = pingTimestamp;
        return msg;
    }
};

/**
 * @brief Move message for game moves
 * 
 * Reference: Egaroucid gtp_command.hpp gtp_play() pattern
 */
struct MoveMessage {
    int row;                ///< Row (0-7)
    int col;                ///< Column (0-7)
    QString player;         ///< Player color ("black" or "white")
    int moveNumber;         ///< Move number in game

    MoveMessage() : row(-1), col(-1), player(""), moveNumber(0) {}
    MoveMessage(int r, int c, const QString& p, int num) 
        : row(r), col(c), player(p), moveNumber(num) {}

    /**
     * @brief Convert to JSON
     * @return JSON object
     */
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["row"] = row;
        obj["col"] = col;
        obj["player"] = player;
        obj["moveNumber"] = moveNumber;
        return obj;
    }

    /**
     * @brief Create from JSON
     * @param json JSON object
     * @return MoveMessage
     */
    static MoveMessage fromJson(const QJsonObject& json) {
        return MoveMessage(
            json["row"].toInt(-1),
            json["col"].toInt(-1),
            json["player"].toString(),
            json["moveNumber"].toInt(0)
        );
    }
};

/**
 * @brief Game state message for board synchronization
 * 
 * Reference: Egaroucid ggs.hpp GGS_Board structure
 */
struct GameStateMessage {
    QVector<QVector<int>> board;      ///< 8x8 board (0=empty, 1=black, 2=white)
    QString currentPlayer;             ///< Current player to move
    int blackCount;                   ///< Black disc count
    int whiteCount;                   ///< White disc count
    int moveNumber;                   ///< Total moves made
    QString lastMove;                ///< Last move coordinate (e.g., "D5")
    bool gameOver;                    ///< Is game over
    QString winner;                   ///< Winner if game over

    GameStateMessage() 
        : currentPlayer("")
        , blackCount(0)
        , whiteCount(0)
        , moveNumber(0)
        , lastMove("")
        , gameOver(false)
        , winner("")
    {}

    /**
     * @brief Convert to JSON
     * @return JSON object
     */
    QJsonObject toJson() const {
        QJsonObject obj;
        
        // Serialize board as flat array for efficiency
        QJsonArray boardArray;
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                boardArray.append(board[i][j]);
            }
        }
        obj["board"] = boardArray;
        
        obj["currentPlayer"] = currentPlayer;
        obj["blackCount"] = blackCount;
        obj["whiteCount"] = whiteCount;
        obj["moveNumber"] = moveNumber;
        obj["lastMove"] = lastMove;
        obj["gameOver"] = gameOver;
        obj["winner"] = winner;
        
        return obj;
    }

    /**
     * @brief Create from JSON
     * @param json JSON object
     * @return GameStateMessage
     */
    static GameStateMessage fromJson(const QJsonObject& json) {
        GameStateMessage state;
        
        QJsonArray boardArray = json["board"].toArray();
        state.board.resize(8);
        for (int i = 0; i < 8; ++i) {
            state.board[i].resize(8);
            for (int j = 0; j < 8; ++j) {
                state.board[i][j] = boardArray[i * 8 + j].toInt(0);
            }
        }
        
        state.currentPlayer = json["currentPlayer"].toString();
        state.blackCount = json["blackCount"].toInt(0);
        state.whiteCount = json["whiteCount"].toInt(0);
        state.moveNumber = json["moveNumber"].toInt(0);
        state.lastMove = json["lastMove"].toString();
        state.gameOver = json["gameOver"].toBool(false);
        state.winner = json["winner"].toString();
        
        return state;
    }

    /**
     * @brief Calculate board hash for consistency verification
     * @return Hash value
     * 
     * Reference: Egaroucid engine/hash.hpp
     */
    uint64_t calculateHash() const {
        uint64_t hash = 1469598103934665603ULL; // FNV offset basis
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                hash ^= static_cast<uint64_t>(board[i][j]);
                hash *= 1099511628211ULL; // FNV prime
            }
        }
        return hash;
    }
};

/**
 * @brief Room information structure
 * 
 * Reference: Egaroucid ggs.hpp GGS_Match structure
 */
struct GameRoom {
    QString roomId;            ///< Unique room identifier
    QString roomName;          ///< Room display name
    QString hostName;         ///< Host player name
    QString guestName;        ///< Guest player name (empty if waiting)
    RoomState state;          ///< Room state
    QString gameVersion;      ///< Protocol version
    uint64_t createTime;      ///< Creation timestamp
    uint64_t lastActivityTime; ///< Last activity timestamp
    
    // Players list
    QStringList players;       ///< List of players in room
    
    // Host information (for joining)
    QHostAddress hostAddress;  ///< Host address for joining
    quint16 hostPort;          ///< Host port for joining
    
    // Game settings
    QJsonObject settings;      ///< Room settings
    QString winner;           ///< Winner when game ends
    
    // Game info
    int timeLimit;            ///< Time limit in seconds (0 = unlimited)
    bool allowUndo;           ///< Allow undo moves
    QString firstPlayer;       ///< First player to move
    QString aiDifficulty;     ///< AI difficulty for PvE (empty for PvP)

    GameRoom() 
        : roomId("")
        , roomName("")
        , hostName("")
        , guestName("")
        , state(RoomState::WAITING)
        , gameVersion("0.5.0")
        , createTime(0)
        , lastActivityTime(0)
        , players()
        , hostAddress()
        , hostPort(0)
        , settings()
        , winner("")
        , timeLimit(0)
        , allowUndo(false)
        , firstPlayer("black")
        , aiDifficulty("")
    {}

    /**
     * @brief Convert to JSON
     * @return JSON object
     */
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["roomId"] = roomId;
        obj["roomName"] = roomName;
        obj["hostName"] = hostName;
        obj["guestName"] = guestName;
        obj["state"] = static_cast<int>(state);
        obj["gameVersion"] = gameVersion;
        obj["createTime"] = static_cast<qint64>(createTime);
        obj["lastActivityTime"] = static_cast<qint64>(lastActivityTime);
        
        // Players array
        QJsonArray playersArray;
        for (const QString& player : players) {
            playersArray.append(player);
        }
        obj["players"] = playersArray;
        
        // Host info
        obj["hostAddress"] = hostAddress.toString();
        obj["hostPort"] = hostPort;
        
        obj["settings"] = settings;
        obj["winner"] = winner;
        obj["timeLimit"] = timeLimit;
        obj["allowUndo"] = allowUndo;
        obj["firstPlayer"] = firstPlayer;
        obj["aiDifficulty"] = aiDifficulty;
        return obj;
    }

    /**
     * @brief Create from JSON
     * @param json JSON object
     * @return GameRoom
     */
    static GameRoom fromJson(const QJsonObject& json) {
        GameRoom room;
        room.roomId = json["roomId"].toString();
        room.roomName = json["roomName"].toString();
        room.hostName = json["hostName"].toString();
        room.guestName = json["guestName"].toString();
        room.state = static_cast<RoomState>(json["state"].toInt(0));
        room.gameVersion = json["gameVersion"].toString();
        room.createTime = json["createTime"].toVariant().toULongLong();
        room.lastActivityTime = json["lastActivityTime"].toVariant().toULongLong();
        
        // Players array
        QJsonArray playersArray = json["players"].toArray();
        for (const QJsonValue& player : playersArray) {
            room.players.append(player.toString());
        }
        
        // Host info
        room.hostAddress = QHostAddress(json["hostAddress"].toString());
        room.hostPort = static_cast<quint16>(json["hostPort"].toInt(0));
        
        room.settings = json["settings"].toObject();
        room.winner = json["winner"].toString();
        room.timeLimit = json["timeLimit"].toInt(0);
        room.allowUndo = json["allowUndo"].toBool(false);
        room.firstPlayer = json["firstPlayer"].toString();
        room.aiDifficulty = json["aiDifficulty"].toString();
        return room;
    }

    /**
     * @brief Check if room is joinable
     * @return true if room can be joined
     */
    bool isJoinable() const {
        return state == RoomState::WAITING && guestName.isEmpty();
    }

    /**
     * @brief Check if game is in progress
     * @return true if game is active
     */
    bool isPlaying() const {
        return state == RoomState::PLAYING;
    }
};

/**
 * @brief Discovered host information for LAN discovery
 * 
 * Reference: Custom UDP broadcast design
 */
struct DiscoveredHost {
    QString playerName;        ///< Player name
    QString roomName;          ///< Room name
    QHostAddress address;     ///< Host IP address
    quint16 port;             ///< Connection port
    QString gameVersion;      ///< Game version
    uint64_t discoveredTime;  ///< Discovery timestamp

    DiscoveredHost() 
        : playerName("")
        , roomName("")
        , address()
        , port(0)
        , gameVersion("")
        , discoveredTime(0)
    {}

    /**
     * @brief Convert to JSON for broadcast
     * @return JSON object
     */
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["type"] = "HELLO";
        obj["playerName"] = playerName;
        obj["roomName"] = roomName;
        obj["address"] = address.toString();
        obj["port"] = port;
        obj["gameVersion"] = gameVersion;
        obj["timestamp"] = static_cast<qint64>(QDateTime::currentMSecsSinceEpoch());
        return obj;
    }

    /**
     * @brief Create from received broadcast
     * @param json JSON object
     * @param sender Sender address
     * @return DiscoveredHost
     */
    static DiscoveredHost fromJson(const QJsonObject& json, const QHostAddress& sender) {
        DiscoveredHost host;
        host.playerName = json["playerName"].toString();
        host.roomName = json["roomName"].toString();
        host.address = sender;
        host.port = static_cast<quint16>(json["port"].toInt(0));
        host.gameVersion = json["gameVersion"].toString();
        host.discoveredTime = json["timestamp"].toVariant().toULongLong();
        return host;
    }

    /**
     * @brief Equality operator for QList operations
     */
    bool operator==(const DiscoveredHost& other) const {
        return address == other.address && port == other.port;
    }
};

/**
 * @brief Chat message structure
 * 
 * Reference: Egaroucid ggs.hpp server_replies management
 */
struct ChatMessage {
    QString sender;           ///< Message sender
    QString content;          ///< Message content
    uint64_t timestamp;      ///< Send timestamp
    QString type;            ///< Message type: "normal", "system", "whisper"

    ChatMessage() 
        : sender("")
        , content("")
        , timestamp(0)
        , type("normal")
    {}

    ChatMessage(const QString& s, const QString& c, const QString& t = "normal") 
        : sender(s), content(c), timestamp(QDateTime::currentMSecsSinceEpoch()), type(t) {}

    /**
     * @brief Convert to JSON
     * @return JSON object
     */
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["sender"] = sender;
        obj["content"] = content;
        obj["timestamp"] = static_cast<qint64>(timestamp);
        obj["type"] = type;
        return obj;
    }

    /**
     * @brief Create from JSON
     * @param json JSON object
     * @return ChatMessage
     */
    static ChatMessage fromJson(const QJsonObject& json) {
        ChatMessage msg;
        msg.sender = json["sender"].toString();
        msg.content = json["content"].toString();
        msg.timestamp = json["timestamp"].toVariant().toULongLong();
        msg.type = json["type"].toString("normal");
        return msg;
    }
};

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * @brief Convert board coordinates to GTP notation
 * @param row Row (0-7)
 * @param col Column (0-7)
 * @return GTP coordinate (e.g., "D5")
 * 
 * Reference: Egaroucid gtp_command.hpp gtp_idx_to_coord()
 */
QString coordToGTP(int row, int col);

/**
 * @brief Convert GTP notation to board coordinates
 * @param coord GTP coordinate (e.g., "D5")
 * @param row Output row
 * @param col Output column
 * @return true if conversion successful
 * 
 * Reference: Egaroucid gtp_command.hpp
 */
bool gtpToCoord(const QString& coord, int& row, int& col);

/**
 * @brief Parse GTP move command
 * @param command GTP move command (e.g., "play black D5" or "white E6")
 * @param player Output player color
 * @param row Output row
 * @param col Output column
 * @return true if parsing successful
 * 
 * Reference: Egaroucid gtp_command.hpp
 */
bool parseGTPMove(const QString& command, QString& player, int& row, int& col);

/**
 * @brief Get current timestamp in milliseconds
 * @return Current time as milliseconds since epoch
 */
inline uint64_t currentTimestamp() {
    return QDateTime::currentMSecsSinceEpoch();
}

} // namespace Network

#endif // NETWORK_MESSAGE_HPP

