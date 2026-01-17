#include <iostream>
#include <chrono>
#include "core/BitBoard.h"

using namespace Reversi;

int main(int argc, char** argv) {
    BitBoard board;
    board.resetToStandardOpening();

    const int iterations = (argc > 1) ? std::stoi(argv[1]) : 2000000;

    auto start = std::chrono::high_resolution_clock::now();
    uint64_t acc_moves = 0;
    for (int i = 0; i < iterations; ++i) {
        uint64_t moves = board.getValidMoves(PlayerColor::Black);
        acc_moves += __builtin_popcountll(moves);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> dur = end - start;

    double secs = dur.count();
    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "Time: " << secs << " s\n";
    std::cout << "Calls/sec: " << (iterations / secs) << "\n";
    std::cout << "Avg moves per call: " << (static_cast<double>(acc_moves) / iterations) << "\n";
    return 0;
}


