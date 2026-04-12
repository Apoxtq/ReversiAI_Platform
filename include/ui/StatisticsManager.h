/**
 * @file StatisticsManager.h
 * @brief Game Statistics Manager
 *
 * Records game history, calculates win rate, exports statistics
 *
 * @reference Reference Egaroucid project statistics design
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QTextStream>
#include "Board.h"

namespace Reversi {

/**
 * @enum GameMode
 * @brief Game mode
 */
enum class GameMode {
    PvP,      // Two-player
    PvE,      // Human vs AI
    AIvAI     // AI vs AI
};

/**
 * @enum GameResult
 * @brief Game result
 */
enum class GameResult {
    BlackWins,
    WhiteWins,
    Draw,
    Unknown
};

/**
 * @enum Difficulty
 * @brief AI difficulty level
 */
enum class Difficulty {
    EASY,
    MEDIUM,
    HARD,
    EXPERT
};

/**
 * @class StatisticsManager
 * @brief Game statistics manager
 *
 * Features:
 * - Record each game result
 * - Query history records
 * - Calculate win rate statistics
 * - Export CSV/JSON format
 */
class StatisticsManager : public QObject {
    Q_OBJECT

public:
    /**
     * @struct GameRecord
     * @brief Game record
     */
    struct GameRecord {
        QDateTime timestamp;           // Game start time
        GameMode mode;                 // Game mode
        QString aiType;                // AI type used
        Difficulty difficulty;          // Difficulty level
        GameResult result;             // Game result
        int blackCount;                // Black final count
        int whiteCount;                // White final count
        int moveCount;                 // Total moves
        int durationSeconds;           // Game duration (seconds)
        QString humanColor;            // Human player color
        bool humanWon;                  // Human won
    };

    explicit StatisticsManager(QObject* parent = nullptr);
    ~StatisticsManager() override;

    // ============ Record Games ============

    /**
     * @brief Record a game
     * @param record Game record
     */
    void recordGame(const GameRecord& record);

    /**
     * @brief Create record from current game state
     */
    GameRecord createRecordFromCurrentGame(
        GameMode mode,
        const QString& aiType,
        Difficulty difficulty,
        const Board& finalBoard,
        int moveCount,
        int durationSeconds,
        const QString& humanColor
    );

    // ============ Query History ============

    /**
     * @brief Get the most recent N games
     */
    QVector<GameRecord> getRecentGames(int count = 10) const;

    /**
     * @brief Filter games by mode
     */
    QVector<GameRecord> getGamesByMode(GameMode mode) const;

    /**
     * @brief Filter games by AI type
     */
    QVector<GameRecord> getGamesByAI(const QString& aiType) const;

    /**
     * @brief Get all records
     */
    const QVector<GameRecord>& getAllGames() const { return history_; }

    // ============ Statistics Data ============

    /**
     * @brief Get total games
     */
    int getTotalGames() const;

    /**
     * @brief Get wins
     * @param aiType If specified, only count wins for this AI
     */
    int getWins(const QString& aiType = QString()) const;

    /**
     * @brief Get losses
     */
    int getLosses(const QString& aiType = QString()) const;

    /**
     * @brief Get draws
     */
    int getDraws(const QString& aiType = QString()) const;

    /**
     * @brief Get win rate
     * @param aiType If specified, only count win rate for this AI
     * @return Win rate (0.0 - 1.0), returns -1.0 if no data
     */
    double getWinRate(const QString& aiType = QString()) const;

    /**
     * @brief Get average moves
     */
    double getAverageMoves() const;

    /**
     * @brief Get average game duration (seconds)
     */
    double getAverageDuration() const;

    /**
     * @brief Clear all history records
     */
    void clearHistory();

public slots:
    /**
     * @brief Auto record when game ends
     */
    void onGameEnded(GameResult result, int blackCount, int whiteCount,
                     int moveCount, const QString& humanColor);

signals:
    /**
     * @brief Statistics data update signal
     */
    void statsUpdated();

    /**
     * @brief New game record signal
     */
    void newGameRecorded(const GameRecord& record);

    // ============ Export Signals ============

    /**
     * @brief Export completed signal
     */
    void exportCompleted(bool success, const QString& filename);

    /**
     * @brief Import completed signal
     */
    void importCompleted(bool success, int gamesImported);

    /**
     * @brief Error signal
     */
    void errorOccurred(const QString& message);

public:
    // ============ Export Functions ============

    /**
     * @brief Export to CSV format
     */
    bool exportToCSV(const QString& filename);

    /**
     * @brief Export to JSON format
     */
    bool exportToJSON(const QString& filename);

    /**
     * @brief Import from JSON
     */
    bool importFromJSON(const QString& filename);

    // ============ Persistence ============

    /**
     * @brief Save history to file
     */
    bool saveToFile(const QString& filename = QString());

    /**
     * @brief Load history from file
     */
    bool loadFromFile(const QString& filename = QString());

    /**
     * @brief Get default history file path
     */
    static QString getDefaultHistoryPath();

private:
    /**
     * @brief Convert GameResult to string
     */
    static QString gameResultToString(GameResult result);

    /**
     * @brief Convert string to GameResult
     */
    static GameResult stringToGameResult(const QString& str);

    /**
     * @brief Convert Difficulty to string
     */
    static QString difficultyToString(Difficulty difficulty);

    /**
     * @brief Convert string to Difficulty
     */
    static Difficulty stringToDifficulty(const QString& str);

    /**
     * @brief Convert GameMode to string
     */
    static QString gameModeToString(GameMode mode);

    /**
     * @brief Convert string to GameMode
     */
    static GameMode stringToGameMode(const QString& str);

    QVector<GameRecord> history_;       // Game history records
    QString historyFilePath_;           // History file path
    QDateTime gameStartTime_;           // Current game start time
    int currentMoveCount_;              // Current game move count
};

} // namespace Reversi

