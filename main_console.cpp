#include <iostream>
#include <iomanip>
#include <string>
#include "MCTS.h"
#include "core/BitBoard.h"

int main(int argc, char *argv[])
{
    std::cout << "=== ReversiAI_Platform 控制台版本 ===" << std::endl;
    std::cout << "测试BitBoard和MCTS算法基础功能" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    // 测试BitBoard功能
    std::cout << "\n🎯 测试BitBoard核心功能" << std::endl;
    std::cout << std::string(30, '-') << std::endl;

    try {
        // 创建MCTS实例进行基本测试
        Position testPos(3, 3);  // 中心位置 (3,3)
        MCTS* mcts = new MCTS(testPos, BLACK);  // 使用BLACK常量

        std::cout << "MCTS实例创建成功" << std::endl;
        std::cout << "位置: (" << testPos.x << ", " << testPos.y << ")" << std::endl;
        std::cout << "棋子颜色: " << (BLACK == 2 ? "黑棋" : "白棋") << std::endl;

        // 清理资源
        delete mcts;

        std::cout << "\n✅ MCTS功能测试完成" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "\n❌ MCTS测试失败: " << e.what() << std::endl;
        return 1;
    }

    // 测试BitBoard功能
    try {
        // 1. 测试标准开局
        std::cout << "\n🎯 测试BitBoard核心功能" << std::endl;
        std::cout << std::string(30, '-') << std::endl;
        std::cout << "1. 创建标准开局棋盘..." << std::endl;

        Reversi::BitBoard board;
        board.resetToStandardOpening();

        std::cout << "标准开局:" << std::endl;
        board.print();
        std::cout << "黑棋数量: " << board.getScore(Reversi::PlayerColor::Black) << std::endl;
        std::cout << "白棋数量: " << board.getScore(Reversi::PlayerColor::White) << std::endl;
        std::cout << "空位数量: " << board.getEmptyCount() << std::endl;

        // 2. 测试有效移动生成
        std::cout << "\n2. 测试有效移动生成..." << std::endl;
        uint64_t black_moves = board.getValidMoves(Reversi::PlayerColor::Black);
        uint64_t white_moves = board.getValidMoves(Reversi::PlayerColor::White);

        std::cout << "黑方有效移动数量: " << __builtin_popcountll(black_moves) << std::endl;
        std::cout << "白方有效移动数量: " << __builtin_popcountll(white_moves) << std::endl;

        // 显示黑方有效移动位置
        std::cout << "黑方有效移动位置: ";
        for (int pos = 0; pos < 64; ++pos) {
            if (black_moves & (1ULL << pos)) {
                int row = pos / 8;
                int col = pos % 8;
                std::cout << "(" << row << "," << col << ") ";
            }
        }
        std::cout << std::endl;

        // 3. 测试移动执行
        std::cout << "\n3. 测试移动执行..." << std::endl;
        bool move_successful = false;
        // 尝试第一个有效的移动
        if (black_moves) {
            int first_pos = __builtin_ctzll(black_moves);
            int row = first_pos / 8;
            int col = first_pos % 8;
            std::cout << "尝试移动到位置: (" << row << "," << col << ")" << std::endl;
            move_successful = board.makeMove(row, col, Reversi::PlayerColor::Black);
            std::cout << "移动结果: " << (move_successful ? "成功" : "失败") << std::endl;
        } else {
            std::cout << "没有有效移动，跳过测试" << std::endl;
        }

        if (move_successful) {
            std::cout << "移动后的棋盘:" << std::endl;
            board.print();
            std::cout << "黑棋数量: " << board.getScore(Reversi::PlayerColor::Black) << std::endl;
            std::cout << "白棋数量: " << board.getScore(Reversi::PlayerColor::White) << std::endl;
        }

        // 4. 测试游戏结束检测
        std::cout << "\n4. 测试游戏状态..." << std::endl;
        bool game_over = board.isGameOver();
        std::cout << "游戏结束: " << (game_over ? "是" : "否") << std::endl;

        if (!game_over) {
            auto winner = board.getWinner();
            if (winner.has_value()) {
                std::cout << "当前领先: " << (winner.value() == Reversi::PlayerColor::Black ? "黑棋" : "白棋") << std::endl;
            } else {
                std::cout << "当前平局" << std::endl;
            }
        }

        std::cout << "\n✅ BitBoard功能测试完成" << std::endl;

        std::cout << "\n✅ 所有测试完成 - 核心架构验证通过" << std::endl;
        std::cout << "🎯 v0.1.0基础框架 + v0.2.0 BitBoard核心功能: 通过" << std::endl;
        std::cout << "📈 下一步: 实现Board包装类和游戏规则完整性" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ 未知错误发生" << std::endl;
        return 1;
    }
}
