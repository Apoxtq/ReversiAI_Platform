// 测试MCTS搜索行为
#include <iostream>
#include <chrono>
#include <memory>

#define QT_NO_METAOBJECT
#define QT_NO_SIGNALS

#include "core/BitBoard.h"
#include "Board.h"
#include "ai/Evaluator.h"
#include "ai/MCTSAI.h"

using namespace Reversi;

int main() {
    std::cout << "=== Test: MCTS Multiple Searches ===" << std::endl;
    
    MCTSConfig config;
    config.num_simulations = 50;
    
    auto mcts = std::make_unique<MCTSAI>(config);
    std::cout << "MCTS created" << std::endl;
    
    Board board;
    SearchLimits limits;
    limits.maxNodes = 50;
    limits.timeLimit = std::chrono::seconds(2);
    
    // 第一次搜索
    std::cout << "Search 1..." << std::endl;
    Move m1 = mcts->findBestMove(board, limits);
    std::cout << "  Result: " << m1.toString() << std::endl;
    
    // 走一步棋
    board.makeMove(m1);
    std::cout << "Move made" << std::endl;
    
    // 第二次搜索
    std::cout << "Search 2..." << std::endl;
    Move m2 = mcts->findBestMove(board, limits);
    std::cout << "  Result: " << m2.toString() << std::endl;
    
    std::cout << "\n=== Done ===" << std::endl;
    return 0;
}