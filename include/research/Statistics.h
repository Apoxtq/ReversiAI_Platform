#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <random>

/**
 * @file Statistics.h
 * @brief Statistics Utility Class
 *
 * Implements:
 * - Mean and standard deviation
 * - 95% confidence interval
 * - Wilcoxon signed-rank test
 * - Win rate comparison
 * - Binomial test
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

// 前向声明
struct BattleStats;

/**
 * @brief Statistics utility class
 *
 * Provides various statistical analysis functions for AI algorithm evaluation and comparison
 */
class Statistics {
public:
    // =========================================================================
    // Basic Statistics
    // =========================================================================

    /**
     * @brief Calculate mean
     *
     * @tparam T Numeric type
     * @param data Data vector
     * @return Mean
     */
    template<typename T>
    static double mean(const std::vector<T>& data);

    /**
     * @brief Calculate median
     *
     * @tparam T Numeric type
     * @param data Data vector
     * @return Median
     */
    template<typename T>
    static double median(std::vector<T> data);

    /**
     * @brief Calculate standard deviation
     *
     * @tparam T Numeric type
     * @param data Data vector
     * @param sample true=sample std dev, false=population std dev
     * @return Standard deviation
     */
    template<typename T>
    static double stddev(const std::vector<T>& data, bool sample = true);

    /**
     * @brief Calculate variance
     *
     * @tparam T Numeric type
     * @param data Data vector
     * @return Variance
     */
    template<typename T>
    static double variance(const std::vector<T>& data);

    /**
     * @brief Calculate 95% confidence interval
     *
     * @tparam T Numeric type
     * @param data Data vector
     * @return pair(lower, upper)
     */
    template<typename T>
    static std::pair<double, double> confidenceInterval95(const std::vector<T>& data);

    /**
     * @brief Calculate arbitrary confidence interval
     *
     * @tparam T Numeric type
     * @param data Data vector
     * @param confidence Confidence level (0.9, 0.95, 0.99, etc.)
     * @return pair(lower, upper)
     */
    template<typename T>
    static std::pair<double, double> confidenceInterval(const std::vector<T>& data, double confidence);

    // =========================================================================
    // Hypothesis Testing
    // =========================================================================

    /**
     * @brief Wilcoxon signed-rank test
     *
     * Used for paired sample statistical significance test
     * Applicable to win/loss sequence analysis of battle results
     *
     * @param sample1 Sample 1 (win/loss sequence: +1/-1)
     * @param sample2 Sample 2
     * @return pair(statistic W, p value)
     */
    static std::pair<double, double> wilcoxonSignedRankTest(
        const std::vector<int>& sample1,
        const std::vector<int>& sample2
    );

    /**
     * @brief Binomial test
     *
     * Test whether win rate significantly deviates from expected value
     *
     * @param wins Number of wins
     * @param total Total games
     * @param expected_rate Expected win rate (default: 0.5)
     * @return p value
     */
    static double binomialTest(int wins, int total, double expected_rate = 0.5);

    /**
     * @brief One-sample t-test
     *
     * Test whether sample mean significantly differs from hypothesized mean
     *
     * @param data Data vector
     * @param hypothesized_mean Hypothesized mean
     * @return pair(t statistic, p value)
     */
    template<typename T>
    static std::pair<double, double> oneSampleTTest(
        const std::vector<T>& data,
        double hypothesized_mean
    );

    /**
     * @brief Independent samples t-test
     *
     * Test whether means of two independent samples have significant difference
     *
     * @param sample1 Sample 1
     * @param sample2 Sample 2
     * @return pair(t statistic, p value)
     */
    template<typename T>
    static std::pair<double, double> independentSamplesTTest(
        const std::vector<T>& sample1,
        const std::vector<T>& sample2
    );

    // =========================================================================
    // Win Rate Analysis
    // =========================================================================

    /**
     * @brief Calculate win rate and its confidence interval
     *
     * @param wins Number of wins
     * @param total Total games
     * @param ci_low Output: confidence interval lower bound
     * @param ci_high Output: confidence interval upper bound
     * @return Win rate
     */
    static double winRateWithCI(int wins, int total, double& ci_low, double& ci_high);

    /**
     * @brief Calculate Wilson confidence interval (win rate)
     *
     * More accurate confidence interval method than normal approximation
     *
     * @param wins Number of wins
     * @param total Total games
     * @param confidence Confidence level
     * @return pair(lower, upper)
     */
    static std::pair<double, double> wilsonCI(int wins, int total, double confidence = 0.95);

    /**
     * @brief Test whether win rate is significant
     *
     * @param win_rate Win rate
     * @param total Total games
     * @param threshold Threshold (default: 0.5)
     * @return true if significantly higher than threshold
     */
    static bool isSignificantlyBetter(double win_rate, int total, double threshold = 0.5);

    // =========================================================================
    // Effect Size
    // =========================================================================

    /**
     * @brief Calculate Cohen's d
     *
     * Measure the magnitude of difference between two samples
     *
     * @param sample1 Sample 1
     * @param sample2 Sample 2
     * @return Cohen's d value
     */
    template<typename T>
    static double cohensD(const std::vector<T>& sample1, const std::vector<T>& sample2);

    /**
     * @brief Calculate Cliff's delta
     *
     * Non-parametric effect size
     *
     * @param sample1 Sample 1
     * @param sample2 Sample 2
     * @return Cliff's delta value
     */
    template<typename T>
    static double cliffsDelta(const std::vector<T>& sample1, const std::vector<T>& sample2);

    // =========================================================================
    // Utility Functions
    // =========================================================================

    /**
     * @brief Calculate sample size
     *
     * Sample size required to achieve specified confidence interval width
     *
     * @param std_dev Estimated standard deviation
     * @param margin Desired margin of error
     * @param confidence Confidence level
     * @return Required sample size
     */
    static int requiredSampleSize(double std_dev, double margin, double confidence = 0.95);

    /**
     * @brief Normal distribution CDF
     *
     * @param x Value
     * @return CDF value
     */
    static double normalCDF(double x);

    /**
     * @brief t-distribution CDF approximation
     *
     * @param x Value
     * @param df Degrees of freedom
     * @return CDF value
     */
    static double tCDF(double x, int df);

    /**
     * @brief Generate normally distributed random number
     *
     * @param mean Mean
     * @param std_dev Standard deviation
     * @param seed Random seed
     * @return Random number
     */
    static double normalRandom(double mean, double std_dev, uint64_t seed = 0);

    /**
     * @brief Format statistical summary
     *
     * @param data Data vector
     * @return Formatted statistical summary string
     */
    template<typename T>
    static std::string summary(const std::vector<T>& data);

private:
    /**
     * @brief Calculate factorial
     */
    static double factorial(int n);

    /**
     * @brief Calculate binomial probability
     */
    static double binomialProbability(int k, int n, double p);
};

// ============================================================================
// 模板实现
// ============================================================================

template<typename T>
double Statistics::mean(const std::vector<T>& data) {
    if (data.empty()) return 0.0;
    return static_cast<double>(
        std::accumulate(data.begin(), data.end(), 0LL)
    ) / data.size();
}

template<typename T>
double Statistics::median(std::vector<T> data) {
    if (data.empty()) return 0.0;

    std::sort(data.begin(), data.end());
    size_t n = data.size();

    if (n % 2 == 0) {
        return static_cast<double>(data[n/2 - 1] + data[n/2]) / 2.0;
    }
    return static_cast<double>(data[n/2]);
}

template<typename T>
double Statistics::stddev(const std::vector<T>& data, bool sample) {
    if (data.size() < 2) return 0.0;

    double m = mean(data);
    double sq_sum = 0.0;
    for (const auto& val : data) {
        sq_sum += (static_cast<double>(val) - m) * (static_cast<double>(val) - m);
    }

    size_t n = data.size();
    return std::sqrt(sq_sum / (sample ? n - 1 : n));
}

template<typename T>
double Statistics::variance(const std::vector<T>& data) {
    double s = stddev(data, true);
    return s * s;
}

template<typename T>
std::pair<double, double> Statistics::confidenceInterval95(const std::vector<T>& data) {
    return confidenceInterval(data, 0.95);
}

template<typename T>
std::pair<double, double> Statistics::confidenceInterval(const std::vector<T>& data, double confidence) {
    if (data.empty()) return {0.0, 0.0};

    double m = mean(data);
    double se = stddev(data) / std::sqrt(data.size());

    // z-value approximation (can use more precise values)
    static const double z_values[] = {
        1.645,  // 90%
        1.960,  // 95%
        2.576,  // 99%
    };

    double z = 1.96;  // default 95%
    if (confidence < 0.92) z = 1.645;
    else if (confidence > 0.97) z = 2.576;

    return {m - z * se, m + z * se};
}

template<typename T>
std::pair<double, double> Statistics::oneSampleTTest(
    const std::vector<T>& data,
    double hypothesized_mean
) {
    if (data.size() < 2) return {0.0, 1.0};

    double m = mean(data);
    double se = stddev(data) / std::sqrt(data.size());

    if (se == 0) return {0.0, 1.0};

    double t = (m - hypothesized_mean) / se;
    int df = static_cast<int>(data.size()) - 1;

    // 计算p值 (双尾)
    double p = 2.0 * (1.0 - tCDF(std::abs(t), df));

    return {t, p};
}

template<typename T>
std::pair<double, double> Statistics::independentSamplesTTest(
    const std::vector<T>& sample1,
    const std::vector<T>& sample2
) {
    if (sample1.size() < 2 || sample2.size() < 2) return {0.0, 1.0};

    double m1 = mean(sample1);
    double m2 = mean(sample2);
    double v1 = variance(sample1);
    double v2 = variance(sample2);
    int n1 = static_cast<int>(sample1.size());
    int n2 = static_cast<int>(sample2.size());

    // Welch's t-test
    double se = std::sqrt(v1/n1 + v2/n2);
    if (se == 0) return {0.0, 1.0};

    double t = (m1 - m2) / se;

    // Welch-Satterthwaite degrees of freedom
    double num = std::pow(v1/n1 + v2/n2, 2);
    double denom = std::pow(v1/n1, 2) / (n1 - 1) + std::pow(v2/n2, 2) / (n2 - 1);
    int df = static_cast<int>(num / denom);

    double p = 2.0 * (1.0 - tCDF(std::abs(t), df));  // Calculate p-value (two-tailed)
    if (sample1.empty() || sample2.empty()) return 0.0;

    double m1 = mean(sample1);
    double m2 = mean(sample2);
    double s1 = variance(sample1);
    double s2 = variance(sample2);

    int n1 = static_cast<int>(sample1.size());
    int n2 = static_cast<int>(sample2.size());

    // Pooled standard deviation
    double pooled_std = std::sqrt(((n1 - 1) * s1 + (n2 - 1) * s2) / (n1 + n2 - 2));

    if (pooled_std == 0) return 0.0;

    return (m1 - m2) / pooled_std;
}

template<typename T>
double Statistics::cliffsDelta(const std::vector<T>& sample1, const std::vector<T>& sample2) {
    if (sample1.empty() || sample2.empty()) return 0.0;

    int more = 0, less = 0, equal = 0;

    for (const auto& x : sample1) {
        for (const auto& y : sample2) {
            if (x > y) more++;
            else if (x < y) less++;
            else equal++;
        }
    }

    int total = more + less + equal;
    if (total == 0) return 0.0;

    return static_cast<double>(more - less) / total;
}

template<typename T>
std::string Statistics::summary(const std::vector<T>& data) {
    if (data.empty()) return "Empty data";

    std::ostringstream oss;
    oss << "n=" << data.size() << ", ";
    oss << "mean=" << std::fixed << std::setprecision(2) << mean(data) << ", ";
    oss << "median=" << median(data) << ", ";
    oss << "std=" << stddev(data) << ", ";
    auto ci = confidenceInterval95(data);
    oss << "95%CI[" << ci.first << ", " << ci.second << "]";

    return oss.str();
}

} // namespace Reversi

