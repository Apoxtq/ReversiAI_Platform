#ifndef MCTS_H
#define MCTS_H

#include <vector>

#define WHITE  1
#define BLACK  2

typedef struct posnode {
    int x;
    int y;
    posnode(int x, int y) {
        this->x = x;
        this->y = y;
    }
    posnode() {
        x = y = -1;
    }
}Position, Postion; // Keep both for compatibility

class MCTS {
public:
    std::vector<MCTS*> child;
    MCTS* bestchild = nullptr;
    int visit;
    double score;
    bool expandable = true;
    MCTS* father = nullptr;
    bool root = false;
    Position pos;
    int mytile;
    double C = 0.02;
    float realscore;

    MCTS(Position pos, int mytile);
    void rootClear();
    double calculate();
    int bestChild();
};

#endif // MCTS_H
