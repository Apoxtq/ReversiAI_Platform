/**
 * @file PvPWindow.cpp
 * @brief 双人对战窗口实现
 *
 * 功能:
 * - 双人同机对战
 * - 轮流执黑/白棋
 * - 悔棋功能
 * - 返回菜单功能
 *
 * @reference QtReversi/代码/chess/widget.cpp - 游戏规则和界面交互
 */

#include "ui/PvPWindow.h"
#include "ui_pvpwindow.h"
#include "QPainter"
#include "QPixmap"
#include "QMouseEvent"
#include "QDebug"

PvPWindow::PvPWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::PvPWindow) {
    ui->setupUi(this);

    // 初始化UI组件
    initUI();

    // 创建GameController（PvP模式不需要AI）
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
            this, &PvPWindow::onStartGameClicked);
    connect(ui->undoButton, &QPushButton::clicked,
            this, &PvPWindow::onUndoClicked);
    connect(ui->backButton, &QPushButton::clicked,
            this, &PvPWindow::onBackToMenuClicked);
}

PvPWindow::~PvPWindow() {
    delete ui;
}

void PvPWindow::initUI() {
    // init() 在 ui_pvpwindow.h 中定义，由 uic 自动生成
}

void PvPWindow::setupConnections() {
    // GameController 信号槽连接（PvP不需要AI相关信号）
    connect(gameController_.get(), &Reversi::GameController::gameStarted,
            this, &PvPWindow::onGameStarted);
    connect(gameController_.get(), &Reversi::GameController::phaseChanged,
            this, &PvPWindow::onPhaseChanged);
    connect(gameController_.get(), &Reversi::GameController::turnChanged,
            this, &PvPWindow::onTurnChanged);
    connect(gameController_.get(), &Reversi::GameController::moveMade,
            this, &PvPWindow::onMoveMade);
    connect(gameController_.get(), &Reversi::GameController::gameEnded,
            this, &PvPWindow::onGameEnded);
    connect(gameController_.get(), &Reversi::GameController::errorOccurred,
            this, &PvPWindow::onErrorOccurred);
}

void PvPWindow::paintEvent(QPaintEvent* event) {
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
            if (markHaveDraw[j][i] == 2) {  // Black的合法位置
                painter.drawPixmap(0 + 50 * i, 0 + 50 * j, 50, 50, hintblack);
            }
            if (markHaveDraw[j][i] == 1) {  // White的合法位置
                painter.drawPixmap(0 + 50 * i, 0 + 50 * j, 50, 50, hintwhite);
            }
        }
    }
}

void PvPWindow::mousePressEvent(QMouseEvent* e) {
    // 检查游戏状态（双人模式下只有HumanTurn）
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

void PvPWindow::updateScoreDisplay() {
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

void PvPWindow::onStartGameClicked() {
    // PvP模式：黑棋先手，玩家1是黑棋，玩家2是白棋
    Reversi::GameMode mode = Reversi::GameMode::PvP;
    Reversi::PlayerColor humanColor = Reversi::PlayerColor::Black;  // 玩家1先手
    Reversi::Difficulty difficulty = Reversi::Difficulty::MEDIUM;   // PvP不需要AI

    qDebug() << "PvPWindow: Starting PvP game";

    // 开始游戏
    gameController_->startNewGame(mode, humanColor, difficulty);
}

void PvPWindow::onUndoClicked() {
    // 悔棋功能 - 撤销上一步
    qDebug() << "PvPWindow: Undo clicked";
    gameController_->undoMove();
    updateScoreDisplay();
    repaint();
}

void PvPWindow::onGameStarted(Reversi::GameMode mode, Reversi::PlayerColor humanColor) {
    qDebug() << "PvPWindow: Game started, mode:" << (int)mode
             << "first player:" << (int)humanColor;
    updateScoreDisplay();
    repaint();
}

void PvPWindow::onPhaseChanged(Reversi::GamePhase phase) {
    qDebug() << "PvPWindow: Phase changed to" << (int)phase;
    repaint();
}

void PvPWindow::onTurnChanged(Reversi::PlayerColor player) {
    qDebug() << "PvPWindow: Turn changed to" << (int)player;

    // 更新当前回合显示
    if (ui->turnLabel) {
        QString turnText = (player == Reversi::PlayerColor::Black) ?
                           "当前回合: Black" : "当前回合: White";
        ui->turnLabel->setText(turnText);
    }

    updateScoreDisplay();
    repaint();
}

void PvPWindow::onMoveMade(int row, int col, Reversi::PlayerColor player) {
    qDebug() << "PvPWindow: Move made at" << row << col << "by player" << (int)player;
    updateScoreDisplay();
    repaint();
}

void PvPWindow::onGameEnded(Reversi::GameResult result) {
    qDebug() << "PvPWindow: Game ended, result:" << (int)result;

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
    if (ui->turnLabel) {
        ui->turnLabel->setText(resultText);
    }

    updateScoreDisplay();
    repaint();
}

void PvPWindow::onErrorOccurred(const QString& message) {
    qDebug() << "PvPWindow: Error -" << message;
}

void PvPWindow::onBackToMenuClicked() {
    emit backToMenu();
}

