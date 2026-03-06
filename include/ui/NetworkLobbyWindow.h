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
#include <QTabWidget>
#include <QComboBox>
#include <QTextEdit>

#include "network/networkdiscovery.hpp"
#include "network/roommanager.hpp"
#include "network/networkclient.hpp"
#include "network/ggsgameclient.hpp"

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

    /**
     * @brief Emitted when GGS game starts
     * @param ggsClient The GGS game client
     * @param gameId Game ID
     * @param playerBlack Black player name
     * @param playerWhite White player name
     * @param isBlack Whether player is black
     */
    void ggsGameStarted(Network::GGSGameClient* ggsClient, const QString& gameId, 
                       const QString& playerBlack, const QString& playerWhite, bool isBlack);

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

    // ==================== GGS Online Mode Slots ====================

    /**
     * @brief Handle GGS connect button clicked
     */
    void onGGSConnectClicked();

    /**
     * @brief Handle GGS disconnect button clicked
     */
    void onGGSDisconnectClicked();

    /**
     * @brief Handle GGS send challenge clicked
     */
    void onGGSSendChallengeClicked();

    /**
     * @brief Handle GGS accept challenge clicked
     */
    void onGGSAcceptChallengeClicked(const QString& requestId);

    /**
     * @brief Handle GGS decline challenge clicked
     */
    void onGGSDeclineChallengeClicked(const QString& requestId);

    /**
     * @brief Handle GGS connected
     */
    void onGGSConnected();

    /**
     * @brief Handle GGS disconnected
     */
    void onGGSDisconnected();

    /**
     * @brief Handle GGS connection error
     */
    void onGGSConnectionError(const QString& error);

    /**
     * @brief Handle GGS login successful
     */
    void onGGSLoginSuccessful(const QString& username);

    /**
     * @brief Handle GGS login failed
     */
    void onGGSLoginFailed(const QString& error);

    /**
     * @brief Handle GGS match request received
     */
    void onGGSMatchRequestReceived(const Network::GGSMatchRequest& request);

    /**
     * @brief Handle GGS game started
     */
    void onGGSGameStarted(const QString& gameId, const QString& playerBlack,
                         const QString& playerWhite, bool isPlayerBlack);

    /**
     * @brief Handle GGS chat message received
     */
    void onGGSChatReceived(const QString& sender, const QString& message);

    /**
     * @brief Handle GGS send chat clicked
     */
    void onGGSSendChatClicked();

    /**
     * @brief Handle tab change
     */
    void onTabChanged(int index);

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
    QSpinBox* timeLimitSpin_;
    QCheckBox* rankedCheck_;
    QPushButton* createButton_;

    // Navigation
    QPushButton* backButton_;

    // ==================== Network ====================
    Network::NetworkDiscovery* discovery_;
    Network::RoomManager* roomManager_;
    Network::NetworkClient* networkClient_;

    // ==================== GGS Online Mode ====================
    QTabWidget* modeTabWidget_;

    // LAN Mode widgets (existing)
    // - roomTable_, createGroup_, etc. (already exist)

    // Online Mode widgets
    QWidget* onlineModeWidget_;
    QLabel* ggsStatusLabel_;
    QPushButton* ggsConnectButton_;
    QPushButton* ggsDisconnectButton_;
    QLineEdit* opponentEdit_;
    QComboBox* timeLimitCombo_;
    QCheckBox* ratedCheck_;
    QPushButton* sendChallengeButton_;
    QListWidget* challengeList_;
    QTextEdit* ggsChatDisplay_;
    QLineEdit* ggsChatInput_;
    QPushButton* ggsSendChatButton_;

    // GGS Client
    Network::GGSGameClient* ggsClient_;
    QString ggsUsername_;
    QString ggsPassword_;  // Store password for login
    bool isGGSSConnected_;

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

    /**
     * @brief Setup Online Mode UI (GGS)
     */
    void setupOnlineModeUI();

    /**
     * @brief Update GGS connection status display
     */
    void updateGGSStatus();

    /**
     * @brief Add challenge to list
     * @param request Challenge request
     */
    void addChallengeToList(const Network::GGSMatchRequest& request);

    /**
     * @brief Setup GGS client signal connections
     */
    void setupGGSConnections();
};

#endif // NETWORK_LOBBY_WINDOW_HPP

