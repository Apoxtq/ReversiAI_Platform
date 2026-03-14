// 调试MCTS崩溃 - 完整对局测试
#include <iostream>
#include <chrono>
#include <memory>

#define QT_NO_METAOBJECT
#define QT_NO_SIGNALS

#include "core/BitBoard.h"
#include "Board.h"
#include "ai/Evaluator.h"
#include "ai/MCTSAI.h"
#include "ai/MinimaxAI.h"

using namespace Reversi;

int main() {
    std::cout << "=== Debug: Full Game ===" << std::endl;
    
    MCTSConfig mctsConfig;
    mctsConfig.num_simulations = 50;
    auto mcts = std::make_unique<MCTSAI>(mctsConfig);
    std::cout << "MCTS created" << std::endl;
    
    MinimaxConfig miniConfig;
    miniConfig.maxDepth = 4;
    auto minimax = std::make_unique<MinimaxAI>(miniConfig);
    std::cout << "Minimax created" << std::endl;
    
    Board board;
    bool mctsIsBlack = true;
    
    SearchLimits limits;
    limits.maxNodes = 50;
    limits.timeLimit = std::chrono::seconds(2);
    
    int moveNum = 0;
    while (!board.isGameOver()) {
        std::cout << "Move " << (moveNum + 1) << "..." << std::flush;
        
        bool isMCTSTurn = mctsIsBlack == (board.getCurrentTurn() == PlayerColor::Black);
        
        if (isMCTSTurn) {
            Move m = mcts->findBestMove(board, limits);
            if (m.isValid() && !m.is_pass) {
                board.makeMove(m);
            } else {
                auto valid = board.getValidMoves();
                if (!valid.empty()) board.makeMove(valid[0]);
            }
        } else {
            Move m = minimax->findBestMove(board, limits);
            if (m.isValid() && !m.is_pass) {
                board.makeMove(m);
            } else {
                auto valid = board.getValidMoves();
                if (!valid.empty()) board.makeMove(valid[0]);
            }
        }
        moveNum++;
        std::cout << " done" << std::endl;
        
        if (moveNum > 60) {
            std::cout << "Too many moves, stopping" << std::endl;
            break;
        }
    }
    
    auto winner = board.getWinner();
    if (winner.has_value()) {
        std::cout << "Winner: " << (winner.value() == PlayerColor::Black ? "Black" : "White") << std::endl;
    } else {
        std::cout << "Draw" << std::endl;
    }
    
    std::cout << "\n=== Game completed ===" << std::endl;
    return 0;
}