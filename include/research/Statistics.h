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
 * @brief 统计工具类
 *
 * 实现:
 * - 均值和标准差
 * - 95%置信区间
 * - Wilcoxon signed-rank测试
 * - 胜率比较
 * - 二项分布检验
 *
 * @author AI Assistant
 * @date 2026
 * @version 1.0
 */

namespace Reversi {

// 前向声明
struct BattleStats;

/**
 * @brief 统计工具类
 *
 * 提供各种统计分析功能，用于AI算法评估和比较
 */
class Statistics {
public:
    // =========================================================================
    // 基础统计
    // =========================================================================

    /**
     * @brief 计算均值
     *
     * @tparam T 数值类型
     * @param data 数据向量
     * @return 均值
     */
    template<typename T>
    static double mean(const std::vector<T>& data);

    /**
     * @brief 计算中位数
     *
     * @tparam T 数值类型
     * @param data 数据向量
     * @return 中位数
     */
    template<typename T>
    static double median(std::vector<T> data);

    /**
     * @brief 计算标准差
     *
     * @tparam T 数值类型
     * @param data 数据向量
     * @param sample true=样本标准差, false=总体标准差
     * @return 标准差
     */
    template<typename T>
    static double stddev(const std::vector<T>& data, bool sample = true);

    /**
     * @brief 计算方差
     *
     * @tparam T 数值类型
     * @param data 数据向量
     * @return 方差
     */
    template<typename T>
    static double variance(const std::vector<T>& data);

    /**
     * @brief 计算95%置信区间
     *
     * @tparam T 数值类型
     * @param data 数据向量
     * @return pair(下界, 上界)
     */
    template<typename T>
    static std::pair<double, double> confidenceInterval95(const std::vector<T>& data);

    /**
     * @brief 计算任意置信区间
     *
     * @tparam T 数值类型
     * @param data 数据向量
     * @param confidence 置信度 (0.9, 0.95, 0.99等)
     * @return pair(下界, 上界)
     */
    template<typename T>
    static std::pair<double, double> confidenceInterval(const std::vector<T>& data, double confidence);

    // =========================================================================
    // 假设检验
    // =========================================================================

    /**
     * @brief Wilcoxon signed-rank测试
     *
     * 用于配对样本的统计显著性检验
     * 适用于对战结果的胜/负序列分析
     *
     * @param sample1 样本1 (胜/负序列: +1/-1)
     * @param sample2 样本2
     * @return pair(统计量W, p值)
     */
    static std::pair<double, double> wilcoxonSignedRankTest(
        const std::vector<int>& sample1,
        const std::vector<int>& sample2
    );

    /**
     * @brief 二项分布检验
     *
     * 检验胜率是否显著偏离期望值
     *
     * @param wins 胜场数
     * @param total 总场数
     * @param expected_rate 期望胜率 (default: 0.5)
     * @return p值
     */
    static double binomialTest(int wins, int total, double expected_rate = 0.5);

    /**
     * @brief 单样本t检验
     *
     * 检验样本均值是否显著不同于假设均值
     *
     * @param data 数据向量
     * @param hypothesized_mean 假设均值
     * @return pair(t统计量, p值)
     */
    template<typename T>
    static std::pair<double, double> oneSampleTTest(
        const std::vector<T>& data,
        double hypothesized_mean
    );

    /**
     * @brief 独立样本t检验
     *
     * 检验两个独立样本的均值是否有显著差异
     *
     * @param sample1 样本1
     * @param sample2 样本2
     * @return pair(t统计量, p值)
     */
    template<typename T>
    static std::pair<double, double> independentSamplesTTest(
        const std::vector<T>& sample1,
        const std::vector<T>& sample2
    );

    // =========================================================================
    // 胜率分析
    // =========================================================================

    /**
     * @brief 计算胜率及其置信区间
     *
     * @param wins 胜场数
     * @param total 总场数
     * @param ci_low 输出: 置信区间下界
     * @param ci_high 输出: 置信区间上界
     * @return 胜率
     */
    static double winRateWithCI(int wins, int total, double& ci_low, double& ci_high);

    /**
     * @brief 计算Wilson置信区间 (胜率)
     *
     * 比正态近似更准确的置信区间方法
     *
     * @param wins 胜场数
     * @param total 总场数
     * @param confidence 置信度
     * @return pair(下界, 上界)
     */
    static std::pair<double, double> wilsonCI(int wins, int total, double confidence = 0.95);

    /**
     * @brief 检验胜率是否显著
     *
     * @param win_rate 胜率
     * @param total 总场数
     * @param threshold 阈值 (default: 0.5)
     * @return true 如果显著高于阈值
     */
    static bool isSignificantlyBetter(double win_rate, int total, double threshold = 0.5);

    // =========================================================================
    // 效果量 (Effect Size)
    // =========================================================================

    /**
     * @brief 计算Cohen's d
     *
     * 衡量两个样本之间的差异大小
     *
     * @param sample1 样本1
     * @param sample2 样本2
     * @return Cohen's d值
     */
    template<typename T>
    static double cohensD(const std::vector<T>& sample1, const std::vector<T>& sample2);

    /**
     * @brief 计算 Cliff's delta
     *
     * 非参数效果量
     *
     * @param sample1 样本1
     * @param sample2 样本2
     * @return Cliff's delta值
     */
    template<typename T>
    static double cliffsDelta(const std::vector<T>& sample1, const std::vector<T>& sample2);

    // =========================================================================
    // 实用工具
    // =========================================================================

    /**
     * @brief 计算样本量
     *
     * 达到指定置信区间宽度所需的样本量
     *
     * @param std_dev 估计的标准差
     * @param margin 期望的边际误差
     * @param confidence 置信度
     * @return 所需样本量
     */
    static int requiredSampleSize(double std_dev, double margin, double confidence = 0.95);

    /**
     * @brief 正态分布CDF
     *
     * @param x 值
     * @return CDF值
     */
    static double normalCDF(double x);

    /**
     * @brief t分布CDF近似
     *
     * @param x 值
     * @param df 自由度
     * @return CDF值
     */
    static double tCDF(double x, int df);

    /**
     * @brief 生成正态分布随机数
     *
     * @param mean 均值
     * @param std_dev 标准差
     * @param seed 随机种子
     * @return 随机数
     */
    static double normalRandom(double mean, double std_dev, uint64_t seed = 0);

    /**
     * @brief 格式化统计摘要
     *
     * @param data 数据向量
     * @return 格式化的统计摘要字符串
     */
    template<typename T>
    static std::string summary(const std::vector<T>& data);

private:
    /**
     * @brief 计算阶乘
     */
    static double factorial(int n);

    /**
     * @brief 计算二项分布概率
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

    // z值近似 (可以使用更精确的值)
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

    // Welch-Satterthwaite 自由度
    double num = std::pow(v1/n1 + v2/n2, 2);
    double denom = std::pow(v1/n1, 2) / (n1 - 1) + std::pow(v2/n2, 2) / (n2 - 1);
    int df = static_cast<int>(num / denom);

    double p = 2.0 * (1.0 - tCDF(std::abs(t), df));

    return {t, p};
}

template<typename T>
double Statistics::cohensD(const std::vector<T>& sample1, const std::vector<T>& sample2) {
    if (sample1.empty() || sample2.empty()) return 0.0;

    double m1 = mean(sample1);
    double m2 = mean(sample2);
    double s1 = variance(sample1);
    double s2 = variance(sample2);

    int n1 = static_cast<int>(sample1.size());
    int n2 = static_cast<int>(sample2.size());

    // 合并标准差
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

