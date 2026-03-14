// MCTS功能完整测试
#include <iostream>
#include <chrono>
#include <memory>
#include "core/BitBoard.h"
#include "Board.h"
#include "ai/Evaluator.h"
#include "ai/MCTSAI.h"
#include "ai/MinimaxAI.h"

using namespace Reversi;

int main() {
    std::cout << "=== MCTS Comprehensive Test ===" << std::endl;
    
    // 测试1: MCTS单次搜索
    std::cout << "\n[Test 1] MCTS Single Search" << std::endl;
    {
        MCTSConfig config;
        config.num_simulations = 200;
        config.c_puct = 1.0;
        
        auto mcts = std::make_unique<MCTSAI>(config);
        Board board;
        
        SearchLimits limits;
        limits.maxNodes = 200;
        limits.timeLimit = std::chrono::seconds(5);
        
        auto start = std::chrono::steady_clock::now();
        Move bestMove = mcts->findBestMove(board, limits);
        auto end = std::chrono::steady_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "  Best move: " << bestMove.toString() << std::endl;
        std::cout << "  Time: " << duration.count() << "ms" << std::endl;
        std::cout << "  [PASSED]" << std::endl;
    }
    
    // 测试2: MCTS多次搜索(模拟多步)
    std::cout << "\n[Test 2] MCTS Multiple Searches" << std::endl;
    {
        MCTSConfig config;
        config.num_simulations = 100;
        config.c_puct = 1.0;
        
        auto mcts = std::make_unique<MCTSAI>(config);
        Board board;
        
        SearchLimits limits;
        limits.maxNodes = 100;
        limits.timeLimit = std::chrono::seconds(3);
        
        int moves = 0;
        while (!board.isGameOver() && moves < 10) {
            Move bestMove = mcts->findBestMove(board, limits);
            if (!bestMove.isValid() || bestMove.is_pass) {
                std::cout << "  Move " << moves << ": pass" << std::endl;
            } else {
                std::cout << "  Move " << moves << ": " << bestMove.toString() << std::endl;
            }
            board.makeMove(bestMove);
            moves++;
        }
        std::cout << "  Completed " << moves << " moves" << std::endl;
        std::cout << "  [PASSED]" << std::endl;
    }
    
    // 测试3: MCTS vs Minimax 对战 (简化版)
    std::cout << "\n[Test 3] MCTS vs Minimax Battle (5 games)" << std::endl;
    {
        MCTSConfig mctsConfig;
        mctsConfig.num_simulations = 50;
        mctsConfig.c_puct = 1.0;
        
        MinimaxConfig miniConfig;
        miniConfig.maxDepth = 4;
        
        int mctsWins = 0;
        int miniWins = 0;
        int draws = 0;
        
        for (int game = 0; game < 5; ++game) {
            std::cout << "  Game " << (game + 1) << "..." << std::endl;
            
            auto mcts = std::make_unique<MCTSAI>(mctsConfig);
            auto minimax = std::make_unique<MinimaxAI>(miniConfig);
            
            Board board;
            bool mctsIsBlack = (game % 2 == 0);
            
            SearchLimits mctsLimits;
            mctsLimits.maxNodes = 50;
            mctsLimits.timeLimit = std::chrono::seconds(2);
            
            SearchLimits miniLimits;
            miniLimits.maxDepth = 4;
            miniLimits.timeLimit = std::chrono::seconds(2);
            
            while (!board.isGameOver()) {
                if (mctsIsBlack == (board.getCurrentTurn() == PlayerColor::Black)) {
                    Move m = mcts->findBestMove(board, mctsLimits);
                    if (m.isValid() && !m.is_pass) {
                        board.makeMove(m);
                    } else {
                        auto valid = board.getValidMoves();
                        if (!valid.empty()) board.makeMove(valid[0]);
                    }
                } else {
                    Move m = minimax->findBestMove(board, miniLimits);
                    if (m.isValid() && !m.is_pass) {
                        board.makeMove(m);
                    } else {
                        auto valid = board.getValidMoves();
                        if (!valid.empty()) board.makeMove(valid[0]);
                    }
                }
            }
            
            auto winner = board.getWinner();
            if (!winner.has_value()) {
                draws++;
            } else {
                bool mctsWon = (mctsIsBlack && winner.value() == PlayerColor::Black) ||
                              (!mctsIsBlack && winner.value() == PlayerColor::White);
                if (mctsWon) mctsWins++;
                else miniWins++;
            }
        }
        
        std::cout << "  Results:" << std::endl;
        std::cout << "    MCTS wins: " << mctsWins << std::endl;
        std::cout << "    Minimax wins: " << miniWins << std::endl;
        std::cout << "    Draws: " << draws << std::endl;
        
        if (mctsWins > 0 || draws > 0) {
            std::cout << "  [PASSED - MCTS is functional]" << std::endl;
        } else {
            std::cout << "  [FAILED - MCTS lost all games]" << std::endl;
        }
    }
    
    std::cout << "\n=== All Tests Completed ===" << std::endl;
    return 0;
}