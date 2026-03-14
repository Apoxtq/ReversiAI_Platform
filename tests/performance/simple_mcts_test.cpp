// 简单的MCTS测试 - 隔离测试
#include <iostream>
#include <chrono>
#include <memory>
#include "core/BitBoard.h"
#include "Board.h"
#include "ai/Evaluator.h"
#include "ai/MCTSAI.h"

using namespace Reversi;

int main() {
    std::cout << "=== Simple MCTS Test ===" << std::endl;
    
    // 创建MCTS AI
    MCTSConfig config;
    config.num_simulations = 100;  // 少量simulations
    config.c_puct = 1.0;
    
    std::cout << "Creating MCTSAI..." << std::endl;
    auto mcts = std::make_unique<MCTSAI>(config);
    std::cout << "MCTSAI created successfully" << std::endl;
    
    // 创建标准开局
    Board board;
    std::cout << "Board created" << std::endl;
    
    // 搜索限制
    SearchLimits limits;
    limits.maxDepth = 10;
    limits.maxNodes = 100;
    limits.timeLimit = std::chrono::seconds(5);
    
    std::cout << "Starting MCTS search..." << std::endl;
    
    try {
        Move bestMove = mcts->findBestMove(board, limits);
        std::cout << "MCTS search completed" << std::endl;
        std::cout << "Best move: " << bestMove.toString() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        return 1;
    }
    
    std::cout << "Test completed successfully" << std::endl;
    return 0;
}