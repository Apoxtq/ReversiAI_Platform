// MCTS vs Minimax 对战测试 - 交替搜索
#include <iostream>
#include <chrono>
#include <memory>

#define QT_NO_METAOBJECT
#define QT_NO_SIGNALS

#include "core/BitBoard.h"
#include "Board.h"
#include "ai/AIStrategy.h"
#include "ai/MCTSAI.h"
#include "ai/MinimaxAI.h"

using namespace Reversi;

int main() {
    std::cout << "=== MCTS vs Minimax Sequential Test ===" << std::endl;
    
    MCTSConfig mctsConfig;
    mctsConfig.num_simulations = 50;
    auto mcts = std::make_unique<MCTSAI>(mctsConfig);
    std::cout << "MCTS created" << std::endl;
    
    MinimaxConfig miniConfig;
    miniConfig.maxDepth = 4;
    auto minimax = std::make_unique<MinimaxAI>(miniConfig);
    std::cout << "Minimax created" << std::endl;
    
    const int NUM_GAMES = 5;
    int mctsWins = 0;
    int minimaxWins = 0;
    int draws = 0;
    
    for (int game = 0; game < NUM_GAMES; ++game) {
        std::cout << "\nGame " << (game + 1) << "/" << NUM_GAMES << "..." << std::endl;
        
        Board board;
        bool mctsIsBlack = (game % 2 == 0);
        
        while (!board.isGameOver()) {
            if (mctsIsBlack == (board.getCurrentTurn() == PlayerColor::Black)) {
                SearchLimits lim;
                lim.maxNodes = 50;
                lim.timeLimit = std::chrono::milliseconds(1000);
                
                Move m = mcts->findBestMove(board, lim);
                if (m.isValid() && !m.is_pass) {
                    board.makeMove(m);
                } else {
                    auto valid = board.getValidMoves();
                    if (!valid.empty()) board.makeMove(valid[0]);
                }
            } else {
                SearchLimits lim;
                lim.maxDepth = 4;
                lim.timeLimit = std::chrono::milliseconds(1000);
                
                Move m = minimax->findBestMove(board, lim);
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
            else minimaxWins++;
        }
        
        std::cout << "  Game " << (game + 1) << " done" << std::endl;
    }
    
    std::cout << "\n=== Results ===" << std::endl;
    std::cout << "MCTS wins: " << mctsWins << " (" << (mctsWins * 100.0 / NUM_GAMES) << "%)" << std::endl;
    std::cout << "Minimax wins: " << minimaxWins << " (" << (minimaxWins * 100.0 / NUM_GAMES) << "%)" << std::endl;
    std::cout << "Draws: " << draws << " (" << (draws * 100.0 / NUM_GAMES) << "%)" << std::endl;
    
    return 0;
}
