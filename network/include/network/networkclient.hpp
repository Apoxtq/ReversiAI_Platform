/*
    ReversiAI_Platform - Network Module

    @file networkclient.hpp
    @brief TCP network client for LAN multiplayer
    @date 2026
    @author Project Team
    @license GPL-3.0

    Reference: Egaroucid ggs.hpp (Takuto Yamana, 2021-2026)
    Uses Qt Network module instead of Winsock.
*/

#ifndef NETWORK_CLIENT_HPP
#define NETWORK_CLIENT_HPP

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>
#include <QTimer>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <queue>

#include "network/message.hpp"

namespace Network {

/**
 * @brief TCP Network Client for Reversi LAN multiplayer
 * 
 * Reference: Egaroucid ggs.hpp ggs_connect() and ggs_client() pattern
 * Uses Qt Signal/Slot instead of Winsock and std::future/async.
 */
class NetworkClient : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a network client
     * @param parent Parent QObject
     */
    explicit NetworkClient(QObject* parent = nullptr);
    
    /**
     * @brief Destructor - clean up connections
     */
    ~NetworkClient() override;

    // ==================== Connection Management ====================

    /**
     * @brief Connect to a remote host
     * @param address Host address
     * @param port Port number
     * @return true if connection initiated successfully
     * 
     * Reference: Egaroucid ggs.hpp ggs_connect() (line 139-190)
     */
    bool connectToHost(const QHostAddress& address, quint16 port);

    /**
     * @brief Disconnect from current host
     * 
     * Reference: Egaroucid ggs.hpp ggs_close() (line 192-195)
     */
    void disconnectFromHost();

    /**
     * @brief Get current connection state
     * @return Connection state
     */
    ConnectionState getState() const { return state_; }

    /**
     * @brief Check if connected
     * @return true if connected
     */
    bool isConnected() const { return state_ == ConnectionState::Connected; }

    /**
     * @brief Get peer address
     * @return Peer address, or invalid if not connected
     */
    QHostAddress getPeerAddress() const;

    /**
     * @brief Get peer port
     * @return Peer port, or 0 if not connected
     */
    quint16 getPeerPort() const;

    // ==================== Message Sending ====================

    /**
     * @brief Send a message
     * @param message Message to send
     * @return true if message queued successfully
     * 
     * Reference: Egaroucid ggs.hpp ggs_send_message() (line 197-203)
     */
    virtual bool sendMessage(const Message& message);

    /**
     * @brief Send a move message
     * @param row Row (0-7)
     * @param col Column (0-7)
     * @param player Player color
     * @param moveNumber Move number
     * @return true if message queued
     */
    bool sendMove(int row, int col, const QString& player, int moveNumber);

    /**
     * @brief Send game state
     * @param state Game state message
     * @return true if message queued
     */
    bool sendGameState(const GameStateMessage& state);

    /**
     * @brief Send ping for latency measurement
     */
    void sendPing();

    /**
     * @brief Send heartbeat
     * 
     * Reference: Egaroucid ggs.hpp line 554-556
     */
    void sendHeartbeat();

    // ==================== Message Queue ====================

    /**
     * @brief Get number of pending messages
     * @return Pending message count
     */
    int getPendingMessageCount() const { return sendQueue_.size(); }

    /**
     * @brief Check if send queue is empty
     * @return true if no pending sends
     */
    bool isSendQueueEmpty() const { return sendQueue_.empty(); }

signals:
    // Connection signals (reference: ggs.hpp connected state machine)
    /**
     * @brief Emitted when connected successfully
     */
    void connected();

    /**
     * @brief Emitted when disconnected
     */
    void disconnected();

    /**
     * @brief Emitted on connection error
     * @param error Error code
     * @param message Error description
     */
    void errorOccurred(NetworkError error, const QString& message);

    // Message signals
    /**
     * @brief Emitted when a complete message is received
     * @param message Received message
     */
    void messageReceived(const Message& message);

    /**
     * @brief Emitted when a move is received
     * @param row Row
     * @param col Column
     * @param player Player color
     */
    void moveReceived(int row, int col, const QString& player);

    /**
     * @brief Emitted when game state is received
     * @param state Game state
     */
    void gameStateReceived(const GameStateMessage& state);

    /**
     * @brief Emitted when ping response received
     * @param latency Latency in milliseconds
     */
    void pongReceived(qint64 latency);

    /**
     * @brief Emitted when heartbeat received
     */
    void heartbeatReceived();

    /**
     * @brief Emitted when a chat message is received
     * @param sender Sender name
     * @param message Message content
     */
    void chatMessageReceived(const QString& sender, const QString& message);

    /**
     * @brief Emitted when a player is ready
     * @param playerName Player name from payload
     * @param sender Sender name from message header
     * @param timestamp Message timestamp
     */
    void playerReadyReceived(const QString& playerName, const QString& sender, qint64 timestamp);

    // State signals
    /**
     * @brief Emitted when connection state changes
     * @param state New state
     */
    void stateChanged(ConnectionState state);

    /**
     * @brief Emitted when connection timeout occurs
     */
    void connectionTimeout();

private slots:
    // Socket event handlers (reference: ggs.hpp onConnected pattern)
    /**
     * @brief Handle socket connected
     */
    void onConnected();

    /**
     * @brief Handle socket disconnected
     */
    void onDisconnected();

    /**
     * @brief Handle socket error
     * @param socketError Socket error code
     * 
     * Reference: Egaroucid ggs.hpp error handling pattern
     */
    void onError(QAbstractSocket::SocketError socketError);

    /**
     * @brief Handle ready to read data
     * 
     * Reference: Egaroucid ggs.hpp ggs_receive_message() (line 205-217)
     */
    void onReadyRead();

    /**
     * @brief Process pending sends
     */
    void processSendQueue();

    /**
     * @brief Send queued messages if connected
     */
    void trySendQueued();

    /**
     * @brief Handle heartbeat timeout
     */
    void onHeartbeatTimeout();

protected:
    // ==================== Core Socket ====================
    QTcpSocket* socket_;              ///< TCP socket (replaces SOCKET)

    // ==================== State Management ====================
    ConnectionState state_;           ///< Current connection state
    QTimer* reconnectTimer_;        ///< Auto-reconnect timer

    // ==================== Message Handling ====================
    QByteArray receiveBuffer_;       ///< Receive buffer (reference: ggs.hpp server_reply[20000])
    uint32_t messageSize_;           ///< Expected message size (0 = header not read)
    static constexpr int MAX_MESSAGE_SIZE = 65536;  ///< Max message size

    // ==================== Send Queue ====================
    std::queue<Message> sendQueue_;   ///< Message send queue
    QTimer* sendTimer_;               ///< Batch send timer
    static constexpr int SEND_INTERVAL = 16;  ///< 16ms batch interval (~60fps)

    // ==================== Heartbeat ====================
    QTimer* heartbeatTimer_;          ///< Heartbeat timer
    QTimer* heartbeatTimeout_;        ///< Heartbeat timeout timer
    uint64_t lastHeartbeatTime_;      ///< Last heartbeat timestamp
    int missedHeartbeats_;            ///< Missed heartbeat count
    static constexpr int HEARTBEAT_INTERVAL = 30000;   ///< 30 seconds
    static constexpr int HEARTBEAT_TIMEOUT = 10000;    ///< 10 seconds timeout
    static constexpr int MAX_MISSED_HEARTBEATS = 3;    ///< Max missed

    // ==================== Statistics ====================
    uint64_t messagesSent_;           ///< Total messages sent
    uint64_t messagesReceived_;       ///< Total messages received
    uint64_t lastPingTime_;           ///< Last ping timestamp

    // ==================== Private Methods ====================

    /**
     * @brief Update connection state
     * @param newState New state
     * 
     * Reference: Egaroucid state machine pattern
     */
    void setState(ConnectionState newState);

    /**
     * @brief Handle received data
     * @param data Received data
     * 
     * Reference: Egaroucid ggs.hpp ggs_receive_message() pattern
     */
    void handleReceivedData(const QByteArray& data);

    /**
     * @brief Parse message from buffer
     * @return true if complete message parsed
     */
    bool parseMessage();

    /**
     * @brief Emit appropriate signal based on message type
     * @param message Message to dispatch
     */
protected:
    void dispatchMessage(const Message& message);

    /**
     * @brief Convert socket error to NetworkError
     * @param socketError Socket error
     * @return NetworkError
     * 
     * Reference: Egaroucid ggs.hpp Winsock error handling
     */
    NetworkError socketErrorToNetworkError(QAbstractSocket::SocketError socketError);

    /**
     * @brief Start heartbeat mechanism
     * 
     * Reference: Egaroucid ggs.hpp line 554-557
     */
    void startHeartbeat();

    /**
     * @brief Stop heartbeat mechanism
     */
    void stopHeartbeat();

    /**
     * @brief Send next queued message
     * @return true if message sent
     */
    bool sendQueuedMessage();

    /**
     * @brief Log send operation
     * @param message Message sent
     * 
     * Reference: Egaroucid ggs.hpp ggs_print_send()
     */
    void logSend(const Message& message);

    /**
     * @brief Log receive operation
     * @param message Message received
     * 
     * Reference: Egaroucid ggs.hpp ggs_print_receive()
     */
    void logReceive(const Message& message);
};

/**
 * @brief Network client for hosting games (P2P server mode)
 * 
 * Simplified server that accepts one client connection for direct P2P play.
 */
class NetworkHost : public NetworkClient
{
    Q_OBJECT

public:
    /**
     * @brief Construct a host
     * @param parent Parent QObject
     */
    explicit NetworkHost(QObject* parent = nullptr);

    /**
     * @brief Start hosting on a port
     * @param port Port to listen on (0 = auto-select)
     * @return true if server started
     */
    bool startHosting(quint16 port = 0);

    /**
     * @brief Stop hosting
     */
    void stopHosting();

    /**
     * @brief Get listening port
     * @return Port number, or 0 if not listening
     */
    quint16 getListeningPort() const { return listeningPort_; }

signals:
    /**
     * @brief Emitted when a client connects
     */
    void clientConnected(const QHostAddress& clientAddress, quint16 clientPort);

    /**
     * @brief Emitted when client disconnects
     */
    void clientDisconnected();

private slots:
    void onNewConnection();
    void onClientReadyRead();
    void onClientDisconnected();

private:
    void processReceiveBuffer();

public:
    /**
     * @brief Send message to client (overrides NetworkClient::sendMessage)
     * @param message Message to send
     * @return true if sent successfully
     */
    virtual bool sendMessage(const Message& message) override;

    /**
     * @brief Send move directly via client socket (host mode)
     * @param row Row (0-7)
     * @param col Column (0-7)
     * @param player Player color
     * @param moveNumber Move number
     * @return true if sent successfully
     */
    bool sendMove(int row, int col, const QString& player, int moveNumber);

private:
    QTcpServer* server_;
    quint16 listeningPort_;
    QTcpSocket* clientSocket_;
};

} // namespace Network

#endif // NETWORK_CLIENT_HPP
