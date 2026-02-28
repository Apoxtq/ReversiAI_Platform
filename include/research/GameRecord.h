/**
 * @file GameRecord.h
 * @brief 对局记录系统 - v0.9.0可视化增强版
 *
 * 提供棋谱记录、存储、导入导出功能
 * 支持JSON、PGN、SGF等标准格式
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
 * @brief 单步棋谱记录
 */
struct MoveRecord {
    int move;                      // 走法 (0-63)
    int player;                    // 玩家 (0=黑, 1=白)
    int discCountBlack;            // 黑子数
    int discCountWhite;            // 白子数
    double aiEvaluation;           // AI评估值
    int searchDepth;               // 搜索深度
    int64_t nodesSearched;         // 搜索节点数
    double thinkingTime;           // 思考时间(毫秒)
    QDateTime timestamp;           // 时间戳

    MoveRecord() : move(-1), player(0), discCountBlack(0),
                  discCountWhite(0), aiEvaluation(0.0),
                  searchDepth(0), nodesSearched(0), thinkingTime(0.0) {}

    /**
     * @brief 转换为JSON对象
     */
    QJsonObject toJson() const;

    /**
     * @brief 从JSON对象创建
     */
    static MoveRecord fromJson(const QJsonObject& json);

    /**
     * @brief 获取走法的棋盘坐标字符串 (如 "D3")
     */
    QString toCoordinateString() const;
};

/**
 * @brief 完整对局记录
 */
struct GameRecord {
    QString recordId;              // 记录唯一ID
    QDateTime startTime;           // 开始时间
    QDateTime endTime;             // 结束时间

    QString player1Type;           // 玩家1类型 (Human/AI/Random)
    QString player1Name;           // 玩家1名称/算法名
    QString player1Difficulty;     // 玩家1难度 (Easy/Medium/Hard)

    QString player2Type;           // 玩家2类型
    QString player2Name;          // 玩家2名称/算法名
    QString player2Difficulty;     // 玩家2难度

    int winner;                    // 胜者 (0=黑, 1=白, 2=平局, -1=未知)
    int finalBlack;                // 最终黑子数
    int finalWhite;                // 最终白子数

    QVector<MoveRecord> moves;     // 走法序列

    // 附加信息
    QString gameMode;              // 游戏模式 (PvE/PvP/AIvsAI/Network)
    QString tournament;             // 比赛名称
    QString event;                 // 赛事/事件
    QString venue;                // 地点

    GameRecord() : winner(-1), finalBlack(0), finalWhite(0),
                   gameMode("PvE"), tournament(""), event(""), venue("") {}

    /**
     * @brief 转换为JSON对象
     */
    QJsonObject toJson() const;

    /**
     * @brief 从JSON对象创建
     */
    static GameRecord fromJson(const QJsonObject& json);

    /**
     * @brief 导出为PGN格式
     */
    QString toPGN() const;

    /**
     * @brief 导出为SGF格式
     */
    QString toSGF() const;

    /**
     * @brief 从PGN格式导入
     */
    static GameRecord fromPGN(const QString& pgn);

    /**
     * @brief 从SGF格式导入
     */
    static GameRecord fromSGF(const QString& sgf);

    /**
     * @brief 获取总回合数
     */
    int getTotalMoves() const { return moves.size(); }

    /**
     * @brief 获取对局时长(毫秒)
     */
    qint64 getDurationMs() const {
        return startTime.msecsTo(endTime);
    }

    /**
     * @brief 获取胜者名称
     */
    QString getWinnerName() const {
        if (winner == 0) return player1Name;
        if (winner == 1) return player2Name;
        if (winner == 2) return "Draw";
        return "Unknown";
    }
};

/**
 * @brief 对局复盘控制器 (简化版，不使用Qt信号)
 */
class GameReplay {
public:
    /**
     * @brief 构造函数
     */
    GameReplay();

    /**
     * @brief 加载对局记录
     */
    bool loadRecord(const GameRecord& record);

    /**
     * @brief 获取当前对局记录
     */
    const GameRecord& getRecord() const { return record_; }

    /**
     * @brief 播放
     */
    void play();

    /**
     * @brief 暂停
     */
    void pause();

    /**
     * @brief 停止(回到开始)
     */
    void stop();

    /**
     * @brief 前进一步
     */
    void stepForward();

    /**
     * @brief 后退一步
     */
    void stepBackward();

    /**
     * @brief 跳转到指定步数
     */
    void jumpToMove(int moveIndex);

    /**
     * @brief 设置播放速度 (0.25x - 4.0x)
     */
    void setPlaybackSpeed(double speed);

    /**
     * @brief 获取当前步数索引
     */
    int getCurrentMoveIndex() const { return currentMoveIndex_; }

    /**
     * @brief 获取当前走法记录
     */
    const MoveRecord& getCurrentMove() const;

    /**
     * @brief 获取当前棋盘状态
     */
    class Board* getCurrentBoard() const { return currentBoard_; }

    /**
     * @brief 是否正在播放
     */
    bool isPlaying() const { return isPlaying_; }

    /**
     * @brief 是否播放完毕
     */
    bool isFinished() const { return currentMoveIndex_ >= record_.moves.size(); }

    /**
     * @brief 更新回调函数类型
     */
    using BoardUpdateCallback = std::function<void(class Board*)>;
    using MoveChangeCallback = std::function<void(int, const MoveRecord&)>;
    using PlaybackFinishedCallback = std::function<void()>;
    using PlayStateChangedCallback = std::function<void(bool)>;

    /**
     * @brief 设置棋盘更新回调
     */
    void setBoardUpdateCallback(BoardUpdateCallback cb) { boardUpdateCallback_ = cb; }

    /**
     * @brief 设置走法变化回调
     */
    void setMoveChangeCallback(MoveChangeCallback cb) { moveChangeCallback_ = cb; }

    /**
     * @brief 设置播放完毕回调
     */
    void setPlaybackFinishedCallback(PlaybackFinishedCallback cb) { playbackFinishedCallback_ = cb; }

    /**
     * @brief 设置播放状态变化回调
     */
    void setPlayStateChangedCallback(PlayStateChangedCallback cb) { playStateChangedCallback_ = cb; }

    /**
     * @brief 处理定时器触发(供外部定时器调用)
     */
    void onTimerTriggered();

private:
    /**
     * @brief 重建棋盘到指定步数
     */
    void rebuildBoard(int moveIndex);

    /**
     * @brief 更新定时器间隔
     */
    void updateTimerInterval();

    GameRecord record_;                    // 对局记录
    class Board* currentBoard_;            // 当前棋盘状态
    int currentMoveIndex_;                 // 当前步数索引
    bool isPlaying_;                       // 是否正在播放
    double playbackSpeed_;                 // 播放速度
    int timerInterval_;                   // 定时器间隔(毫秒)

    static const int DEFAULT_INTERVAL_MS = 1000;  // 默认间隔(毫秒)

    // 回调函数
    BoardUpdateCallback boardUpdateCallback_;
    MoveChangeCallback moveChangeCallback_;
    PlaybackFinishedCallback playbackFinishedCallback_;
    PlayStateChangedCallback playStateChangedCallback_;
};

} // namespace Reversi
