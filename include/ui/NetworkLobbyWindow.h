/*
    ReversiAI_Platform - Network Module

    @file networklobbywindow.hpp
    @brief Network multiplayer lobby window
    @date 2026
    @author Project Team
    @license GPL-3.0

    UI Reference: QtReversi widget.h/cpp structure
*/

#ifndef NETWORK_LOBBY_WINDOW_HPP
#define NETWORK_LOBBY_WINDOW_HPP

#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QGroupBox>
#include <QSpinBox>
#include <QCheckBox>

#include "network/networkdiscovery.hpp"
#include "network/roommanager.hpp"
#include "network/networkclient.hpp"

// Remove Ui::NetworkLobbyWindow - we build UI programmatically
// namespace Ui {
// class NetworkLobbyWindow;
// }

class NetworkLobbyWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Construct a network lobby window
     * @param parent Parent widget
     */
    explicit NetworkLobbyWindow(QWidget* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~NetworkLobbyWindow() override;

    // ==================== Initialization ====================

    /**
     * @brief Initialize network modules
     */
    void initNetwork();

    /**
     * @brief Start discovering hosts
     */
    void startDiscovery();

    /**
     * @brief Stop discovering
     */
    void stopDiscovery();

signals:
    /**
     * @brief Emitted when user wants to return to menu
     */
    void backToMenu();

    /**
     * @brief Emitted when user joins a game
     * @param hostAddress Remote host address
     * @param port Remote port
     * @param playerName Local player name
     */
    void joinGame(const QHostAddress& hostAddress, quint16 port, const QString& playerName);

    /**
     * @brief Emitted when user creates a game
     * @param roomName Room name
     * @param playerName Host player name
     * @param settings Room settings
     */
    void createGame(const QString& roomName, const QString& playerName, const QJsonObject& settings);

private slots:
    // Discovery slots
    void onHostFound(const Network::DiscoveredHost& host);
    void onHostLost(const Network::DiscoveredHost& host);
    void onDiscoveryError(const QString& error);

    // Room slots
    void onRoomCreated(const Network::GameRoom& room);
    void onRoomListChanged();
    void onPlayerJoined(const QString& roomId, const QString& playerName);
    void onPlayerLeft(const QString& roomId, const QString& playerName);

    // Button slots
    void onCreateRoomClicked();
    void onJoinRoomClicked();
    void onRefreshClicked();
    void onBackClicked();

    // Selection slots
    void onRoomTableItemClicked(QTableWidgetItem* item);
    void onRoomTableItemDoubleClicked(QTableWidgetItem* item);

private:
    // ==================== UI ====================
    QWidget* centralWidget_;
    QVBoxLayout* mainLayout_;

    // Room list
    QTableWidget* roomTable_;
    QPushButton* refreshButton_;
    QPushButton* joinRoomButton_;
    QLabel* statusLabel_;

    // Create room
    QGroupBox* createGroup_;
    QLineEdit* roomNameEdit_;
    QLineEdit* playerNameEdit_;
    QPushButton* createButton_;

    // Navigation
    QPushButton* backButton_;

    // ==================== Network ====================
    Network::NetworkDiscovery* discovery_;
    Network::RoomManager* roomManager_;
    Network::NetworkClient* networkClient_;

    // ==================== State ====================
    QString localPlayerName_;
    QString localRoomName_;
    bool isHosting_;

    // ==================== Private Methods ====================

    /**
     * @brief Setup the UI layout
     */
    void setupUI();

    /**
     * @brief Connect UI signals to slots
     */
    void setupConnections();

    /**
     * @brief Update room table with available rooms
     */
    void updateRoomTable();

    /**
     * @brief Show status message
     * @param message Message to show
     * @param isError Whether this is an error
     */
    void showStatus(const QString& message, bool isError = false);

    /**
     * @brief Get selected room ID from table
     * @return Room ID or empty string
     */
    QString getSelectedRoomId() const;

    /**
     * @brief Validate room creation input
     * @return true if valid
     */
    bool validateCreateRoomInput();

    /**
     * @brief Update UI state based on discovery status
     */
    void updateDiscoveryStatus();
};

#endif // NETWORK_LOBBY_WINDOW_HPP

