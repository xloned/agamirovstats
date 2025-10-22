// MLS для распределения Вейбулла (цензурированные данные)
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "mle_methods.h"
#include "nelder_mead.h"
#include "boost_distributions.h"
#include "matrix_operations.h"

// Реализация MLS для распределения Вейбулла с цензурированными данными
MLEResult mls_weibull_censored(const std::vector<double>& data, const std::vector<int>& censored) {
    MLEResult result;
    int n = data.size();

    // Установка глобальных данных для оптимизации
    nesm.n = n;
    nesm.x = data;
    nesm.r = censored;
    nesm.nsample.clear();

    // Начальные оценки (используем только полные наблюдения)
    double sum = 0.0;
    int n_complete = 0;
    for (int i = 0; i < n; i++) {
        if (censored[i] == 0) {
            sum += data[i];
            n_complete++;
        }
    }

    double mean_init = sum / n_complete;
    double variance_init = 0.0;
    for (int i = 0; i < n; i++) {
        if (censored[i] == 0) {
            variance_init += (data[i] - mean_init) * (data[i] - mean_init);
        }
    }
    variance_init /= n_complete;

    // Начальная оценка параметра формы из коэффициента вариации
    double cv = std::sqrt(variance_init) / mean_init;
    double c_init = std::max(1.0, 1.0 / cv);

    // Оптимизация методом Nelder-Mead
    std::vector<double> x0 = {c_init};
    double eps = 1e-6;
    std::vector<double> optimal = neldermead(x0, eps, WeibullMinFunction);

    double k = optimal[0];  // параметр формы

    // Вычисление параметра масштаба λ
    // λ = (1/n * Σ x_i^k)^(1/k) для всех данных (включая цензурированные)
    double sum_xk = 0.0;
    for (int i = 0; i < n; i++) {
        sum_xk += std::pow(data[i], k);
    }
    double lambda = std::pow(sum_xk / n, 1.0 / k);

    // Сохранение параметров
    result.parameters.push_back(lambda);  // lambda (масштаб)
    result.parameters.push_back(k);       // k (форма)

    // Вычисление логарифма функции правдоподобия
    // log L = Σ[r_i=0: log(k/λ) + (k-1)*log(x_i) - (x_i/λ)^k] + Σ[r_i=1: -(x_i/λ)^k]
    double log_likelihood = 0.0;
    for (int i = 0; i < n; i++) {
        if (censored[i] == 0) {
            // Полное наблюдение
            log_likelihood += std::log(k / lambda) + (k - 1) * std::log(data[i]) - std::pow(data[i] / lambda, k);
        } else {
            // Цензурированное наблюдение (логарифм вероятности выживания)
            log_likelihood += -std::pow(data[i] / lambda, k);
        }
    }
    result.log_likelihood = log_likelihood;

    // Вычисление ковариационной матрицы
    result.cov_size = 2;
    result.covariance = createMatrix(2, 2);

    // Приближенная ковариационная матрица для цензурированных данных
    double var_lambda = (lambda * lambda) / (n_complete * k * k);
    double var_k = 1.644 * (k * k) / n_complete;
    double cov_lambda_k = 0.0;

    result.covariance[0][0] = var_lambda;
    result.covariance[0][1] = cov_lambda_k;
    result.covariance[1][0] = cov_lambda_k;
    result.covariance[1][1] = var_k;

    // Стандартные ошибки
    result.std_errors.push_back(std::sqrt(var_lambda));
    result.std_errors.push_back(std::sqrt(var_k));

    result.iterations = 1;
    result.converged = true;

    return result;
}
