/*
    ReversiAI_Platform - Network Module

    @file networkgamewindow.hpp
    @brief Network multiplayer game window
    @date 2026
    @author Project Team
    @license GPL-3.0

    Reference: PvPWindow.h structure
*/

#ifndef NETWORK_GAME_WINDOW_HPP
#define NETWORK_GAME_WINDOW_HPP

#include <QMainWindow>
#include <QLabel>
#include <QLCDNumber>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <memory>

#include "network/networkclient.hpp"
#include "network/gamesynchronizer.hpp"
#include "network/reconnectionmanager.hpp"
#include "ui/GameController.h"

class NetworkGameWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Construct a network game window
     * @param parent Parent widget
     */
    explicit NetworkGameWindow(QWidget* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~NetworkGameWindow() override;

    // ==================== Initialization ====================

    /**
     * @brief Initialize network connection as client
     * @param hostAddress Remote host address
     * @param port Remote port
     * @param playerName Local player name
     */
    void initNetwork(const QHostAddress& hostAddress, quint16 port, const QString& playerName);

    /**
     * @brief Start hosting a game
     * @param playerName Host player name
     * @param port Listen port
     */
    void startHosting(const QString& playerName, quint16 port);

    /**
     * @brief Connect to remote host
     * @param address Host address
     * @param port Port number
     * @param playerName Local player name
     */
    void connectToHost(const QHostAddress& address, quint16 port, const QString& playerName);

    /**
     * @brief Disconnect from current game
     */
    void disconnectFromGame();

    /**
     * @brief Check if connected
     * @return true if connected
     */
    bool isConnected() const { return networkClient_ && networkClient_->isConnected(); }

signals:
    /**
     * @brief Emitted when user wants to return to menu
     */
    void backToMenu();

    /**
     * @brief Emitted when connection is lost
     */
    void connectionLost();

    /**
     * @brief Emitted when reconnection is in progress
     * @param attempt Current attempt
     * @param maxAttempts Maximum attempts
     */
    void reconnecting(int attempt, int maxAttempts);

    /**
     * @brief Emitted when reconnected successfully
     */
    void reconnected();

    /**
     * @brief Emitted when opponent disconnected
     */
    void opponentDisconnected();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* e) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    // Game slots
    void onStartGameClicked();
    void onUndoClicked();
    void onBackToMenuClicked();
    void onGameStarted(Reversi::GameMode mode, Reversi::PlayerColor humanColor);
    void onPhaseChanged(Reversi::GamePhase phase);
    void onTurnChanged(Reversi::PlayerColor player);
    void onMoveMade(int row, int col, Reversi::PlayerColor player);
    void onGameEnded(Reversi::GameResult result);
    void onErrorOccurred(const QString& message);

    // Network slots
    void onConnected();
    void onDisconnected();
    void onNetworkError(Network::NetworkError error, const QString& message);
    void onConnectionTimeout();
    void onReconnectingChanged(bool state);
    void onReconnected();
    void onReconnectFailed(int attempt, const QString& reason);
    void onMaxAttemptsReached();
    void onMoveReceived(int row, int col, const QString& player);
    void onGameStateReceived(const Network::GameStateMessage& state);

    // Chat slots
    void onSendChatMessage();
    void onChatMessageReceived(const QString& sender, const QString& message);

    // Latency slots
    void onPongReceived(qint64 latency);
    void updateLatencyDisplay();

private:
    // ==================== UI Components ====================
    // Reference: PvPWindow.h UI structure
    
    QPixmap background;
    QPixmap white, black;
    QPixmap hintwhite, hintblack, hintred;

    // Network panel
    QGroupBox* connectionGroup_;
    QLabel* connectionStatusLabel_;
    QLabel* opponentNameLabel_;
    QLabel* latencyLabel_;
    QLCDNumber* latencyLCD_;
    
    // Chat
    QTextEdit* chatDisplay_;
    QLineEdit* chatInput_;
    QPushButton* sendChatButton_;
    
    // Game controls
    QPushButton* startButton_;
    QPushButton* undoButton_;
    QPushButton* backButton_;
    
    // ==================== Game Controller ====================
    std::unique_ptr<Reversi::GameController> gameController_;

    // ==================== Network ====================
    Network::NetworkClient* networkClient_;
    Network::GameSynchronizer* synchronizer_;
    Network::ReconnectionManager* reconnector_;
    
    // ==================== State ====================
    QString localPlayerName_;
    QString opponentPlayerName_;
    bool isHost_;
    bool isReconnecting_;
    int currentLatency_;
    
    // ==================== Timers ====================
    QTimer* latencyUpdateTimer_;

    // ==================== Private Methods ====================

    /**
     * @brief Setup game UI components
     */
    void initGameUI();

    /**
     * @brief Setup network-specific UI
     */
    void setupNetworkUI();

    /**
     * @brief Connect game signals
     */
    void setupGameConnections();

    /**
     * @brief Connect network signals
     */
    void setupNetworkConnections();

    /**
     * @brief Update connection status display
     */
    void updateConnectionStatus();

    /**
     * @brief Update opponent name display
     */
    void updateOpponentDisplay();

    /**
     * @brief Add message to chat display
     * @param sender Message sender
     * @param message Message content
     */
    void addChatMessage(const QString& sender, const QString& message);

    /**
     * @brief Handle connection loss
     */
    void handleConnectionLoss();

    /**
     * @brief Send local move to network
     * @param row Row
     * @param col Column
     */
    void sendLocalMove(int row, int col);

    /**
     * @brief Apply remote move to game
     * @param row Row
     * @param col Column
     * @param player Player color
     */
    void applyRemoteMove(int row, int col, const QString& player);

    /**
     * @brief Load game resources
     */
    void loadResources();
};

#endif // NETWORK_GAME_WINDOW_HPP
