/**
 * @file PvEWindow.cpp
 * @brief 人机对战窗口实现
 *
 * 功能:
 * - 人机对战模式
 * - AI难度选择
 * - 先手/后手选择
 * - 返回菜单功能
 *
 * @reference QtReversi/代码/chess/widget.cpp - 游戏规则和界面交互
 * @reference MCTS-AI-Reversi/mainwindow.cpp - paintEvent()实现保留
 */

#include "ui/PvEWindow.h"
#include "ui_pvewindow.h"
#include "QPainter"
#include "QPixmap"
#include "QMouseEvent"
#include "QDebug"
#include "QTimer"

PvEWindow::PvEWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::PvEWindow) {
    ui->setupUi(this);

    // 初始化UI组件
    initUI();

    // 创建GameController
    gameController_ = std::make_unique<Reversi::GameController>(this);

    // 连接信号槽
    setupConnections();

    // 加载资源文件
    background.load(":/rsc/board.png");
    black.load(":/rsc/black.png");
    white.load(":/rsc/white.png");
    hintwhite.load(":/rsc/whitepotential.png");
    hintblack.load(":/rsc/blackpotential.png");
    hintred.load(":/rsc/redpotential.png");

    // 连接按钮信号
    connect(ui->pushButton, &QPushButton::clicked,
            this, &PvEWindow::onStartGameClicked);
    connect(ui->backButton, &QPushButton::clicked,
            this, &PvEWindow::onBackToMenuClicked);
}

PvEWindow::~PvEWindow() {
    delete ui;
}

void PvEWindow::initUI() {
    // init() 在 ui_pvewindow.h 中定义，由 uic 自动生成
}

void PvEWindow::setupConnections() {
    // GameController 信号槽连接
    connect(gameController_.get(), &Reversi::GameController::gameStarted,
            this, &PvEWindow::onGameStarted);
    connect(gameController_.get(), &Reversi::GameController::phaseChanged,
            this, &PvEWindow::onPhaseChanged);
    connect(gameController_.get(), &Reversi::GameController::turnChanged,
            this, &PvEWindow::onTurnChanged);
    connect(gameController_.get(), &Reversi::GameController::moveMade,
            this, &PvEWindow::onMoveMade);
    connect(gameController_.get(), &Reversi::GameController::gameEnded,
            this, &PvEWindow::onGameEnded);
    connect(gameController_.get(), &Reversi::GameController::aiThinkingStarted,
            this, &PvEWindow::onAIThinkingStarted);
    connect(gameController_.get(), &Reversi::GameController::aiThinkingFinished,
            this, &PvEWindow::onAIThinkingFinished);
    connect(gameController_.get(), &Reversi::GameController::aiStatsUpdated,
            this, &PvEWindow::onAIStatsUpdated);
    connect(gameController_.get(), &Reversi::GameController::errorOccurred,
            this, &PvEWindow::onErrorOccurred);
}

void PvEWindow::paintEvent(QPaintEvent* event) {
    this->resize(600, 400);
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawPixmap(0, 0, 400, 400, background);

    // 从GameController获取当前状态
    const Reversi::Board& board = gameController_->getBoard();
    Reversi::PlayerColor currentPlayer = gameController_->getCurrentPlayer();

    // 确定当前显示的棋子颜色（合法移动高亮用）
    int tile = (currentPlayer == Reversi::PlayerColor::Black) ? 2 : 1;

    // 获取合法移动并绘制高亮
    int markHaveDraw[8][8] = {0};
    auto validMoves = board.getValidMoves();
    for (const auto& move : validMoves) {
        markHaveDraw[move.row][move.col] = tile;
    }

    // 绘制棋子和高亮
    // 注意：board.at(row, col) - row 是垂直方向(y)，col 是水平方向(x)
    // painter.drawPixmap(x, y) - x 是水平方向，y 是垂直方向
    // 所以应该用 board.at(j, i) 其中 j=row, i=col
    for (int i = 0; i < 8; i++) {  // i = col (水平方向)
        for (int j = 0; j < 8; j++) {  // j = row (垂直方向)
            int cellValue = board.at(j, i);  // j=row, i=col

            // 绘制棋子
            if (cellValue == 1) {  // White
                painter.drawPixmap(0 + 50 * i, 0 + 50 * j, 50, 50, white);
            } else if (cellValue == 2) {  // Black
                painter.drawPixmap(0 + 50 * i, 0 + 50 * j, 50, 50, black);
            }

            // 绘制合法移动高亮
            // markHaveDraw[row][col] -> markHaveDraw[j][i]
            if (markHaveDraw[j][i] == 2) {  // Black的合法位置
                painter.drawPixmap(0 + 50 * i, 0 + 50 * j, 50, 50, hintblack);
            }
            if (markHaveDraw[j][i] == 1) {  // White的合法位置
                painter.drawPixmap(0 + 50 * i, 0 + 50 * j, 50, 50, hintwhite);
            }
        }
    }
}

void PvEWindow::mousePressEvent(QMouseEvent* e) {
    // 检查游戏状态
    if (gameController_->getCurrentPhase() != Reversi::GamePhase::HumanTurn) {
        return;
    }

    // 坐标转换
    int row = e->y() / 50;
    int col = e->x() / 50;

    // 检查是否在棋盘范围内
    if (row < 0 || row >= 8 || col < 0 || col >= 8) {
        return;
    }

    // 使用GameController处理落子
    gameController_->makeHumanMove(row, col);
}

void PvEWindow::updateScoreDisplay() {
    // 从GameController获取分数
    const Reversi::Board& board = gameController_->getBoard();

    // 统计棋子数量
    int blackCount = 0;
    int whiteCount = 0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int cellValue = board.at(i, j);
            if (cellValue == 2) blackCount++;
            else if (cellValue == 1) whiteCount++;
        }
    }

    // 更新UI显示
    if (ui->blabel) {
        ui->blabel->setText(QString::number(blackCount));
    }
    if (ui->wlabel) {
        ui->wlabel->setText(QString::number(whiteCount));
    }
}

void PvEWindow::onStartGameClicked() {
    // 确定游戏模式：默认是人机对战 (PvE)
    Reversi::GameMode mode = Reversi::GameMode::PvE;
    Reversi::Difficulty difficulty = Reversi::Difficulty::MEDIUM;

    // 确定人类玩家颜色（根据UI选择谁先手）
    Reversi::PlayerColor humanColor;
    if (ui->AIchoice && ui->AIchoice->isChecked()) {
        humanColor = Reversi::PlayerColor::White;  // AI先手，人类是白棋
    } else if (ui->playerchoice && ui->playerchoice->isChecked()) {
        humanColor = Reversi::PlayerColor::Black;  // 玩家先手，人类是黑棋
    } else {
        // 默认玩家先手
        humanColor = Reversi::PlayerColor::Black;
    }

    qDebug() << "PvEWindow: Starting game, humanColor:" << (int)humanColor;

    // 开始游戏（始终是人机对战PvE模式）
    gameController_->startNewGame(mode, humanColor, difficulty);
}

void PvEWindow::onGameStarted(Reversi::GameMode mode, Reversi::PlayerColor humanColor) {
    qDebug() << "PvEWindow: Game started, mode:" << (int)mode
             << "humanColor:" << (int)humanColor;
    updateScoreDisplay();
    repaint();
}

void PvEWindow::onPhaseChanged(Reversi::GamePhase phase) {
    qDebug() << "PvEWindow: Phase changed to" << (int)phase;

    // 根据阶段更新UI
    switch (phase) {
    case Reversi::GamePhase::Waiting:
        break;
    case Reversi::GamePhase::HumanTurn:
        break;
    case Reversi::GamePhase::AITurn:
        break;
    case Reversi::GamePhase::GameOver:
        qDebug() << "PvEWindow: Game Over";
        break;
    }

    repaint();
}

void PvEWindow::onTurnChanged(Reversi::PlayerColor player) {
    qDebug() << "PvEWindow: Turn changed to" << (int)player;
    updateScoreDisplay();
    repaint();
}

void PvEWindow::onMoveMade(int row, int col, Reversi::PlayerColor player) {
    qDebug() << "PvEWindow: Move made at" << row << col << "by player" << (int)player;
    updateScoreDisplay();
    repaint();
}

void PvEWindow::onGameEnded(Reversi::GameResult result) {
    qDebug() << "PvEWindow: Game ended, result:" << (int)result;

    QString resultText;
    switch (result) {
    case Reversi::GameResult::BlackWins:
        resultText = "黑棋获胜!";
        break;
    case Reversi::GameResult::WhiteWins:
        resultText = "白棋获胜!";
        break;
    case Reversi::GameResult::Draw:
        resultText = "平局!";
        break;
    default:
        resultText = "游戏结束";
        break;
    }

    // 显示游戏结果
    if (ui->AIGO) {
        ui->AIGO->setText(resultText);
    }

    updateScoreDisplay();
    repaint();
}

void PvEWindow::onAIThinkingStarted(const QString& aiName) {
    qDebug() << "PvEWindow: AI thinking started:" << aiName;
    if (ui->AIGO) {
        ui->AIGO->setText("AI思考中: " + aiName);
    }
}

void PvEWindow::onAIThinkingFinished(int row, int col) {
    qDebug() << "PvEWindow: AI move finished at" << row << col;
    if (ui->AIGO) {
        ui->AIGO->setText(QString("AI落子: %1,%2").arg(col).arg(row));
    }
}

void PvEWindow::onAIStatsUpdated(const Reversi::AIStats& stats) {
    qDebug() << "PvEWindow: AI stats - nodes:" << stats.nodesExplored
             << "time:" << stats.timeUsed.count() << "ms";
}

void PvEWindow::onErrorOccurred(const QString& message) {
    qDebug() << "PvEWindow: Error -" << message;
}

void PvEWindow::onBackToMenuClicked() {
    emit backToMenu();
}

