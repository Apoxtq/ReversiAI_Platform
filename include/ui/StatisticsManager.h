/**
 * @file StatisticsManager.h
 * @brief 游戏统计管理器
 *
 * 记录游戏历史、计算胜率、导出统计
 *
 * @reference 参考Egaroucid项目的统计功能设计
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
 * @brief 游戏模式
 */
enum class GameMode {
    PvP,      // 双人对战
    PvE,      // 人机对战
    AIvAI     // AI对战
};

/**
 * @enum GameResult
 * @brief 游戏结果
 */
enum class GameResult {
    BlackWins,
    WhiteWins,
    Draw,
    Unknown
};

/**
 * @enum Difficulty
 * @brief AI难度等级
 */
enum class Difficulty {
    EASY,
    MEDIUM,
    HARD,
    EXPERT
};

/**
 * @class StatisticsManager
 * @brief 游戏统计管理器
 *
 * 功能：
 * - 记录每局游戏的结果
 * - 查询历史记录
 * - 计算胜率统计
 * - 导出CSV/JSON格式
 */
class StatisticsManager : public QObject {
    Q_OBJECT

public:
    /**
     * @struct GameRecord
     * @brief 游戏记录
     */
    struct GameRecord {
        QDateTime timestamp;           // 游戏开始时间
        GameMode mode;                 // 游戏模式
        QString aiType;                // 使用的AI类型
        Difficulty difficulty;         // 难度等级
        GameResult result;             // 游戏结果
        int blackCount;                // 黑棋最终数量
        int whiteCount;                // 白棋最终数量
        int moveCount;                 // 总回合数
        int durationSeconds;           // 游戏时长（秒）
        QString humanColor;            // 人类玩家颜色
        bool humanWon;                 // 人类是否获胜
    };

    explicit StatisticsManager(QObject* parent = nullptr);
    ~StatisticsManager() override;

    // ============ 记录游戏 ============

    /**
     * @brief 记录一局游戏
     * @param record 游戏记录
     */
    void recordGame(const GameRecord& record);

    /**
     * @brief 从当前游戏状态创建记录
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

    // ============ 查询历史 ============

    /**
     * @brief 获取最近的N局游戏
     */
    QVector<GameRecord> getRecentGames(int count = 10) const;

    /**
     * @brief 按模式筛选游戏
     */
    QVector<GameRecord> getGamesByMode(GameMode mode) const;

    /**
     * @brief 按AI类型筛选游戏
     */
    QVector<GameRecord> getGamesByAI(const QString& aiType) const;

    /**
     * @brief 获取所有记录
     */
    const QVector<GameRecord>& getAllGames() const { return history_; }

    // ============ 统计数据 ============

    /**
     * @brief 获取总局数
     */
    int getTotalGames() const;

    /**
     * @brief 获取获胜局数
     * @param aiType 如果指定，只统计该AI的胜局
     */
    int getWins(const QString& aiType = QString()) const;

    /**
     * @brief 获取失败局数
     */
    int getLosses(const QString& aiType = QString()) const;

    /**
     * @brief 获取平局数
     */
    int getDraws(const QString& aiType = QString()) const;

    /**
     * @brief 获取胜率
     * @param aiType 如果指定，只统计该AI的胜率
     * @return 胜率（0.0 - 1.0），返回-1.0表示无数据
     */
    double getWinRate(const QString& aiType = QString()) const;

    /**
     * @brief 获取平均回合数
     */
    double getAverageMoves() const;

    /**
     * @brief 获取平均游戏时长（秒）
     */
    double getAverageDuration() const;

    /**
     * @brief 清除所有历史记录
     */
    void clearHistory();

public slots:
    /**
     * @brief 游戏结束时自动记录
     */
    void onGameEnded(GameResult result, int blackCount, int whiteCount,
                     int moveCount, const QString& humanColor);

signals:
    /**
     * @brief 统计数据更新信号
     */
    void statsUpdated();

    /**
     * @brief 新游戏记录信号
     */
    void newGameRecorded(const GameRecord& record);

    // ============ 导出信号 ============

    /**
     * @brief 导出完成信号
     */
    void exportCompleted(bool success, const QString& filename);

    /**
     * @brief 导入完成信号
     */
    void importCompleted(bool success, int gamesImported);

    /**
     * @brief 错误信号
     */
    void errorOccurred(const QString& message);

public:
    // ============ 导出功能 ============

    /**
     * @brief 导出为CSV格式
     */
    bool exportToCSV(const QString& filename);

    /**
     * @brief 导出为JSON格式
     */
    bool exportToJSON(const QString& filename);

    /**
     * @brief 从JSON导入
     */
    bool importFromJSON(const QString& filename);

    // ============ 持久化 ============

    /**
     * @brief 保存历史到文件
     */
    bool saveToFile(const QString& filename = QString());

    /**
     * @brief 从文件加载历史
     */
    bool loadFromFile(const QString& filename = QString());

    /**
     * @brief 获取默认历史文件路径
     */
    static QString getDefaultHistoryPath();

private:
    /**
     * @brief 将GameResult转换为字符串
     */
    static QString gameResultToString(GameResult result);

    /**
     * @brief 将字符串转换为GameResult
     */
    static GameResult stringToGameResult(const QString& str);

    /**
     * @brief 将Difficulty转换为字符串
     */
    static QString difficultyToString(Difficulty difficulty);

    /**
     * @brief 将字符串转换为Difficulty
     */
    static Difficulty stringToDifficulty(const QString& str);

    /**
     * @brief 将GameMode转换为字符串
     */
    static QString gameModeToString(GameMode mode);

    /**
     * @brief 将字符串转换为GameMode
     */
    static GameMode stringToGameMode(const QString& str);

    QVector<GameRecord> history_;       // 游戏历史记录
    QString historyFilePath_;           // 历史文件路径
    QDateTime gameStartTime_;           // 当前游戏开始时间
    int currentMoveCount_;              // 当前游戏回合数
};

} // namespace Reversi

