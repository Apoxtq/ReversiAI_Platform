/**
 * @file GameRecord.h
 * @brief Game Record System - v0.9.0 Visualization Enhanced
 *
 * Provides game record, storage, import/export functionality
 * Supports JSON, PGN, SGF and other standard formats
 */

#pragma once

#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QDateTime>
#include <QStringList>

// 前向声明
namespace Reversi {
class Board;
}

namespace Reversi {

/**
 * @brief Single move record
 */
struct MoveRecord {
    int move;                      // Move (0-63)
    int player;                    // Player (0=black, 1=white)
    int discCountBlack;            // Black piece count
    int discCountWhite;            // White piece count
    double aiEvaluation;           // AI evaluation value
    int searchDepth;               // Search depth
    int64_t nodesSearched;         // Searched nodes
    double thinkingTime;           // Thinking time (milliseconds)
    QDateTime timestamp;           // Timestamp

    MoveRecord() : move(-1), player(0), discCountBlack(0),
                  discCountWhite(0), aiEvaluation(0.0),
                  searchDepth(0), nodesSearched(0), thinkingTime(0.0) {}

    /**
     * @brief Convert to JSON object
     */
    QJsonObject toJson() const;

    /**
     * @brief Create from JSON object
     */
    static MoveRecord fromJson(const QJsonObject& json);

    /**
     * @brief Get board coordinate string (e.g. "D3")
     */
    QString toCoordinateString() const;
};

/**
 * @brief Complete game record
 */
struct GameRecord {
    QString recordId;              // Record unique ID
    QDateTime startTime;          // Start time
    QDateTime endTime;            // End time

    QString player1Type;           // Player 1 type (Human/AI/Random)
    QString player1Name;          // Player 1 name/algorithm name
    QString player1Difficulty;     // Player 1 difficulty (Easy/Medium/Hard)

    QString player2Type;          // Player 2 type
    QString player2Name;          // Player 2 name/algorithm name
    QString player2Difficulty;     // Player 2 difficulty

    int winner;                   // Winner (0=black, 1=white, 2=draw, -1=unknown)
    int finalBlack;               // Final black count
    int finalWhite;               // Final white count

    QVector<MoveRecord> moves;    // Move sequence

    // Additional info
    QString gameMode;              // Game mode (PvE/PvP/AIvsAI/Network)
    QString tournament;            // Tournament name
    QString event;                // Event
    QString venue;                // Venue

    GameRecord() : winner(-1), finalBlack(0), finalWhite(0),
                   gameMode("PvE"), tournament(""), event(""), venue("") {}

    /**
     * @brief Convert to JSON object
     */
    QJsonObject toJson() const;

    /**
     * @brief Create from JSON object
     */
    static GameRecord fromJson(const QJsonObject& json);

    /**
     * @brief Export as PGN format
     */
    QString toPGN() const;

    /**
     * @brief Export as SGF format
     */
    QString toSGF() const;

    /**
     * @brief Import from PGN format
     */
    static GameRecord fromPGN(const QString& pgn);

    /**
     * @brief Import from SGF format
     */
    static GameRecord fromSGF(const QString& sgf);

    /**
     * @brief Get total moves
     */
    int getTotalMoves() const { return moves.size(); }

    /**
     * @brief Get game duration (milliseconds)
     */
    qint64 getDurationMs() const {
        return startTime.msecsTo(endTime);
    }

    /**
     * @brief Get winner name
     */
    QString getWinnerName() const {
        if (winner == 0) return player1Name;
        if (winner == 1) return player2Name;
        if (winner == 2) return "Draw";
        return "Unknown";
    }
};

/**
 * @brief Game replay controller (simplified, without Qt signals)
 */
class GameReplay {
public:
    /**
     * @brief Constructor
     */
    GameReplay();

    /**
     * @brief Load game record
     */
    bool loadRecord(const GameRecord& record);

    /**
     * @brief Get current game record
     */
    const GameRecord& getRecord() const { return record_; }

    /**
     * @brief Play
     */
    void play();

    /**
     * @brief Pause
     */
    void pause();

    /**
     * @brief Stop (back to start)
     */
    void stop();

    /**
     * @brief Step forward
     */
    void stepForward();

    /**
     * @brief Step backward
     */
    void stepBackward();

    /**
     * @brief Jump to specified move
     */
    void jumpToMove(int moveIndex);

    /**
     * @brief Set playback speed (0.25x - 4.0x)
     */
    void setPlaybackSpeed(double speed);

    /**
     * @brief Get current move index
     */
    int getCurrentMoveIndex() const { return currentMoveIndex_; }

    /**
     * @brief Get current move record
     */
    const MoveRecord& getCurrentMove() const;

    /**
     * @brief Get current board state
     */
    class Board* getCurrentBoard() const { return currentBoard_; }

    /**
     * @brief Whether playing
     */
    bool isPlaying() const { return isPlaying_; }

    /**
     * @brief Whether playback finished
     */
    bool isFinished() const { return currentMoveIndex_ >= record_.moves.size(); }

    /**
     * @brief Update callback function types
     */
    using BoardUpdateCallback = std::function<void(class Board*)>;
    using MoveChangeCallback = std::function<void(int, const MoveRecord&)>;
    using PlaybackFinishedCallback = std::function<void()>;
    using PlayStateChangedCallback = std::function<void(bool)>;

    /**
     * @brief Set board update callback
     */
    void setBoardUpdateCallback(BoardUpdateCallback cb) { boardUpdateCallback_ = cb; }

    /**
     * @brief Set move change callback
     */
    void setMoveChangeCallback(MoveChangeCallback cb) { moveChangeCallback_ = cb; }

    /**
     * @brief Set playback finished callback
     */
    void setPlaybackFinishedCallback(PlaybackFinishedCallback cb) { playbackFinishedCallback_ = cb; }

    /**
     * @brief Set play state changed callback
     */
    void setPlayStateChangedCallback(PlayStateChangedCallback cb) { playStateChangedCallback_ = cb; }

    /**
     * @brief Handle timer trigger (for external timer)
     */
    void onTimerTriggered();

private:
    /**
     * @brief Rebuild board to specified move
     */
    void rebuildBoard(int moveIndex);

    /**
     * @brief Update timer interval
     */
    void updateTimerInterval();

    GameRecord record_;                    // Game record
    class Board* currentBoard_;            // Current board state
    int currentMoveIndex_;                 // Current move index
    bool isPlaying_;                       // Whether playing
    double playbackSpeed_;                 // Playback speed
    int timerInterval_;                   // Timer interval (milliseconds)

    static const int DEFAULT_INTERVAL_MS = 1000;  // Default interval (milliseconds)

    // Callback functions
    BoardUpdateCallback boardUpdateCallback_;
    MoveChangeCallback moveChangeCallback_;
    PlaybackFinishedCallback playbackFinishedCallback_;
    PlayStateChangedCallback playStateChangedCallback_;
};

} // namespace Reversi
