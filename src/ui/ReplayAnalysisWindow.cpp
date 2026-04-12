/**
 * @file ReplayAnalysisWindow.cpp
 * @brief Replay analysis window implementation - v0.9.0 visualization enhancement
 */

#include "ui/ReplayAnalysisWindow.h"
#include "Board.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>

namespace Reversi {

ReplayAnalysisWindow::ReplayAnalysisWindow(QWidget* parent)
    : QMainWindow(parent)
    , centralWidget_(nullptr)
    , mainLayout_(nullptr)
    , boardLabel_(nullptr)
    , controlGroup_(nullptr)
    , controlLayout_(nullptr)
    , moveSlider_(nullptr)
    , moveIndexLabel_(nullptr)
    , progressLabel_(nullptr)
    , playPauseButton_(nullptr)
    , stopButton_(nullptr)
    , stepBackwardButton_(nullptr)
    , stepForwardButton_(nullptr)
    , speedComboBox_(nullptr)
    , playerInfoLabel_(nullptr)
    , moveInfoLabel_(nullptr)
    , playbackTimer_(nullptr)
    , isLoaded_(false)
{
    setupUI();
    setupMenu();
    setupToolbar();
    setupConnections();

    setWindowTitle(tr("Game Replay - Reversi AI Platform"));
    resize(800, 700);
}

ReplayAnalysisWindow::~ReplayAnalysisWindow() = default;

void ReplayAnalysisWindow::setupUI() {
    // Central widget
    centralWidget_ = new QWidget(this);
    setCentralWidget(centralWidget_);

    // Main layout
    mainLayout_ = new QVBoxLayout(centralWidget_);
    mainLayout_->setContentsMargins(10, 10, 10, 10);
    mainLayout_->setSpacing(10);

    // Board display
    boardLabel_ = new QLabel(this);
    boardLabel_->setAlignment(Qt::AlignCenter);
    boardLabel_->setMinimumSize(400, 400);
    boardLabel_->setStyleSheet("background-color: #769656; border: 2px solid #333;");
    mainLayout_->addWidget(boardLabel_, 1);

    // Playback control area
    controlGroup_ = new QGroupBox(tr("Playback Control"), this);
    controlLayout_ = new QHBoxLayout();

    // Slider
    moveSlider_ = new QSlider(Qt::Horizontal, this);
    moveSlider_->setRange(0, 0);
    moveSlider_->setTickPosition(QSlider::TicksBelow);
    moveSlider_->setTickInterval(1);
    controlLayout_->addWidget(moveSlider_);

    // Step display
    moveIndexLabel_ = new QLabel("0/0", this);
    moveIndexLabel_->setMinimumWidth(60);
    moveIndexLabel_->setAlignment(Qt::AlignCenter);
    controlLayout_->addWidget(moveIndexLabel_);

    // Spacer
    controlLayout_->addSpacing(10);

    // Playback control buttons
    stepBackwardButton_ = new QPushButton("<<", this);
    stepBackwardButton_->setFixedWidth(40);
    controlLayout_->addWidget(stepBackwardButton_);

    playPauseButton_ = new QPushButton("Play", this);
    playPauseButton_->setFixedWidth(60);
    controlLayout_->addWidget(playPauseButton_);

    stepForwardButton_ = new QPushButton(">>", this);
    stepForwardButton_->setFixedWidth(40);
    controlLayout_->addWidget(stepForwardButton_);

    stopButton_ = new QPushButton("Stop", this);
    stopButton_->setFixedWidth(60);
    controlLayout_->addWidget(stopButton_);

    // Spacer
    controlLayout_->addSpacing(10);

    // Speed selection
    speedComboBox_ = new QComboBox(this);
    speedComboBox_->addItem("0.25x", 0.25);
    speedComboBox_->addItem("0.5x", 0.5);
    speedComboBox_->addItem("1x", 1.0);
    speedComboBox_->addItem("2x", 2.0);
    speedComboBox_->addItem("4x", 4.0);
    speedComboBox_->setCurrentIndex(2); // Default: 1x
    controlLayout_->addWidget(new QLabel(tr("Speed:"), this));
    controlLayout_->addWidget(speedComboBox_);

    controlGroup_->setLayout(controlLayout_);
    mainLayout_->addWidget(controlGroup_);

    // Information display
    playerInfoLabel_ = new QLabel(this);
    playerInfoLabel_->setStyleSheet("padding: 5px; background-color: #333; color: #ccc;");
    mainLayout_->addWidget(playerInfoLabel_);

    moveInfoLabel_ = new QLabel(this);
    moveInfoLabel_->setStyleSheet("padding: 5px; background-color: #222; color: #aaa;");
    mainLayout_->addWidget(moveInfoLabel_);

    // Set button states
    updateControlButtons();
}

void ReplayAnalysisWindow::setupMenu() {
    QMenuBar* menuBar = this->menuBar();

    // File menu
    QMenu* fileMenu = menuBar->addMenu(tr("File"));

    QAction* openAction = new QAction(tr("Open..."), this);
    openAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openAction);
    connect(openAction, &QAction::triggered, this, &ReplayAnalysisWindow::onOpenFile);

    fileMenu->addSeparator();

    QAction* exportPgnAction = new QAction(tr("Export as PGN..."), this);
    fileMenu->addAction(exportPgnAction);
    connect(exportPgnAction, &QAction::triggered, this, &ReplayAnalysisWindow::onExportPGN);

    QAction* exportSgfAction = new QAction(tr("Export as SGF..."), this);
    fileMenu->addAction(exportSgfAction);
    connect(exportSgfAction, &QAction::triggered, this, &ReplayAnalysisWindow::onExportSGF);

    fileMenu->addSeparator();

    QAction* exitAction = new QAction(tr("Exit"), this);
    exitAction->setShortcut(QKeySequence::Quit);
    fileMenu->addAction(exitAction);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
}

void ReplayAnalysisWindow::setupToolbar() {
    QToolBar* toolbar = addToolBar(tr("Toolbar"));
    toolbar->setMovable(false);

    QAction* backAction = new QAction(tr("Back to Menu"), this);
    toolbar->addAction(backAction);
    connect(backAction, &QAction::triggered, this, &ReplayAnalysisWindow::onBackClicked);
}

void ReplayAnalysisWindow::setupConnections() {
    // Playback control
    connect(playPauseButton_, &QPushButton::clicked, this, &ReplayAnalysisWindow::onPlayPauseClicked);
    connect(stopButton_, &QPushButton::clicked, this, &ReplayAnalysisWindow::onStopClicked);
    connect(stepBackwardButton_, &QPushButton::clicked, this, &ReplayAnalysisWindow::onStepBackwardClicked);
    connect(stepForwardButton_, &QPushButton::clicked, this, &ReplayAnalysisWindow::onStepForwardClicked);
    connect(moveSlider_, &QSlider::valueChanged, this, &ReplayAnalysisWindow::onSliderValueChanged);
    connect(speedComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReplayAnalysisWindow::onPlaybackSpeedChanged);

    // Set playback callbacks
    replay_.setBoardUpdateCallback([this](Board* board) {
        onBoardUpdated(board);
    });
    replay_.setMoveChangeCallback([this](int index, const MoveRecord& move) {
        onMoveChanged(index, move);
    });
    replay_.setPlaybackFinishedCallback([this]() {
        onPlaybackFinished();
    });
    replay_.setPlayStateChangedCallback([this](bool isPlaying) {
        updateControlButtons();
    });

    // Connect timer
    playbackTimer_ = new QTimer(this);
    connect(playbackTimer_, &QTimer::timeout, this, &ReplayAnalysisWindow::onTimerTimeout);
}

bool ReplayAnalysisWindow::loadGameFile(const QString& filepath) {
    QFileInfo fileInfo(filepath);
    QString suffix = fileInfo.suffix().toLower();

    if (suffix == "json") {
        return loadJsonFile(filepath);
    } else if (suffix == "pgn") {
        QFile file(filepath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Error"), tr("Cannot open file: ") + filepath);
            return false;
        }
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        currentRecord_ = GameRecord::fromPGN(content);
        loadRecord(currentRecord_);
        return true;
    } else if (suffix == "sgf") {
        QFile file(filepath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Error"), tr("Cannot open file: ") + filepath);
            return false;
        }
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        currentRecord_ = GameRecord::fromSGF(content);
        loadRecord(currentRecord_);
        return true;
    }

    QMessageBox::warning(this, tr("Error"), tr("Unsupported file format: ") + suffix);
    return false;
}

bool ReplayAnalysisWindow::loadJsonFile(const QString& filepath) {
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file: ") + filepath);
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid JSON: ") + parseError.errorString());
        return false;
    }

    if (!doc.isObject()) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid JSON format"));
        return false;
    }

    currentRecord_ = GameRecord::fromJson(doc.object());
    loadRecord(currentRecord_);
    return true;
}

void ReplayAnalysisWindow::loadRecord(const GameRecord& record) {
    if (record.moves.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("No moves in record"));
        return;
    }

    currentRecord_ = record;
    isLoaded_ = true;

    // Load to replay controller
    replay_.loadRecord(record);

    // Update slider range
    moveSlider_->setRange(0, record.moves.size() - 1);
    moveSlider_->setValue(0);

    // Update display
    updateDisplay();
    updateControlButtons();
}

void ReplayAnalysisWindow::updateDisplay() {
    if (!isLoaded_) return;

    // Update player info
    QString info = QString("%1 (%2) vs %3 (%4)")
                      .arg(currentRecord_.player1Name)
                      .arg(currentRecord_.player1Type)
                      .arg(currentRecord_.player2Name)
                      .arg(currentRecord_.player2Type);
    playerInfoLabel_->setText(info);
}

void ReplayAnalysisWindow::updateControlButtons() {
    if (!isLoaded_) {
        playPauseButton_->setEnabled(false);
        stopButton_->setEnabled(false);
        stepBackwardButton_->setEnabled(false);
        stepForwardButton_->setEnabled(false);
        moveSlider_->setEnabled(false);
        return;
    }

    bool isPlaying = replay_.isPlaying();
    bool isAtStart = replay_.getCurrentMoveIndex() == 0;
    bool isAtEnd = replay_.isFinished();

    playPauseButton_->setEnabled(true);
    playPauseButton_->setText(isPlaying ? "Pause" : "Play");

    stopButton_->setEnabled(!isAtStart);
    stepBackwardButton_->setEnabled(!isAtStart && !isPlaying);
    stepForwardButton_->setEnabled(!isAtEnd && !isPlaying);
    moveSlider_->setEnabled(!isPlaying);
}

void ReplayAnalysisWindow::onOpenFile() {
    QString filepath = QFileDialog::getOpenFileName(
        this,
        tr("Open Game Record"),
        QString(),
        tr("Game Files (*.json *.pgn *.sgf);;JSON Files (*.json);;PGN Files (*.pgn);;SGF Files (*.sgf);;All Files (*.*)")
    );

    if (!filepath.isEmpty()) {
        loadGameFile(filepath);
    }
}

void ReplayAnalysisWindow::onExportPGN() {
    if (!isLoaded_) {
        QMessageBox::warning(this, tr("Error"), tr("No game loaded"));
        return;
    }

    QString filepath = QFileDialog::getSaveFileName(
        this,
        tr("Export as PGN"),
        QString(),
        tr("PGN Files (*.pgn);;All Files (*.*)")
    );

    if (!filepath.isEmpty()) {
        QFile file(filepath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Error"), tr("Cannot create file: ") + filepath);
            return;
        }

        QTextStream out(&file);
        out << currentRecord_.toPGN();
        file.close();

        QMessageBox::information(this, tr("Success"), tr("Game exported to PGN successfully"));
    }
}

void ReplayAnalysisWindow::onExportSGF() {
    if (!isLoaded_) {
        QMessageBox::warning(this, tr("Error"), tr("No game loaded"));
        return;
    }

    QString filepath = QFileDialog::getSaveFileName(
        this,
        tr("Export as SGF"),
        QString(),
        tr("SGF Files (*.sgf);;All Files (*.*)")
    );

    if (!filepath.isEmpty()) {
        QFile file(filepath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Error"), tr("Cannot create file: ") + filepath);
            return;
        }

        QTextStream out(&file);
        out << currentRecord_.toSGF();
        file.close();

        QMessageBox::information(this, tr("Success"), tr("Game exported to SGF successfully"));
    }
}

void ReplayAnalysisWindow::onPlaybackSpeedChanged(int index) {
    double speed = speedComboBox_->itemData(index).toDouble();
    replay_.setPlaybackSpeed(speed);
}

void ReplayAnalysisWindow::onPlayPauseClicked() {
    if (replay_.isPlaying()) {
        replay_.pause();
        playbackTimer_->stop();
    } else {
        replay_.play();
        // Get playback interval and start timer
        // Simplified: use fixed interval
        playbackTimer_->start(1000);
    }
}

void ReplayAnalysisWindow::onStopClicked() {
    replay_.stop();
    playbackTimer_->stop();
    moveSlider_->setValue(0);
}

void ReplayAnalysisWindow::onStepBackwardClicked() {
    replay_.stepBackward();
    moveSlider_->setValue(replay_.getCurrentMoveIndex());
}

void ReplayAnalysisWindow::onStepForwardClicked() {
    replay_.stepForward();
    moveSlider_->setValue(replay_.getCurrentMoveIndex());
}

void ReplayAnalysisWindow::onSliderValueChanged(int value) {
    replay_.jumpToMove(value);
}

void ReplayAnalysisWindow::onBackClicked() {
    replay_.stop();
    emit backToMenu();
}

void ReplayAnalysisWindow::onBoardUpdated(Board* board) {
    Q_UNUSED(board);
    // Board drawing code can be added here
    // Simplified implementation: only update statistics for now
}

void ReplayAnalysisWindow::onMoveChanged(int moveIndex, const MoveRecord& move) {
    // Update step display
    moveIndexLabel_->setText(QString("%1/%2").arg(moveIndex + 1).arg(currentRecord_.moves.size()));

    // Update move info
    QString moveInfo = QString("Move %1: %2 - %3 (Black: %4, White: %5)")
                           .arg(moveIndex + 1)
                           .arg(move.toCoordinateString())
                           .arg(move.player == 0 ? "Black" : "White")
                           .arg(move.discCountBlack)
                           .arg(move.discCountWhite);
    moveInfoLabel_->setText(moveInfo);

    // Update slider
    moveSlider_->blockSignals(true);
    moveSlider_->setValue(moveIndex);
    moveSlider_->blockSignals(false);
}

void ReplayAnalysisWindow::onPlaybackFinished() {
    playPauseButton_->setText("Play");
    updateControlButtons();
    playbackTimer_->stop();
}

void ReplayAnalysisWindow::onTimerTimeout() {
    replay_.onTimerTriggered();
}

} // namespace Reversi
