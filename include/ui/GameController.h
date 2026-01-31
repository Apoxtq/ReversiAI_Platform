/**
 * @file GameController.h
 * @brief 游戏状态控制器
 *
 * 封装游戏状态管理、回合控制、AI集成
 * 基于QtReversi/widget.h的状态设计模式
 *
 * @reference QtReversi/代码/chess/widget.h - 游戏状态定义和信号槽设计
 * @reference Reversi(Java)/src/game/GamePanel.java - manageTurn()回合管理逻辑
 */

#pragma once

#include <QObject>
#include <memory>
#include <vector>
#include <chrono>

// 包含核心类型定义（在Reversi命名空间内）
#include "Board.h"
#include "ai/AIStrategy.h"

// 前向声明，避免循环引用
namespace Ui {
class MainWindow;
}

namespace Reversi {

// 前向声明Board
class Board;

// 前向声明AIStrategy
class AIStrategy;

/**
 * @enum GameMode
 * @brief 游戏模式
 */
enum class GameMode {
    PvP,      // 双人对战 (同一台电脑)
    PvE,      // 人机对战 (人类 vs AI)
    AIvAI     // AI对战 (AI1 vs AI2)
};

/**
 * @enum GamePhase
 * @brief 游戏阶段
 */
enum class GamePhase {
    Waiting,    // 等待开始
    HumanTurn,  // 人类玩家回合
    AITurn,     // AI回合
    GameOver    // 游戏结束
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
 * @class GameController
 * @brief 游戏控制器 - 封装游戏状态管理
 *
 * 核心职责：
 * - 管理游戏状态（棋盘、回合、阶段）
 * - 处理玩家操作（人类落子）
 * - 协调AI执行（AI移动、状态显示）
 * - 回合切换逻辑
 *
 * 信号槽设计参考 QtReversi/widget.cpp:
 * - 使用Qt信号槽机制解耦UI和逻辑
 * - 状态变化时发出信号，由UI更新显示
 */
class GameController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit GameController(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~GameController() override;

    // ============ 游戏控制 ============

    /**
     * @brief 开始新游戏
     * @param mode 游戏模式
     * @param humanColor 人类玩家颜色
     * @param difficulty AI难度 (PvE/AIvAI模式有效)
     *
     * @reference QtReversi/widget.cpp:changeRole() - 角色切换逻辑
     */
    void startNewGame(GameMode mode, PlayerColor humanColor,
                      Difficulty difficulty = Difficulty::MEDIUM);

    /**
     * @brief 重置游戏
     */
    void resetGame();

    /**
     * @brief 悔棋
     */
    void undoMove();

    /**
     * @brief 是否可以悔棋
     */
    bool canUndo() const;

    // ============ 状态查询 ============

    const Board& getBoard() const { return *board_; }
    GamePhase getCurrentPhase() const { return currentPhase_; }
    GameMode getGameMode() const { return gameMode_; }
    PlayerColor getCurrentPlayer() const { return currentPlayer_; }
    PlayerColor getHumanColor() const { return humanColor_; }

    /**
     * @brief 获取合法移动列表
     */
    std::vector<Move> getValidMoves() const;

    // ============ 玩家操作 ============

    /**
     * @brief 人类玩家落子
     * @param row 行 (0-7)
     * @param col 列 (0-7)
     * @return 是否落子成功
     *
     * @reference QtReversi/widget.cpp:mousePressEvent() - 鼠标点击处理
     * @reference Reversi(Java)/GamePanel.java:makeHumanMove() - 人类落子逻辑
     */
    bool makeHumanMove(int row, int col);

    /**
     * @brief 检查位置是否可落子
     */
    bool isValidMove(int row, int col) const;

signals:
    // ============ 状态变化信号 ============

    /**
     * @brief 游戏开始信号
     */
    void gameStarted(GameMode mode, PlayerColor humanColor);

    /**
     * @brief 游戏阶段变化信号
     */
    void phaseChanged(GamePhase phase);

    /**
     * @brief 回合变化信号
     */
    void turnChanged(PlayerColor player);

    /**
     * @brief 落子信号
     * @param row 行
     * @param col 列
     * @param player 执行落子的玩家
     */
    void moveMade(int row, int col, PlayerColor player);

    /**
     * @brief 游戏结束信号
     */
    void gameEnded(GameResult result);

    // ============ AI相关信号 ============

    /**
     * @brief AI开始思考信号
     */
    void aiThinkingStarted(const QString& aiName);

    /**
     * @brief AI思考完成信号
     */
    void aiThinkingFinished(int row, int col);

    /**
     * @brief AI统计更新信号
     */
    void aiStatsUpdated(const AIStats& stats);

    // ============ 错误信号 ============

    /**
     * @brief 错误发生信号
     */
    void errorOccurred(const QString& message);

private slots:
    /**
     * @brief 执行AI移动
     */
    void executeAIMove();

private:
    // ============ 私有方法 ============

    /**
     * @brief 切换回合
     *
     * @reference Reversi(Java)/GamePanel.java:manageTurn() - 回合管理逻辑
     */
    void switchTurn();

    /**
     * @brief 检查游戏是否结束
     */
    void checkGameOver();

    /**
     * @brief 保存移动历史（用于悔棋）
     */
    void saveMoveForUndo();

    /**
     * @brief 发射状态变化信号
     */
    void emitStatusSignals();

    // ============ 成员变量 ============

    std::unique_ptr<Board> board_;           // 棋盘状态
    std::unique_ptr<AIStrategy> ai_;         // AI策略 (PvE/AIvAI模式)
    GameMode gameMode_;                      // 游戏模式
    GamePhase currentPhase_;                 // 当前阶段
    PlayerColor currentPlayer_;              // 当前玩家
    PlayerColor humanColor_;                 // 人类玩家颜色

    // 悔棋支持 (最多10步)
    std::vector<std::unique_ptr<Board>> moveHistory_;

    // 禁止拷贝
    Q_DISABLE_COPY(GameController)
};

} // namespace Reversi

