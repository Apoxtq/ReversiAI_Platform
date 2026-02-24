/**
 * @file StatisticsManager.cpp
 * @brief 游戏统计管理器实现
 */

#include "ui/StatisticsManager.h"
#include <QJsonArray>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>

namespace Reversi {

StatisticsManager::StatisticsManager(QObject* parent)
    : QObject(parent)
    , currentMoveCount_(0) {
    // 设置默认历史文件路径
    historyFilePath_ = getDefaultHistoryPath();

    // 尝试加载历史记录
    loadFromFile();
}

StatisticsManager::~StatisticsManager() {
    // 保存历史记录
    saveToFile();
}

void StatisticsManager::recordGame(const GameRecord& record) {
    history_.append(record);

    // 保存到文件
    saveToFile();

    // 发送信号
    emit newGameRecorded(record);
    emit statsUpdated();

    qDebug() << "StatisticsManager: Recorded game -"
             << "Result:" << gameResultToString(record.result)
             << "Moves:" << record.moveCount
             << "Duration:" << record.durationSeconds << "s";
}

StatisticsManager::GameRecord StatisticsManager::createRecordFromCurrentGame(
    GameMode mode,
    const QString& aiType,
    Difficulty difficulty,
    const Board& finalBoard,
    int moveCount,
    int durationSeconds,
    const QString& humanColor) {

    GameRecord record;
    record.timestamp = gameStartTime_;
    record.mode = mode;
    record.aiType = aiType;
    record.difficulty = difficulty;
    record.moveCount = moveCount;
    record.durationSeconds = durationSeconds;
    record.humanColor = humanColor;

    // 统计棋子数量
    record.blackCount = 0;
    record.whiteCount = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int cell = finalBoard.at(i, j);
            if (cell == 2) record.blackCount++;
            else if (cell == 1) record.whiteCount++;
        }
    }

    // 判断游戏结果
    if (record.blackCount > record.whiteCount) {
        record.result = GameResult::BlackWins;
        record.humanWon = (humanColor == "Black");
    } else if (record.whiteCount > record.blackCount) {
        record.result = GameResult::WhiteWins;
        record.humanWon = (humanColor == "White");
    } else {
        record.result = GameResult::Draw;
        record.humanWon = false;
    }

    return record;
}

QVector<StatisticsManager::GameRecord> StatisticsManager::getRecentGames(int count) const {
    QVector<GameRecord> recent;
    int start = qMax(0, history_.size() - count);
    for (int i = history_.size() - 1; i >= start; i--) {
        recent.append(history_[i]);
    }
    return recent;
}

QVector<StatisticsManager::GameRecord> StatisticsManager::getGamesByMode(GameMode mode) const {
    QVector<GameRecord> result;
    for (const auto& record : history_) {
        if (record.mode == mode) {
            result.append(record);
        }
    }
    return result;
}

QVector<StatisticsManager::GameRecord> StatisticsManager::getGamesByAI(const QString& aiType) const {
    QVector<GameRecord> result;
    for (const auto& record : history_) {
        if (record.aiType == aiType) {
            result.append(record);
        }
    }
    return result;
}

int StatisticsManager::getTotalGames() const {
    return history_.size();
}

int StatisticsManager::getWins(const QString& aiType) const {
    int wins = 0;
    for (const auto& record : history_) {
        if (!aiType.isEmpty() && record.aiType != aiType) continue;

        if (record.humanWon) {
            wins++;
        }
    }
    return wins;
}

int StatisticsManager::getLosses(const QString& aiType) const {
    int losses = 0;
    for (const auto& record : history_) {
        if (!aiType.isEmpty() && record.aiType != aiType) continue;

        if (!record.humanWon && record.result != GameResult::Draw) {
            losses++;
        }
    }
    return losses;
}

int StatisticsManager::getDraws(const QString& aiType) const {
    int draws = 0;
    for (const auto& record : history_) {
        if (!aiType.isEmpty() && record.aiType != aiType) continue;

        if (record.result == GameResult::Draw) {
            draws++;
        }
    }
    return draws;
}

double StatisticsManager::getWinRate(const QString& aiType) const {
    int total = 0;
    int wins = 0;

    for (const auto& record : history_) {
        if (!aiType.isEmpty() && record.aiType != aiType) continue;

        if (record.result != GameResult::Unknown) {
            total++;
            if (record.humanWon) {
                wins++;
            }
        }
    }

    if (total == 0) return -1.0;
    return static_cast<double>(wins) / total;
}

double StatisticsManager::getAverageMoves() const {
    if (history_.empty()) return 0.0;

    int total = 0;
    for (const auto& record : history_) {
        total += record.moveCount;
    }
    return static_cast<double>(total) / history_.size();
}

double StatisticsManager::getAverageDuration() const {
    if (history_.empty()) return 0.0;

    int total = 0;
    for (const auto& record : history_) {
        total += record.durationSeconds;
    }
    return static_cast<double>(total) / history_.size();
}

void StatisticsManager::clearHistory() {
    history_.clear();
    emit statsUpdated();
}

void StatisticsManager::onGameEnded(GameResult result, int blackCount, int whiteCount,
                                     int moveCount, const QString& humanColor) {
    // 创建记录
    GameRecord record;
    record.timestamp = gameStartTime_;
    record.mode = GameMode::PvE;  // 默认
    record.aiType = "MCTS";
    record.difficulty = Difficulty::MEDIUM;
    record.result = result;
    record.blackCount = blackCount;
    record.whiteCount = whiteCount;
    record.moveCount = moveCount;

    // 计算游戏时长
    int duration = 0;
    if (gameStartTime_.isValid()) {
        duration = gameStartTime_.secsTo(QDateTime::currentDateTime());
    }
    record.durationSeconds = duration;

    record.humanColor = humanColor;
    record.humanWon = (humanColor == "Black" && result == GameResult::BlackWins) ||
                      (humanColor == "White" && result == GameResult::WhiteWins);

    recordGame(record);
}

bool StatisticsManager::exportToCSV(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit errorOccurred(QString("Failed to open file: %1").arg(filename));
        return false;
    }

    QTextStream out(&file);

    // 写入CSV头部
    out << "Time,Mode,AI Type,Difficulty,Result,Black,White,Moves,Duration,Human Color,Human Won\n";

    // 写入每条记录
    for (const auto& record : history_) {
        out << record.timestamp.toString("yyyy-MM-dd hh:mm:ss") << ","
            << gameModeToString(record.mode) << ","
            << record.aiType << ","
            << difficultyToString(record.difficulty) << ","
            << gameResultToString(record.result) << ","
            << record.blackCount << ","
            << record.whiteCount << ","
            << record.moveCount << ","
            << record.durationSeconds << ","
            << record.humanColor << ","
            << (record.humanWon ? "YES" : "NO") << "\n";
    }

    file.close();
    emit exportCompleted(true, filename);
    return true;
}

bool StatisticsManager::exportToJSON(const QString& filename) {
    QJsonArray gamesArray;

    for (const auto& record : history_) {
        QJsonObject gameObj;
        gameObj["timestamp"] = record.timestamp.toString(Qt::ISODate);
        gameObj["mode"] = gameModeToString(record.mode);
        gameObj["aiType"] = record.aiType;
        gameObj["difficulty"] = difficultyToString(record.difficulty);
        gameObj["result"] = gameResultToString(record.result);
        gameObj["blackCount"] = record.blackCount;
        gameObj["whiteCount"] = record.whiteCount;
        gameObj["moveCount"] = record.moveCount;
        gameObj["durationSeconds"] = record.durationSeconds;
        gameObj["humanColor"] = record.humanColor;
        gameObj["humanWon"] = record.humanWon;

        gamesArray.append(gameObj);
    }

    QJsonObject rootObj;
    rootObj["version"] = "1.0";
    rootObj["totalGames"] = history_.size();
    rootObj["games"] = gamesArray;

    QJsonDocument doc(rootObj);

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred(QString("Failed to open file: %1").arg(filename));
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    emit exportCompleted(true, filename);
    return true;
}

bool StatisticsManager::importFromJSON(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(QString("Failed to open file: %1").arg(filename));
        emit importCompleted(false, 0);
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        emit errorOccurred(QString("JSON parse error: %1").arg(error.errorString()));
        emit importCompleted(false, 0);
        return false;
    }

    QJsonObject rootObj = doc.object();
    QJsonArray gamesArray = rootObj["games"].toArray();

    int imported = 0;
    for (int i = 0; i < gamesArray.size(); i++) {
        QJsonObject gameObj = gamesArray[i].toObject();

        GameRecord record;
        record.timestamp = QDateTime::fromString(gameObj["timestamp"].toString(), Qt::ISODate);
        record.mode = stringToGameMode(gameObj["mode"].toString());
        record.aiType = gameObj["aiType"].toString();
        record.difficulty = stringToDifficulty(gameObj["difficulty"].toString());
        record.result = stringToGameResult(gameObj["result"].toString());
        record.blackCount = gameObj["blackCount"].toInt();
        record.whiteCount = gameObj["whiteCount"].toInt();
        record.moveCount = gameObj["moveCount"].toInt();
        record.durationSeconds = gameObj["durationSeconds"].toInt();
        record.humanColor = gameObj["humanColor"].toString();
        record.humanWon = gameObj["humanWon"].toBool();

        history_.append(record);
        imported++;
    }

    // 保存导入的数据
    saveToFile();
    emit importCompleted(true, imported);
    emit statsUpdated();

    return true;
}

bool StatisticsManager::saveToFile(const QString& filename) {
    QString targetFile = filename.isEmpty() ? historyFilePath_ : filename;
    return exportToJSON(targetFile);
}

bool StatisticsManager::loadFromFile(const QString& filename) {
    QString targetFile = filename.isEmpty() ? historyFilePath_ : filename;

    QFile file(targetFile);
    if (!file.exists()) {
        // 文件不存在不是错误，只是没有历史记录
        return true;
    }

    return importFromJSON(targetFile);
}

QString StatisticsManager::getDefaultHistoryPath() {
    // 在用户数据目录创建历史文件
    QString dataPath = QDir(QCoreApplication::applicationDirPath()).filePath("game_history.json");
    return dataPath;
}

QString StatisticsManager::gameResultToString(GameResult result) {
    switch (result) {
    case GameResult::BlackWins: return "BlackWins";
    case GameResult::WhiteWins: return "WhiteWins";
    case GameResult::Draw: return "Draw";
    default: return "Unknown";
    }
}

GameResult StatisticsManager::stringToGameResult(const QString& str) {
    if (str == "BlackWins") return GameResult::BlackWins;
    if (str == "WhiteWins") return GameResult::WhiteWins;
    if (str == "Draw") return GameResult::Draw;
    return GameResult::Unknown;
}

QString StatisticsManager::difficultyToString(Difficulty difficulty) {
    switch (difficulty) {
    case Difficulty::EASY: return "EASY";
    case Difficulty::MEDIUM: return "MEDIUM";
    case Difficulty::HARD: return "HARD";
    case Difficulty::EXPERT: return "EXPERT";
    default: return "MEDIUM";
    }
}

Difficulty StatisticsManager::stringToDifficulty(const QString& str) {
    if (str == "EASY") return Difficulty::EASY;
    if (str == "MEDIUM") return Difficulty::MEDIUM;
    if (str == "HARD") return Difficulty::HARD;
    if (str == "EXPERT") return Difficulty::EXPERT;
    return Difficulty::MEDIUM;
}

QString StatisticsManager::gameModeToString(GameMode mode) {
    switch (mode) {
    case GameMode::PvP: return "PvP";
    case GameMode::PvE: return "PvE";
    case GameMode::AIvAI: return "AIvAI";
    default: return "Unknown";
    }
}

GameMode StatisticsManager::stringToGameMode(const QString& str) {
    if (str == "PvP") return GameMode::PvP;
    if (str == "PvE") return GameMode::PvE;
    if (str == "AIvAI") return GameMode::AIvAI;
    return GameMode::PvE;
}

} // namespace Reversi

