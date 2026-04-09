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
    , loopbackSocket_(nullptr)
    , discoveryTimer_(nullptr)
    , broadcastTimer_(nullptr)
    , broadcastPort_(0)
    , listenPort_(0)
    , gamePort_(0)
    , isDiscovering_(false)
    , isBroadcasting_(false)
{
    // Create sockets (reference: Custom UDP implementation)
    broadcastSocket_ = new QUdpSocket(this);
    listenSocket_ = new QUdpSocket(this);
    loopbackSocket_ = new QUdpSocket(this);
    
    // Create timers
    discoveryTimer_ = new QTimer(this);
    discoveryTimer_->setSingleShot(false);
    
    broadcastTimer_ = new QTimer(this);
    broadcastTimer_->setSingleShot(false);
    
    // Connect signals
    // broadcastTimer_ fires sendBroadcast() in hosting (broadcasting) mode
    connect(broadcastTimer_, &QTimer::timeout, this, &NetworkDiscovery::sendBroadcast);
    // listenSocket_ fires when a discovery datagram arrives from the LAN
    connect(listenSocket_, &QUdpSocket::readyRead, this, &NetworkDiscovery::onDatagramReceived);
    // loopbackSocket_ fires when a discovery datagram arrives from the same machine
    connect(loopbackSocket_, &QUdpSocket::readyRead, this, &NetworkDiscovery::onLoopbackDatagramReceived);
    
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
    // Discovery = receive-only mode (listening for other hosts' broadcasts)
    // Do NOT broadcast from here — only the hosting side broadcasts
    if (isDiscovering_) {
        return;
    }

    if (!initializeSockets()) {
        emit discoveryError("Failed to initialize discovery sockets");
        return;
    }

    // Start listening on 0.0.0.0 (all interfaces)
    if (!listenSocket_->bind(QHostAddress::Any, listenPort_, QUdpSocket::ShareAddress)) {
        emit discoveryError(QString("Failed to bind to port %1").arg(listenPort_));
        return;
    }

    // Also bind loopback interface for same-machine discovery
    // On Windows, QHostAddress::Broadcast does not reach 127.0.0.1, so we need
    // a separate socket bound to 127.0.0.1 to receive loopback broadcasts
    if (!loopbackSocket_->bind(QHostAddress::LocalHost, listenPort_, QUdpSocket::ShareAddress)) {
        qWarning() << "Failed to bind loopback socket to port" << listenPort_
                   << "- same-machine discovery may not work";
    } else {
        qInfo() << "Loopback discovery listening on 127.0.0.1:" << listenPort_;
    }

    isDiscovering_ = true;
    clearHosts();

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
    if (loopbackSocket_) {
        loopbackSocket_->close();
    }

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

    // port = TCP game port for incoming connections (goes in JSON)
    gamePort_ = port;
    // broadcastPort_ = UDP destination port (must match where receivers listen)
    // Both receiver sockets (0.0.0.0 and 127.0.0.1) are bound to listenPort_ (45455)
    broadcastPort_ = listenPort_;

    // Bind socket for broadcasting
    if (!broadcastSocket_->bind(QHostAddress::Any, 0, QUdpSocket::ShareAddress)) {
        emit discoveryError("Failed to bind broadcast socket");
        return;
    }

    // Enable broadcast (32 = BroadcastSocketOption in Qt5/6)
    broadcastSocket_->setSocketOption(static_cast<QAbstractSocket::SocketOption>(32), 1);

    isBroadcasting_ = true;

    // Send initial broadcast
    sendBroadcast();

    // Start broadcast timer
    broadcastTimer_->start(BROADCAST_INTERVAL);

    qInfo() << "Broadcasting started (TCP game port:" << gamePort_
            << "UDP broadcast to:" << broadcastPort_ << ")";
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
    // gamePort_ = TCP listen port (for incoming game connections)
    msg["port"] = static_cast<int>(gamePort_);
    msg["version"] = gameVersion_;
    msg["timestamp"] = static_cast<qint64>(QDateTime::currentMSecsSinceEpoch());
    return msg;
}

void NetworkDiscovery::sendBroadcast()
{
    // Reference: Egaroucid ggs.hpp line 554-556 (heartbeat pattern)

    QJsonObject msg = createDiscoveryMessage("HELLO");
    QByteArray data = QJsonDocument(msg).toJson();

    qInfo() << "[BROADCAST] isBroadcasting=" << isBroadcasting_
            << "socket=" << broadcastSocket_->state()
            << "dest=255.255.255.255:" << broadcastPort_;

    // Send to broadcast address (for other machines on the network)
    qint64 bytesWritten = broadcastSocket_->writeDatagram(
        data,
        QHostAddress::Broadcast,
        broadcastPort_
    );

    if (bytesWritten < 0) {
        qWarning() << "[BROADCAST] Failed:" << broadcastSocket_->errorString();
    }

    // Also send to loopback address (127.0.0.1) for same-machine discovery
    // QHostAddress::Broadcast does not reach loopback on Windows, so we send directly
    qint64 loopbackWritten = broadcastSocket_->writeDatagram(
        data,
        QHostAddress::LocalHost,
        broadcastPort_
    );

    if (loopbackWritten < 0) {
        qWarning() << "[BROADCAST] Loopback failed:" << broadcastSocket_->errorString();
    }
}

void NetworkDiscovery::onDatagramReceived()
{
    // Reference: Egaroucid ggs.hpp receive pattern (line 205-217)
    while (listenSocket_->hasPendingDatagrams()) {
        QNetworkDatagram datagram = listenSocket_->receiveDatagram();
        QByteArray data = datagram.data();
        QHostAddress sender = datagram.senderAddress();

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

void NetworkDiscovery::onLoopbackDatagramReceived()
{
    // Handle incoming datagrams on the loopback socket (127.0.0.1)
    // This is identical to onDatagramReceived but reads from loopbackSocket_
    while (loopbackSocket_ && loopbackSocket_->hasPendingDatagrams()) {
        QNetworkDatagram datagram = loopbackSocket_->receiveDatagram();
        QByteArray data = datagram.data();
        QHostAddress sender = datagram.senderAddress();

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError) {
            qWarning() << "Failed to parse loopback discovery message:" << error.errorString();
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
        // Match by playerName + TCP game port (port is unique per host process)
        // This deduplicates when the same broadcast arrives via LAN + loopback
        // with different sender addresses (::ffff:x.x.x.x vs 127.0.0.1)
        if (h.playerName == host.playerName && h.port == host.port) {
            return true;
        }
    }
    return false;
}

void NetworkDiscovery::updateHostTimestamp(DiscoveredHost& host)
{
    for (int i = 0; i < hosts_.size(); ++i) {
        if (hosts_[i].playerName == host.playerName && hosts_[i].port == host.port) {
            hosts_[i].discoveredTime = host.discoveredTime;
            // Prefer loopback address for same-machine connections
            if (host.address == QHostAddress::LocalHost) {
                hosts_[i].address = host.address;
            }
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

