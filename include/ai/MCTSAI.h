#pragma once

#include "ai/AIStrategy.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <random>

/**
 * @file MCTSAI.h
 * @brief MCTS算法AI实现
 *
 * 基于alpha-zero-general的UCT算法，转换为C++ BitBoard实现
 * 参考: alpha-zero-general/MCTS.py
 *       alpha-zero-general/othello/OthelloGame.py
 */

namespace Reversi {

/**
 * @brief MCTS节点结构
 *
 * 表示搜索树中的一个节点
 * 参考: alpha-zero-general/MCTS.py中的树节点概念
 */
struct MCTSNode {
    /**
     * @brief 构造函数
     * @param parent 父节点指针
     */
    explicit MCTSNode(MCTSNode* parent = nullptr);

    // 节点统计信息
    double value_sum = 0.0;        ///< Q值总和 (所有访问的价值的总和)
    int visit_count = 0;           ///< 访问次数 N(s,a)
    double prior = 0.0;            ///< 先验概率 P(s,a)

    // 树结构
    MCTSNode* parent;              ///< 父节点
    std::vector<std::unique_ptr<MCTSNode>> children;  ///< 子节点
    std::vector<Move> child_moves;  ///< 对应的移动

    /**
     * @brief 获取价值估计 (Q值)
     * @return 平均价值
     */
    double getValue() const {
        return visit_count > 0 ? value_sum / visit_count : 0.0;
    }

    /**
     * @brief 检查是否为叶子节点
     * @return true如果没有子节点
     */
    bool isLeaf() const;

    /**
     * @brief 查找子节点
     * @param move 移动
     * @return 子节点指针，如果不存在返回nullptr
     */
    MCTSNode* findChild(const Move& move);

    /**
     * @brief 添加子节点
     * @param move 移动
     * @param node 子节点
     */
    void addChild(Move move, std::unique_ptr<MCTSNode> node);

    /**
     * @brief 获取子节点数量
     */
    size_t getChildCount() const { return children.size(); }

    /**
     * @brief 获取第i个子节点
     */
    MCTSNode* getChild(size_t index) { return children[index].get(); }
    const MCTSNode* getChild(size_t index) const { return children[index].get(); }

    /**
     * @brief 获取第i个子移动
     */
    const Move& getChildMove(size_t index) const { return child_moves[index]; }

    /**
     * @brief 获取UCT值 (Upper Confidence Bound)
     * @param c_puct 探索常数
     * @return UCT值
     *
     * 公式: Q(s,a) + c_puct * P(s,a) * sqrt(N(s)) / (1 + N(s,a))
     * 参考: alpha-zero-general/MCTS.py line 112-113
     */
    double getUCT(double c_puct) const;
};

/**
 * @brief MCTS配置参数
 */
struct MCTSConfig {
    int num_simulations = 1000;    ///< 每次移动的仿真次数
    double c_puct = 1.0;           ///< 探索常数 (alpha-zero-general默认1.0)
    bool use_dirichlet_noise = false;  ///< 是否使用Dirichlet噪声
    double dirichlet_alpha = 0.3;  ///< Dirichlet噪声参数
    double dirichlet_epsilon = 0.25;  ///< 噪声混合比例

    // 时间控制
    std::chrono::milliseconds time_limit = std::chrono::milliseconds(3000);
};

/**
 * @brief MCTS AI实现
 *
 * 基于alpha-zero-general的UCT算法，适配BitBoard系统
 * 参考: alpha-zero-general/MCTS.py
 */
class MCTSAI : public AIStrategy {
public:
    /**
     * @brief 构造函数
     * @param config MCTS配置
     * @param evaluator 评估器（用于叶子节点评估）
     */
    explicit MCTSAI(MCTSConfig config = MCTSConfig(),
                   std::unique_ptr<Evaluator> evaluator = nullptr);

    // AIStrategy接口实现
    Move findBestMove(const Board& board, const SearchLimits& limits) override;
    std::string getName() const override { return "MCTSAI"; }
    std::string getDescription() const override;
    Difficulty getDifficulty() const override;
    AIStats getStats() const override;
    std::string getConfigDescription() const override;
    void reset() override;
    bool supportsFeature(const std::string& feature) const override;

    /**
     * @brief 获取MCTS配置
     */
    const MCTSConfig& getConfig() const { return config_; }

    /**
     * @brief 设置MCTS配置
     */
    void setConfig(const MCTSConfig& config) { config_ = config; }

private:
    /**
     * @brief 执行一次MCTS搜索
     * @param board 当前棋盘状态
     * @param limits 搜索限制
     *
     * 参考: alpha-zero-general/MCTS.py search()方法
     */
    void search(const Board& board, const SearchLimits& limits);

    /**
     * @brief 仿真阶段：随机仿真或快速评估
     * @param board 当前棋盘状态
     * @param max_depth 最大仿真深度
     * @return 仿真结果 (-1到1之间)
     *
     * 参考: alpha-zero-general/MCTS.py 随机仿真
     */
    double simulate(const Board& board, int max_depth = 50);

    /**
     * @brief 回传阶段：向上传播结果
     * @param node 叶子节点
     * @param value 仿真结果
     *
     * 参考: alpha-zero-general/MCTS.py 回传逻辑
     */
    void backpropagate(MCTSNode* node, double value);

    /**
     * @brief 获取移动的先验概率
     * @param board 当前棋盘状态
     * @return 每个移动的概率向量
     *
     * 简化版本：所有有效移动概率相等
     * 未来可以集成神经网络策略
     */
    std::vector<double> getPriorProbabilities(const Board& board);

    /**
     * @brief 添加Dirichlet噪声（用于根节点多样性）
     * @param priors 先验概率
     * @return 添加噪声后的概率
     *
     * 参考: alpha-zero-general论文
     */
    std::vector<double> addDirichletNoise(const std::vector<double>& priors);

    /**
     * @brief 生成Dirichlet分布随机数
     * @param alpha 浓度参数
     * @param size 向量大小
     * @return Dirichlet分布样本
     */
    std::vector<double> sampleDirichlet(double alpha, size_t size);

    // 配置和组件
    MCTSConfig config_;
    std::unique_ptr<Evaluator> evaluator_;

    // MCTS树
    std::unique_ptr<MCTSNode> root_;

    // 统计信息
    mutable AIStats stats_;

    // 随机数生成器
    std::mt19937 rng_;
    std::uniform_real_distribution<double> uniform_dist_;
};

} // namespace Reversi
