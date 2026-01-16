// 标准AI策略接口实现模式
// 参考：https://en.wikipedia.org/wiki/Strategy_pattern

#include <memory>
#include <string>
#include <vector>

// 前向声明（减少头文件依赖）
class Board;
struct Move;
struct SearchLimits;

// AI策略接口
class AIStrategy {
public:
    virtual ~AIStrategy() = default;

    // 核心接口：寻找最佳移动
    virtual Move findBestMove(const Board& board, const SearchLimits& limits) = 0;

    // 策略信息
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual int getDifficulty() const = 0; // 1-10, 10最难
};

// 随机AI实现（最简单策略）
class RandomAI : public AIStrategy {
public:
    Move findBestMove(const Board& board, const SearchLimits& limits) override {
        // 1. 生成合法移动
        auto valid_moves = board.getValidMoves();

        // 2. 随机选择一个移动
        if (valid_moves.empty()) {
            return Move::pass(); // 无合法移动，跳过回合
        }

        // 使用现代随机数生成
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, valid_moves.size() - 1);

        return valid_moves[dis(gen)];
    }

    std::string getName() const override { return "RandomAI"; }
    std::string getDescription() const override {
        return "随机选择一个合法移动，适合测试和基准对比";
    }
    int getDifficulty() const override { return 1; }
};

// Minimax AI实现框架
class MinimaxAI : public AIStrategy {
private:
    int max_depth_;  // 搜索深度

public:
    explicit MinimaxAI(int depth = 4) : max_depth_(depth) {}

    Move findBestMove(const Board& board, const SearchLimits& limits) override {
        // 实现Minimax算法
        // 1. 生成所有可能移动
        // 2. 对每个移动进行Minimax评估
        // 3. 返回分数最高的移动

        // 占位符实现
        auto valid_moves = board.getValidMoves();
        return valid_moves.empty() ? Move::pass() : valid_moves[0];
    }

    std::string getName() const override {
        return "MinimaxAI (Depth " + std::to_string(max_depth_) + ")";
    }

    std::string getDescription() const override {
        return "经典的Minimax算法实现，支持alpha-beta剪枝";
    }

    int getDifficulty() const override {
        return std::min(10, max_depth_ * 2); // 深度越高难度越大
    }
};

// 使用示例：
// auto ai = std::make_unique<RandomAI>();
// Move best_move = ai->findBestMove(board, limits);
// std::cout << "AI选择了: " << best_move.toString() << std::endl;

/*
策略模式的优势：
1. 运行时切换AI算法，无需修改调用代码
2. 易于扩展新的AI算法
3. 符合开闭原则
4. 支持依赖注入

Cursor实现新AI时参考此模式：
1. 继承AIStrategy接口
2. 实现所有纯虚函数
3. 在构造函数中设置算法参数
4. 添加适当的文档注释
*/
