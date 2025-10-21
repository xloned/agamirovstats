// MLE для нормального распределения (полные данные)
#include <iostream>
#include <vector>
#include <cmath>
#include <numeric>
#include "mle_methods.h"
#include "boost_distributions.h"
#include "matrix_operations.h"

// Реализация MLE для нормального распределения с полными данными
MLEResult mle_normal_complete(const std::vector<double>& data) {
    MLEResult result;
    int n = data.size();

    // Вычисление оценок максимального правдоподобия
    // Для нормального распределения с полными данными:
    // μ_MLE = (1/n) * Σx_i
    // σ²_MLE = (1/n) * Σ(x_i - μ)²

    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    double mean = sum / n;

    double variance_sum = 0.0;
    for (double x : data) {
        variance_sum += (x - mean) * (x - mean);
    }
    double variance = variance_sum / n;
    double std_dev = std::sqrt(variance);

    // Сохранение оценок параметров
    result.parameters.push_back(mean);
    result.parameters.push_back(std_dev);

    // Вычисление логарифма функции правдоподобия
    double log_likelihood = 0.0;
    for (double x : data) {
        double z = (x - mean) / std_dev;
        log_likelihood += std::log(norm_pdf(z) / std_dev);
    }
    result.log_likelihood = log_likelihood;

    // Вычисление ковариационной матрицы
    result.cov_size = 2;
    result.covariance = createMatrix(2, 2);

    // Для нормального распределения:
    // Var(μ) = σ²/n
    // Var(σ) = σ²/(2n)
    // Cov(μ, σ) = 0
    result.covariance[0][0] = variance / n;
    result.covariance[0][1] = 0.0;
    result.covariance[1][0] = 0.0;
    result.covariance[1][1] = variance / (2.0 * n);

    // Стандартные ошибки
    result.std_errors.push_back(std::sqrt(result.covariance[0][0]));
    result.std_errors.push_back(std::sqrt(result.covariance[1][1]));

    result.iterations = 0;  // Аналитическое решение
    result.converged = true;

    return result;
}
