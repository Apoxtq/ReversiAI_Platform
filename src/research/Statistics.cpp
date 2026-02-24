#include "research/Statistics.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>
#include <iostream>

namespace Reversi {

std::pair<double, double> Statistics::wilcoxonSignedRankTest(
    const std::vector<int>& sample1,
    const std::vector<int>& sample2)
{
    if (sample1.size() != sample2.size() || sample1.empty()) {
        return std::make_pair(0.0, 1.0);
    }

    std::vector<double> diffs;
    std::vector<double> nonzero_diffs;

    // Calculate differences
    for (size_t i = 0; i < sample1.size(); ++i) {
        double d = static_cast<double>(sample1[i] - sample2[i]);
        diffs.push_back(d);
        if (std::abs(d) > 1e-9) {
            nonzero_diffs.push_back(d);
        }
    }

    if (nonzero_diffs.empty()) {
        return std::make_pair(0.0, 1.0);
    }

    // Create ranked list of absolute differences
    std::vector<std::pair<double, double> > ranked;
    for (size_t i = 0; i < nonzero_diffs.size(); ++i) {
        ranked.push_back(std::make_pair(std::abs(nonzero_diffs[i]), 0.0));
    }

    // Sort by absolute difference
    std::sort(ranked.begin(), ranked.end(),
        [](const std::pair<double, double>& a, const std::pair<double, double>& b) {
                  return a.first < b.first;
              });

    // Assign ranks (handle ties)
    size_t pos = 0;
    while (pos < ranked.size()) {
        size_t next_pos = pos;
        while (next_pos < ranked.size() && ranked[next_pos].first == ranked[pos].first) {
            next_pos++;
        }
        double rank_val = (pos + 1 + next_pos) / 2.0;
        for (size_t k = pos; k < next_pos; ++k) {
            ranked[k].second = rank_val;
        }
        pos = next_pos;
    }

    // Calculate W statistic
    double W_plus = 0.0;
    double W_minus = 0.0;
    for (size_t i = 0; i < nonzero_diffs.size(); ++i) {
        if (nonzero_diffs[i] > 0) {
            W_plus += ranked[i].second;
        } else {
            W_minus += ranked[i].second;
        }
    }

    double W = std::min(W_plus, W_minus);
    int n = static_cast<int>(nonzero_diffs.size());

    // Expected value and variance
    double expected = n * (n + 1) / 4.0;
    double variance = n * (n + 1) * (2 * n + 1) / 24.0;

    // Calculate p-value
    double p = 1.0;
    if (n > 10) {
        double z = (W - expected) / std::sqrt(variance);
        p = 2.0 * (1.0 - normalCDF(std::abs(z)));
    } else {
        p = binomialTest(static_cast<int>(W_minus), n * (n + 1) / 2, 0.5);
    }

    return std::make_pair(W, p);
}

double Statistics::binomialTest(int wins, int total, double expected_rate)
{
    if (total <= 0) return 1.0;

    double p_value = 0.0;
    for (int k = wins; k <= total; ++k) {
        p_value += binomialProbability(k, total, expected_rate);
    }

    double p_both = std::min(2.0 * p_value, 1.0);
    return p_both;
}

double Statistics::winRateWithCI(int wins, int total, double& ci_low, double& ci_high)
{
    if (total <= 0) {
        ci_low = ci_high = 0.0;
        return 0.0;
    }

    double rate = static_cast<double>(wins) / total;
    std::pair<double, double> ci = wilsonCI(wins, total, 0.95);
    ci_low = ci.first;
    ci_high = ci.second;

    return rate;
}

std::pair<double, double> Statistics::wilsonCI(int wins, int total, double confidence)
{
    if (total <= 0) return std::make_pair(0.0, 0.0);

    double p = static_cast<double>(wins) / total;
    double z = 1.96;
    if (confidence < 0.92) z = 1.645;
    else if (confidence > 0.97) z = 2.576;

    double n = total;
    double denom = 1.0 + z * z / n;
    double center = (p + z * z / (2.0 * n)) / denom;
    double margin = z * std::sqrt((p * (1.0 - p) + z * z / (4.0 * n)) / n) / denom;

    return std::make_pair(std::max(0.0, center - margin), std::min(1.0, center + margin));
}

bool Statistics::isSignificantlyBetter(double win_rate, int total, double threshold)
{
    if (total <= 0) return false;
    int expected = static_cast<int>(threshold * total);
    int observed = static_cast<int>(win_rate * total);
    double p = binomialTest(observed, total, threshold);
    return p < 0.05 && win_rate > threshold;
}

double Statistics::normalCDF(double x)
{
    double a1 = 0.254829592;
    double a2 = -0.284496736;
    double a3 = 1.421413741;
    double a4 = -1.453152027;
    double a5 = 1.061405429;
    double p = 0.3275911;

    double sign = (x < 0.0) ? -1.0 : 1.0;
    x = std::abs(x) / std::sqrt(2.0);
    double t = 1.0 / (1.0 + p * x);
    double y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * std::exp(-x * x);

    return 0.5 * (1.0 + sign * y);
}

double Statistics::tCDF(double x, int df)
{
    if (df > 30) {
        return normalCDF(x);
    }
    return normalCDF(x);
}

double Statistics::normalRandom(double mean, double std_dev, uint64_t seed)
{
    static std::mt19937_64 rng(seed != 0 ? seed : std::random_device{}());
    std::normal_distribution<double> dist(mean, std_dev);
    return dist(rng);
}

int Statistics::requiredSampleSize(double std_dev, double margin, double confidence)
{
    double z = 1.96;
    if (confidence < 0.92) z = 1.645;
    else if (confidence > 0.97) z = 2.576;

    return static_cast<int>(std::pow(z * std_dev / margin, 2.0)) + 1;
}

double Statistics::factorial(int n)
{
    if (n <= 1) return 1.0;
    double result = 1.0;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

double Statistics::binomialProbability(int k, int n, double p)
{
    if (k < 0 || k > n) return 0.0;

    double log_n_fact = 0.0;
    for (int i = 1; i <= n; ++i) {
        log_n_fact += std::log(static_cast<double>(i));
    }

    double log_k_fact = 0.0;
    for (int i = 1; i <= k; ++i) {
        log_k_fact += std::log(static_cast<double>(i));
    }

    double log_nk_fact = 0.0;
    for (int i = 1; i <= n - k; ++i) {
        log_nk_fact += std::log(static_cast<double>(i));
    }

    double log_prob = log_n_fact - log_k_fact - log_nk_fact
        + k * std::log(p) + (n - k) * std::log(1.0 - p);

    return std::exp(log_prob);
}

} // namespace Reversi
