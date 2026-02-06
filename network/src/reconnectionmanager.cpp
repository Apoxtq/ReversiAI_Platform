/*
    ReversiAI_Platform - Network Module

    @file reconnectionmanager.cpp
    @brief Reconnection management implementation
    @date 2026
    @author Project Team
    @license GPL-3.0

    Reference: Egaroucid ggs.hpp reconnection handling (line 643-682)
*/

#include "network/reconnectionmanager.hpp"
#include <QDebug>
#include <cmath>

namespace Network {

ReconnectionManager::ReconnectionManager(QObject* parent)
    : QObject(parent)
    , reconnectTimer_(nullptr)
    , disconnectTimer_(nullptr)
    , currentAttempt_(0)
    , maxAttempts_(5)
    , disconnectTime_(0)
{
    // Create timers
    reconnectTimer_ = new QTimer(this);
    reconnectTimer_->setSingleShot(true);
    
    disconnectTimer_ = new QTimer(this);
    disconnectTimer_->setSingleShot(false);
    disconnectTimer_->setInterval(1000);  // Update every second
    
    connect(reconnectTimer_, &QTimer::timeout, this, &ReconnectionManager::onRetryTimeout);
    connect(disconnectTimer_, &QTimer::timeout, this, [this]() {
        emit reconnectingChanged(isReconnecting());
    });
}

ReconnectionManager::~ReconnectionManager()
{
    reconnectTimer_->stop();
    disconnectTimer_->stop();
}

// ==================== Reconnection Control ====================

void ReconnectionManager::startReconnection(const QString& address, quint16 port,
                                           const QJsonObject& savedState)
{
    // Reference: Egaroucid ggs.hpp reconnection start (line 643-655)
    
    lastAddress_ = address;
    lastPort_ = port;
    savedState_ = savedState;
    
    currentAttempt_ = 0;
    updateDisconnectTime();
    
    // Start disconnect time tracking
    disconnectTimer_->start();
    
    // Start first reconnection attempt
    int delay = getNextRetryDelay();
    reconnectTimer_->start(delay);
    
    qInfo() << "Starting reconnection to" << address << ":" << port 
            << "(attempt" << (currentAttempt_ + 1) << "in" << delay << "ms)";
}

void ReconnectionManager::cancelReconnection()
{
    reconnectTimer_->stop();
    disconnectTimer_->stop();
    currentAttempt_ = 0;
    
    emit reconnectingChanged(false);
    
    qInfo() << "Reconnection cancelled";
}

void ReconnectionManager::notifyReconnected()
{
    reconnectTimer_->stop();
    disconnectTimer_->stop();
    currentAttempt_ = 0;
    
    emit reconnectingChanged(false);
    emit reconnected();
    
    qInfo() << "Reconnected successfully";
}

void ReconnectionManager::notifyReconnectFailed()
{
    currentAttempt_++;
    
    if (currentAttempt_ >= maxAttempts_) {
        // Max attempts reached
        reconnectTimer_->stop();
        disconnectTimer_->stop();
        
        emit reconnectingChanged(false);
        emit maxAttemptsReached();
        emit reconnectFailed(currentAttempt_, "Max attempts reached");
        
        qWarning() << "Reconnection failed after" << currentAttempt_ << "attempts";
    } else {
        // Schedule next attempt
        int delay = getNextRetryDelay();
        reconnectTimer_->start(delay);
        
        emit reconnectAttempt(currentAttempt_, delay);
        
        qWarning() << "Reconnection attempt" << currentAttempt_ << "failed, next in" << delay << "ms";
    }
}

// ==================== State Management ====================

void ReconnectionManager::saveState(const QJsonObject& state)
{
    savedState_ = state;
    qDebug() << "Game state saved for reconnection";
}

void ReconnectionManager::clearState()
{
    savedState_ = QJsonObject();
    qDebug() << "Saved state cleared";
}

// ==================== Timing ====================

int ReconnectionManager::getNextRetryDelay() const
{
    return calculateNextDelay();
}

void ReconnectionManager::resetAttempts()
{
    currentAttempt_ = 0;
    qDebug() << "Retry attempts reset";
}

uint64_t ReconnectionManager::getTimeSinceDisconnect() const
{
    if (disconnectTime_ == 0) {
        return 0;
    }
    return QDateTime::currentMSecsSinceEpoch() - disconnectTime_;
}

// ==================== Private Methods ====================

int ReconnectionManager::calculateNextDelay() const
{
    // Reference: Egaroucid ggs.hpp exponential backoff pattern
    // Exponential backoff: 1s, 2s, 4s, 8s, 16s, ...
    
    int delay = static_cast<int>(INITIAL_DELAY * std::pow(BACKOFF_MULTIPLIER, currentAttempt_));
    
    // Cap at maximum delay
    if (delay > MAX_DELAY) {
        delay = MAX_DELAY;
    }
    
    // Add small jitter (+/- 10%)
    int jitter = delay / 10;
    delay += QRandomGenerator::global()->bounded(-jitter, jitter);
    
    return delay;
}

void ReconnectionManager::updateDisconnectTime()
{
    disconnectTime_ = QDateTime::currentMSecsSinceEpoch();
}

void ReconnectionManager::onRetryTimeout()
{
    currentAttempt_++;
    
    int delay = getNextRetryDelay();
    
    emit reconnectAttempt(currentAttempt_, delay);
    
    // Schedule next attempt if not at max
    if (currentAttempt_ < maxAttempts_) {
        reconnectTimer_->start(delay);
    }
}

} // namespace Network

