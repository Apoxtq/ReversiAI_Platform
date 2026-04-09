/*
    ReversiAI_Platform - Network Module

    @file networkdiscovery.hpp
    @brief LAN network discovery using UDP broadcast
    @date 2026
    @author Project Team
    @license GPL-3.0

    LAN multiplayer host discovery using UDP broadcast.
    Uses QUdpSocket for network discovery.
*/

#ifndef NETWORK_DISCOVERY_HPP
#define NETWORK_DISCOVERY_HPP

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QList>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkDatagram>

#include "network/message.hpp"

namespace Network {

/**
 * @brief LAN Network Discovery Service
 * 
 * Discovers other players on the local network using UDP broadcast.
 * 
 * Reference: Custom implementation using Qt QUdpSocket
 * Message format inspired by Egaroucid ggs.hpp protocol
 */
class NetworkDiscovery : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a network discovery service
     * @param parent Parent QObject
     */
    explicit NetworkDiscovery(QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~NetworkDiscovery() override;

    // ==================== Discovery Control ====================

    /**
     * @brief Start discovering hosts on the network
     * 
     * Reference: Egaroucid heartbeat timer pattern (ggs.hpp 554-557)
     */
    void startDiscovery();

    /**
     * @brief Stop discovery
     */
    void stopDiscovery();

    /**
     * @brief Check if currently discovering
     * @return true if discovery is active
     */
    bool isDiscovering() const { return isDiscovering_; }

    // ==================== Broadcasting ====================

    /**
     * @brief Start broadcasting our presence
     * @param port Port to broadcast on
     * 
     * Use startDiscovery() instead for regular discovery.
     */
    void startBroadcasting(quint16 port);

    /**
     * @brief Stop broadcasting
     */
    void stopBroadcasting();

    /**
     * @brief Send a goodbye message before exiting
     */
    void sendGoodbye();

    // ==================== Host Management ====================

    /**
     * @brief Get list of discovered hosts
     * @return List of discovered hosts
     */
    QList<DiscoveredHost> getDiscoveredHosts() const { return hosts_; }

    /**
     * @brief Clear all discovered hosts
     */
    void clearHosts();

    /**
     * @brief Remove stale hosts (older than timeout)
     * @param timeoutMs Timeout in milliseconds
     */
    void removeStaleHosts(uint64_t timeoutMs = 10000);

    // ==================== Configuration ====================

    /**
     * @brief Set our player name
     * @param name Player name
     */
    void setPlayerName(const QString& name) { playerName_ = name; }

    /**
     * @brief Set our room name
     * @param name Room name
     */
    void setRoomName(const QString& name) { roomName_ = name; }

    /**
     * @brief Set broadcast port
     * @param port Port number
     */
    void setBroadcastPort(quint16 port) { broadcastPort_ = port; }

    /**
     * @brief Set listening port
     * @param port Port number
     */
    void setListenPort(quint16 port) { listenPort_ = port; }

    /**
     * @brief Set game version
     * @param version Version string
     */
    void setGameVersion(const QString& version) { gameVersion_ = version; }

signals:
    /**
     * @brief Emitted when a new host is discovered
     * @param host Discovered host information
     */
    void hostFound(const DiscoveredHost& host);

    /**
     * @brief Emitted when a host is lost (timeout or goodbye)
     * @param host Lost host information
     */
    void hostLost(const DiscoveredHost& host);

    /**
     * @brief Emitted when discovery starts
     */
    void discoveryStarted();

    /**
     * @brief Emitted when discovery stops
     */
    void discoveryStopped();

    /**
     * @brief Emitted on discovery error
     * @param error Error message
     */
    void discoveryError(const QString& error);

    /**
     * @brief Emitted when our broadcast is received
     * @param address Receiver address
     */
    void broadcastReceived(const QHostAddress& address);

private slots:
    /**
     * @brief Send discovery broadcast
     *
     * Reference: Egaroucid heartbeat pattern (ggs.hpp 554-556)
     */
    void sendBroadcast();

    /**
     * @brief Handle incoming datagrams
     */
    void onDatagramReceived();

    /**
     * @brief Handle incoming datagrams on loopback socket
     */
    void onLoopbackDatagramReceived();

    /**
     * @brief Handle broadcast timeout
     */
    void onBroadcastTimeout();

private:
    // ==================== Sockets ====================
    QUdpSocket* broadcastSocket_;      ///< For sending broadcasts
    QUdpSocket* listenSocket_;          ///< For receiving broadcasts (binds 0.0.0.0)
    QUdpSocket* loopbackSocket_;        ///< For receiving loopback broadcasts (binds 127.0.0.1)

    // ==================== Timers ====================
    QTimer* discoveryTimer_;          ///< Discovery interval timer
    QTimer* broadcastTimer_;          ///< Broadcast interval timer

    // ==================== Configuration ====================
    QString playerName_;              ///< Our player name
    QString roomName_;                ///< Our room name
    quint16 broadcastPort_;           ///< Port to broadcast on
    quint16 listenPort_;              ///< Port to listen on (UDP discovery)
    quint16 gamePort_;                ///< TCP game port for incoming connections
    QString gameVersion_;             ///< Game version
    bool isDiscovering_;              ///< Discovery state
    bool isBroadcasting_;             ///< Broadcasting state

    // ==================== Host Tracking ====================
    QList<DiscoveredHost> hosts_;     ///< Discovered hosts
    static constexpr int DISCOVERY_INTERVAL = 2000;    ///< 2 seconds
    static constexpr int BROADCAST_INTERVAL = 3000;    ///< 3 seconds
    static constexpr int HOST_TIMEOUT = 10000;         ///< 10 seconds

    // ==================== Private Methods ====================

    /**
     * @brief Process received discovery message
     * @param json Message JSON
     * @param sender Sender address
     */
    void processDiscoveryMessage(const QJsonObject& json, const QHostAddress& sender);

    /**
     * @brief Create discovery message
     * @param type Message type (HELLO, GOODBYE, PING)
     * @return JSON message
     */
    QJsonObject createDiscoveryMessage(const QString& type);

    /**
     * @brief Check if host already known
     * @param host Host to check
     * @return true if known
     */
    bool isHostKnown(const DiscoveredHost& host) const;

    /**
     * @brief Update host timestamp
     * @param host Host to update
     */
    void updateHostTimestamp(DiscoveredHost& host);

    /**
     * @brief Initialize sockets
     * @return true if successful
     */
    bool initializeSockets();
};

} // namespace Network

#endif // NETWORK_DISCOVERY_HPP

