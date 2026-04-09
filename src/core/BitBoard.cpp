#include "core/BitBoard.h"
#include <iostream>
#include <bit>
#include <stdexcept>
#include <optional>

// Cross-platform bit operations
#if defined(_MSC_VER)
    #include <intrin.h>
    #define POPCOUNT64 __popcnt64

    // MSVC inline function for CTZ (Count Trailing Zeros)
    inline int CTZ64(uint64_t x) {
        unsigned long index;
        _BitScanForward64(&index, x);
        return static_cast<int>(index);
    }
#else
    #define POPCOUNT64 __builtin_popcountll
    #define CTZ64(x) __builtin_ctzll(x)
#endif

// File: BitBoard.cpp
// Bitboard core implementation
// Based on Egaroucid and edax-reversi algorithms

namespace Reversi {

// Direction constants for bit operations
static constexpr uint64_t RIGHT_MASK     = 0x7F7F7F7F7F7F7F7FULL;
static constexpr uint64_t LEFT_MASK      = 0xFEFEFEFEFEFEFEFEULL;
static constexpr uint64_t DOWN_MASK      = 0x00FFFFFFFFFFFFFFULL;
static constexpr uint64_t UP_MASK        = 0xFFFFFFFFFFFFFF00ULL;
static constexpr uint64_t DOWN_RIGHT_MASK = 0x7F7F7F7F7F7F7F00ULL;
static constexpr uint64_t DOWN_LEFT_MASK  = 0x007F7F7F7F7F7F7FULL;
static constexpr uint64_t UP_RIGHT_MASK   = 0xFEFEFEFEFEFEFE00ULL;
static constexpr uint64_t UP_LEFT_MASK    = 0x00FEFEFEFEFEFEFEULL;

// Kogge-Stone algorithm for move generation
static uint64_t get_some_moves(uint64_t player, uint64_t mask, int direction) {
    if (direction > 0) {
        uint64_t flip_l = mask & (player << direction);
        uint64_t mask_l = mask & (mask << direction);
        flip_l |= mask_l & (flip_l << (direction * 2));
        flip_l |= mask_l & (flip_l << (direction * 2));
        return flip_l << direction;
    } else {
        int abs_dir = -direction;
        uint64_t flip_r = mask & (player >> abs_dir);
        uint64_t mask_r = mask & (mask >> abs_dir);
        flip_r |= mask_r & (flip_r >> (abs_dir * 2));
        flip_r |= mask_r & (flip_r >> (abs_dir * 2));
        return flip_r >> abs_dir;
    }
}

BitBoard::BitBoard() : player_pieces_(0), opponent_pieces_(0) {}

BitBoard::BitBoard(uint64_t player_bits, uint64_t opponent_bits)
    : player_pieces_(player_bits), opponent_pieces_(opponent_bits) {
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

    uint64_t empties = empty_bits;
    while (empties) {
        int pos = CTZ64(empties);
        if (calculateFlips(pos, player_bits, opponent_bits) != 0) {
            valid_moves |= (1ULL << pos);
        }
        empties &= empties - 1;
    }

    return valid_moves;
}

bool BitBoard::makeMove(int row, int col, PlayerColor color) {
    if (row < 0 || row >= 8 || col < 0 || col >= 8) {
        return false;
    }

    int position = row * 8 + col;
    uint64_t pos_bit = 1ULL << position;

    if (getOccupiedBits() & pos_bit) {
        return false;
    }

    uint64_t player_bits = (color == PlayerColor::Black) ? player_pieces_ : opponent_pieces_;
    uint64_t opponent_bits = (color == PlayerColor::Black) ? opponent_pieces_ : player_pieces_;

    uint64_t flips = calculateFlips(position, player_bits, opponent_bits);

    if (flips == 0) {
        return false;
    }

    if (color == PlayerColor::Black) {
        player_pieces_ |= pos_bit;
        player_pieces_ |= flips;
        opponent_pieces_ &= ~flips;
    } else {
        opponent_pieces_ |= pos_bit;
        opponent_pieces_ |= flips;
        player_pieces_ &= ~flips;
    }

    return true;
}

bool BitBoard::isGameOver() const {
    uint64_t black_moves = getValidMoves(PlayerColor::Black);
    if (black_moves != 0) {
        return false;
    }
    uint64_t white_moves = getValidMoves(PlayerColor::White);
    if (white_moves != 0) {
        return false;
    }
    return true;
}

int BitBoard::getScore(PlayerColor color) const {
    if (color == PlayerColor::Black) {
        return POPCOUNT64(player_pieces_);
    } else {
        return POPCOUNT64(opponent_pieces_);
    }
}

int BitBoard::getEmptyCount() const {
    uint64_t occupied = getOccupiedBits();
    return 64 - POPCOUNT64(occupied);
}

std::optional<PlayerColor> BitBoard::getWinner() const {
    int black_score = getScore(PlayerColor::Black);
    int white_score = getScore(PlayerColor::White);

    if (black_score > white_score) {
        return PlayerColor::Black;
    } else if (white_score > black_score) {
        return PlayerColor::White;
    } else {
        return std::nullopt;
    }
}

BitBoard BitBoard::copy() const {
    return BitBoard(player_pieces_, opponent_pieces_);
}

void BitBoard::resetToStandardOpening() {
    player_pieces_ = (1ULL << 27) | (1ULL << 36);
    opponent_pieces_ = (1ULL << 28) | (1ULL << 35);
}

void BitBoard::clear() {
    player_pieces_ = 0;
    opponent_pieces_ = 0;
}

void BitBoard::setBit(int pos, bool isBlack) {
    uint64_t bit = 1ULL << pos;
    if (isBlack) {
        player_pieces_ |= bit;
        opponent_pieces_ &= ~bit;
    } else {
        opponent_pieces_ |= bit;
        player_pieces_ &= ~bit;
    }
}

std::string BitBoard::toString(PlayerColor current_player) const {
    std::string result;
    result.reserve(72);

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            int pos = row * 8 + col;
            uint64_t bit = 1ULL << pos;

            if (player_pieces_ & bit) {
                result += 'B';
            } else if (opponent_pieces_ & bit) {
                result += 'W';
            } else {
                if (current_player != PlayerColor::Black) {
                    uint64_t moves = getValidMoves(current_player);
                    if (moves & bit) {
                        result += '*';
                    } else {
                        result += '.';
                    }
                } else {
                    result += '.';
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
    }

    if (player_pieces_ & opponent_pieces_) {
        throw std::invalid_argument("Board string contains overlapping pieces");
    }
}

uint64_t BitBoard::calculateFlips(int position, uint64_t player_bits, uint64_t opponent_bits) {
    uint64_t flips = 0;

    const int directions[8] = {1, -1, 8, -8, 7, -7, 9, -9};

    for (int dir : directions) {
        uint64_t dir_flips = 0;
        int current_pos = position + dir;

        while (current_pos >= 0 && current_pos < 64) {
            uint64_t current_bit = 1ULL << current_pos;

            bool valid_pos = true;
            if (dir == 1 && (current_pos % 8 == 0)) valid_pos = false;
            if (dir == -1 && (current_pos % 8 == 7)) valid_pos = false;
            if (abs(dir) == 8) valid_pos = true;
            if (abs(dir) == 7 && (current_pos % 8 == 0 || current_pos % 8 == 7)) valid_pos = false;
            if (abs(dir) == 9 && (current_pos % 8 == 0 || current_pos % 8 == 7)) valid_pos = false;

            if (!valid_pos) break;

            if (current_bit & opponent_bits) {
                dir_flips |= current_bit;
            } else if (current_bit & player_bits) {
                flips |= dir_flips;
                break;
            } else {
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

    uint64_t current = (pos_bit & mask);

    while (current) {
        current = (current & mask);

        if (current & opponent_bits) {
            flips |= current;
        } else if (current & player_bits) {
            return flips;
        } else {
            return 0;
        }

        current = (current & mask);
    }

    return 0;
}

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
