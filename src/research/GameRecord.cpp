/**
 * @file GameRecord.cpp
 * @brief 对局记录系统实现 - v0.9.0可视化增强版
 */

#include "research/GameRecord.h"
#include "Board.h"
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QtMath>
#include <QRegularExpression>
#include <functional>

namespace Reversi {

// ===== MoveRecord 实现 =====

QJsonObject MoveRecord::toJson() const {
    QJsonObject json;
    json["move"] = move;
    json["player"] = player;
    json["discCountBlack"] = discCountBlack;
    json["discCountWhite"] = discCountWhite;
    json["aiEvaluation"] = aiEvaluation;
    json["searchDepth"] = searchDepth;
    json["nodesSearched"] = static_cast<int>(nodesSearched);
    json["thinkingTime"] = thinkingTime;
    json["timestamp"] = timestamp.toString(Qt::ISODate);
    return json;
}

MoveRecord MoveRecord::fromJson(const QJsonObject& json) {
    MoveRecord record;
    record.move = json["move"].toInt(-1);
    record.player = json["player"].toInt(0);
    record.discCountBlack = json["discCountBlack"].toInt(0);
    record.discCountWhite = json["discCountWhite"].toInt(0);
    record.aiEvaluation = json["aiEvaluation"].toDouble(0.0);
    record.searchDepth = json["searchDepth"].toInt(0);
    record.nodesSearched = json["nodesSearched"].toInt(0);
    record.thinkingTime = json["thinkingTime"].toDouble(0.0);
    record.timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    return record;
}

QString MoveRecord::toCoordinateString() const {
    if (move < 0 || move >= 64) return "--";
    int row = move / 8;
    int col = move % 8;
    QString file = QString(QChar('A' + col));
    QString rank = QString(QChar('1' + row));
    return file + rank;
}

// ===== GameRecord 实现 =====

QJsonObject GameRecord::toJson() const {
    QJsonObject json;
    json["recordId"] = recordId;
    json["startTime"] = startTime.toString(Qt::ISODate);
    json["endTime"] = endTime.toString(Qt::ISODate);
    json["player1Type"] = player1Type;
    json["player1Name"] = player1Name;
    json["player1Difficulty"] = player1Difficulty;
    json["player2Type"] = player2Type;
    json["player2Name"] = player2Name;
    json["player2Difficulty"] = player2Difficulty;
    json["winner"] = winner;
    json["finalBlack"] = finalBlack;
    json["finalWhite"] = finalWhite;
    json["gameMode"] = gameMode;
    json["tournament"] = tournament;
    json["event"] = event;
    json["venue"] = venue;

    // 序列化走法列表
    QJsonArray movesArray;
    for (const auto& move : moves) {
        movesArray.append(move.toJson());
    }
    json["moves"] = movesArray;

    return json;
}

GameRecord GameRecord::fromJson(const QJsonObject& json) {
    GameRecord record;
    record.recordId = json["recordId"].toString();
    record.startTime = QDateTime::fromString(json["startTime"].toString(), Qt::ISODate);
    record.endTime = QDateTime::fromString(json["endTime"].toString(), Qt::ISODate);
    record.player1Type = json["player1Type"].toString();
    record.player1Name = json["player1Name"].toString();
    record.player1Difficulty = json["player1Difficulty"].toString();
    record.player2Type = json["player2Type"].toString();
    record.player2Name = json["player2Name"].toString();
    record.player2Difficulty = json["player2Difficulty"].toString();
    record.winner = json["winner"].toInt(-1);
    record.finalBlack = json["finalBlack"].toInt(0);
    record.finalWhite = json["finalWhite"].toInt(0);
    record.gameMode = json["gameMode"].toString("PvE");
    record.tournament = json["tournament"].toString();
    record.event = json["event"].toString();
    record.venue = json["venue"].toString();

    // 反序列化走法列表
    QJsonArray movesArray = json["moves"].toArray();
    for (int i = 0; i < movesArray.size(); ++i) {
        record.moves.append(MoveRecord::fromJson(movesArray[i].toObject()));
    }

    return record;
}

QString GameRecord::toPGN() const {
    QString pgn;

    // PGN 头部信息
    pgn += "[Event \"" + (event.isEmpty() ? "Reversi Game" : event) + "\"]\n";
    pgn += "[Site \"" + (venue.isEmpty() ? "Unknown" : venue) + "\"]\n";
    pgn += "[Date \"" + startTime.toString("yyyy.MM.dd") + "\"]\n";
    pgn += "[Time \"" + startTime.toString("hh:mm:ss") + "\"]\n";
    pgn += "[Player1 \"" + player1Name + "\"]\n";
    pgn += "[Player2 \"" + player2Name + "\"]\n";
    pgn += "[Result \"";

    switch (winner) {
        case 0: pgn += "1-0"; break;  // 黑胜
        case 1: pgn += "0-1"; break;  //白胜
        case 2: pgn += "1/2-1/2"; break; // 平局
        default: pgn += "*"; break;
    }
    pgn += "\"]\n";

    // 添加额外标签
    if (!player1Type.isEmpty()) {
        pgn += "[Player1Type \"" + player1Type + "\"]\n";
    }
    if (!player1Difficulty.isEmpty()) {
        pgn += "[Player1Elo \"" + player1Difficulty + "\"]\n";
    }
    if (!player2Difficulty.isEmpty()) {
        pgn += "[Player2Elo \"" + player2Difficulty + "\"]\n";
    }

    pgn += "\n";

    // 走法列表
    int moveNumber = 1;
    for (int i = 0; i < moves.size(); ++i) {
        const auto& move = moves[i];

        // 黑棋走法
        if (move.player == 0) {
            pgn += QString::number(moveNumber) + ". " + move.toCoordinateString();
            if (!qIsNull(move.aiEvaluation)) {
                pgn += " {" + QString::number(move.aiEvaluation, 'f', 2) + "}";
            }
            pgn += " ";
        } else {
            // 白棋走法
            pgn += move.toCoordinateString();
            if (!qIsNull(move.aiEvaluation)) {
                pgn += " {" + QString::number(move.aiEvaluation, 'f', 2) + "}";
            }
            pgn += " ";
            moveNumber++;
        }

        // 每行不超过80个字符
        if (i % 8 == 7) {
            pgn += "\n";
        }
    }

    // 添加结果
    switch (winner) {
        case 0: pgn += "1-0"; break;
        case 1: pgn += "0-1"; break;
        case 2: pgn += "1/2-1/2"; break;
        default: pgn += "*"; break;
    }

    return pgn;
}

QString GameRecord::toSGF() const {
    QString sgf;

    // SGF 头部
    sgf = "(;FF[4]GM[11]RU[Othello]\n";

    // 玩家信息
    sgf += "PB[" + player1Name + "]\n";
    sgf += "PW[" + player2Name + "]\n";

    // 日期时间
    sgf += "DT[" + startTime.toString("yyyy-MM-dd") + "]\n";
    sgf += "TM[" + QString::number(getDurationMs() / 1000) + "]\n";

    // 结果
    QString result;
    switch (winner) {
        case 0: result = "B+"; break;
        case 1: result = "W+"; break;
        case 2: result = "0"; break;
        default: result = ""; break;
    }
    if (!result.isEmpty()) {
        sgf += "RE[" + result + "]\n";
    }

    // 附加信息
    if (!event.isEmpty()) sgf += "EV[" + event + "]\n";
    if (!venue.isEmpty()) sgf += "PC[" + venue + "]\n";

    // 走法
    for (int i = 0; i < moves.size(); ++i) {
        const auto& move = moves[i];
        QString color = (move.player == 0) ? "B" : "W";
        // SGF坐标: 0-63 -> a1-h8
        int col = move.move % 8;
        int row = move.move / 8;
        QString coord;
        coord.append(QChar('a' + col));
        coord.append(QChar('a' + row));
        sgf += color + "[" + coord + "]";
    }

    sgf += ")";

    return sgf;
}

GameRecord GameRecord::fromPGN(const QString& pgn) {
    GameRecord record;
    // 简化实现 - 完整PGN解析需要更复杂的解析器
    // 这里仅提供基本框架

    QStringList lines = pgn.split('\n');
    for (const QString& line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith('[')) {
            // 解析标签
            int start = trimmed.indexOf('"');
            int end = trimmed.lastIndexOf('"');
            if (start >= 0 && end > start) {
                QString tag = trimmed.mid(1, start - 1);
                QString value = trimmed.mid(start + 1, end - start - 1);

                if (tag == "Player1") record.player1Name = value;
                else if (tag == "Player2") record.player2Name = value;
                else if (tag == "Event") record.event = value;
                else if (tag == "Site") record.venue = value;
                else if (tag == "Date") {
                    QStringList parts = value.split('.');
                    if (parts.size() >= 3) {
                        QDate date(parts[0].toInt(), parts[1].toInt(), parts[2].toInt());
                        record.startTime = QDateTime(date, QTime(0, 0, 0));
                    }
                }
                else if (tag == "Result") {
                    if (value == "1-0") record.winner = 0;
                    else if (value == "0-1") record.winner = 1;
                    else if (value == "1/2-1/2") record.winner = 2;
                }
            }
        } else if (!trimmed.isEmpty() && !trimmed.startsWith('(')) {
            // 解析走法
            // 简化: 实际需要更复杂的解析
        }
    }

    return record;
}

GameRecord GameRecord::fromSGF(const QString& sgf) {
    GameRecord record;
    // 简化实现 - 完整SGF解析需要更复杂的解析器
    // 这里仅提供基本框架

    // 提取基本标签
    QRegularExpression pbRegex("PB\\[([^\\]]+)\\]");
    QRegularExpressionMatch match = pbRegex.match(sgf);
    if (match.hasMatch()) {
        record.player1Name = match.captured(1);
    }

    QRegularExpression pwRegex("PW\\[([^\\]]+)\\]");
    match = pwRegex.match(sgf);
    if (match.hasMatch()) {
        record.player2Name = match.captured(1);
    }

    QRegularExpression reRegex("RE\\[([^\\]]+)\\]");
    match = reRegex.match(sgf);
    if (match.hasMatch()) {
        QString result = match.captured(1);
        if (result.startsWith("B+")) record.winner = 0;
        else if (result.startsWith("W+")) record.winner = 1;
        else if (result == "0") record.winner = 2;
    }

    // 提取走法
    // 简化实现 - 暂不解析具体走法

    return record;
}

// ===== GameReplay 实现 =====

GameReplay::GameReplay()
    : currentBoard_(nullptr)
    , currentMoveIndex_(0)
    , isPlaying_(false)
    , playbackSpeed_(1.0)
    , timerInterval_(DEFAULT_INTERVAL_MS)
{
}

bool GameReplay::loadRecord(const GameRecord& record) {
    if (record.moves.isEmpty()) {
        return false;
    }

    record_ = record;
    currentMoveIndex_ = 0;
    isPlaying_ = false;

    // 重建初始棋盘
    rebuildBoard(0);

    // 触发回调
    if (boardUpdateCallback_) {
        boardUpdateCallback_(currentBoard_);
    }
    if (currentMoveIndex_ < record_.moves.size() && moveChangeCallback_) {
        moveChangeCallback_(currentMoveIndex_, record_.moves[currentMoveIndex_]);
    }

    return true;
}

void GameReplay::play() {
    if (record_.moves.isEmpty()) return;
    if (isFinished()) {
        // 如果已经播放完毕，重新开始
        jumpToMove(0);
    }

    isPlaying_ = true;
    updateTimerInterval();

    if (playStateChangedCallback_) {
        playStateChangedCallback_(true);
    }
}

void GameReplay::pause() {
    isPlaying_ = false;

    if (playStateChangedCallback_) {
        playStateChangedCallback_(false);
    }
}

void GameReplay::stop() {
    isPlaying_ = false;
    jumpToMove(0);

    if (playStateChangedCallback_) {
        playStateChangedCallback_(false);
    }
}

void GameReplay::stepForward() {
    if (currentMoveIndex_ < record_.moves.size() - 1) {
        ++currentMoveIndex_;
        rebuildBoard(currentMoveIndex_);

        if (boardUpdateCallback_) {
            boardUpdateCallback_(currentBoard_);
        }
        if (moveChangeCallback_) {
            moveChangeCallback_(currentMoveIndex_, record_.moves[currentMoveIndex_]);
        }

        if (isFinished() && playbackFinishedCallback_) {
            playbackFinishedCallback_();
        }
    }
}

void GameReplay::stepBackward() {
    if (currentMoveIndex_ > 0) {
        --currentMoveIndex_;
        rebuildBoard(currentMoveIndex_);

        if (boardUpdateCallback_) {
            boardUpdateCallback_(currentBoard_);
        }
        if (moveChangeCallback_) {
            moveChangeCallback_(currentMoveIndex_, record_.moves[currentMoveIndex_]);
        }
    }
}

void GameReplay::jumpToMove(int moveIndex) {
    if (moveIndex >= 0 && moveIndex < record_.moves.size()) {
        currentMoveIndex_ = moveIndex;
        rebuildBoard(currentMoveIndex_);

        if (boardUpdateCallback_) {
            boardUpdateCallback_(currentBoard_);
        }
        if (moveChangeCallback_) {
            moveChangeCallback_(currentMoveIndex_, record_.moves[currentMoveIndex_]);
        }
    }
}

void GameReplay::setPlaybackSpeed(double speed) {
    playbackSpeed_ = qBound(0.25, speed, 4.0);
    updateTimerInterval();
}

const MoveRecord& GameReplay::getCurrentMove() const {
    if (currentMoveIndex_ >= 0 && currentMoveIndex_ < record_.moves.size()) {
        return record_.moves[currentMoveIndex_];
    }
    static const MoveRecord empty;
    return empty;
}

void GameReplay::rebuildBoard(int moveIndex) {
    // 从头开始重建棋盘到指定步数
    currentBoard_ = new Board();

    for (int i = 0; i <= moveIndex && i < record_.moves.size(); ++i) {
        Move move;
        int pos = record_.moves[i].move;
        move.row = pos / 8;
        move.col = pos % 8;
        move.is_pass = false;
        currentBoard_->makeMove(move);
    }
}

void GameReplay::updateTimerInterval() {
    timerInterval_ = static_cast<int>(DEFAULT_INTERVAL_MS / playbackSpeed_);
}

void GameReplay::onTimerTriggered() {
    if (isFinished()) {
        pause();
        if (playbackFinishedCallback_) {
            playbackFinishedCallback_();
        }
    } else {
        stepForward();
    }
}

} // namespace Reversi
