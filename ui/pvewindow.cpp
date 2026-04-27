/**
 * @file PvEWindow.cpp
 * @brief PvE (Player vs AI) game window implementation
 */

#include "ui/PvEWindow.h"
#include <QPainter>
#include <QFont>
#include <QDebug>
#include <QApplication>
#include <QIcon>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>

PvEWindow::PvEWindow(QWidget* parent)
    : QMainWindow(parent)
    , centralWidget_(nullptr)
    , leftLayout_(nullptr)
    , rightLayout_(nullptr)
    , resultLabel_(nullptr)
    , turnIndicator_(nullptr)
    , blackScoreLabel_(nullptr)
    , whiteScoreLabel_(nullptr)
    , aiConfigGroup_(nullptr)
    , aiFirstRadio_(nullptr)
    , playerFirstRadio_(nullptr)
    , algorithmCombo_(nullptr)
    , difficultyCombo_(nullptr)
    , depthCombo_(nullptr)
    , delayCombo_(nullptr)
    , startButton_(nullptr)
    , newGameButton_(nullptr)
    , backButton_(nullptr)
    , gameState_(GameWatchState::IDLE)
    , currentPlayer_(Reversi::PlayerColor::Black)
    , gameResult_(Reversi::GameResult::Unknown)
    , moveDelayMs_(500)
    , moveDelayTimer_(nullptr)
    , boardLabel_(nullptr)
    , boardScale_(1.0)
{
    qDebug() << "PvEWindow::PvEWindow() - Constructor START";

    board_ = Reversi::Board();

    qDebug() << "PvEWindow: calling setupUI()";
    setupUI();
    qDebug() << "PvEWindow: setupUI() done";

    qDebug() << "PvEWindow: calling loadResources()";
    loadResources();
    qDebug() << "PvEWindow: loadResources() done";

    qDebug() << "PvEWindow: calling initGame()";
    initGame();
    qDebug() << "PvEWindow: initGame() done";

    // Initial board render
    repaint();

    updateButtonStates();

    qDebug() << "PvEWindow::PvEWindow() - Constructor END";
}

PvEWindow::~PvEWindow()
{
    qDebug() << "PvEWindow destroyed";

    if (moveDelayTimer_) {
        moveDelayTimer_->stop();
        delete moveDelayTimer_;
    }
}

void PvEWindow::setupUI()
{
    setWindowTitle(tr("PvE Mode"));
    setWindowIcon(QIcon(":/rsc/black.png"));
    setFixedSize(830, 580);
    setAttribute(Qt::WA_DeleteOnClose);

    centralWidget_ = new QWidget(this);
    setCentralWidget(centralWidget_);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget_);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    QWidget* leftPanel = new QWidget(this);
    leftLayout_ = new QVBoxLayout(leftPanel);
    leftLayout_->setContentsMargins(0, 5, 0, 5);
    leftLayout_->setSpacing(8);

    resultLabel_ = new QLabel("", this);
    resultLabel_->setAlignment(Qt::AlignCenter);
    resultLabel_->setFixedHeight(40);
    resultLabel_->setVisible(false);
    resultLabel_->setStyleSheet(
        "QLabel { font-size: 28px; font-weight: bold; color: #e74c3c; "
        "background-color: rgba(255,255,255,220); border-radius: 8px; padding: 5px 20px; }"
    );
    leftLayout_->addWidget(resultLabel_, 0, Qt::AlignHCenter);

    QWidget* boardContainer = new QWidget(this);
    boardContainer->setFixedSize(BOARD_SIZE, BOARD_SIZE);

    boardLabel_ = new QLabel(boardContainer);
    boardLabel_->setFixedSize(BOARD_SIZE, BOARD_SIZE);
    boardLabel_->move(BOARD_OFFSET_X, BOARD_OFFSET_Y);

    leftLayout_->addWidget(boardContainer, 0, Qt::AlignHCenter);

    QHBoxLayout* infoLayout = new QHBoxLayout();
    infoLayout->setSpacing(10);
    infoLayout->setContentsMargins(0, 5, 0, 10);

    turnIndicator_ = new QLabel(tr("Black's Turn"), this);
    turnIndicator_->setAlignment(Qt::AlignCenter);
    turnIndicator_->setFixedSize(130, 35);
    turnIndicator_->setStyleSheet(
        "QLabel { font-weight: bold; font-size: 14px; color: #000000; "
        "background-color: rgba(220,220,220,220); border-radius: 6px; padding: 4px 10px; }"
    );
    infoLayout->addWidget(turnIndicator_);

    blackScoreLabel_ = new QLabel(tr("Black: 2"), this);
    blackScoreLabel_->setAlignment(Qt::AlignCenter);
    blackScoreLabel_->setFixedSize(100, 35);
    blackScoreLabel_->setStyleSheet(
        "QLabel { font-weight: bold; font-size: 14px; color: #000000; "
        "background-color: rgba(220,220,220,220); border-radius: 6px; padding: 4px 10px; }"
    );
    infoLayout->addWidget(blackScoreLabel_);

    whiteScoreLabel_ = new QLabel(tr("White: 2"), this);
    whiteScoreLabel_->setAlignment(Qt::AlignCenter);
    whiteScoreLabel_->setFixedSize(100, 35);
    whiteScoreLabel_->setStyleSheet(
        "QLabel { font-weight: bold; font-size: 14px; color: #ffffff; "
        "background-color: rgba(50,50,50,220); border-radius: 6px; padding: 4px 10px; }"
    );
    infoLayout->addWidget(whiteScoreLabel_);

    leftLayout_->addLayout(infoLayout);
    leftLayout_->addStretch();

    QWidget* rightPanel = new QWidget(this);
    rightLayout_ = new QVBoxLayout(rightPanel);
    rightLayout_->setContentsMargins(0, 0, 0, 0);
    rightLayout_->setSpacing(10);

    setupAIConfig();
    rightLayout_->addWidget(aiConfigGroup_);

    setupControls();
    rightLayout_->addWidget(controlsGroup_);

    rightLayout_->addStretch(2);

    mainLayout->addWidget(leftPanel, 65);
    mainLayout->addWidget(rightPanel, 35);
}

void PvEWindow::setupAIConfig()
{
    aiConfigGroup_ = new QGroupBox(tr("AI Configuration"), this);
    aiConfigGroup_->setAlignment(Qt::AlignLeft);
    aiConfigGroup_->setStyleSheet(
        "QGroupBox { font-weight: bold; font-size: 13px; border: 2px solid #34495e; "
        "border-radius: 6px; margin-top: 8px; padding-top: 8px; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; }"
    );
    QVBoxLayout* groupLayout = new QVBoxLayout(aiConfigGroup_);
    groupLayout->setSpacing(10);

    // AI First / Player First vertical layout
    aiFirstRadio_ = new QRadioButton(tr("AI First"), this);
    playerFirstRadio_ = new QRadioButton(tr("Player First"), this);
    playerFirstRadio_->setChecked(true);

    QVBoxLayout* firstMoveLayout = new QVBoxLayout();
    firstMoveLayout->setSpacing(5);
    firstMoveLayout->addWidget(aiFirstRadio_);
    firstMoveLayout->addWidget(playerFirstRadio_);
    groupLayout->addLayout(firstMoveLayout);

    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(8);

    QLabel* algoLabel = new QLabel(tr("Algorithm:"), this);
    algoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    algorithmCombo_ = new QComboBox(this);
    algorithmCombo_->setFixedHeight(30);
    algorithmCombo_->addItem("Minimax", 0);
    algorithmCombo_->addItem("MCTS", 1);
    algorithmCombo_->addItem("Random", 2);
    algorithmCombo_->setStyleSheet(
        "QComboBox { padding: 5px 10px; font-size: 13px; border: 1px solid #34495e; border-radius: 4px; }"
    );
    gridLayout->addWidget(algoLabel, 0, 0, Qt::AlignLeft);
    gridLayout->addWidget(algorithmCombo_, 0, 1);

    QLabel* diffLabel = new QLabel(tr("Difficulty:"), this);
    diffLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    difficultyCombo_ = new QComboBox(this);
    difficultyCombo_->setFixedHeight(30);
    difficultyCombo_->addItem("Easy", static_cast<int>(Reversi::Difficulty::EASY));
    difficultyCombo_->addItem("Medium", static_cast<int>(Reversi::Difficulty::MEDIUM));
    difficultyCombo_->addItem("Hard", static_cast<int>(Reversi::Difficulty::HARD));
    difficultyCombo_->setCurrentIndex(1);
    difficultyCombo_->setStyleSheet(
        "QComboBox { padding: 5px 10px; font-size: 13px; border: 1px solid #34495e; border-radius: 4px; }"
    );
    gridLayout->addWidget(diffLabel, 1, 0, Qt::AlignLeft);
    gridLayout->addWidget(difficultyCombo_, 1, 1);

    QLabel* depthLabel = new QLabel(tr("Depth:"), this);
    depthLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    depthCombo_ = new QComboBox(this);
    depthCombo_->setFixedHeight(30);
    depthCombo_->addItem("2", 2);
    depthCombo_->addItem("3", 3);
    depthCombo_->addItem("4", 4);
    depthCombo_->addItem("5", 5);
    depthCombo_->addItem("6", 6);
    depthCombo_->setCurrentIndex(2);
    depthCombo_->setStyleSheet(
        "QComboBox { padding: 5px 10px; font-size: 13px; border: 1px solid #34495e; border-radius: 4px; }"
    );
    gridLayout->addWidget(depthLabel, 2, 0, Qt::AlignLeft);
    gridLayout->addWidget(depthCombo_, 2, 1);

    QLabel* delayLabel = new QLabel(tr("Move Delay:"), this);
    delayLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    delayCombo_ = new QComboBox(this);
    delayCombo_->setFixedHeight(30);
    delayCombo_->addItem("0s", 0);
    delayCombo_->addItem("0.5s", 500);
    delayCombo_->addItem("1s", 1000);
    delayCombo_->addItem("2s", 2000);
    delayCombo_->setCurrentIndex(1);
    delayCombo_->setStyleSheet(
        "QComboBox { padding: 5px 10px; font-size: 13px; border: 1px solid #34495e; border-radius: 4px; }"
    );
    gridLayout->addWidget(delayLabel, 3, 0, Qt::AlignLeft);
    gridLayout->addWidget(delayCombo_, 3, 1);

    groupLayout->addLayout(gridLayout);

    connect(delayCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                moveDelayMs_ = delayCombo_->itemData(index).toInt();
            });
}

void PvEWindow::setupControls()
{
    controlsGroup_ = new QGroupBox(tr("Controls"), this);
    controlsGroup_->setAlignment(Qt::AlignLeft);
    controlsGroup_->setStyleSheet(
        "QGroupBox { font-weight: bold; font-size: 13px; border: 2px solid #34495e; "
        "border-radius: 6px; margin-top: 8px; padding-top: 8px; }"
        "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top center; padding: 0 5px; }"
    );
    QVBoxLayout* groupLayout = new QVBoxLayout(controlsGroup_);
    groupLayout->setSpacing(15);

    startButton_ = new QPushButton(tr("Start"), this);
    startButton_->setFixedHeight(40);
    startButton_->setStyleSheet(
        "QPushButton { background-color: #2ecc71; color: white; border: none; "
        "border-radius: 5px; font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { background-color: #27ae60; }"
        "QPushButton:disabled { background-color: #bdc3c7; }"
    );
    groupLayout->addWidget(startButton_);

    newGameButton_ = new QPushButton(tr("New Game"), this);
    newGameButton_->setFixedHeight(40);
    newGameButton_->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; border: none; "
        "border-radius: 5px; font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { background-color: #2980b9; }"
        "QPushButton:disabled { background-color: #bdc3c7; }"
    );
    groupLayout->addWidget(newGameButton_);

    backButton_ = new QPushButton(tr("Back to Menu"), this);
    backButton_->setFixedHeight(40);
    backButton_->setStyleSheet(
        "QPushButton { background-color: #95a5a6; color: white; border: none; "
        "border-radius: 5px; font-weight: bold; font-size: 14px; }"
        "QPushButton:hover { background-color: #7f8c8d; }"
    );
    groupLayout->addWidget(backButton_);
}

void PvEWindow::loadResources()
{
    qDebug() << "PvEWindow::loadResources() - START";
    pixmapBackground_.load(":/rsc/board.png");
    qDebug() << "  board:" << pixmapBackground_.isNull();
    pixmapBlack_.load(":/rsc/black.png");
    qDebug() << "  black:" << pixmapBlack_.isNull();
    pixmapWhite_.load(":/rsc/white.png");
    qDebug() << "  white:" << pixmapWhite_.isNull();
    pixmapHintWhite_.load(":/rsc/whitepotential.png");
    pixmapHintBlack_.load(":/rsc/blackpotential.png");
    pixmapHintRed_.load(":/rsc/redpotential.png");
    qDebug() << "PvEWindow::loadResources() - END";
}

void PvEWindow::initGame()
{
    board_ = Reversi::Board();
    gameState_ = GameWatchState::IDLE;
    currentPlayer_ = Reversi::PlayerColor::Black;
    gameResult_ = Reversi::GameResult::Unknown;

    // Only create timer and controller on first time
    if (!moveDelayTimer_) {
        moveDelayTimer_ = new QTimer(this);
        moveDelayTimer_->setSingleShot(true);
        connect(moveDelayTimer_, &QTimer::timeout, this, &PvEWindow::onDelayTimerTimeout);
    }

    if (!gameController_) {
        gameController_ = std::make_unique<Reversi::GameController>(this);
        setupConnections();
    }

    updateButtonStates();
    repaint();
}

void PvEWindow::setupConnections()
{
    connect(startButton_, &QPushButton::clicked, this, &PvEWindow::onStartGameClicked);
    connect(newGameButton_, &QPushButton::clicked, this, &PvEWindow::onNewGameClicked);
    connect(backButton_, &QPushButton::clicked, this, &PvEWindow::onBackToMenuClicked);

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

void PvEWindow::updateButtonStates()
{
    switch (gameState_) {
    case GameWatchState::IDLE:
        startButton_->setEnabled(true);
        newGameButton_->setEnabled(false);
        aiFirstRadio_->setEnabled(true);
        playerFirstRadio_->setEnabled(true);
        algorithmCombo_->setEnabled(true);
        difficultyCombo_->setEnabled(true);
        depthCombo_->setEnabled(true);
        delayCombo_->setEnabled(true);
        break;
    case GameWatchState::PLAYING:
        startButton_->setEnabled(false);
        newGameButton_->setEnabled(true);
        aiFirstRadio_->setEnabled(false);
        playerFirstRadio_->setEnabled(false);
        algorithmCombo_->setEnabled(false);
        difficultyCombo_->setEnabled(false);
        depthCombo_->setEnabled(false);
        delayCombo_->setEnabled(false);
        break;
    case GameWatchState::PAUSED:
        startButton_->setEnabled(false);
        newGameButton_->setEnabled(true);
        break;
    case GameWatchState::GAME_OVER:
        startButton_->setEnabled(false);
        newGameButton_->setEnabled(true);
        aiFirstRadio_->setEnabled(false);
        playerFirstRadio_->setEnabled(false);
        algorithmCombo_->setEnabled(false);
        difficultyCombo_->setEnabled(false);
        depthCombo_->setEnabled(false);
        delayCombo_->setEnabled(false);
        break;
    }
}

void PvEWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    if (!boardLabel_ || !gameController_) return;

    QSize labelSize = boardLabel_->size();
    int scaledSize = qMin(labelSize.width(), labelSize.height());
    if (scaledSize < 8) return;

    int cellSize = scaledSize / 8;
    boardScale_ = static_cast<double>(cellSize) / CELL_SIZE;

    QPixmap offScreen(BOARD_SIZE, BOARD_SIZE);
    offScreen.fill(Qt::transparent);
    {
        QPainter p(&offScreen);
        p.setRenderHint(QPainter::SmoothPixmapTransform, true);
        p.drawPixmap(0, 0, BOARD_SIZE, BOARD_SIZE, pixmapBackground_);

        const Reversi::Board& board = gameController_->getBoard();
        currentPlayer_ = gameController_->getCurrentPlayer();
        int tile = (currentPlayer_ == Reversi::PlayerColor::Black) ? 2 : 1;

        int markHaveDraw[8][8] = {0};
        auto validMoves = board.getValidMoves();
        for (const auto& move : validMoves) {
            markHaveDraw[move.row][move.col] = tile;
        }

        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                int cellValue = board.at(j, i);
                if (cellValue == 1) {
                    p.drawPixmap(CELL_SIZE * i, CELL_SIZE * j, CELL_SIZE, CELL_SIZE, pixmapWhite_);
                } else if (cellValue == 2) {
                    p.drawPixmap(CELL_SIZE * i, CELL_SIZE * j, CELL_SIZE, CELL_SIZE, pixmapBlack_);
                }
                if (markHaveDraw[j][i] == 2) {
                    p.drawPixmap(CELL_SIZE * i, CELL_SIZE * j, CELL_SIZE, CELL_SIZE, pixmapHintBlack_);
                }
                if (markHaveDraw[j][i] == 1) {
                    p.drawPixmap(CELL_SIZE * i, CELL_SIZE * j, CELL_SIZE, CELL_SIZE, pixmapHintWhite_);
                }
            }
        }
    }

    QPixmap scaled = offScreen.scaled(scaledSize, scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    boardLabel_->setPixmap(scaled);
}

void PvEWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    repaint();
}

void PvEWindow::mousePressEvent(QMouseEvent* e)
{
    if (gameState_ != GameWatchState::PLAYING) return;
    if (gameController_->getCurrentPhase() != Reversi::GamePhase::HumanTurn) return;

    int row = (e->y() - BOARD_OFFSET_Y) / CELL_SIZE;
    int col = (e->x() - BOARD_OFFSET_X) / CELL_SIZE;

    if (row < 0 || row >= 8 || col < 0 || col >= 8) return;

    gameController_->makeHumanMove(row, col);
}

void PvEWindow::onStartGameClicked()
{
    qDebug() << "PvEWindow::onStartGameClicked()";

    Reversi::PlayerColor humanColor = playerFirstRadio_->isChecked()
        ? Reversi::PlayerColor::Black : Reversi::PlayerColor::White;

    Reversi::Difficulty difficulty = static_cast<Reversi::Difficulty>(difficultyCombo_->currentData().toInt());

    // Read AI configuration from ComboBox
    int algorithm = algorithmCombo_->currentData().toInt();
    int depth = depthCombo_->currentText().toInt();

    qDebug() << "PvEWindow: Starting game with algorithm=" << algorithm
             << "difficulty=" << (int)difficulty << "depth=" << depth;

    resultLabel_->setVisible(false);
    gameController_->startNewGame(Reversi::GameMode::PvE, humanColor, difficulty, algorithm, depth);

    gameState_ = GameWatchState::PLAYING;
    updateButtonStates();
}

void PvEWindow::onNewGameClicked()
{
    moveDelayTimer_->stop();

    gameState_ = GameWatchState::IDLE;
    gameResult_ = Reversi::GameResult::Unknown;
    resultLabel_->setVisible(false);

    board_ = Reversi::Board();
    initGame();

    updateButtonStates();
    repaint();
}

void PvEWindow::onDelayTimerTimeout()
{
    // GameController handles AI moves internally
}

void PvEWindow::onGameStarted(Reversi::GameMode, Reversi::PlayerColor)
{
    updateScoreDisplay();
    repaint();
}

void PvEWindow::onPhaseChanged(Reversi::GamePhase phase)
{
    if (phase == Reversi::GamePhase::AITurn && gameState_ == GameWatchState::PLAYING && moveDelayMs_ > 0) {
        moveDelayTimer_->start(moveDelayMs_);
    }
    repaint();
}

void PvEWindow::onTurnChanged(Reversi::PlayerColor player)
{
    currentPlayer_ = player;
    QString turnText = (player == Reversi::PlayerColor::Black) ? tr("Black's Turn") : tr("White's Turn");
    turnIndicator_->setText(turnText);
    updateScoreDisplay();
    repaint();
}

void PvEWindow::onMoveMade(int, int, Reversi::PlayerColor)
{
    updateScoreDisplay();
    repaint();
}

void PvEWindow::onGameEnded(Reversi::GameResult result)
{
    gameState_ = GameWatchState::GAME_OVER;
    gameResult_ = result;
    moveDelayTimer_->stop();

    turnIndicator_->setText(tr("Game Over"));

    QString resultText;
    switch (result) {
    case Reversi::GameResult::BlackWins: resultText = tr("Black Wins!"); break;
    case Reversi::GameResult::WhiteWins: resultText = tr("White Wins!"); break;
    case Reversi::GameResult::Draw: resultText = tr("Draw!"); break;
    default: resultText = tr("Game Over"); break;
    }

    resultLabel_->setText(resultText);
    resultLabel_->setVisible(true);

    updateScoreDisplay();
    updateButtonStates();
    repaint();
}

void PvEWindow::onAIThinkingStarted(const QString&) {}
void PvEWindow::onAIThinkingFinished(int, int) {}
void PvEWindow::onAIStatsUpdated(const Reversi::AIStats&) {}
void PvEWindow::onErrorOccurred(const QString& message)
{
    qDebug() << "PvEWindow::onErrorOccurred:" << message;
}

void PvEWindow::updateScoreDisplay()
{
    const Reversi::Board& board = gameController_->getBoard();

    int blackCount = 0, whiteCount = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int cellValue = board.at(i, j);
            if (cellValue == 2) blackCount++;
            else if (cellValue == 1) whiteCount++;
        }
    }

    blackScoreLabel_->setText(QString("Black: %1").arg(blackCount));
    whiteScoreLabel_->setText(QString("White: %1").arg(whiteCount));
}

void PvEWindow::onBackToMenuClicked()
{
    moveDelayTimer_->stop();
    emit backToMenu();
}
