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
    qDebug() << "NetworkClient::sendMessage called, type:" << static_cast<int>(message.type);
    
    if (state_ != ConnectionState::Connected) {
        qWarning() << "Cannot send: not connected, state:" << static_cast<int>(state_);
        return false;
    }
    
    qDebug() << "NetworkClient::sendMessage: Connected, queuing message";
    // Queue message for batch sending
    sendQueue_.push(message);
    qDebug() << "NetworkClient::sendMessage: Queue now has" << sendQueue_.size() << "messages";
    
    // Trigger send timer
    if (!sendTimer_->isActive()) {
        qDebug() << "NetworkClient::sendMessage: Starting send timer";
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
    
    // Try to parse complete messages (now handles newline-delimited format)
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
    
    // Find newline to separate messages
    int newlineIdx = receiveBuffer_.indexOf('\n');
    if (newlineIdx < 0) {
        // No complete message yet
        return false;
    }
    
    // Extract line up to and including newline
    QByteArray line = receiveBuffer_.left(newlineIdx);
    receiveBuffer_.remove(0, newlineIdx + 1);
    
    // Try to parse message
    bool ok = false;
    Message msg = Message::deserialize(line, &ok);
    
    if (!ok) {
        qWarning() << "Failed to deserialize message:" << QString::fromUtf8(line.left(100));
        return true; // Continue trying to parse next message
    }
    
    // Log and dispatch message
    messagesReceived_++;
    logReceive(msg);
    dispatchMessage(msg);
    
    return true;
}

void NetworkClient::dispatchMessage(const Message& message)
{
    // Reference: Egaroucid ggs.hpp message dispatch (line 622-685)
    
    qDebug() << "NetworkClient::dispatchMessage called, type:" << static_cast<int>(message.type);
    emit messageReceived(message);
    
    // Dispatch based on type
    switch (message.type) {
        case MessageType::MOVE_MADE: {
            qDebug() << "NetworkClient: Dispatching MOVE_MADE";
            int row = message.payload["row"].toInt(-1);
            int col = message.payload["col"].toInt(-1);
            QString player = message.payload["player"].toString();
            qDebug() << "NetworkClient: MOVE_MADE row=" << row << "col=" << col << "player=" << player;
            emit moveReceived(row, col, player);
            break;
        }
            
        case MessageType::GAME_STATE_UPDATE: {
            qDebug() << "NetworkClient: Dispatching GAME_STATE_UPDATE";
            GameStateMessage state = GameStateMessage::fromJson(message.payload);
            emit gameStateReceived(state);
            break;
        }
        
        case MessageType::PLAYER_READY: {
            QString playerName = message.payload["player"].toString();
            QString sender = message.sender;
            qint64 timestamp = message.timestamp;
            qDebug() << "NetworkClient: Dispatching PLAYER_READY from" << playerName << "sender:" << sender << "timestamp:" << timestamp;
            emit playerReadyReceived(playerName, sender, timestamp);
            break;
        }
        
        case MessageType::PONG:
            qDebug() << "NetworkClient: Dispatching PONG";
            emit pongReceived(QDateTime::currentMSecsSinceEpoch() - lastPingTime_);
            break;
            
        case MessageType::HEARTBEAT:
            qDebug() << "NetworkClient: Dispatching HEARTBEAT";
            emit heartbeatReceived();
            missedHeartbeats_ = 0;
            heartbeatTimeout_->stop();
            break;
            
        case MessageType::ERROR:
            qDebug() << "NetworkClient: Dispatching ERROR";
            emit errorOccurred(
                static_cast<NetworkError>(message.payload["error"].toInt()),
                message.payload["message"].toString()
            );
            break;

        case MessageType::CHAT_MESSAGE: {
            qDebug() << "NetworkClient: Dispatching CHAT_MESSAGE";
            ChatMessage chat = ChatMessage::fromJson(message.payload);
            emit chatMessageReceived(chat.sender, chat.content);
            break;
        }

        case MessageType::UNDO_REQUEST: {
            qDebug() << "NetworkClient: Dispatching UNDO_REQUEST";
            emit undoRequestReceived();
            break;
        }

        case MessageType::UNDO_RESPONSE: {
            qDebug() << "NetworkClient: Dispatching UNDO_RESPONSE";
            bool accepted = message.payload["accepted"].toBool(false);
            emit undoResponseReceived(accepted);
            break;
        }

        default:
            qDebug() << "NetworkClient: Unknown message type:" << static_cast<int>(message.type);
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
    qDebug() << "NetworkClient::sendQueuedMessage called, queue size:" << sendQueue_.size();
    
    if (sendQueue_.empty() || state_ != ConnectionState::Connected) {
        qWarning() << "NetworkClient::sendQueuedMessage: Cannot send, queue empty or not connected";
        return false;
    }
    
    Message msg = sendQueue_.front();
    QByteArray data = msg.serialize();
    data.append("\n");  // 添加换行符
    
    qDebug() << "NetworkClient::sendQueuedMessage: Sending" << data.size() << "bytes";
    qint64 bytesWritten = socket_->write(data);
    
    if (bytesWritten < 0) {
        // Write error
        qWarning() << "Send failed:" << socket_->errorString();
        return false;
    }
    
    qDebug() << "NetworkClient::sendQueuedMessage: Wrote" << bytesWritten << "bytes";
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
    qWarning() << "NetworkClient: Socket error:" << socketError 
               << "-" << socket_->errorString()
               << "Peer:" << socket_->peerAddress().toString() << ":" << socket_->peerPort();
    
    // Add more context to the error message
    QString fullMessage = socket_->errorString();
    if (socketError == QAbstractSocket::ConnectionRefusedError) {
        fullMessage = QString("Connection refused. Ensure the host is running and port %1 is not blocked by firewall. (%2)")
                        .arg(socket_->peerPort())
                        .arg(socket_->errorString());
    } else if (socketError == QAbstractSocket::HostNotFoundError) {
        fullMessage = QString("Host not found at %1. Please check the IP address.")
                        .arg(socket_->peerAddress().toString());
    } else if (socketError == QAbstractSocket::SocketTimeoutError) {
        fullMessage = QString("Connection timed out to %1:%2. Network may be slow or host is unreachable.")
                        .arg(socket_->peerAddress().toString())
                        .arg(socket_->peerPort());
    }
    
    NetworkError error = socketErrorToNetworkError(socketError);
    emit errorOccurred(error, fullMessage);
    
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
    , clientSocket_(nullptr)
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
    if (server_) {
        server_->close();
    }
    if (clientSocket_) {
        clientSocket_->disconnect();
        clientSocket_->close();
        clientSocket_->deleteLater();
        clientSocket_ = nullptr;
    }
    listeningPort_ = 0;
    state_ = ConnectionState::Disconnected;
    qInfo() << "Stopped hosting";
}

void NetworkHost::onNewConnection()
{
    qDebug() << "NetworkHost::onNewConnection called";
    
    if (!server_ || !server_->isListening()) {
        qWarning() << "NetworkHost: Server not listening, ignoring new connection";
        return;
    }
    
    qDebug() << "NetworkHost: Server is listening, processing new connection";
    
    // 如果已有客户端连接，先断开旧连接并安全清理
    if (clientSocket_) {
        qWarning() << "NetworkHost: Already have a client, closing old connection";
        
        // 断开所有信号连接，防止旧的 disconnected 信号触发
        clientSocket_->disconnect();
        clientSocket_->abort();  // 立即中止连接
        clientSocket_->deleteLater();
        clientSocket_ = nullptr;
    }
    
    if (server_->hasPendingConnections()) {
        clientSocket_ = server_->nextPendingConnection();

        if (clientSocket_) {
            qInfo() << "NetworkHost: Client connected from" << clientSocket_->peerAddress().toString()
                     << ":" << clientSocket_->peerPort();

            // 连接信号
            qDebug() << "NetworkHost: Connecting readyRead signal...";
            connect(clientSocket_, &QTcpSocket::readyRead,
                    this, &NetworkHost::onClientReadyRead,
                    Qt::AutoConnection);
            qDebug() << "NetworkHost: Connecting disconnected signal...";
            connect(clientSocket_, &QTcpSocket::disconnected,
                    this, &NetworkHost::onClientDisconnected,
                    Qt::AutoConnection);
            qDebug() << "NetworkHost: Signals connected";

            state_ = ConnectionState::Connected;

            qDebug() << "NetworkHost: Emitting clientConnected signal";
            emit clientConnected(clientSocket_->peerAddress(), clientSocket_->peerPort());
            qDebug() << "NetworkHost: clientConnected signal emitted";
        }
    } else {
        qWarning() << "NetworkHost: No pending connections found";
    }
}

void NetworkHost::onClientReadyRead()
{
    if (!clientSocket_) {
        qWarning() << "NetworkHost::onClientReadyRead: clientSocket_ is null";
        return;
    }
    
    qDebug() << "NetworkHost::onClientReadyRead called, bytesAvailable:" << clientSocket_->bytesAvailable();
    
    while (clientSocket_ && clientSocket_->bytesAvailable() > 0) {
        QByteArray data = clientSocket_->readAll();
        qDebug() << "NetworkHost: read" << data.size() << "bytes, appending to buffer";
        receiveBuffer_.append(data);
        qDebug() << "NetworkHost: receiveBuffer_ now has" << receiveBuffer_.size() << "bytes";
    }

    qDebug() << "NetworkHost: Processing buffer, looking for newlines...";
    while (true) {
        int newlineIdx = receiveBuffer_.indexOf('\n');
        if (newlineIdx < 0) {
            qDebug() << "NetworkHost: No newline found, breaking";
            break;
        }

        QByteArray line = receiveBuffer_.left(newlineIdx);
        receiveBuffer_.remove(0, newlineIdx + 1);

        qDebug() << "NetworkHost: Found newline, parsing message:" << line;
        
        try {
            Message message = Message::deserialize(line);
            qDebug() << "NetworkHost: Message deserialized, type:" << static_cast<int>(message.type);
            
            emit messageReceived(message);
            
            // 分发消息（调用父类的 dispatchMessage）
            try {
                dispatchMessage(message);
            } catch (const std::exception& e) {
                qCritical() << "NetworkHost: Exception in dispatchMessage:" << e.what();
            } catch (...) {
                qCritical() << "NetworkHost: Unknown exception in dispatchMessage";
            }
        } catch (const std::exception& e) {
            qCritical() << "NetworkHost: Exception deserializing message:" << e.what();
        } catch (...) {
            qCritical() << "NetworkHost: Unknown exception deserializing message";
        }
    }
}

void NetworkHost::onClientDisconnected()
{
    qInfo() << "NetworkHost: Client disconnected";
    if (clientSocket_) {
        qDebug() << "NetworkHost: Cleaning up client socket";
        // 不要再次调用 disconnect()，因为已经在 onNewConnection 中处理了
        clientSocket_->deleteLater();
        clientSocket_ = nullptr;
    }
    state_ = ConnectionState::Disconnected;
    emit clientDisconnected();
}

bool NetworkHost::sendMessage(const Message& message)
{
    qDebug() << "NetworkHost::sendMessage called, type:" << static_cast<int>(message.type);
    
    if (!clientSocket_) {
        qWarning() << "NetworkHost::sendMessage: clientSocket_ is null";
        return false;
    }
    
    qDebug() << "NetworkHost::sendMessage: clientSocket state:" << clientSocket_->state() << "ConnectedState:" << QAbstractSocket::ConnectedState;
    
    if (state_ != ConnectionState::Connected) {
        qWarning() << "NetworkHost::sendMessage: state is not Connected (state:" << static_cast<int>(state_) << ")";
        return false;
    }
    
    if (clientSocket_->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "NetworkHost::sendMessage: socket state is not Connected (socket state:" << clientSocket_->state() << ")";
        return false;
    }

    QByteArray data = message.serialize();
    data.append("\n");
    
    qDebug() << "NetworkHost::sendMessage: Before write - bytesToWrite:" << clientSocket_->bytesToWrite();
    
    qint64 written = clientSocket_->write(data);
    qDebug() << "NetworkHost::sendMessage: write returned:" << written << "expected:" << data.size();
    
    qDebug() << "NetworkHost::sendMessage: After write - bytesToWrite:" << clientSocket_->bytesToWrite();
    
    if (written != data.size()) {
        qWarning() << "NetworkHost::sendMessage: wrote" << written << "bytes, expected" << data.size();
        return false;
    }
    
    // 确保数据被发送
    clientSocket_->flush();
    qDebug() << "NetworkHost::sendMessage: After flush - bytesToWrite:" << clientSocket_->bytesToWrite();
    
    // 检查是否有错误
    if (clientSocket_->error() != QAbstractSocket::UnknownSocketError) {
        qWarning() << "NetworkHost::sendMessage: Socket error:" << clientSocket_->errorString();
    }
    
    return true;
}

bool NetworkHost::sendMove(int row, int col, const QString& player, int moveNumber)
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
    
    qDebug() << "NetworkHost::sendMove: creating MOVE_MADE message, row:" << row << "col:" << col << "player:" << player;
    qDebug() << "NetworkHost::sendMove: calling sendMessage";
    
    bool result = sendMessage(msg);
    qDebug() << "NetworkHost::sendMove: sendMessage returned:" << result;
    
    return result;
}

} // namespace Network

