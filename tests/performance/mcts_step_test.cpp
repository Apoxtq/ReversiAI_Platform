// 简化MCTS测试
#include <iostream>
#include <chrono>
#include <memory>
#include "core/BitBoard.h"
#include "Board.h"
#include "ai/Evaluator.h"
#include "ai/MCTSAI.h"

using namespace Reversi;

int main() {
    std::cout << "=== Step-by-Step MCTS Test ===" << std::endl;
    
    std::cout << "Step 1: Creating config..." << std::endl;
    MCTSConfig config;
    config.num_simulations = 50;
    config.c_puct = 1.0;
    
    std::cout << "Step 2: Creating MCTS AI..." << std::endl;
    auto mcts = std::make_unique<MCTSAI>(config);
    std::cout << "  Created successfully" << std::endl;
    
    std::cout << "Step 3: Creating board..." << std::endl;
    Board board;
    std::cout << "  Created successfully" << std::endl;
    
    std::cout << "Step 4: Creating limits..." << std::endl;
    SearchLimits limits;
    limits.maxNodes = 50;
    limits.timeLimit = std::chrono::seconds(3);
    std::cout << "  Created successfully" << std::endl;
    
    std::cout << "Step 5: First MCTS search..." << std::endl;
    try {
        Move bestMove = mcts->findBestMove(board, limits);
        std::cout << "  Success! Best move: " << bestMove.toString() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "  EXCEPTION: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Step 6: Second MCTS search (same board)..." << std::endl;
    try {
        Move bestMove = mcts->findBestMove(board, limits);
        std::cout << "  Success! Best move: " << bestMove.toString() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "  EXCEPTION: " << e.what() << std::endl;
    }
    
    std::cout << "Step 7: Making a move..." << std::endl;
    Move m(2, 3); // c4
    board.makeMove(m);
    std::cout << "  Move made" << std::endl;
    
    std::cout << "Step 8: Third MCTS search (after move)..." << std::endl;
    try {
        Move bestMove = mcts->findBestMove(board, limits);
        std::cout << "  Success! Best move: " << bestMove.toString() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "  EXCEPTION: " << e.what() << std::endl;
    }
    
    std::cout << "\n=== Test Completed ===" << std::endl;
    return 0;
}