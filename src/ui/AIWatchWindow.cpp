/**
 * @file AIWatchWindow.cpp
 * @brief Single AI vs AI battle watch window implementation
 *
 * @date 2026
 * @author Project Team
 * @license GPL-3.0
 */

#include "ui/AIWatchWindow.h"
#include <QPainter>
#include <QFont>
#include <QDebug>
#include <QApplication>
#include <QIcon>
#include <QResizeEvent>

namespace Reversi {

AIWatchWindow::AIWatchWindow(QWidget* parent)
    : QMainWindow(parent)
    , centralWidget_(nullptr)
    , boardLabel_(nullptr)
    , turnIndicator_(nullptr)
    , blackScoreLabel_(nullptr)
    , whiteScoreLabel_(nullptr)
    , resultLabel_(nullptr)
    , blackConfigGroup_(nullptr)
    , blackAlgorithmCombo_(nullptr)
    , blackDifficultyCombo_(nullptr)
    , blackDepthCombo_(nullptr)
    , whiteConfigGroup_(nullptr)
    , whiteAlgorithmCombo_(nullptr)
    , whiteDifficultyCombo_(nullptr)
    , whiteDepthCombo_(nullptr)
    , delayCombo_(nullptr)
    , startButton_(nullptr)
    , pauseResumeButton_(nullptr)
    , newGameButton_(nullptr)
    , backButton_(nullptr)
    , gameState_(GameWatchState::IDLE)
    , currentPlayer_(PlayerColor::Black)
    , moveDelayMs_(DELAY_1S)
    , gameResult_(GameResult::Unknown)
    , moveTimer_(nullptr)
    , boardScale_(1.0)
{
    qDebug() << "AIWatchWindow::AIWatchWindow() - Constructor START";

    // Initialize board in body to avoid initialization order issues
    board_ = Board();

    setupUI();
    loadResources();
    initGame();
    updateButtonStates();

    qDebug() << "AIWatchWindow::AIWatchWindow() - Constructor END";

    // Initial board render
    repaint();
}

AIWatchWindow::~AIWatchWindow()
{
    qDebug() << "AIWatchWindow destroyed";

    if (moveTimer_) {
        moveTimer_->stop();
        delete moveTimer_;
    }
}

// ==================== UI Setup ====================

void AIWatchWindow::setupUI()
{
    setWindowTitle(tr("AI Watch"));
    setWindowIcon(QIcon(":/rsc/black.png"));
    setFixedSize(830, 580);
    setAttribute(Qt::WA_DeleteOnClose);

    // Central widget with main layout
    centralWidget_ = new QWidget(this);
    setCentralWidget(centralWidget_);

    // Main layout: left panel (board) + right panel (controls)
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget_);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    // ==================== Left Panel: Board Area ====================
    QWidget* leftPanel = new QWidget(this);
    leftLayout_ = new QVBoxLayout(leftPanel);
    leftLayout_->setContentsMargins(0, 5, 0, 5);
    leftLayout_->setSpacing(8);

    // Board container - fixed size to match other windows
    QWidget* boardContainer = new QWidget(this);
    boardContainer->setFixedSize(BOARD_SIZE, BOARD_SIZE);

    // We'll draw the board in paintEvent
    boardLabel_ = new QLabel(boardContainer);
    boardLabel_->setFixedSize(BOARD_SIZE, BOARD_SIZE);
    boardLabel_->move(BOARD_OFFSET_X, BOARD_OFFSET_Y);

    leftLayout_->addWidget(boardContainer, 0, Qt::AlignHCenter);

    // Info row: Turn indicator + Black score + White score in one line
    QHBoxLayout* infoLayout = new QHBoxLayout();
    infoLayout->setSpacing(10);
    infoLayout->setContentsMargins(0, 5, 0, 10);

    // Turn indicator
    turnIndicator_ = new QLabel(tr("Black's Turn"), this);
    turnIndicator_->setAlignment(Qt::AlignCenter);
    turnIndicator_->setFixedSize(130, 35);
    turnIndicator_->setStyleSheet(
        "QLabel {"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    color: #000000;"
        "    background-color: rgba(220,220,220,220);"
        "    border-radius: 6px;"
        "    padding: 4px 10px;"
        "}"
    );
    infoLayout->addWidget(turnIndicator_);

    // Black score
    blackScoreLabel_ = new QLabel(tr("Black: 2"), this);
    blackScoreLabel_->setAlignment(Qt::AlignCenter);
    blackScoreLabel_->setFixedSize(100, 35);
    blackScoreLabel_->setStyleSheet(
        "QLabel {"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    color: #000000;"
        "    background-color: rgba(220,220,220,220);"
        "    border-radius: 6px;"
        "    padding: 4px 10px;"
        "}"
    );
    infoLayout->addWidget(blackScoreLabel_);

    // White score
    whiteScoreLabel_ = new QLabel(tr("White: 2"), this);
    whiteScoreLabel_->setAlignment(Qt::AlignCenter);
    whiteScoreLabel_->setFixedSize(100, 35);
    whiteScoreLabel_->setStyleSheet(
        "QLabel {"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    color: #ffffff;"
        "    background-color: rgba(50,50,50,220);"
        "    border-radius: 6px;"
        "    padding: 4px 10px;"
        "}"
    );
    infoLayout->addWidget(whiteScoreLabel_);

    leftLayout_->addLayout(infoLayout);

    // Add spacing to avoid overlapping with board
    leftLayout_->insertSpacing(1, 15);

    // Result label (hidden initially)
    resultLabel_ = new QLabel(this);
    resultLabel_->setAlignment(Qt::AlignCenter);
    resultLabel_->setVisible(false);
    resultLabel_->setFixedHeight(40);
    resultLabel_->setStyleSheet(
        "QLabel {"
        "    font-weight: bold;"
        "    font-size: 24px;"
        "    color: #ffffff;"
        "    background-color: #27ae60;"
        "    border-radius: 8px;"
        "    padding: 6px 20px;"
        "}"
    );
    leftLayout_->addWidget(resultLabel_, 0, Qt::AlignHCenter);

    // Add stretch to fill remaining space
    leftLayout_->addStretch();

    // Left panel takes 70% of width
    mainLayout->addWidget(leftPanel, 70);

    // ==================== Right Panel: Controls ====================
    QWidget* rightPanel = new QWidget(this);
    rightLayout_ = new QVBoxLayout(rightPanel);
    rightLayout_->setContentsMargins(5, 5, 5, 5);
    rightLayout_->setSpacing(10);

    // Right panel takes 30% of width
    mainLayout->addWidget(rightPanel, 30);

    // Setup AI configuration
    setupAIConfig();

    // Delay configuration
    QGroupBox* delayGroup = new QGroupBox(tr("Move Delay"), this);
    delayGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #34495e;"
        "    border-radius: 6px;"
        "    margin-top: 8px;"
        "    padding-top: 8px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top center;"
        "    padding: 0 5px;"
        "}"
    );
    QVBoxLayout* delayLayout = new QVBoxLayout(delayGroup);

    delayCombo_ = new QComboBox(this);
    delayCombo_->addItem("0.5 seconds", DELAY_500MS);
    delayCombo_->addItem("1 second", DELAY_1S);
    delayCombo_->addItem("2 seconds", DELAY_2S);
    delayCombo_->addItem("5 seconds", DELAY_5S);
    delayCombo_->setCurrentIndex(1); // Default: 1 second
    delayCombo_->setFixedHeight(30);
    delayCombo_->setStyleSheet(
        "QComboBox {"
        "    padding: 5px 10px;"
        "    border: 1px solid #34495e;"
        "    border-radius: 4px;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "}"
    );
    delayLayout->addWidget(delayCombo_);

    connect(delayCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                moveDelayMs_ = delayCombo_->itemData(index).toInt();
            });

    rightLayout_->addWidget(delayGroup);

    // Control buttons
    setupControls();

    // Add stretch to fill remaining space
    rightLayout_->addStretch();

    qDebug() << "AIWatchWindow::setupUI() - UI setup complete";
}

void AIWatchWindow::setupAIConfig()
{
    // ==================== Black AI Configuration ====================
    blackConfigGroup_ = new QGroupBox(tr("Black (First)"), this);
    blackConfigGroup_->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #2c3e50;"
        "    border-radius: 6px;"
        "    margin-top: 8px;"
        "    padding-top: 8px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top center;"
        "    padding: 0 5px;"
        "    color: #2c3e50;"
        "}"
    );
    QGridLayout* blackLayout = new QGridLayout(blackConfigGroup_);
    blackLayout->setSpacing(5);

    // Algorithm type
    QLabel* algoLabel = new QLabel(tr("Algorithm:"), this);
    blackLayout->addWidget(algoLabel, 0, 0);

    blackAlgorithmCombo_ = new QComboBox(this);
    blackAlgorithmCombo_->addItem("Minimax", static_cast<int>(AIWatchConfig::AIType::MINIMAX));
    blackAlgorithmCombo_->addItem("MCTS", static_cast<int>(AIWatchConfig::AIType::MCTS));
    blackAlgorithmCombo_->addItem("Random", static_cast<int>(AIWatchConfig::AIType::RANDOM));
    blackAlgorithmCombo_->setFixedHeight(30);
    blackAlgorithmCombo_->setStyleSheet(
        "QComboBox {"
        "    padding: 3px 8px;"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 4px;"
        "}"
    );
    blackLayout->addWidget(blackAlgorithmCombo_, 0, 1);

    // Difficulty
    QLabel* diffLabel = new QLabel(tr("Difficulty:"), this);
    blackLayout->addWidget(diffLabel, 1, 0);

    blackDifficultyCombo_ = new QComboBox(this);
    blackDifficultyCombo_->addItem("Easy", static_cast<int>(Difficulty::EASY));
    blackDifficultyCombo_->addItem("Medium", static_cast<int>(Difficulty::MEDIUM));
    blackDifficultyCombo_->addItem("Hard", static_cast<int>(Difficulty::HARD));
    blackDifficultyCombo_->setFixedHeight(30);
    blackDifficultyCombo_->setStyleSheet(
        "QComboBox {"
        "    padding: 3px 8px;"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 4px;"
        "}"
    );
    blackLayout->addWidget(blackDifficultyCombo_, 1, 1);

    // Depth
    QLabel* depthLabel = new QLabel(tr("Depth:"), this);
    blackLayout->addWidget(depthLabel, 2, 0);

    blackDepthCombo_ = new QComboBox(this);
    blackDepthCombo_->addItem("2");
    blackDepthCombo_->addItem("3");
    blackDepthCombo_->addItem("4");
    blackDepthCombo_->addItem("5");
    blackDepthCombo_->addItem("6");
    blackDepthCombo_->setCurrentIndex(2);
    blackDepthCombo_->setFixedHeight(30);
    blackDepthCombo_->setStyleSheet(
        "QComboBox {"
        "    padding: 3px 8px;"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 4px;"
        "}"
    );
    blackLayout->addWidget(blackDepthCombo_, 2, 1);

    // ==================== White AI Configuration ====================
    whiteConfigGroup_ = new QGroupBox(tr("White (Second)"), this);
    whiteConfigGroup_->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #7f8c8d;"
        "    border-radius: 6px;"
        "    margin-top: 8px;"
        "    padding-top: 8px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top center;"
        "    padding: 0 5px;"
        "    color: #7f8c8d;"
        "}"
    );
    QGridLayout* whiteLayout = new QGridLayout(whiteConfigGroup_);
    whiteLayout->setSpacing(5);

    // Algorithm type
    algoLabel = new QLabel(tr("Algorithm:"), this);
    whiteLayout->addWidget(algoLabel, 0, 0);

    whiteAlgorithmCombo_ = new QComboBox(this);
    whiteAlgorithmCombo_->addItem("Minimax", static_cast<int>(AIWatchConfig::AIType::MINIMAX));
    whiteAlgorithmCombo_->addItem("MCTS", static_cast<int>(AIWatchConfig::AIType::MCTS));
    whiteAlgorithmCombo_->addItem("Random", static_cast<int>(AIWatchConfig::AIType::RANDOM));
    whiteAlgorithmCombo_->setFixedHeight(30);
    whiteAlgorithmCombo_->setStyleSheet(
        "QComboBox {"
        "    padding: 3px 8px;"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 4px;"
        "}"
    );
    whiteLayout->addWidget(whiteAlgorithmCombo_, 0, 1);

    // Difficulty
    diffLabel = new QLabel(tr("Difficulty:"), this);
    whiteLayout->addWidget(diffLabel, 1, 0);

    whiteDifficultyCombo_ = new QComboBox(this);
    whiteDifficultyCombo_->addItem("Easy", static_cast<int>(Difficulty::EASY));
    whiteDifficultyCombo_->addItem("Medium", static_cast<int>(Difficulty::MEDIUM));
    whiteDifficultyCombo_->addItem("Hard", static_cast<int>(Difficulty::HARD));
    whiteDifficultyCombo_->setFixedHeight(30);
    whiteDifficultyCombo_->setStyleSheet(
        "QComboBox {"
        "    padding: 3px 8px;"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 4px;"
        "}"
    );
    whiteLayout->addWidget(whiteDifficultyCombo_, 1, 1);

    // Depth
    depthLabel = new QLabel(tr("Depth:"), this);
    whiteLayout->addWidget(depthLabel, 2, 0);

    whiteDepthCombo_ = new QComboBox(this);
    whiteDepthCombo_->addItem("2");
    whiteDepthCombo_->addItem("3");
    whiteDepthCombo_->addItem("4");
    whiteDepthCombo_->addItem("5");
    whiteDepthCombo_->addItem("6");
    whiteDepthCombo_->setCurrentIndex(2);
    whiteDepthCombo_->setFixedHeight(30);
    whiteDepthCombo_->setStyleSheet(
        "QComboBox {"
        "    padding: 3px 8px;"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 4px;"
        "}"
    );
    whiteLayout->addWidget(whiteDepthCombo_, 2, 1);

    // Connect configuration change handlers
    connect(blackAlgorithmCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AIWatchWindow::onAIConfigChanged);
    connect(whiteAlgorithmCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AIWatchWindow::onAIConfigChanged);

    // Add to right panel layout
    if (rightLayout_) {
        rightLayout_->insertWidget(0, blackConfigGroup_);
        rightLayout_->insertWidget(1, whiteConfigGroup_);
    }
}

void AIWatchWindow::setupControls()
{
    // Control button group
    QGroupBox* controlGroup = new QGroupBox(tr("Controls"), this);
    controlGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #34495e;"
        "    border-radius: 6px;"
        "    margin-top: 8px;"
        "    padding-top: 8px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top center;"
        "    padding: 0 5px;"
        "}"
    );
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroup);
    controlLayout->setSpacing(8);

    // Start button
    startButton_ = new QPushButton(tr("Start"), this);
    startButton_->setFixedHeight(30);
    startButton_->setStyleSheet(
        "QPushButton {"
        "    background-color: #27ae60;"
        "    color: white;"
        "    border-radius: 6px;"
        "    font-weight: bold;"
        "    padding: 8px;"
        "}"
        "QPushButton:hover { background-color: #219a52; }"
    );
    controlLayout->addWidget(startButton_);

    // Pause/Resume button (combined)
    pauseResumeButton_ = new QPushButton(tr("Pause"), this);
    pauseResumeButton_->setFixedHeight(30);
    pauseResumeButton_->setEnabled(false);
    pauseResumeButton_->setStyleSheet(
        "QPushButton {"
        "    background-color: #FF9800;"
        "    color: white;"
        "    border-radius: 6px;"
        "    font-weight: bold;"
        "    padding: 8px;"
        "}"
        "QPushButton:hover { background-color: #e68a00; }"
    );
    controlLayout->addWidget(pauseResumeButton_);

    // New Game button
    newGameButton_ = new QPushButton(tr("New Game"), this);
    newGameButton_->setFixedHeight(30);
    newGameButton_->setEnabled(false);
    newGameButton_->setStyleSheet(
        "QPushButton {"
        "    background-color: #9b59b6;"
        "    color: white;"
        "    border-radius: 6px;"
        "    font-weight: bold;"
        "    padding: 8px;"
        "}"
        "QPushButton:hover { background-color: #8e44ad; }"
    );
    controlLayout->addWidget(newGameButton_);

    // Back button
    backButton_ = new QPushButton(tr("Back to Menu"), this);
    backButton_->setFixedHeight(30);
    backButton_->setStyleSheet(
        "QPushButton {"
        "    background-color: #95a5a6;"
        "    color: white;"
        "    border-radius: 6px;"
        "    font-weight: bold;"
        "    padding: 8px;"
        "}"
        "QPushButton:hover { background-color: #7f8c8d; }"
    );
    controlLayout->addWidget(backButton_);

    // Add to right panel layout
    if (rightLayout_) {
        rightLayout_->addWidget(controlGroup);
    }

    // Connect signals
    connect(startButton_, &QPushButton::clicked, this, &AIWatchWindow::onStartClicked);
    connect(pauseResumeButton_, &QPushButton::clicked, this, &AIWatchWindow::onPauseResumeClicked);
    connect(newGameButton_, &QPushButton::clicked, this, &AIWatchWindow::onNewGameClicked);
    connect(backButton_, &QPushButton::clicked, this, &AIWatchWindow::onBackClicked);
}

void AIWatchWindow::loadResources()
{
    // Load board images from resource files (same as NetworkGameWindow)
    background_.load(":/rsc/board.png");
    black_.load(":/rsc/black.png");
    white_.load(":/rsc/white.png");
    hintwhite_.load(":/rsc/whitepotential.png");
}

void AIWatchWindow::setupConnections()
{
    // Connections are set up in setupUI()
    qDebug() << "AIWatchWindow::setupConnections() - All connections established";
}

// ==================== Game Logic ====================

void AIWatchWindow::initGame()
{
    // Create new board (Board doesn't have reset() method)
    board_ = Board();
    currentPlayer_ = PlayerColor::Black;
    gameState_ = GameWatchState::IDLE;
    gameResult_ = GameResult::Unknown;
    boardHistory_ = {};

    // Reset AI
    blackAI_.reset();
    whiteAI_.reset();

    // Update display
    updateBoardDisplay();
    updateTurnIndicator();
    updateScoreDisplay();

    resultLabel_->setVisible(false);
}

void AIWatchWindow::createAIs()
{
    // Get black AI configuration
    blackConfig_.type = static_cast<AIWatchConfig::AIType>(
        blackAlgorithmCombo_->currentData().toInt());
    blackConfig_.difficulty = static_cast<Difficulty>(
        blackDifficultyCombo_->currentData().toInt());
    blackConfig_.depth = blackDepthCombo_->currentText().toInt();

    // Get white AI configuration
    whiteConfig_.type = static_cast<AIWatchConfig::AIType>(
        whiteAlgorithmCombo_->currentData().toInt());
    whiteConfig_.difficulty = static_cast<Difficulty>(
        whiteDifficultyCombo_->currentData().toInt());
    whiteConfig_.depth = whiteDepthCombo_->currentText().toInt();

    // Create AI instances
    blackAI_ = createAI(blackConfig_);
    whiteAI_ = createAI(whiteConfig_);

    // Set AI colors
    blackAI_->setColor(PlayerColor::Black);
    whiteAI_->setColor(PlayerColor::White);

    // Configure search limits
    searchLimits_ = SearchLimits::createDefault();
    searchLimits_.maxDepth = std::max(blackConfig_.depth, whiteConfig_.depth);

    qDebug() << "AIWatchWindow::createAIs() - AIs created";
}

std::unique_ptr<AIStrategy> AIWatchWindow::createAI(const AIWatchConfig& config)
{
    std::unique_ptr<AIStrategy> ai;

    switch (config.type) {
        case AIWatchConfig::AIType::RANDOM:
            ai = AIStrategyFactory::createRandomAI();
            break;

        case AIWatchConfig::AIType::MCTS:
            ai = AIStrategyFactory::createMCTSAI(config.difficulty);
            break;

        case AIWatchConfig::AIType::MINIMAX:
            ai = AIStrategyFactory::createMinimaxAI(config.difficulty);
            break;
    }

    return ai;
}

bool AIWatchWindow::hasValidMoves()
{
    return !board_.getValidMoves().empty();
}

void AIWatchWindow::executeMove(const Move& move)
{
    // Save current board state for potential undo
    boardHistory_.push(board_);

    // Make the move
    board_.makeMove(move);

    // Update display
    updateBoardDisplay();
    updateScoreDisplay();

    // Switch player
    currentPlayer_ = (currentPlayer_ == PlayerColor::Black) ?
        PlayerColor::White : PlayerColor::Black;

    updateTurnIndicator();
}

bool AIWatchWindow::isGameOver()
{
    // Check if both players have no valid moves
    bool blackHasMoves = !board_.getValidMoves().empty();
    bool whiteHasMoves = false;

    // Check if white has valid moves by temporarily checking
    // For simplicity, we check after black's turn
    if (!blackHasMoves) {
        // Switch to white and check
        currentPlayer_ = PlayerColor::White;
        whiteHasMoves = !board_.getValidMoves().empty();
        currentPlayer_ = PlayerColor::Black;
    }

    // Game is over if both have no moves, or board is full
    bool boardFull = (board_.getBitBoard().getEmptyCount() == 0);
    return (!blackHasMoves && !whiteHasMoves) || boardFull;
}

void AIWatchWindow::determineResult()
{
    int blackCount = board_.getBitBoard().getScore(PlayerColor::Black);
    int whiteCount = board_.getBitBoard().getScore(PlayerColor::White);

    if (blackCount > whiteCount) {
        gameResult_ = GameResult::BlackWins;
    } else if (whiteCount > blackCount) {
        gameResult_ = GameResult::WhiteWins;
    } else {
        gameResult_ = GameResult::Draw;
    }

    // Display result
    QString resultText;
    switch (gameResult_) {
        case GameResult::BlackWins:
            resultText = tr("Black Wins!");
            break;
        case GameResult::WhiteWins:
            resultText = tr("White Wins!");
            break;
        case GameResult::Draw:
            resultText = tr("Draw!");
            break;
        default:
            resultText = tr("Game Over");
    }

    resultLabel_->setText(resultText);
    resultLabel_->setVisible(true);

    qDebug() << "AIWatchWindow::determineResult() -" << resultText
             << "Black:" << blackCount << "White:" << whiteCount;
}

void AIWatchWindow::updateBoardDisplay()
{
    update(); // Trigger paintEvent
}

void AIWatchWindow::updateTurnIndicator()
{
    QString turnText = (currentPlayer_ == PlayerColor::Black) ?
        tr("Black's Turn") : tr("White's Turn");

    turnIndicator_->setText(turnText);

    if (currentPlayer_ == PlayerColor::Black) {
        turnIndicator_->setStyleSheet(
            "QLabel {"
            "    font-weight: bold;"
            "    font-size: 14px;"
            "    color: #000000;"
            "    background-color: rgba(220,220,220,220);"
            "    border-radius: 6px;"
            "    padding: 4px 10px;"
            "}"
        );
    } else {
        turnIndicator_->setStyleSheet(
            "QLabel {"
            "    font-weight: bold;"
            "    font-size: 14px;"
            "    color: #ffffff;"
            "    background-color: rgba(50,50,50,220);"
            "    border-radius: 6px;"
            "    padding: 4px 10px;"
            "}"
        );
    }
}

void AIWatchWindow::updateScoreDisplay()
{
    int blackCount = board_.getBitBoard().getScore(PlayerColor::Black);
    int whiteCount = board_.getBitBoard().getScore(PlayerColor::White);

    blackScoreLabel_->setText(tr("Black: %1").arg(blackCount));
    whiteScoreLabel_->setText(tr("White: %1").arg(whiteCount));
}

void AIWatchWindow::updateButtonStates()
{
    switch (gameState_) {
        case GameWatchState::IDLE:
            startButton_->setEnabled(true);
            pauseResumeButton_->setEnabled(false);
            pauseResumeButton_->setText(tr("Pause"));
            newGameButton_->setEnabled(false);
            blackConfigGroup_->setEnabled(true);
            whiteConfigGroup_->setEnabled(true);
            delayCombo_->setEnabled(true);
            break;

        case GameWatchState::PLAYING:
            startButton_->setEnabled(false);
            pauseResumeButton_->setEnabled(true);
            pauseResumeButton_->setText(tr("Pause"));
            newGameButton_->setEnabled(false);
            blackConfigGroup_->setEnabled(false);
            whiteConfigGroup_->setEnabled(false);
            delayCombo_->setEnabled(false);
            break;

        case GameWatchState::PAUSED:
            startButton_->setEnabled(false);
            pauseResumeButton_->setEnabled(true);
            pauseResumeButton_->setText(tr("Resume"));
            newGameButton_->setEnabled(true);
            blackConfigGroup_->setEnabled(false);
            whiteConfigGroup_->setEnabled(false);
            delayCombo_->setEnabled(false);
            break;

        case GameWatchState::FINISHED:
            startButton_->setEnabled(false);
            pauseResumeButton_->setEnabled(false);
            pauseResumeButton_->setText(tr("Pause"));
            newGameButton_->setEnabled(true);
            blackConfigGroup_->setEnabled(false);
            whiteConfigGroup_->setEnabled(false);
            delayCombo_->setEnabled(false);
            break;
    }
}

// ==================== Paint Event ====================

void AIWatchWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    if (!boardLabel_) return;

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

        // Draw board background
        if (!background_.isNull()) {
            p.drawPixmap(0, 0, BOARD_SIZE, BOARD_SIZE, background_);
        } else {
            p.fillRect(0, 0, BOARD_SIZE, BOARD_SIZE, QColor(120, 150, 90));
            p.setPen(QPen(Qt::black, 1));
            for (int i = 0; i <= 8; ++i) {
                p.drawLine(i * CELL_SIZE, 0, i * CELL_SIZE, BOARD_SIZE);
                p.drawLine(0, i * CELL_SIZE, BOARD_SIZE, i * CELL_SIZE);
            }
        }

        // Draw valid move hints
        auto validMoves = board_.getValidMoves();
        if (!hintwhite_.isNull()) {
            for (const auto& move : validMoves) {
                p.drawPixmap(move.col * CELL_SIZE, move.row * CELL_SIZE,
                           CELL_SIZE, CELL_SIZE, hintwhite_);
            }
        }

        // Draw pieces
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                int cellValue = board_.at(row, col);
                if (cellValue == 1) {
                    if (!white_.isNull()) {
                        p.drawPixmap(col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE, white_);
                    } else {
                        p.setBrush(Qt::white);
                        p.setPen(QPen(Qt::lightGray, 1));
                        p.drawEllipse(col * CELL_SIZE + 4, row * CELL_SIZE + 4, CELL_SIZE - 8, CELL_SIZE - 8);
                    }
                } else if (cellValue == 2) {
                    if (!black_.isNull()) {
                        p.drawPixmap(col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE, black_);
                    } else {
                        p.setBrush(Qt::black);
                        p.setPen(Qt::NoPen);
                        p.drawEllipse(col * CELL_SIZE + 4, row * CELL_SIZE + 4, CELL_SIZE - 8, CELL_SIZE - 8);
                    }
                }
            }
        }
    }

    QPixmap scaled = offScreen.scaled(scaledSize, scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    boardLabel_->setPixmap(scaled);
}

void AIWatchWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    repaint();
}

// ==================== Slots ====================

void AIWatchWindow::onStartClicked()
{
    qDebug() << "AIWatchWindow::onStartClicked()";

    // Reset and start new game
    initGame();
    createAIs();

    gameState_ = GameWatchState::PLAYING;
    updateButtonStates();

    // Start move timer
    moveTimer_ = new QTimer(this);
    connect(moveTimer_, &QTimer::timeout, this, &AIWatchWindow::playNextMove);

    // Start with initial delay to let user see the board
    QTimer::singleShot(500, this, &AIWatchWindow::playNextMove);
}

void AIWatchWindow::onPauseResumeClicked()
{
    qDebug() << "AIWatchWindow::onPauseResumeClicked() - current state:"
             << (gameState_ == GameWatchState::PLAYING ? "PLAYING" : "PAUSED");

    if (gameState_ == GameWatchState::PLAYING) {
        // Pause the game
        gameState_ = GameWatchState::PAUSED;
        pauseResumeButton_->setText(tr("Resume"));
        if (moveTimer_) {
            moveTimer_->stop();
        }
    } else if (gameState_ == GameWatchState::PAUSED) {
        // Resume the game
        gameState_ = GameWatchState::PLAYING;
        pauseResumeButton_->setText(tr("Pause"));
        if (moveTimer_) {
            moveTimer_->start(moveDelayMs_);
        }
    }

    updateButtonStates();
}

void AIWatchWindow::onNewGameClicked()
{
    qDebug() << "AIWatchWindow::onNewGameClicked()";

    // Stop current timer
    if (moveTimer_) {
        moveTimer_->stop();
        delete moveTimer_;
        moveTimer_ = nullptr;
    }

    // Reset game to IDLE state (wait for user to click Start)
    initGame();

    // Reset AI with current configuration
    createAIs();

    gameState_ = GameWatchState::IDLE;
    updateButtonStates();
}

void AIWatchWindow::onBackClicked()
{
    qDebug() << "AIWatchWindow::onBackClicked()";

    // Stop timer
    if (moveTimer_) {
        moveTimer_->stop();
        delete moveTimer_;
        moveTimer_ = nullptr;
    }

    // Emit signal to go back to menu
    emit backToMenu();
}

void AIWatchWindow::onAIConfigChanged()
{
    // All configuration options remain enabled regardless of algorithm selection
    // (Random AI doesn't need to lock difficulty/depth menus)
}

void AIWatchWindow::playNextMove()
{
    if (gameState_ != GameWatchState::PLAYING) {
        return;
    }

    // Check if game is over
    if (isGameOver()) {
        gameState_ = GameWatchState::FINISHED;
        determineResult();
        updateButtonStates();

        if (moveTimer_) {
            moveTimer_->stop();
        }
        return;
    }

    // Get current AI
    AIStrategy* currentAI = (currentPlayer_ == PlayerColor::Black) ?
        blackAI_.get() : whiteAI_.get();

    if (!currentAI) {
        qWarning() << "AIWatchWindow::playNextMove() - AI is null!";
        return;
    }

    // Get valid moves for current player (board tracks current turn internally)
    auto validMoves = board_.getValidMoves();

    if (validMoves.empty()) {
        // No valid moves - skip to next player
        qDebug() << "AIWatchWindow::playNextMove() - No valid moves for"
                 << (currentPlayer_ == PlayerColor::Black ? "Black" : "White")
                 << "- skipping";

        currentPlayer_ = (currentPlayer_ == PlayerColor::Black) ?
            PlayerColor::White : PlayerColor::Black;
        updateTurnIndicator();

        // Schedule next move
        if (moveTimer_) {
            moveTimer_->start(moveDelayMs_);
        }
        return;
    }

    // AI makes a decision
    qDebug() << "AIWatchWindow::playNextMove() -"
             << (currentPlayer_ == PlayerColor::Black ? "Black" : "White")
             << "is thinking...";

    Move bestMove = currentAI->findBestMove(board_, searchLimits_);

    qDebug() << "AIWatchWindow::playNextMove() - Best move:"
             << "row=" << bestMove.row << "col=" << bestMove.col;

    // Execute the move
    executeMove(bestMove);

    // Check if game is over after this move
    if (isGameOver()) {
        gameState_ = GameWatchState::FINISHED;
        determineResult();
        updateButtonStates();

        if (moveTimer_) {
            moveTimer_->stop();
        }
        return;
    }

    // Schedule next move with delay
    if (moveTimer_) {
        moveTimer_->start(moveDelayMs_);
    }
}

void AIWatchWindow::closeEvent(QCloseEvent* event)
{
    qDebug() << "AIWatchWindow::closeEvent()";

    // Stop timer
    if (moveTimer_) {
        moveTimer_->stop();
        delete moveTimer_;
        moveTimer_ = nullptr;
    }

    event->accept();
}

} // namespace Reversi
