#include "research/PositionSuite.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <bitset>

namespace Reversi {

// ============================================================================
// TestPosition 实现
// ============================================================================

std::string TestPosition::toString() const {
    std::ostringstream oss;
    oss << "[" << name << "] ";
    oss << "Player: " << (player == PlayerColor::Black ? "Black" : "White");
    oss << ", Difficulty: " << difficulty;
    oss << ", Source: " << source;
    return oss.str();
}

// ============================================================================
// PositionSuite 实现
// ============================================================================

std::vector<TestPosition> PositionSuite::getStandard64() {
    std::vector<TestPosition> positions;

    // 开局位置 (0-20步)
    for (int i = 0; i < 15; ++i) {
        TestPosition pos;
        pos.name = "Opening_" + std::to_string(i + 1);
        pos.difficulty = 1 + (i / 3);
        pos.source = "Standard";
        pos.player = PlayerColor::Black;

        // 生成指定步数的随机开局位置
        std::string board_str(64, '.');
        board_str[27] = 'W'; board_str[28] = 'B';
        board_str[35] = 'B'; board_str[36] = 'W';

        // 根据步数添加棋子
        int pos_idx[] = {19, 26, 37, 44};
        for (int j = 0; j < std::min(i, 4); ++j) {
            if (j % 2 == 0) board_str[pos_idx[j]] = 'B';
            else board_str[pos_idx[j]] = 'W';
        }

        pos.board = BitBoard(board_str);
        positions.push_back(pos);
    }

    // 预设的合法落子序列（每步是 row, col）
    static const std::vector<std::pair<int,int>> MOVE_SEQ = {
        {2,3},{2,4},{2,5},{3,5},{4,5},{5,5},{5,4},{5,3},
        {5,2},{4,2},{3,2},{2,2},{1,3},{1,4},{1,5},{1,6},
        {2,6},{3,6},{4,6},{5,6},{6,5},{6,4},{6,3},{6,2},
        {6,1},{5,1},{4,1},{3,1},{2,1},{1,2}
    };

    // 中局位置 (21-40步)
    for (int i = 0; i < 20; ++i) {
        TestPosition pos;
        pos.name = "Midgame_" + std::to_string(i + 1);
        pos.difficulty = 5 + (i / 4);
        pos.source = "Standard";

        BitBoard board;
        board.resetToStandardOpening();
        int steps = 10 + i;
        PlayerColor current = PlayerColor::Black;
        for (int s = 0; s < steps && s < (int)MOVE_SEQ.size(); ++s) {
            auto [r, c] = MOVE_SEQ[s];
            uint64_t valid = board.getValidMoves(current);
            if (valid & (1ULL << (r * 8 + c))) {
                board.makeMove(r, c, current);
            }
            current = (current == PlayerColor::Black) ?
                      PlayerColor::White : PlayerColor::Black;
        }
        pos.board = board;
        pos.player = current;
        positions.push_back(pos);
    }

    // 残局位置 (41-60步)
    for (int i = 0; i < 29; ++i) {
        TestPosition pos;
        pos.name = "Endgame_" + std::to_string(i + 1);
        pos.difficulty = 8 + (i / 5);
        pos.source = "Standard";

        BitBoard board;
        board.resetToStandardOpening();
        int steps = std::min(20 + i, (int)MOVE_SEQ.size());
        PlayerColor current = PlayerColor::Black;
        for (int s = 0; s < steps; ++s) {
            auto [r, c] = MOVE_SEQ[s];
            uint64_t valid = board.getValidMoves(current);
            if (valid & (1ULL << (r * 8 + c))) {
                board.makeMove(r, c, current);
            }
            current = (current == PlayerColor::Black) ?
                      PlayerColor::White : PlayerColor::Black;
        }
        pos.board = board;
        pos.player = current;
        positions.push_back(pos);
    }

    return positions;
}

std::vector<TestPosition> PositionSuite::getOpening() {
    std::vector<TestPosition> all = getStandard64();
    std::vector<TestPosition> opening;

    for (const auto& pos : all) {
        if (getMoveCount(pos.board) <= 20) {
            opening.push_back(pos);
        }
    }

    return opening;
}

std::vector<TestPosition> PositionSuite::getMidgame() {
    std::vector<TestPosition> all = getStandard64();
    std::vector<TestPosition> midgame;

    for (const auto& pos : all) {
        int m = getMoveCount(pos.board);
        if (m > 20 && m <= 40) {
            midgame.push_back(pos);
        }
    }

    return midgame;
}

std::vector<TestPosition> PositionSuite::getEndgame() {
    std::vector<TestPosition> all = getStandard64();
    std::vector<TestPosition> endgame;

    for (const auto& pos : all) {
        if (getMoveCount(pos.board) > 40) {
            endgame.push_back(pos);
        }
    }

    return endgame;
}

std::vector<TestPosition> PositionSuite::getByPhase(GamePhase phase) {
    switch (phase) {
        case GamePhase::OPENING:
            return getOpening();
        case GamePhase::MIDGAME:
            return getMidgame();
        case GamePhase::ENDGAME:
            return getEndgame();
        case GamePhase::ALL:
        default:
            return getStandard64();
    }
}

std::vector<TestPosition> PositionSuite::getAll() {
    return getStandard64();
}

std::vector<TestPosition> PositionSuite::loadFromFile(const std::string& filepath) {
    return PositionSuiteLoader::load(filepath, PositionSuiteLoader::Format::AUTO);
}

bool PositionSuite::saveToFile(const std::vector<TestPosition>& positions,
                                const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[PositionSuite] Failed to open file: " << filepath << std::endl;
        return false;
    }

    for (const auto& pos : positions) {
        file << pos.name << ";"
             << pos.board.getPlayerBits() << ";"
             << pos.board.getOpponentBits() << ";"
             << static_cast<int>(pos.player) << ";"
             << pos.source << "\n";
    }

    file.close();
    return true;
}

bool PositionSuite::validate(const TestPosition& position) {
    // 验证基本约束
    if (position.name.empty()) {
        return false;
    }

    // 验证棋子数量
    int black_count = position.board.getScore(PlayerColor::Black);
    int white_count = position.board.getScore(PlayerColor::White);
    int total = black_count + white_count;

    if (total < 4 || total > 64) {
        return false;
    }

    // 验证没有重叠棋子
    if ((position.board.getPlayerBits() & position.board.getOpponentBits()) != 0) {
        return false;
    }

    return true;
}

std::string PositionSuite::getStats(const std::vector<TestPosition>& positions) {
    std::ostringstream oss;

    int opening = 0, midgame = 0, endgame = 0;
    int total_difficulty = 0;

    for (const auto& pos : positions) {
        int moves = getMoveCount(pos.board);
        if (moves <= 20) opening++;
        else if (moves <= 40) midgame++;
        else endgame++;

        total_difficulty += pos.difficulty;
    }

    oss << "Position Suite Statistics:\n";
    oss << "  Total positions: " << positions.size() << "\n";
    oss << "  Opening (0-20): " << opening << "\n";
    oss << "  Midgame (21-40): " << midgame << "\n";
    oss << "  Endgame (41+): " << endgame << "\n";
    oss << "  Avg difficulty: " << (positions.empty() ? 0 : total_difficulty / positions.size());

    return oss.str();
}

TestPosition PositionSuite::createRandom(int move_count, uint64_t seed) {
    TestPosition pos;
    pos.name = "Random_" + std::to_string(move_count);
    pos.difficulty = std::min(10, move_count / 4);
    pos.source = "Generated";
    pos.player = PlayerColor::Black;

    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> dist(0, 63);

    // 初始化标准开局
    std::string board_str(64, '.');
    board_str[27] = 'W'; board_str[28] = 'B';
    board_str[35] = 'B'; board_str[36] = 'W';

    int black = 2, white = 2;
    std::vector<int> empty_positions;
    for (int i = 0; i < 64; ++i) {
        if (board_str[i] == '.') {
            empty_positions.push_back(i);
        }
    }

    // 随机添加棋子直到达到指定步数
    std::shuffle(empty_positions.begin(), empty_positions.end(), rng);
    int target_total = 2 + move_count;
    int filled = 0;

    for (int pos_idx : empty_positions) {
        if (black + white >= target_total) break;
        if (board_str[pos_idx] == '.') {
            board_str[pos_idx] = (filled % 2 == 0) ? 'B' : 'W';
            if (board_str[pos_idx] == 'B') black++;
            else white++;
            filled++;
        }
    }

    pos.board = BitBoard(board_str);

    return pos;
}

int PositionSuite::getMoveCount(const BitBoard& board) {
    int total = board.getScore(PlayerColor::Black) + board.getScore(PlayerColor::White);
    return total - 4;  // 初始4子不算回合
}

GamePhase PositionSuite::getGamePhase(const BitBoard& board) {
    int moves = getMoveCount(board);

    if (moves <= 20) return GamePhase::OPENING;
    if (moves <= 40) return GamePhase::MIDGAME;
    return GamePhase::ENDGAME;
}

// ============================================================================
// PositionSuiteLoader 实现
// ============================================================================

PositionSuiteLoader::Format PositionSuiteLoader::detectFormat(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return Format::AUTO;
    }

    std::string first_line;
    std::getline(file, first_line);
    file.close();

    // 简单格式检测
    if (first_line.find(';') != std::string::npos) {
        return Format::SIMPLE;
    }
    if (first_line.find("fen") != std::string::npos ||
        first_line.find("/") != std::string::npos) {
        return Format::FEN;
    }
    if (first_line.find("epd") != std::string::npos) {
        return Format::EPD;
    }

    return Format::SIMPLE;
}

std::vector<TestPosition> PositionSuiteLoader::load(const std::string& filepath, Format format) {
    if (format == Format::AUTO) {
        format = detectFormat(filepath);
    }

    switch (format) {
        case Format::SIMPLE:
            return loadSimple(filepath);
        case Format::FEN:
            return loadFEN(filepath);
        case Format::EPD:
            return loadEPD(filepath);
        default:
            return loadSimple(filepath);
    }
}

std::vector<TestPosition> PositionSuiteLoader::loadSimple(const std::string& filepath) {
    std::vector<TestPosition> positions;
    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "[PositionSuiteLoader] Failed to open: " << filepath << std::endl;
        return positions;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string token;

        TestPosition pos;
        std::getline(ss, token, ';');
        pos.name = token;

        std::getline(ss, token, ';');
        uint64_t player_bits = std::stoull(token);

        std::getline(ss, token, ';');
        uint64_t opponent_bits = std::stoull(token);

        std::getline(ss, token, ';');
        int player = std::stoi(token);

        std::getline(ss, token, ';');
        pos.source = token;

        pos.board = BitBoard(player_bits, opponent_bits);
        pos.player = static_cast<PlayerColor>(player);

        if (PositionSuite::validate(pos)) {
            positions.push_back(pos);
        }
    }

    file.close();
    std::cout << "[PositionSuiteLoader] Loaded " << positions.size() << " positions from " << filepath << std::endl;

    return positions;
}

std::vector<TestPosition> PositionSuiteLoader::loadEPD(const std::string& filepath) {
    std::cerr << "[PositionSuiteLoader] EPD format not yet implemented" << std::endl;
    return {};
}

std::vector<TestPosition> PositionSuiteLoader::loadFEN(const std::string& filepath) {
    std::cerr << "[PositionSuiteLoader] FEN format not yet implemented" << std::endl;
    return {};
}

} // namespace Reversi
