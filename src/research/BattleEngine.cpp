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
// Static member initialization - use deterministic seed to avoid static initialization order issue
// ============================================================================
uint64_t BattleEngine::global_seed_ = 0;
// Use fixed seed 42 instead of random_device{}() to avoid static initialization order issue
// This ensures deterministic behavior in multi-threaded environment
std::mt19937_64 BattleEngine::rng_(42);

// ============================================================================
// BattleStats Implementation
// ============================================================================
void BattleStats::calculate() {
    if (total_games == 0) return;
    // Win rate
    win_rate1 = static_cast<double>(player1_wins) / total_games;
    win_rate2 = static_cast<double>(player2_wins) / total_games;
    // Average moves
    double total_moves = 0;
    double total_duration = 0;
    double total_score1 = 0;
    double total_score2 = 0;
    int max_m = 0;
    double total_margin = 0;
    for (const auto& game : games) {
        total_moves += game.moves_count;
        total_duration += game.duration_ms;
        total_score1 += game.black_score;
        total_score2 += game.white_score;
        total_margin += game.getMargin();
        max_m = std::max(max_m, game.getMargin());
    }
    avg_moves = total_moves / total_games;
    avg_duration_ms = total_duration / total_games;
    avg_score1 = total_score1 / total_games;
    avg_score2 = total_score2 / total_games;
    avg_margin = total_margin / total_games;  // Average margin per game, not difference of total averages
    max_margin = max_m;
    // Calculate first player win rate
    if (player1_first_games > 0) {
        // Win rate when player1 goes first is already included in player1_wins
    }
}
std::pair<double, double> BattleStats::getWinRateCI1() const {
    // Simple approximation: use normal distribution to approximate binomial distribution
    double p = win_rate1;
    double n = total_games;
    double se = std::sqrt(p * (1 - p) / n);
    double z = 1.96;  // 95% confidence
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
// BattleEngine Implementation
// ============================================================================
BattleStats BattleEngine::runBattle(const BattleConfig& config,
                                      BattleProgressCallback progress_callback) {
    BattleStats stats;
    stats.player1_name = config.player1_name;
    stats.player2_name = config.player2_name;
    // Set random seed
    if (config.random_seed != 0) {
        setRandomSeed(config.random_seed);
    }
    // Validate config
    if (!validateConfig(config)) {
        std::cerr << "[BattleEngine] Invalid config, aborting battle" << std::endl;
        return stats;
    }
    // Reset AI state
    config.player1->reset();
    config.player2->reset();
    // Select battle method based on config
    std::vector<SingleGameResult> results;
    if (config.parallel && config.num_games > 1) {
        results = runParallelBattle(config);
    } else {
        results.reserve(config.num_games);
        for (int i = 0; i < config.num_games; ++i) {
            bool player1_first = (i % 2 == 0) == config.alternate_first;
            SingleGameResult result = playGameInternal(
                *config.player1, *config.player2,
                player1_first ? PlayerColor::Black : PlayerColor::White,
                config.limits1, config.limits2
            );
            result.game_number = i + 1;
            updateStats(stats, result, player1_first);
            results.push_back(result);
            // Progress callback
            if (progress_callback) {
                progress_callback(i + 1, config.num_games, result);
            }
            // Reset AI state
            config.player1->reset();
            config.player2->reset();
        }
    }
    stats.games = std::move(results);
    stats.total_games = config.num_games;
    stats.calculate();
    // Calculate statistical significance (using binomial test)
    if (stats.total_games >= 10) {
        // Hypothesis: null hypothesis is 50% win rate
        int wins = stats.player1_wins;
        int n = stats.total_games;
        double p_win = 0.5;  // Null hypothesis: 50% win rate
        // Calculate one-sided p-value of binomial distribution
        double p_value = 0.0;
        for (int k = wins; k <= n; ++k) {
            // Simplified calculation: use normal approximation
            // Here uses simplified method
        }
        // Calculate p-value using normal approximation
        double expected = n * p_win;
        double std_dev = std::sqrt(n * p_win * (1 - p_win));
        if (std_dev > 0) {
            double z = (wins - expected) / std_dev;
            // Standard normal distribution CDF
            double cdf = 0.5 * (1.0 + std::erf(z / std::sqrt(2.0)));
            if (z > 0) {
                p_value = 2.0 * (1.0 - cdf);  // Two-sided test
            } else {
                p_value = 2.0 * cdf;  // Two-sided test
            }
        }
        stats.p_value = std::min(p_value, 1.0);
        stats.significant = stats.p_value < 0.05;
    }
    return stats;
}
SingleGameResult BattleEngine::playSingleGame(
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
SingleGameResult BattleEngine::playGameInternal(
    AIStrategy& p1,
    AIStrategy& p2,
    PlayerColor first,
    const SearchLimits& l1,
    const SearchLimits& l2
) {
    SingleGameResult result;
    auto start_time = std::chrono::steady_clock::now();
    Board board;
    // Set AI color, let them know their role
    // p1 plays the side specified by first, p2 plays the other side
    p1.setColor(first);
    p2.setColor((first == PlayerColor::Black) ? PlayerColor::White : PlayerColor::Black);
    PlayerColor current = board.getCurrentTurn();
    // Record moves
    std::vector<Move> moves;
    // Game loop
    while (!board.isGameOver()) {
        // Determine which AI to use based on current player on board
        // p1 plays the color specified by first, p2 plays the other color
        AIStrategy& current_ai = (current == first) ? p1 : p2;
        const SearchLimits& current_limits = (current == first) ? l1 : l2;
        Move move = current_ai.findBestMove(board, current_limits);
        if (!move.isValid()) {
            move = Move::pass();
        }
        if (move.is_pass) {
            // Skip turn
            board.makeMove(move);
            moves.push_back(move);
            current = board.getCurrentTurn();
            // Check if both players have no moves
            if (board.getValidMoves().empty() && board.isGameOver()) {
                break;
            }
            continue;
        }
        if (!board.makeMove(move)) {
            // Move invalid, use first valid move
            auto valid = board.getValidMoves();
            if (!valid.empty()) {
                move = valid[0];
                board.makeMove(move);
            }
        }
        moves.push_back(move);
        current = board.getCurrentTurn();
    }
    auto end_time = std::chrono::steady_clock::now();
    result.duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    result.moves_count = static_cast<int>(moves.size());
    result.game_moves = std::move(moves);
    // Calculate final score
    result.black_score = board.getBitBoard().getScore(PlayerColor::Black);
    result.white_score = board.getBitBoard().getScore(PlayerColor::White);
    // Determine winner
    if (result.black_score > result.white_score) {
        result.winner = PlayerColor::Black;
    } else if (result.white_score > result.black_score) {
        result.winner = PlayerColor::White;
    } else {
        result.winner = PlayerColor::Black;  // Default to black on draw (does not affect statistics, isDraw handles it)
    }
    return result;
}
void BattleEngine::updateStats(BattleStats& stats, const SingleGameResult& result,
                                bool player1_first) {
    // Statistics for first player games
    if (player1_first) {
        stats.player1_first_games++;
    } else {
        stats.player2_first_games++;
    }

    if (result.isDraw()) {
        stats.draws++;
        return;
    }

    // winner is Black or White (actual winner color)
    // player1 (MinimaxAI) is Black when player1_first=true, otherwise White
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
std::vector<SingleGameResult> BattleEngine::runParallelBattle(const BattleConfig& config) {
    std::vector<SingleGameResult> results;
    results.reserve(config.num_games);
    std::vector<std::future<SingleGameResult>> futures;
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

// Position Suite Integration Implementation

namespace {
    // Helper: Convert BitBoard to 8x8 vector representation
    std::vector<std::vector<int>> bitboardToVector(const BitBoard& bb, PlayerColor currentPlayer) {
        std::vector<std::vector<int>> state(8, std::vector<int>(8, 0));
        uint64_t player = bb.getPlayerBits();
        uint64_t opponent = bb.getOpponentBits();

        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                int bit = row * 8 + col;
                if (player & (1ULL << bit)) {
                    state[row][col] = (currentPlayer == PlayerColor::Black) ? 1 : 2;
                } else if (opponent & (1ULL << bit)) {
                    state[row][col] = (currentPlayer == PlayerColor::Black) ? 2 : 1;
                }
            }
        }
        return state;
    }
}

SingleGameResult BattleEngine::playFromPosition(
    const TestPosition& position,
    AIStrategy& player1,
    AIStrategy& player2,
    const SearchLimits& limits1,
    const SearchLimits& limits2
) {
    // Start with the given position
    Board board;
    auto state = bitboardToVector(position.board, position.player);
    int moveCount = position.board.getPlayerBits() + position.board.getOpponentBits() - 4;  // Initial 4 pieces
    board.syncFrom(state, position.player, moveCount);
    PlayerColor current = position.player;

    std::vector<Move> moves;
    auto start_time = std::chrono::steady_clock::now();
    SingleGameResult result;

    // Play from this position until game over
    while (!board.isGameOver()) {
        if (board.getValidMoves().empty()) {
            // Pass
            board.makeMove(Move::pass());
            moves.push_back(Move::pass());
            current = board.getCurrentTurn();
            // Check if both players can't move
            if (board.getValidMoves().empty()) {
                break;
            }
            continue;
        }

        // Get current AI and limits
        AIStrategy& current_ai = (current == PlayerColor::Black) ? player1 : player2;
        const SearchLimits& current_limits = (current == PlayerColor::Black) ? limits1 : limits2;

        Move move = current_ai.findBestMove(board, current_limits);
        if (!move.isValid()) {
            // Use first valid move if AI returns invalid
            auto valid = board.getValidMoves();
            if (!valid.empty()) {
                move = valid[0];
            } else {
                move = Move::pass();
            }
        }

        if (move.is_pass) {
            board.makeMove(move);
        } else {
            if (!board.makeMove(move)) {
                // Try first valid move if move fails
                auto valid = board.getValidMoves();
                if (!valid.empty()) {
                    move = valid[0];
                    board.makeMove(move);
                }
            }
        }

        moves.push_back(move);
        current = board.getCurrentTurn();
    }

    auto end_time = std::chrono::steady_clock::now();
    result.duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    result.moves_count = static_cast<int>(moves.size());
    result.game_moves = std::move(moves);
    result.black_score = board.getBitBoard().getScore(PlayerColor::Black);
    result.white_score = board.getBitBoard().getScore(PlayerColor::White);

    if (result.black_score > result.white_score) {
        result.winner = PlayerColor::Black;
    } else if (result.white_score > result.black_score) {
        result.winner = PlayerColor::White;
    } else {
        result.winner = PlayerColor::Black;  // Draw
    }

    return result;
}

BattleStats BattleEngine::runSuiteBattle(
    const std::vector<TestPosition>& positions,
    const BattleConfig& config,
    BattleProgressCallback progress_callback
) {
    BattleStats stats;
    stats.player1_name = config.player1_name;
    stats.player2_name = config.player2_name;

    // Set random seed
    if (config.random_seed != 0) {
        setRandomSeed(config.random_seed);
    }

    // Validate config
    if (!validateConfig(config)) {
        std::cerr << "[BattleEngine] Invalid config, aborting suite battle" << std::endl;
        return stats;
    }

    // Reset AI state
    config.player1->reset();
    config.player2->reset();

    int total_positions = static_cast<int>(positions.size());
    int games_per_position = config.num_games / total_positions;
    if (games_per_position < 1) games_per_position = 1;

    // Run battle on each position
    for (int pos_idx = 0; pos_idx < total_positions; ++pos_idx) {
        const auto& position = positions[pos_idx];

        for (int game = 0; game < games_per_position; ++game) {
            bool player1_first = (game % 2 == 0) == config.alternate_first;

            SingleGameResult result = playFromPosition(
                position,
                *config.player1,
                *config.player2,
                config.limits1,
                config.limits2
            );

            result.game_number = pos_idx * games_per_position + game + 1;
            updateStats(stats, result, player1_first);
            stats.games.push_back(result);

            // Progress callback
            if (progress_callback) {
                progress_callback(result.game_number, config.num_games, result);
            }

            // Reset AI state
            config.player1->reset();
            config.player2->reset();
        }
    }

    stats.total_games = config.num_games;
    stats.calculate();

    return stats;
}

std::vector<TestPosition> BattleEngine::getSuiteByType(int type) {
    switch (type) {
        case 1: return PositionSuite::getOpening();
        case 2: return PositionSuite::getMidgame();
        case 3: return PositionSuite::getEndgame();
        case 0:
        default: return PositionSuite::getStandard64();
    }
}

} // namespace Reversi

