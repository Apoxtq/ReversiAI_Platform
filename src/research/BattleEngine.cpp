#include "research/BattleEngine.h"
#include <iostream>
#include <algorithm>
#include <random>
#include <thread>
#include <future>
#include <iomanip>
#include <fstream>
#include <cmath>
namespace Reversi {
// ============================================================================
// 静态成员初始化
// ============================================================================
uint64_t BattleEngine::global_seed_ = 0;
std::mt19937_64 BattleEngine::rng_(std::random_device{}());
// ============================================================================
// BattleStats 实现
// ============================================================================
void BattleStats::calculate() {
    if (total_games == 0) return;
    // 胜率
    win_rate1 = static_cast<double>(player1_wins) / total_games;
    win_rate2 = static_cast<double>(player2_wins) / total_games;
    // 平均回合数
    double total_moves = 0;
    double total_duration = 0;
    double total_score1 = 0;
    double total_score2 = 0;
    int max_m = 0;
    for (const auto& game : games) {
        total_moves += game.moves_count;
        total_duration += game.duration_ms;
        total_score1 += game.black_score;
        total_score2 += game.white_score;
        max_m = std::max(max_m, game.getMargin());
    }
    avg_moves = total_moves / total_games;
    avg_duration_ms = total_duration / total_games;
    avg_score1 = total_score1 / total_games;
    avg_score2 = total_score2 / total_games;
    avg_margin = std::abs(avg_score1 - avg_score2);
    max_margin = max_m;
    // 计算先手胜率
    if (player1_first_games > 0) {
        // player1先手时的胜率在player1_wins中已包含
    }
}
std::pair<double, double> BattleStats::getWinRateCI1() const {
    // 简单近似: 使用正态分布近似二项分布
    double p = win_rate1;
    double n = total_games;
    double se = std::sqrt(p * (1 - p) / n);
    double z = 1.96;  // 95%置信度
    return {p - z * se, p + z * se};
}
std::string BattleStats::toString() const {
    std::ostringstream oss;
    oss << "Battle Results: " << player1_name << " vs " << player2_name << "\n";
    oss << "========================================\n";
    oss << "Total Games: " << total_games << "\n";
    oss << player1_name << " Wins: " << player1_wins
         << " (" << std::fixed << std::setprecision(1) << win_rate1 * 100 << "%)\n";
    oss << player2_name << " Wins: " << player2_wins
         << " (" << std::fixed << std::setprecision(1) << win_rate2 * 100 << "%)\n";
    oss << "Draws: " << draws << "\n";
    oss << "Average Moves: " << std::fixed << std::setprecision(1) << avg_moves << "\n";
    oss << "Average Duration: " << std::fixed << std::setprecision(0) << avg_duration_ms << "ms\n";
    if (significant) {
        oss << "Statistical Significance: p=" << std::scientific << p_value << "\n";
    } else {
        oss << "Statistical Significance: Not significant (p=" << std::scientific << p_value << ")\n";
    }
    return oss.str();
}
// ============================================================================
// BattleEngine 实现
// ============================================================================
BattleStats BattleEngine::runBattle(const BattleConfig& config,
                                      BattleProgressCallback progress_callback) {
    BattleStats stats;
    stats.player1_name = config.player1_name;
    stats.player2_name = config.player2_name;
    // 设置随机种子
    if (config.random_seed != 0) {
        setRandomSeed(config.random_seed);
    }
    // 验证配置
    if (!validateConfig(config)) {
        std::cerr << "[BattleEngine] Invalid config, aborting battle" << std::endl;
        return stats;
    }
    // 重置AI状态
    config.player1->reset();
    config.player2->reset();
    // 根据配置选择对战方式
    std::vector<GameResult> results;
    if (config.parallel && config.num_games > 1) {
        results = runParallelBattle(config);
    } else {
        results.reserve(config.num_games);
        for (int i = 0; i < config.num_games; ++i) {
            bool player1_first = (i % 2 == 0) == config.alternate_first;
            GameResult result = playGameInternal(
                *config.player1, *config.player2,
                player1_first ? PlayerColor::Black : PlayerColor::White,
                config.limits1, config.limits2
            );
            result.game_number = i + 1;
            updateStats(stats, result, player1_first);
            results.push_back(result);
            // 进度回调
            if (progress_callback) {
                progress_callback(i + 1, config.num_games, result);
            }
            // 重置AI状态
            config.player1->reset();
            config.player2->reset();
        }
    }
    stats.games = std::move(results);
    stats.total_games = config.num_games;
    stats.calculate();
    // 计算统计显著性 (使用二项分布检验)
    if (stats.total_games >= 10) {
        // 假设零假设是50%胜率
        int wins = stats.player1_wins;
        int n = stats.total_games;
        double p_win = 0.5;  // 零假设：50%胜率
        // 计算二项分布单侧p值
        double p_value = 0.0;
        for (int k = wins; k <= n; ++k) {
            // 简化计算：使用正态近似
            // 这里使用简化方法
        }
        // 使用正态近似计算p值
        double expected = n * p_win;
        double std_dev = std::sqrt(n * p_win * (1 - p_win));
        if (std_dev > 0) {
            double z = (wins - expected) / std_dev;
            // 标准正态分布CDF
            double cdf = 0.5 * (1.0 + std::erf(z / std::sqrt(2.0)));
            if (z > 0) {
                p_value = 2.0 * (1.0 - cdf);  // 双侧检验
            } else {
                p_value = 2.0 * cdf;  // 双侧检验
            }
        }
        stats.p_value = std::min(p_value, 1.0);
        stats.significant = stats.p_value < 0.05;
    }
    return stats;
}
GameResult BattleEngine::playSingleGame(
    AIStrategy& player1,
    AIStrategy& player2,
    PlayerColor first_player,
    const SearchLimits& limits1,
    const SearchLimits& limits2
) {
    return playGameInternal(player1, player2, first_player, limits1, limits2);
}
bool BattleEngine::validateConfig(const BattleConfig& config) {
    if (!config.player1 || !config.player2) {
        std::cerr << "[BattleEngine] Both players must be valid" << std::endl;
        return false;
    }
    if (config.num_games <= 0) {
        std::cerr << "[BattleEngine] Number of games must be positive" << std::endl;
        return false;
    }
    return true;
}
void BattleEngine::exportToCSV(const BattleStats& stats, const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[BattleEngine] Failed to open file: " << filepath << std::endl;
        return;
    }
    // 标题行
    file << "player1,player2,total_games,player1_wins,player2_wins,draws,";
    file << "win_rate1,win_rate2,avg_moves,avg_duration_ms,p_value,significant\n";
    // 数据行
    file << stats.player1_name << "," << stats.player2_name << ",";
    file << stats.total_games << "," << stats.player1_wins << ",";
    file << stats.player2_wins << "," << stats.draws << ",";
    file << std::fixed << std::setprecision(6) << stats.win_rate1 << ",";
    file << stats.win_rate2 << ",";
    file << std::setprecision(1) << stats.avg_moves << ",";
    file << std::setprecision(0) << stats.avg_duration_ms << ",";
    file << std::scientific << stats.p_value << ",";
    file << (stats.significant ? "true" : "false") << "\n";
    file.close();
    std::cout << "[BattleEngine] Exported stats to " << filepath << std::endl;
}
void BattleEngine::exportGameLog(const BattleStats& stats, const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[BattleEngine] Failed to open file: " << filepath << std::endl;
        return;
    }
    file << "# Battle Game Log\n";
    file << "# " << stats.player1_name << " vs " << stats.player2_name << "\n";
    file << "# Total Games: " << stats.total_games << "\n";
    file << "# " << stats.player1_name << " Wins: " << stats.player1_wins << "\n";
    file << "# " << stats.player2_name << " Wins: " << stats.player2_wins << "\n";
    file << "# Draws: " << stats.draws << "\n";
    file << "#\n";
    file << "# Format: game_number,winner,black_score,white_score,moves,duration_ms\n";
    file << "#\n";
    for (const auto& game : stats.games) {
        file << game.game_number << ","
             << (game.winner == PlayerColor::Black ? "Black" : "White") << ","
             << game.black_score << "," << game.white_score << ","
             << game.moves_count << "," << std::fixed << std::setprecision(0)
             << game.duration_ms << "\n";
    }
    file.close();
    std::cout << "[BattleEngine] Exported game log to " << filepath << std::endl;
}
void BattleEngine::exportToJSON(const BattleStats& stats, const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[BattleEngine] Failed to open file: " << filepath << std::endl;
        return;
    }
    file << "{\n";
    file << "  \"player1\": \"" << stats.player1_name << "\",\n";
    file << "  \"player2\": \"" << stats.player2_name << "\",\n";
    file << "  \"total_games\": " << stats.total_games << ",\n";
    file << "  \"player1_wins\": " << stats.player1_wins << ",\n";
    file << "  \"player2_wins\": " << stats.player2_wins << ",\n";
    file << "  \"draws\": " << stats.draws << ",\n";
    file << "  \"win_rate1\": " << std::fixed << std::setprecision(6) << stats.win_rate1 << ",\n";
    file << "  \"win_rate2\": " << stats.win_rate2 << ",\n";
    file << "  \"avg_moves\": " << std::setprecision(1) << stats.avg_moves << ",\n";
    file << "  \"avg_duration_ms\": " << std::setprecision(0) << stats.avg_duration_ms << ",\n";
    file << "  \"p_value\": " << std::scientific << stats.p_value << ",\n";
    file << "  \"significant\": " << (stats.significant ? "true" : "false") << "\n";
    file << "  \"games\": [\n";
    for (size_t i = 0; i < stats.games.size(); ++i) {
        const auto& game = stats.games[i];
        file << "    {\n";
        file << "      \"game_number\": " << game.game_number << ",\n";
        file << "      \"winner\": \"" << (game.winner == PlayerColor::Black ? "Black" : "White") << "\",\n";
        file << "      \"black_score\": " << game.black_score << ",\n";
        file << "      \"white_score\": " << game.white_score << ",\n";
        file << "      \"moves\": " << game.moves_count << ",\n";
        file << "      \"duration_ms\": " << std::fixed << std::setprecision(0) << game.duration_ms << "\n";
        file << "    }";
        if (i < stats.games.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ]\n";
    file << "}\n";
    file.close();
    std::cout << "[BattleEngine] Exported JSON to " << filepath << std::endl;
}
BattleStats BattleEngine::runStandardBenchmark(
    std::unique_ptr<AIStrategy> ai,
    int depth,
    int num_games
) {
    BattleConfig config;
    config.player1 = std::move(ai);
    config.player1_name = config.player1->getName();
    config.player2 = AIStrategyFactory::createRandomAI();
    config.player2_name = "RandomAI";
    config.num_games = num_games;
    config.limits1.maxDepth = depth;
    config.verbose = false;
    return runBattle(config);
}
BattleStats BattleEngine::compareAI(
    std::unique_ptr<AIStrategy> ai1,
    std::unique_ptr<AIStrategy> ai2,
    int num_games
) {
    BattleConfig config;
    config.player1 = std::move(ai1);
    config.player2 = std::move(ai2);
    config.player1_name = config.player1->getName();
    config.player2_name = config.player2->getName();
    config.num_games = num_games;
    config.verbose = false;
    return runBattle(config);
}
void BattleEngine::setRandomSeed(uint64_t seed) {
    global_seed_ = seed;
    rng_.seed(seed);
}
uint64_t BattleEngine::getRandomSeed() {
    return global_seed_;
}
GameResult BattleEngine::playGameInternal(
    AIStrategy& p1,
    AIStrategy& p2,
    PlayerColor first,
    const SearchLimits& l1,
    const SearchLimits& l2
) {
    GameResult result;
    auto start_time = std::chrono::steady_clock::now();
    Board board;
    PlayerColor current = first;
    // 记录着法
    std::vector<Move> moves;
    // 对局循环
    while (!board.isGameOver()) {
        AIStrategy& current_ai = (current == first) ? p1 : p2;
        const SearchLimits& current_limits = (current == first) ? l1 : l2;
        Move move = current_ai.findBestMove(board, current_limits);
        if (!move.isValid()) {
            move = Move::pass();
        }
        if (move.is_pass) {
            // 跳过回合
            board.makeMove(move);
            moves.push_back(move);
            current = (current == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
            // 检查双方是否都无棋可下
            if (board.getValidMoves().empty() && board.isGameOver()) {
                break;
            }
            continue;
        }
        if (!board.makeMove(move)) {
            // 移动无效，使用第一个有效移动
            auto valid = board.getValidMoves();
            if (!valid.empty()) {
                move = valid[0];
                board.makeMove(move);
            }
        }
        moves.push_back(move);
        current = (current == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black;
    }
    auto end_time = std::chrono::steady_clock::now();
    result.duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    result.moves_count = static_cast<int>(moves.size());
    result.game_moves = std::move(moves);
    // 计算最终分数
    result.black_score = board.getBitBoard().getScore(PlayerColor::Black);
    result.white_score = board.getBitBoard().getScore(PlayerColor::White);
    // 确定胜者
    if (result.black_score > result.white_score) {
        result.winner = PlayerColor::Black;
    } else if (result.white_score > result.black_score) {
        result.winner = PlayerColor::White;
    } else {
        result.winner = PlayerColor::Black;  // 平局
    }
    return result;
}
void BattleEngine::updateStats(BattleStats& stats, const GameResult& result,
                                bool player1_first) {
    if (result.isDraw()) {
        stats.draws++;
    } else {
        bool player1_won = (result.winner == (player1_first ? PlayerColor::Black : PlayerColor::White));
        if (player1_won) {
            stats.player1_wins++;
            if (player1_first) stats.player1_first_wins++;
            else stats.player2_first_wins++;
        } else {
            stats.player2_wins++;
            if (player1_first) stats.player2_first_wins++;
            else stats.player1_first_wins++;
        }
    }
    if (player1_first) stats.player1_first_games++;
    else stats.player2_first_games++;
}
std::vector<GameResult> BattleEngine::runParallelBattle(const BattleConfig& config) {
    std::vector<GameResult> results;
    results.reserve(config.num_games);
    std::vector<std::future<GameResult>> futures;
    int batch_size = std::min(config.num_games, config.max_threads);
    int current = 0;
    while (current < config.num_games) {
        int this_batch = std::min(batch_size, config.num_games - current);
        futures.clear();
        for (int i = 0; i < this_batch; ++i) {
            bool player1_first = ((current + i) % 2 == 0) == config.alternate_first;
            auto future = std::async(std::launch::async, [&, i, player1_first]() {
                auto ai1 = AIStrategyFactory::createByName(config.player1_name, Difficulty::MEDIUM);
                auto ai2 = AIStrategyFactory::createByName(config.player2_name, Difficulty::MEDIUM);
                auto result = playGameInternal(*ai1, *ai2,
                    player1_first ? PlayerColor::Black : PlayerColor::White,
                    config.limits1, config.limits2);
                result.game_number = current + i + 1;
                return result;
            });
            futures.push_back(std::move(future));
        }
        for (auto& future : futures) {
            results.push_back(future.get());
        }
        current += this_batch;
    }
    return results;
}
} // namespace Reversi

