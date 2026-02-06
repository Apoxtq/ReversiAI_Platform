/*
    ReversiAI_Platform - Network Module

    @file reconnectionmanager.hpp
    @brief Reconnection management with exponential backoff
    @date 2026
    @author Project Team
    @license GPL-3.0

    Reference: Egaroucid ggs.hpp reconnection handling (line 643-682)
*/

#ifndef RECONNECTION_MANAGER_HPP
#define RECONNECTION_MANAGER_HPP

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QJsonObject>
#include <QRandomGenerator>

namespace Network {

/**
 * @brief Reconnection Manager
 * 
 * Handles automatic reconnection with exponential backoff.
 * 
 * Reference: Egaroucid ggs.hpp reconnection handling (line 643-682)
 */
class ReconnectionManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Construct a reconnection manager
     * @param parent Parent QObject
     */
    explicit ReconnectionManager(QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~ReconnectionManager() override;

    // ==================== Connection State ====================

    /**
     * @brief Check if currently reconnecting
     * @return true if reconnecting
     */
    bool isReconnecting() const { return reconnectTimer_->isActive(); }

    /**
     * @brief Get current attempt number
     * @return Attempt number (0 = not reconnecting)
     */
    int getCurrentAttempt() const { return currentAttempt_; }

    /**
     * @brief Get maximum retry attempts
     * @return Maximum attempts
     */
    int getMaxAttempts() const { return maxAttempts_; }

    /**
     * @brief Set maximum retry attempts
     * @param max Maximum attempts
     */
    void setMaxAttempts(int max) { maxAttempts_ = max; }

    // ==================== Reconnection Control ====================

    /**
     * @brief Start reconnection process
     * @param lastAddress Last connected address
     * @param lastPort Last connected port
     * @param savedState Saved game state (optional)
     * 
     * Reference: Egaroucid ggs.hpp reconnection start (line 643-655)
     */
    void startReconnection(const QString& address, quint16 port, 
                          const QJsonObject& savedState = QJsonObject());

    /**
     * @brief Cancel reconnection
     */
    void cancelReconnection();

    /**
     * @brief Notify successful reconnection
     */
    void notifyReconnected();

    /**
     * @brief Notify reconnection failed
     */
    void notifyReconnectFailed();

    // ==================== State Management ====================

    /**
     * @brief Save current game state
     * @param state State to save
     */
    void saveState(const QJsonObject& state);

    /**
     * @brief Get saved state
     * @return Saved state
     */
    QJsonObject getSavedState() const { return savedState_; }

    /**
     * @brief Clear saved state
     */
    void clearState();

    // ==================== Timing ====================

    /**
     * @brief Get next retry delay
     * @return Delay in milliseconds
     * 
     * Uses exponential backoff: 1s, 2s, 4s, 8s, ...
     */
    int getNextRetryDelay() const;

    /**
     * @brief Reset retry count
     */
    void resetAttempts();

    /**
     * @brief Get time since disconnect
     * @return Milliseconds since disconnect
     */
    uint64_t getTimeSinceDisconnect() const;

signals:
    /**
     * @brief Emitted when reconnection should be attempted
     * @param attempt Attempt number
     * @param delay Delay in milliseconds
     */
    void reconnectAttempt(int attempt, int delay);

    /**
     * @brief Emitted when reconnection succeeds
     */
    void reconnected();

    /**
     * @brief Emitted when reconnection fails
     * @param attempt Final attempt number
     * @param reason Failure reason
     */
    void reconnectFailed(int attempt, const QString& reason);

    /**
     * @brief Emitted when max attempts reached
     */
    void maxAttemptsReached();

    /**
     * @brief Emitted on reconnection state change
     * @param state New state (true = reconnecting)
     */
    void reconnectingChanged(bool state);

private slots:
    /**
     * @brief Handle retry timer timeout
     */
    void onRetryTimeout();

private:
    // ==================== Timers ====================
    QTimer* reconnectTimer_;          ///< Retry timer
    QTimer* disconnectTimer_;         ///< Disconnect time tracker

    // ==================== State ====================
    QString lastAddress_;            ///< Last connected address
    quint16 lastPort_;               ///< Last connected port
    QJsonObject savedState_;         ///< Saved game state
    int currentAttempt_;             ///< Current retry attempt
    int maxAttempts_;                ///< Maximum retry attempts
    uint64_t disconnectTime_;        ///< Time of disconnect

    // ==================== Timing ====================
    static constexpr int INITIAL_DELAY = 1000;       ///< 1 second initial delay
    static constexpr int MAX_DELAY = 60000;         ///< 60 seconds max delay
    static constexpr double BACKOFF_MULTIPLIER = 2.0;  ///< Exponential backoff

    // ==================== Private Methods ====================

    /**
     * @brief Calculate next retry delay with exponential backoff
     * @return Delay in milliseconds
     */
    int calculateNextDelay() const;

    /**
     * @brief Update disconnect time
     */
    void updateDisconnectTime();
};

} // namespace Network

#endif // RECONNECTION_MANAGER_HPP

