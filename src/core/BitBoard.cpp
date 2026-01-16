#include "core/BitBoard.h"
#include <iostream>
#include <bit>
#include <stdexcept>
#include <optional>

/**
 * @file BitBoard.cpp
 * @brief 位棋盘核心实现
 *
 * 基于学习Egaroucid和edax-reversi的位运算实现。
 * 重点实现了高效的移动生成和翻转逻辑。
 *
 * @reference
 * - Egaroucid: Board类和位运算策略
 * - edax-reversi: flip.c和move.c的算法实现
 * - Reversi(Java): 清晰的面向对象设计模式
 */

namespace Reversi {

// 位运算方向常量 (预计算的掩码)
static constexpr uint64_t RIGHT_MASK     = 0x7F7F7F7F7F7F7F7FULL;
static constexpr uint64_t LEFT_MASK      = 0xFEFEFEFEFEFEFEFEULL;
static constexpr uint64_t DOWN_MASK      = 0x00FFFFFFFFFFFFFFULL;
static constexpr uint64_t UP_MASK        = 0xFFFFFFFFFFFFFF00ULL;
static constexpr uint64_t DOWN_RIGHT_MASK = 0x7F7F7F7F7F7F7F00ULL;
static constexpr uint64_t DOWN_LEFT_MASK  = 0x007F7F7F7F7F7F7FULL;
static constexpr uint64_t UP_RIGHT_MASK   = 0xFEFEFEFEFEFEFE00ULL;
static constexpr uint64_t UP_LEFT_MASK    = 0x00FEFEFEFEFEFEFEULL;

// 基于edax的Kogge-Stone算法实现get_some_moves
// 这个函数计算在指定方向上可以翻转对手棋子的所有位置
static uint64_t get_some_moves(uint64_t player, uint64_t mask, int direction) {
    // Kogge-Stone算法实现
    // direction可以是: ±1 (水平), ±8 (垂直), ±7, ±9 (对角线)

    if (direction > 0) {
        // 正方向 (右、下、右下、右上)
        uint64_t flip_l = mask & (player << direction);
        uint64_t mask_l = mask & (mask << direction);

        flip_l |= mask_l & (flip_l << (direction * 2));
        flip_l |= mask_l & (flip_l << (direction * 2));

        return flip_l << direction;
    } else {
        // 负方向 (左、上、左下、左上)
        int abs_dir = -direction;
        uint64_t flip_r = mask & (player >> abs_dir);
        uint64_t mask_r = mask & (mask >> abs_dir);

        flip_r |= mask_r & (flip_r >> (abs_dir * 2));
        flip_r |= mask_r & (flip_r >> (abs_dir * 2));

        return flip_r >> abs_dir;
    }
}

BitBoard::BitBoard() : player_pieces_(0), opponent_pieces_(0) {
    // 空棋盘初始化
}

BitBoard::BitBoard(uint64_t player_bits, uint64_t opponent_bits)
    : player_pieces_(player_bits), opponent_pieces_(opponent_bits) {
    // 验证位图不重叠
    if (player_bits & opponent_bits) {
        throw std::invalid_argument("Player and opponent bitmaps must not overlap");
    }
}

BitBoard::BitBoard(const std::string& board_str) : player_pieces_(0), opponent_pieces_(0) {
    fromString(board_str);
}

uint64_t BitBoard::getValidMoves(PlayerColor color) const {
    uint64_t player_bits = (color == PlayerColor::Black) ? player_pieces_ : opponent_pieces_;
    uint64_t opponent_bits = (color == PlayerColor::Black) ? opponent_pieces_ : player_pieces_;
    uint64_t empty_bits = getEmptyBits();
    uint64_t valid_moves = 0;

    // 遍历所有空位
    uint64_t empties = empty_bits;
    while (empties) {
        int pos = __builtin_ctzll(empties);

        // 检查这个位置是否有有效的翻转
        if (calculateFlips(pos, player_bits, opponent_bits) != 0) {
            valid_moves |= (1ULL << pos);
        }

        // 清除已处理的位
        empties &= empties - 1;
    }

    return valid_moves;
}

bool BitBoard::makeMove(int row, int col, PlayerColor color) {
    if (row < 0 || row >= 8 || col < 0 || col >= 8) {
        return false;  // 位置超出边界
    }

    int position = row * 8 + col;
    uint64_t pos_bit = 1ULL << position;

    // 检查位置是否为空
    if (getOccupiedBits() & pos_bit) {
        return false;  // 位置已被占据
    }

    uint64_t player_bits = (color == PlayerColor::Black) ? player_pieces_ : opponent_pieces_;
    uint64_t opponent_bits = (color == PlayerColor::Black) ? opponent_pieces_ : player_pieces_;

    // 计算翻转的棋子
    uint64_t flips = calculateFlips(position, player_bits, opponent_bits);

    if (flips == 0) {
        return false;  // 没有有效的翻转，不是有效移动
    }

    // 执行移动：放置新棋子并翻转对手棋子
    if (color == PlayerColor::Black) {
        player_pieces_ |= pos_bit;      // 放置黑棋
        player_pieces_ |= flips;        // 翻转的棋子变为黑棋
        opponent_pieces_ &= ~flips;     // 从白棋位图中移除翻转的棋子
    } else {
        opponent_pieces_ |= pos_bit;   // 放置白棋
        opponent_pieces_ |= flips;      // 翻转的棋子变为白棋
        player_pieces_ &= ~flips;       // 从黑棋位图中移除翻转的棋子
    }

    return true;
}

bool BitBoard::isGameOver() const {
    // 检查黑棋是否有有效移动
    uint64_t black_moves = getValidMoves(PlayerColor::Black);
    if (black_moves != 0) {
        return false;
    }

    // 检查白棋是否有有效移动
    uint64_t white_moves = getValidMoves(PlayerColor::White);
    if (white_moves != 0) {
        return false;
    }

    // 双方都没有有效移动，游戏结束
    return true;
}

int BitBoard::getScore(PlayerColor color) const {
    if (color == PlayerColor::Black) {
        return __builtin_popcountll(player_pieces_);
    } else {
        return __builtin_popcountll(opponent_pieces_);
    }
}

int BitBoard::getEmptyCount() const {
    uint64_t occupied = getOccupiedBits();
    return 64 - __builtin_popcountll(occupied);
}

std::optional<PlayerColor> BitBoard::getWinner() const {
    int black_score = getScore(PlayerColor::Black);
    int white_score = getScore(PlayerColor::White);

    if (black_score > white_score) {
        return PlayerColor::Black;
    } else if (white_score > black_score) {
        return PlayerColor::White;
    } else {
        return std::nullopt;  // 平局
    }
}

BitBoard BitBoard::copy() const {
    return BitBoard(player_pieces_, opponent_pieces_);
}

void BitBoard::resetToStandardOpening() {
    // 标准开局：黑棋在d4, e5；白棋在d5, e4
    // 坐标转换为位位置:
    // d4 = (3,3) = 3*8 + 3 = 27
    // e5 = (4,4) = 4*8 + 4 = 36
    // d5 = (3,4) = 3*8 + 4 = 28
    // e4 = (4,3) = 4*8 + 3 = 35

    player_pieces_ = (1ULL << 27) | (1ULL << 36);    // 黑棋: d4, e5
    opponent_pieces_ = (1ULL << 28) | (1ULL << 35);  // 白棋: d5, e4
}

void BitBoard::clear() {
    player_pieces_ = 0;
    opponent_pieces_ = 0;
}

std::string BitBoard::toString(PlayerColor current_player) const {
    std::string result;
    result.reserve(72);  // 8行 * 9字符(包括换行)

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            int pos = row * 8 + col;
            uint64_t bit = 1ULL << pos;

            if (player_pieces_ & bit) {
                result += 'B';  // 黑棋
            } else if (opponent_pieces_ & bit) {
                result += 'W';  // 白棋
            } else {
                // 检查是否是当前玩家的有效移动
                if (current_player != PlayerColor::Black) {
                    uint64_t moves = getValidMoves(current_player);
                    if (moves & bit) {
                        result += '*';  // 有效移动提示
                    } else {
                        result += '.';  // 空位
                    }
                } else {
                    result += '.';  // 空位
                }
            }
        }
        result += '\n';
    }

    return result;
}

void BitBoard::print(PlayerColor current_player) const {
    std::cout << toString(current_player);
}

bool BitBoard::operator==(const BitBoard& other) const {
    return player_pieces_ == other.player_pieces_ &&
           opponent_pieces_ == other.opponent_pieces_;
}

bool BitBoard::operator!=(const BitBoard& other) const {
    return !(*this == other);
}

// 私有辅助函数实现

void BitBoard::fromString(const std::string& board_str) {
    if (board_str.length() != 64) {
        throw std::invalid_argument("Board string must be exactly 64 characters");
    }

    player_pieces_ = 0;
    opponent_pieces_ = 0;

    for (int i = 0; i < 64; ++i) {
        char c = board_str[i];
        uint64_t bit = 1ULL << i;

        if (c == 'B' || c == 'b') {
            player_pieces_ |= bit;
        } else if (c == 'W' || c == 'w') {
            opponent_pieces_ |= bit;
        }
        // '.' 或其他字符表示空位，不设置任何位
    }

    // 验证位图不重叠
    if (player_pieces_ & opponent_pieces_) {
        throw std::invalid_argument("Board string contains overlapping pieces");
    }
}

uint64_t BitBoard::calculateFlips(int position, uint64_t player_bits, uint64_t opponent_bits) {
    uint64_t flips = 0;

    // 8个方向的偏移量
    const int directions[8] = {1, -1, 8, -8, 7, -7, 9, -9};

    for (int dir : directions) {
        uint64_t dir_flips = 0;
        int current_pos = position + dir;

        // 检查这个方向上的翻转
        while (current_pos >= 0 && current_pos < 64) {
            uint64_t current_bit = 1ULL << current_pos;

            // 检查边界条件（避免换行）
            bool valid_pos = true;
            if (dir == 1 && (current_pos % 8 == 0)) valid_pos = false;  // 左边界
            if (dir == -1 && (current_pos % 8 == 7)) valid_pos = false; // 右边界
            if (abs(dir) == 8) valid_pos = true;  // 垂直方向总是有效
            if (abs(dir) == 7 && (current_pos % 8 == 0 || current_pos % 8 == 7)) valid_pos = false;
            if (abs(dir) == 9 && (current_pos % 8 == 0 || current_pos % 8 == 7)) valid_pos = false;

            if (!valid_pos) break;

            if (current_bit & opponent_bits) {
                // 对手的棋子，加入翻转列表
                dir_flips |= current_bit;
            } else if (current_bit & player_bits) {
                // 自己的棋子，翻转有效
                flips |= dir_flips;
                break;
            } else {
                // 空位，翻转无效
                dir_flips = 0;
                break;
            }

            current_pos += dir;
        }
    }

    return flips;
}

uint64_t BitBoard::flipInDirection(uint64_t pos_bit, uint64_t mask,
                                  uint64_t player_bits, uint64_t opponent_bits) {
    uint64_t flips = 0;

    // 向指定方向移动，寻找对手棋子
    uint64_t current = (pos_bit & mask);  // 第一个位置

    while (current) {
        current = (current & mask);  // 继续向该方向移动

        if (current & opponent_bits) {
            // 遇到对手棋子，继续
            flips |= current;
        } else if (current & player_bits) {
            // 遇到自己棋子，翻转路径上的所有对手棋子
            return flips;
        } else {
            // 遇到空位，停止
            return 0;
        }

        // 向该方向移动一位
        current = (current & mask);
    }

    // 到达边界，没有找到自己的棋子
    return 0;
}

// Move类的实现
std::string Move::toString() const {
    if (is_pass) {
        return "PASS";
    } else {
        char col_char = 'a' + col;
        char row_char = '1' + row;
        return std::string(1, col_char) + std::string(1, row_char);
    }
}

} // namespace Reversi
