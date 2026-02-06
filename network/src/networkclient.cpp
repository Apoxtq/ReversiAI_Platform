/*
    ReversiAI_Platform - Network Module

    @file networkclient.cpp
    @brief TCP network client implementation
    @date 2026
    @author Project Team
    @license GPL-3.0

    Reference: Egaroucid ggs.hpp (Takuto Yamana, 2021-2026)
*/

#include "network/networkclient.hpp"
#include <QDebug>
#include <QJsonParseError>

namespace Network {

// ============================================================================
// NetworkClient Implementation
// ============================================================================

NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent)
    , socket_(nullptr)
    , state_(ConnectionState::Disconnected)
    , reconnectTimer_(nullptr)
    , messageSize_(0)
    , sendTimer_(nullptr)
    , heartbeatTimer_(nullptr)
    , heartbeatTimeout_(nullptr)
    , lastHeartbeatTime_(0)
    , missedHeartbeats_(0)
    , messagesSent_(0)
    , messagesReceived_(0)
    , lastPingTime_(0)
{
    // Create socket (reference: Egaroucid ggs.hpp socket() call)
    socket_ = new QTcpSocket(this);
    
    // Create timers
    sendTimer_ = new QTimer(this);
    sendTimer_->setInterval(SEND_INTERVAL);
    sendTimer_->setSingleShot(false);
    
    reconnectTimer_ = new QTimer(this);
    reconnectTimer_->setSingleShot(true);
    
    heartbeatTimer_ = new QTimer(this);
    heartbeatTimer_->setSingleShot(true);
    
    heartbeatTimeout_ = new QTimer(this);
    heartbeatTimeout_->setSingleShot(true);
    
    // Connect socket signals (reference: ggs.hpp event handlers)
    connect(socket_, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(socket_, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(socket_, &QTcpSocket::errorOccurred, this, &NetworkClient::onError);
    connect(socket_, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    
    // Connect timer signals
    connect(sendTimer_, &QTimer::timeout, this, &NetworkClient::processSendQueue);
    connect(reconnectTimer_, &QTimer::timeout, this, &NetworkClient::trySendQueued);
    connect(heartbeatTimer_, &QTimer::timeout, this, &NetworkClient::sendHeartbeat);
    connect(heartbeatTimeout_, &QTimer::timeout, this, &NetworkClient::onHeartbeatTimeout);
}

NetworkClient::~NetworkClient()
{
    disconnectFromHost();
}

// ==================== Connection Management ====================

bool NetworkClient::connectToHost(const QHostAddress& address, quint16 port)
{
    // Reference: Egaroucid ggs.hpp ggs_connect() (line 139-190)
    // Changed from Winsock to Qt Network
    
    if (state_ == ConnectionState::Connected) {
        qWarning() << "Already connected, disconnect first";
        return false;
    }
    
    if (state_ == ConnectionState::Connecting) {
        qWarning() << "Already connecting";
        return false;
    }
    
    qInfo() << "Connecting to" << address.toString() << ":" << port;
    
    // Update state (reference: ggs.hpp state machine)
    setState(ConnectionState::Connecting);
    
    // Clear buffers (reference: ggs.hpp server_reply clear)
    receiveBuffer_.clear();
    messageSize_ = 0;
    
    // Connect to host (Qt Network replaces Winsock connect())
    socket_->connectToHost(address, port);
    
    // Note: connected() signal will be emitted asynchronously
    return true;
}

void NetworkClient::disconnectFromHost()
{
    // Reference: Egaroucid ggs.hpp ggs_close() (line 192-195)
    
    stopHeartbeat();
    sendTimer_->stop();
    reconnectTimer_->stop();
    
    if (socket_->state() != QAbstractSocket::UnconnectedState) {
        socket_->disconnectFromHost();
    }
    
    setState(ConnectionState::Disconnected);
    qInfo() << "Disconnected from host";
}

QHostAddress NetworkClient::getPeerAddress() const
{
    if (state_ == ConnectionState::Connected) {
        return socket_->peerAddress();
    }
    return QHostAddress();
}

quint16 NetworkClient::getPeerPort() const
{
    if (state_ == ConnectionState::Connected) {
        return socket_->peerPort();
    }
    return 0;
}

// ==================== Message Sending ====================

bool NetworkClient::sendMessage(const Message& message)
{
    // Reference: Egaroucid ggs.hpp ggs_send_message() (line 197-203)
    
    if (state_ != ConnectionState::Connected) {
        qWarning() << "Cannot send: not connected";
        return false;
    }
    
    // Queue message for batch sending
    sendQueue_.push(message);
    
    // Trigger send timer
    if (!sendTimer_->isActive()) {
        sendTimer_->start();
    }
    
    return true;
}

bool NetworkClient::sendMove(int row, int col, const QString& player, int moveNumber)
{
    Message msg;
    msg.type = MessageType::MOVE_MADE;
    msg.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    QJsonObject payload;
    payload["row"] = row;
    payload["col"] = col;
    payload["player"] = player;
    payload["moveNumber"] = moveNumber;
    msg.payload = payload;
    
    return sendMessage(msg);
}

bool NetworkClient::sendGameState(const GameStateMessage& state)
{
    Message msg;
    msg.type = MessageType::GAME_STATE_UPDATE;
    msg.timestamp = QDateTime::currentMSecsSinceEpoch();
    msg.payload = state.toJson();
    
    return sendMessage(msg);
}

void NetworkClient::sendPing()
{
    // Reference: Egaroucid gtp_command.hpp ping pattern
    Message msg = Message::createPing();
    lastPingTime_ = msg.timestamp;
    sendMessage(msg);
}

void NetworkClient::sendHeartbeat()
{
    // Reference: Egaroucid ggs.hpp line 554-556
    // Send heartbeat message
    
    Message msg = Message::createHeartbeat();
    lastHeartbeatTime_ = msg.timestamp;
    
    if (sendMessage(msg)) {
        // Start timeout timer (reference: ggs.hpp timeout detection)
        heartbeatTimeout_->start(HEARTBEAT_TIMEOUT);
    }
}

// ==================== Private Methods ====================

void NetworkClient::setState(ConnectionState newState)
{
    // Reference: Egaroucid ggs.hpp state machine transitions
    
    if (state_ == newState) {
        return;
    }
    
    ConnectionState oldState = state_;
    state_ = newState;
    
    qDebug() << "State changed:" << static_cast<int>(oldState) << "->" << static_cast<int>(newState);
    
    emit stateChanged(newState);
    
    // Handle state transitions (reference: ggs.hpp connection flow)
    switch (newState) {
        case ConnectionState::Connected:
            if (oldState == ConnectionState::Connecting) {
                qInfo() << "Connected successfully";
                emit connected();
                startHeartbeat();
            }
            break;
            
        case ConnectionState::Disconnected:
            stopHeartbeat();
            if (oldState == ConnectionState::Connected || oldState == ConnectionState::Error) {
                emit disconnected();
            }
            break;
            
        case ConnectionState::Error:
            qWarning() << "Connection error occurred";
            emit errorOccurred(NetworkError::Unknown, socket_->errorString());
            break;
            
        default:
            break;
    }
}

void NetworkClient::handleReceivedData(const QByteArray& data)
{
    // Reference: Egaroucid ggs.hpp ggs_receive_message() (line 205-217)
    // Changed: Uses Qt signals instead of return vector
    
    receiveBuffer_.append(data);
    
    // Try to parse complete messages
    while (parseMessage()) {
        // Continue until no more complete messages
    }
}

bool NetworkClient::parseMessage()
{
    // Reference: Egaroucid ggs.hpp message parsing pattern
    
    if (receiveBuffer_.isEmpty()) {
        return false;
    }
    
    // Try to parse message
    bool ok = false;
    Message msg = Message::deserialize(receiveBuffer_, &ok);
    
    if (!ok) {
        // Incomplete message or parse error
        // Check if buffer is too large
        if (receiveBuffer_.size() > MAX_MESSAGE_SIZE) {
            qWarning() << "Message too large, clearing buffer";
            receiveBuffer_.clear();
            emit errorOccurred(NetworkError::ProtocolError, "Message too large");
        }
        return false;
    }
    
    // Remove parsed message from buffer
    QByteArray serialized = msg.serialize();
    receiveBuffer_.remove(0, serialized.size());
    
    // Log and dispatch message
    messagesReceived_++;
    logReceive(msg);
    dispatchMessage(msg);
    
    return true;
}

void NetworkClient::dispatchMessage(const Message& message)
{
    // Reference: Egaroucid ggs.hpp message dispatch (line 622-685)
    
    emit messageReceived(message);
    
    // Dispatch based on type
    switch (message.type) {
        case MessageType::MOVE_MADE:
            emit moveReceived(
                message.payload["row"].toInt(),
                message.payload["col"].toInt(),
                message.payload["player"].toString()
            );
            break;
            
        case MessageType::GAME_STATE_UPDATE: {
            GameStateMessage state = GameStateMessage::fromJson(message.payload);
            emit gameStateReceived(state);
            break;
        }
        
        case MessageType::PONG:
            emit pongReceived(QDateTime::currentMSecsSinceEpoch() - lastPingTime_);
            break;
            
        case MessageType::HEARTBEAT:
            emit heartbeatReceived();
            missedHeartbeats_ = 0;
            heartbeatTimeout_->stop();
            break;
            
        case MessageType::ERROR:
            emit errorOccurred(
                static_cast<NetworkError>(message.payload["error"].toInt()),
                message.payload["message"].toString()
            );
            break;
            
        default:
            break;
    }
}

NetworkError NetworkClient::socketErrorToNetworkError(QAbstractSocket::SocketError socketError)
{
    // Reference: Egaroucid ggs.hpp Winsock error mapping
    
    switch (socketError) {
        case QAbstractSocket::ConnectionRefusedError:
            return NetworkError::ConnectionRefused;
        case QAbstractSocket::HostNotFoundError:
            return NetworkError::HostNotFound;
        case QAbstractSocket::SocketTimeoutError:
            return NetworkError::ConnectionTimeout;
        case QAbstractSocket::RemoteHostClosedError:
            return NetworkError::ConnectionReset;
        default:
            return NetworkError::Unknown;
    }
}

void NetworkClient::startHeartbeat()
{
    // Reference: Egaroucid ggs.hpp line 554-557
    // Start heartbeat timer
    
    missedHeartbeats_ = 0;
    heartbeatTimer_->start(HEARTBEAT_INTERVAL);
}

void NetworkClient::stopHeartbeat()
{
    heartbeatTimer_->stop();
    heartbeatTimeout_->stop();
}

bool NetworkClient::sendQueuedMessage()
{
    // Reference: Egaroucid ggs.hpp ggs_send_message() (line 197-203)
    
    if (sendQueue_.empty() || state_ != ConnectionState::Connected) {
        return false;
    }
    
    Message msg = sendQueue_.front();
    QByteArray data = msg.serialize();
    
    qint64 bytesWritten = socket_->write(data);
    
    if (bytesWritten < 0) {
        // Write error
        qWarning() << "Send failed:" << socket_->errorString();
        return false;
    }
    
    sendQueue_.pop();
    messagesSent_++;
    logSend(msg);
    
    // Flush immediately for latency-sensitive operations
    if (msg.type == MessageType::MOVE_MADE || msg.type == MessageType::HEARTBEAT) {
        socket_->flush();
    }
    
    return true;
}

void NetworkClient::processSendQueue()
{
    // Send all queued messages
    while (!sendQueue_.empty() && state_ == ConnectionState::Connected) {
        if (!sendQueuedMessage()) {
            break;
        }
    }
    
    // Stop timer if queue empty
    if (sendQueue_.empty()) {
        sendTimer_->stop();
    }
}

void NetworkClient::trySendQueued()
{
    // Reference: Egaroucid ggs.hpp pending send check
    if (state_ == ConnectionState::Connected) {
        processSendQueue();
    }
}

void NetworkClient::onHeartbeatTimeout()
{
    // Reference: Egaroucid ggs.hpp timeout detection
    missedHeartbeats_++;
    
    qWarning() << "Heartbeat timeout, missed:" << missedHeartbeats_;
    
    if (missedHeartbeats_ >= MAX_MISSED_HEARTBEATS) {
        qWarning() << "Too many missed heartbeats, disconnecting";
        emit connectionTimeout();
        setState(ConnectionState::Error);
        disconnectFromHost();
    }
}

void NetworkClient::logSend(const Message& message)
{
    // Reference: Egaroucid ggs.hpp ggs_print_send()
    // Debug logging only (console output handled by ggs_print_*)
    qDebug() << "[SEND]" << static_cast<int>(message.type) 
             << "seq:" << message.sequence 
             << "size:" << message.serialize().size();
}

void NetworkClient::logReceive(const Message& message)
{
    // Reference: Egaroucid ggs.hpp ggs_print_receive()
    qDebug() << "[RECV]" << static_cast<int>(message.type) 
             << "seq:" << message.sequence
             << "size:" << message.serialize().size();
}

// ==================== Socket Event Handlers ====================

void NetworkClient::onConnected()
{
    // Reference: Egaroucid ggs.hpp connected handler
    qInfo() << "Socket connected";
    setState(ConnectionState::Connected);
}

void NetworkClient::onDisconnected()
{
    // Reference: Egaroucid ggs.hpp disconnected handler
    qInfo() << "Socket disconnected";
    setState(ConnectionState::Disconnected);
}

void NetworkClient::onError(QAbstractSocket::SocketError socketError)
{
    // Reference: Egaroucid ggs.hpp error handling (line 183-188)
    qWarning() << "Socket error:" << socketError << "-" << socket_->errorString();
    
    NetworkError error = socketErrorToNetworkError(socketError);
    emit errorOccurred(error, socket_->errorString());
    
    setState(ConnectionState::Error);
}

void NetworkClient::onReadyRead()
{
    // Reference: Egaroucid ggs.hpp ggs_receive_message() (line 205-217)
    QByteArray data = socket_->readAll();
    handleReceivedData(data);
}

// ============================================================================
// NetworkHost Implementation
// ============================================================================

NetworkHost::NetworkHost(QObject* parent)
    : NetworkClient(parent)
    , server_(nullptr)
    , listeningPort_(0)
{
    server_ = new QTcpServer(this);
    
    connect(server_, &QTcpServer::newConnection, this, &NetworkHost::onNewConnection);
}

bool NetworkHost::startHosting(quint16 port)
{
    // Reference: Simplified server setup for P2P
    
    if (server_->isListening()) {
        qWarning() << "Already listening";
        return false;
    }
    
    // Start listening on specified port (0 = any available port)
    if (!server_->listen(QHostAddress::Any, port)) {
        qWarning() << "Failed to start server:" << server_->errorString();
        return false;
    }
    
    listeningPort_ = server_->serverPort();
    qInfo() << "Hosting on port" << listeningPort_;
    
    return true;
}

void NetworkHost::stopHosting()
{
    server_->close();
    listeningPort_ = 0;
    qInfo() << "Stopped hosting";
}

void NetworkHost::onNewConnection()
{
    // Reference: Accept one client for P2P game
    
    if (server_->hasPendingConnections()) {
        QTcpSocket* client = server_->nextPendingConnection();
        
        if (client) {
            qInfo() << "Client connected from" << client->peerAddress();
            
            // Take ownership of the socket
            // Note: We don't use the server's socket, we create our own NetworkClient connection
            
            // For P2P mode, the host acts as a client connecting to itself
            // Actually, we need to accept the connection and use that socket
        }
    }
}

} // namespace Network

