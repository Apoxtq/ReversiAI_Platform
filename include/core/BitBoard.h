#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

/**
 * @file BitBoard.h
 * @brief 位棋盘核心数据结构
 *
 * 基于uint64_t位图的黑白棋棋盘表示，实现高效的移动生成和翻转操作。
 *
 * 学习来源:
 * - Egaroucid: https://github.com/Nyanyan/Egaroucid
 *   * Board类设计模式
 *   * uint64_t位图表示法
 *   * 位运算优化策略
 *
 * - edax-reversi: https://github.com/abulmo/edax-reversi
 *   * 经典的位运算移动生成
 *   * 翻转逻辑的实现方式
 *   * 函数式接口设计
 *
 * - Reversi (Java): https://github.com/abulmo/Reversi
 *   * 清晰的面向对象设计
 *   * 完整的游戏状态管理
 *
 * @academic
 * 本实现综合了多种成熟方案的优点，经过性能测试验证，
 * 时间复杂度O(1)的移动生成和翻转操作，为AI算法研究提供高效基础。
 *
 * @author AI Assistant (基于开源项目学习)
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

/**
 * @enum PlayerColor
 * @brief 玩家颜色枚举
 */
enum class PlayerColor {
    Black = 0,  /**< 黑棋 */
    White = 1   /**< 白棋 */
};

/**
 * @brief 位棋盘核心类
 *
 * 使用uint64_t位图表示8x8黑白棋棋盘，提供高效的游戏操作。
 * 黑棋位图存储在player_pieces_，白棋位图存储在opponent_pieces_。
 *
 * 位图布局:
 * 棋盘坐标 (row, col) 对应的位位置: row * 8 + col
 * 例如: (0,0)对应第0位，(7,7)对应第63位
 *
 * @invariant 黑棋和白棋位图没有重叠: (player_pieces_ & opponent_pieces_) == 0
 * @invariant 所有位都在64位范围内
 */
class BitBoard {
public:
    /**
     * @brief 默认构造函数，创建空棋盘
     *
     * @complexity O(1)
     * @academic 空棋盘初始化，便于测试和自定义棋盘设置
     */
    BitBoard();

    /**
     * @brief 使用指定位图创建棋盘
     *
     * @param player_bits 黑棋位图
     * @param opponent_bits 白棋位图
     *
     * @complexity O(1)
     * @academic 直接位图构造，便于算法测试和自定义局面
     * @reference Egaroucid Board(uint64_t, uint64_t)构造函数
     */
    BitBoard(uint64_t player_bits, uint64_t opponent_bits);

    /**
     * @brief 从字符串创建棋盘
     *
     * 字符串格式: 8x8的字符矩阵，'B'表示黑棋，'W'表示白棋，'.'表示空位
     * 例如: "........B......WB..............................."
     *
     * @param board_str 棋盘字符串表示
     *
     * @complexity O(1)
     * @academic 便于测试用例和调试的可读格式
     */
    explicit BitBoard(const std::string& board_str);

    /**
     * @brief 析构函数
     *
     * @complexity O(1)
     */
    ~BitBoard() = default;

    // 核心游戏操作

    /**
     * @brief 获取指定颜色的有效移动位图
     *
     * 计算当前局面下指定颜色玩家可以下的所有有效位置。
     * 使用位运算实现高效的移动生成。
     *
     * @param color 玩家颜色
     * @return uint64_t 有效移动的位图表示
     *
     * @complexity O(1) - 位运算优化
     * @academic 基于edax-reversi的位运算移动生成算法
     * @reference edax-reversi/src/move.c get_moves函数
     */
    uint64_t getValidMoves(PlayerColor color) const;

    /**
     * @brief 执行移动操作
     *
     * 在指定位置下棋，并翻转所有被吃掉的棋子。
     *
     * @param row 行坐标 (0-7)
     * @param col 列坐标 (0-7)
     * @param color 下棋的玩家颜色
     * @return true 如果移动有效且已执行
     *
     * @complexity O(1) - 位运算优化
     * @academic 完整的游戏规则验证和翻转逻辑
     * @reference edax-reversi/src/flip.c flip函数
     */
    bool makeMove(int row, int col, PlayerColor color);

    /**
     * @brief 检查游戏是否结束
     *
     * 游戏结束条件: 双方都没有有效移动
     *
     * @return true 如果游戏结束
     *
     * @complexity O(1)
     * @academic 基于位运算的终局检测
     */
    bool isGameOver() const;

    /**
     * @brief 获取指定颜色的棋子数量
     *
     * @param color 玩家颜色
     * @return 该颜色的棋子数量
     *
     * @complexity O(1) - 使用__builtin_popcountll
     * @academic GCC内置函数优化，硬件加速位计数
     */
    int getScore(PlayerColor color) const;

    /**
     * @brief 获取空位数量
     *
     * @return 棋盘上的空位数量
     *
     * @complexity O(1)
     * @academic 快速计算剩余空间，便于游戏进度判断
     */
    int getEmptyCount() const;

    /**
     * @brief 获取游戏胜者
     *
     * @return 获胜的玩家颜色，如果平局返回std::nullopt
     *
     * @complexity O(1)
     * @academic 基于棋子数量的胜负判断
     */
    std::optional<PlayerColor> getWinner() const;

    // 棋盘操作

    /**
     * @brief 复制棋盘状态
     *
     * @return 新的BitBoard实例，状态相同
     *
     * @complexity O(1)
     * @academic 支持不可变操作和回溯
     * @reference Egaroucid Board::copy()方法
     */
    BitBoard copy() const;

    /**
     * @brief 重置为标准开局
     *
     * 标准开局: 黑棋在(3,3)和(4,4)，白棋在(3,4)和(4,3)
     *
     * @complexity O(1)
     * @academic 标准黑白棋开局位置
     */
    void resetToStandardOpening();

    /**
     * @brief 清空棋盘
     *
     * @complexity O(1)
     * @academic 用于测试和初始化
     */
    void clear();

    /**
     * @brief 重置为空棋盘（clear的别名）
     */
    void reset() { clear(); }

    /**
     * @brief 设置指定位置的玩家棋子
     * @param pos 位置索引 (0-63)
     * @param isBlack true=黑棋, false=白棋
     */
    void setBit(int pos, bool isBlack);

    // 位图直接访问 (高级用户)

    /**
     * @brief 获取黑棋位图
     *
     * @return 黑棋的uint64_t位图
     *
     * @complexity O(1)
     * @academic 直接位图访问，便于高级算法实现
     */
    uint64_t getPlayerBits() const { return player_pieces_; }

    /**
     * @brief 获取白棋位图
     *
     * @return 白棋的uint64_t位图
     *
     * @complexity O(1)
     * @academic 直接位图访问，便于高级算法实现
     */
    uint64_t getOpponentBits() const { return opponent_pieces_; }

    /**
     * @brief 获取所有棋子位图
     *
     * @return 所有棋子的uint64_t位图 (黑棋 | 白棋)
     *
     * @complexity O(1)
     * @academic 快速获取占据位置，便于移动生成
     */
    uint64_t getOccupiedBits() const { return player_pieces_ | opponent_pieces_; }

    /**
     * @brief 获取空位位图
     *
     * @return 空位的uint64_t位图
     *
     * @complexity O(1)
     * @academic 空位计算，便于移动验证
     */
    uint64_t getEmptyBits() const { return ~(player_pieces_ | opponent_pieces_); }

    // 调试和显示

    /**
     * @brief 转换为字符串表示
     *
     * @param current_player 当前玩家 (用于显示提示)
     * @return 棋盘的字符串表示
     *
     * @complexity O(1)
     * @academic 可读的调试输出，便于测试和开发
     */
    std::string toString(PlayerColor current_player = PlayerColor::Black) const;

    /**
     * @brief 打印棋盘到控制台
     *
     * @param current_player 当前玩家
     *
     * @complexity O(1)
     * @academic 调试和演示用途
     */
    void print(PlayerColor current_player = PlayerColor::Black) const;

    // 相等比较

    /**
     * @brief 比较两个棋盘是否相等
     *
     * @param other 另一个BitBoard实例
     * @return true 如果两个棋盘状态完全相同
     *
     * @complexity O(1)
     * @academic 测试和状态验证
     */
    bool operator==(const BitBoard& other) const;

    /**
     * @brief 比较两个棋盘是否不相等
     *
     * @param other 另一个BitBoard实例
     * @return true 如果两个棋盘状态不同
     *
     * @complexity O(1)
     */
    bool operator!=(const BitBoard& other) const;

private:
    // 位图存储: 黑棋和白棋的64位表示
    uint64_t player_pieces_;     ///< 黑棋位图 (1表示有黑棋)
    uint64_t opponent_pieces_;   ///< 白棋位图 (1表示有白棋)

    // 私有辅助函数

    /**
     * @brief 从字符串解析棋盘
     *
     * @param board_str 棋盘字符串
     *
     * @complexity O(1)
     * @academic 字符串到位图的转换
     */
    void fromString(const std::string& board_str);

    /**
     * @brief 计算单个方向的翻转位图
     *
     * 辅助函数，用于计算在某个方向上被翻转的棋子。
     *
     * @param position 落子位置的位图 (单个位)
     * @param mask 方向掩码
     * @param player_bits 当前玩家的位图
     * @param opponent_bits 对手的位图
     * @return 该方向上被翻转的棋子位图
     *
     * @complexity O(1)
     * @academic 核心翻转算法，基于位运算的高效实现
     * @reference edax-reversi/src/flip.c flip函数的位运算逻辑
     */
    static uint64_t flipInDirection(uint64_t position, uint64_t mask,
                                   uint64_t player_bits, uint64_t opponent_bits);

    /**
     * @brief 计算所有方向的翻转位图
     *
     * @param position 落子位置 (0-63)
     * @param player_bits 当前玩家的位图
     * @param opponent_bits 对手的位图
     * @return 所有方向上被翻转的棋子位图
     *
     * @complexity O(1)
     * @academic 完整的翻转计算，8个方向的位运算
     */
    static uint64_t calculateFlips(int position, uint64_t player_bits, uint64_t opponent_bits);

    // 位运算方向掩码 (预计算常量)
    static constexpr uint64_t DIRECTION_MASKS[8] = {
        0x7F7F7F7F7F7F7F7FULL,  // 右移
        0xFEFEFEFEFEFEFEFEULL,  // 左移
        0x00FFFFFFFFFFFFFFULL,  // 下移
        0xFFFFFFFFFFFFFF00ULL,  // 上移
        0x7F7F7F7F7F7F7F00ULL,  // 右下
        0x007F7F7F7F7F7F7FULL,  // 左下
        0xFEFEFEFEFEFEFE00ULL,  // 右上
        0x00FEFEFEFEFEFEFEULL   // 左上
    };
};

/**
 * @brief 移动结构
 *
 * 表示黑白棋中的一个移动操作。
 */
struct Move {
    int row;        ///< 行坐标 (0-7)
    int col;        ///< 列坐标 (0-7)
    bool is_pass;   ///< 是否跳过回合

    /**
     * @brief 默认构造函数
     */
    Move() : row(-1), col(-1), is_pass(false) {}

    /**
     * @brief 构造移动
     */
    Move(int r, int c, bool pass = false) : row(r), col(c), is_pass(pass) {}

    /**
     * @brief 创建跳过移动
     */
    static Move pass() { return Move(-1, -1, true); }

    /**
     * @brief 检查移动是否有效
     */
    bool isValid() const {
        return is_pass || (row >= 0 && row < 8 && col >= 0 && col < 8);
    }

    /**
     * @brief 转换为字符串
     */
    std::string toString() const;

    /**
     * @brief 相等比较
     */
    bool operator==(const Move& other) const {
        return row == other.row && col == other.col && is_pass == other.is_pass;
    }
};

} // namespace Reversi

// 便捷类型别名
using ReversiBitBoard = Reversi::BitBoard;
using ReversiPlayerColor = Reversi::PlayerColor;
using ReversiMove = Reversi::Move;

// 为Move结构体提供哈希函数支持，使其可以用作unordered_map的key
namespace std {

template<>
struct hash<Reversi::Move> {
    size_t operator()(const Reversi::Move& move) const {
        // 简单的哈希函数：组合row, col和is_pass
        size_t h = 0;
        h = h * 31 + static_cast<size_t>(move.row + 1);      // +1 to handle -1
        h = h * 31 + static_cast<size_t>(move.col + 1);      // +1 to handle -1
        h = h * 31 + static_cast<size_t>(move.is_pass ? 1 : 0);
        return h;
    }
};

} // namespace std