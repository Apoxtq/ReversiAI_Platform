/*
    ReversiAI_Platform - Network Module

    @file networkdiscovery.cpp
    @brief LAN network discovery implementation
    @date 2026
    @author Project Team
    @license GPL-3.0
*/

#include "network/networkdiscovery.hpp"
#include <QDebug>
#include <QNetworkInterface>
#include <QJsonParseError>

namespace Network {

NetworkDiscovery::NetworkDiscovery(QObject* parent)
    : QObject(parent)
    , broadcastSocket_(nullptr)
    , listenSocket_(nullptr)
    , discoveryTimer_(nullptr)
    , broadcastTimer_(nullptr)
    , broadcastPort_(0)
    , listenPort_(0)
    , isDiscovering_(false)
    , isBroadcasting_(false)
{
    // Create sockets (reference: Custom UDP implementation)
    broadcastSocket_ = new QUdpSocket(this);
    listenSocket_ = new QUdpSocket(this);
    
    // Create timers
    discoveryTimer_ = new QTimer(this);
    discoveryTimer_->setSingleShot(false);
    
    broadcastTimer_ = new QTimer(this);
    broadcastTimer_->setSingleShot(false);
    
    // Connect signals
    connect(discoveryTimer_, &QTimer::timeout, this, &NetworkDiscovery::sendBroadcast);
    connect(broadcastTimer_, &QTimer::timeout, this, &NetworkDiscovery::onBroadcastTimeout);
    connect(listenSocket_, &QUdpSocket::readyRead, this, &NetworkDiscovery::onDatagramReceived);
    
    // Initialize with defaults
    playerName_ = "Player";
    roomName_ = "Reversi Room";
    gameVersion_ = "0.5.0";
    broadcastPort_ = 45454;
    listenPort_ = 45455;
}

NetworkDiscovery::~NetworkDiscovery()
{
    stopDiscovery();
    stopBroadcasting();
}

// ==================== Discovery Control ====================

void NetworkDiscovery::startDiscovery()
{
    // Reference: Egaroucid ggs.hpp discovery pattern
    if (isDiscovering_) {
        return;
    }
    
    if (!initializeSockets()) {
        emit discoveryError("Failed to initialize discovery sockets");
        return;
    }
    
    // Start listening
    if (!listenSocket_->bind(QHostAddress::Any, listenPort_, QUdpSocket::ShareAddress)) {
        emit discoveryError(QString("Failed to bind to port %1").arg(listenPort_));
        return;
    }
    
    // Join multicast group for better discovery
    // This allows receiving broadcasts from other subnets
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& interface : interfaces) {
        if (interface.flags() & QNetworkInterface::IsUp &&
            interface.flags() & QNetworkInterface::IsRunning &&
            !(interface.flags() & QNetworkInterface::IsLoopBack)) {
            
            for (const QNetworkAddressEntry& entry : interface.addressEntries()) {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    // Try to join multicast group (optional)
                    // listenSocket_->joinMulticastGroup(QHostAddress("224.0.0.1"), interface);
                }
            }
        }
    }
    
    isDiscovering_ = true;
    clearHosts();
    
    // Send initial broadcast
    sendBroadcast();
    
    // Start discovery timer (reference: Egaroucid heartbeat interval)
    discoveryTimer_->start(DISCOVERY_INTERVAL);
    
    emit discoveryStarted();
    qInfo() << "Network discovery started on port" << listenPort_;
}

void NetworkDiscovery::stopDiscovery()
{
    if (!isDiscovering_) {
        return;
    }
    
    discoveryTimer_->stop();
    listenSocket_->close();
    
    isDiscovering_ = false;
    emit discoveryStopped();
    qInfo() << "Network discovery stopped";
}

// ==================== Broadcasting ====================

void NetworkDiscovery::startBroadcasting(quint16 port)
{
    if (isBroadcasting_) {
        return;
    }
    
    if (!initializeSockets()) {
        emit discoveryError("Failed to initialize broadcasting sockets");
        return;
    }
    
    broadcastPort_ = port;
    
    // Bind socket for broadcasting
    if (!broadcastSocket_->bind(QHostAddress::Any, 0, QUdpSocket::ShareAddress)) {
        emit discoveryError("Failed to bind broadcast socket");
        return;
    }
    
    // Enable broadcast
    // Enable broadcast (32 = BroadcastSocketOption in Qt5/6)
    broadcastSocket_->setSocketOption(static_cast<QAbstractSocket::SocketOption>(32), 1);
    
    isBroadcasting_ = true;
    
    // Send initial broadcast
    sendBroadcast();
    
    // Start broadcast timer
    broadcastTimer_->start(BROADCAST_INTERVAL);
    
    qInfo() << "Broadcasting started on port" << broadcastPort_;
}

void NetworkDiscovery::stopBroadcasting()
{
    if (!isBroadcasting_) {
        return;
    }
    
    // Send goodbye message
    sendGoodbye();
    
    broadcastTimer_->stop();
    broadcastSocket_->close();
    
    isBroadcasting_ = false;
    qInfo() << "Broadcasting stopped";
}

void NetworkDiscovery::sendGoodbye()
{
    // Reference: Egaroucid ggs.hpp goodbye pattern
    QJsonObject msg = createDiscoveryMessage("GOODBYE");
    QByteArray data = QJsonDocument(msg).toJson();
    
    broadcastSocket_->writeDatagram(data, QHostAddress::Broadcast, broadcastPort_);
    qDebug() << "Sent GOODBYE broadcast";
}

// ==================== Host Management ====================

void NetworkDiscovery::clearHosts()
{
    hosts_.clear();
}

void NetworkDiscovery::removeStaleHosts(uint64_t timeoutMs)
{
    uint64_t now = QDateTime::currentMSecsSinceEpoch();
    QList<DiscoveredHost> stale;
    
    for (const DiscoveredHost& host : hosts_) {
        if (now - host.discoveredTime > timeoutMs) {
            stale.append(host);
        }
    }
    
    for (const DiscoveredHost& host : stale) {
        hosts_.removeOne(host);
        emit hostLost(host);
    }
}

// ==================== Private Methods ====================

bool NetworkDiscovery::initializeSockets()
{
    // Reference: Custom socket initialization
    return true;
}

QJsonObject NetworkDiscovery::createDiscoveryMessage(const QString& type)
{
    // Reference: Egaroucid ggs.hpp message format
    QJsonObject msg;
    msg["type"] = type;
    msg["playerName"] = playerName_;
    msg["roomName"] = roomName_;
    msg["port"] = static_cast<int>(listenPort_);
    msg["version"] = gameVersion_;
    msg["timestamp"] = static_cast<qint64>(QDateTime::currentMSecsSinceEpoch());
    return msg;
}

void NetworkDiscovery::sendBroadcast()
{
    // Reference: Egaroucid ggs.hpp line 554-556 (heartbeat pattern)
    
    QJsonObject msg = createDiscoveryMessage("HELLO");
    QByteArray data = QJsonDocument(msg).toJson();
    
    // Send to broadcast address
    qint64 bytesWritten = broadcastSocket_->writeDatagram(
        data, 
        QHostAddress::Broadcast, 
        broadcastPort_
    );
    
    if (bytesWritten < 0) {
        qWarning() << "Broadcast failed:" << broadcastSocket_->errorString();
    } else {
        emit broadcastReceived(QHostAddress::Broadcast);
    }
}

void NetworkDiscovery::onDatagramReceived()
{
    // Reference: Egaroucid ggs.hpp receive pattern (line 205-217)
    
    while (listenSocket_->hasPendingDatagrams()) {
        QNetworkDatagram datagram = listenSocket_->receiveDatagram();
        QByteArray data = datagram.data();
        QHostAddress sender = datagram.senderAddress();
        quint16 senderPort = datagram.senderPort();
        
        // Parse JSON
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "Failed to parse discovery message:" << error.errorString();
            continue;
        }
        
        QJsonObject json = doc.object();
        processDiscoveryMessage(json, sender);
    }
}

void NetworkDiscovery::processDiscoveryMessage(const QJsonObject& json, const QHostAddress& sender)
{
    // Reference: Egaroucid ggs.hpp message processing pattern
    QString type = json["type"].toString();
    
    if (type == "HELLO") {
        // New host discovered
        DiscoveredHost host = DiscoveredHost::fromJson(json, sender);
        
        if (isHostKnown(host)) {
            // Update timestamp
            updateHostTimestamp(host);
        } else {
            // New host
            hosts_.append(host);
            emit hostFound(host);
        }
    } else if (type == "GOODBYE") {
        // Host leaving
        DiscoveredHost host = DiscoveredHost::fromJson(json, sender);
        
        if (hosts_.removeOne(host)) {
            emit hostLost(host);
        }
    } else if (type == "PING") {
        // Respond to ping (for NAT traversal)
        QJsonObject response = createDiscoveryMessage("PONG");
        QByteArray data = QJsonDocument(response).toJson();
        listenSocket_->writeDatagram(data, sender, json["port"].toInt());
    }
}

bool NetworkDiscovery::isHostKnown(const DiscoveredHost& host) const
{
    for (const DiscoveredHost& h : hosts_) {
        if (h.address == host.address && h.port == host.port) {
            return true;
        }
    }
    return false;
}

void NetworkDiscovery::updateHostTimestamp(DiscoveredHost& host)
{
    for (int i = 0; i < hosts_.size(); ++i) {
        if (hosts_[i].address == host.address && hosts_[i].port == host.port) {
            hosts_[i].discoveredTime = host.discoveredTime;
            break;
        }
    }
}

void NetworkDiscovery::onBroadcastTimeout()
{
    // Reference: Egaroucid ggs.hpp timeout handling
    // Remove stale hosts
    removeStaleHosts(HOST_TIMEOUT);
}

} // namespace Network

