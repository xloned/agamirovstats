// MLE для распределения Вейбулла (полные данные)
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include "mle_methods.h"
#include "nelder_mead.h"
#include "boost_distributions.h"
#include "matrix_operations.h"

// Глобальная структура для хранения данных
ne_simp global_data_weibull;

// Целевая функция для оптимизации Вейбулла (полные данные)
// Минимизируем отрицательный log-likelihood
double WeibullObjective(std::vector<double> params) {
    double k = params[0];  // параметр формы
    if (k <= 0 || k > 100) return 1e10;  // Ограничения

    int n = global_data_weibull.n;

    // Вычисляем λ для данного k
    double sum_xk = 0.0;
    for (int i = 0; i < n; i++) {
        sum_xk += std::pow(global_data_weibull.x[i], k);
    }
    double lambda = std::pow(sum_xk / n, 1.0 / k);

    if (lambda <= 0 || !std::isfinite(lambda)) return 1e10;

    // Вычисляем отрицательный log-likelihood
    double neg_log_likelihood = 0.0;
    for (int i = 0; i < n; i++) {
        double x = global_data_weibull.x[i];
        if (x <= 0) continue;

        // -log L = -log(k/λ) - (k-1)*log(x) + (x/λ)^k
        neg_log_likelihood += -std::log(k / lambda) - (k - 1) * std::log(x) + std::pow(x / lambda, k);
    }

    return neg_log_likelihood;
}

// Реализация MLE для распределения Вейбулла с полными данными
MLEResult mle_weibull_complete(const std::vector<double>& data) {
    MLEResult result;
    int n = data.size();

    // Настройка глобальных данных
    global_data_weibull.n = n;
    global_data_weibull.x = data;
    global_data_weibull.r = std::vector<int>(n, 0);  // Все наблюдения полные
    global_data_weibull.nsample.clear();

    // Начальная оценка параметра формы (метод моментов)
    double mean = std::accumulate(data.begin(), data.end(), 0.0) / n;
    double variance = 0.0;
    for (double x : data) {
        variance += (x - mean) * (x - mean);
    }
    variance /= n;
    double cv = std::sqrt(variance) / mean;

    // Начальная оценка c из коэффициента вариации
    double c_init = 1.0 / cv;  // Грубая начальная оценка
    if (c_init <= 0) c_init = 1.0;

    // Оптимизация методом Nelder-Mead
    std::vector<double> x0 = {c_init};
    double eps = 1e-6;
    std::vector<double> optimal = neldermead(x0, eps, WeibullObjective);

    double k = optimal[0];  // параметр формы

    // Правильная формула для параметра масштаба λ
    // λ = (1/n * Σ x_i^k)^(1/k)
    double sum_xk = 0.0;
    for (double x : data) {
        sum_xk += std::pow(x, k);
    }
    double lambda = std::pow(sum_xk / n, 1.0 / k);  // параметр масштаба

    // Сохранение параметров
    result.parameters.push_back(lambda);  // lambda (масштаб)
    result.parameters.push_back(k);       // k (форма)

    // Вычисление логарифма функции правдоподобия
    // log L = n*log(k/λ) + (k-1)*Σlog(x_i) - Σ(x_i/λ)^k
    double log_likelihood = 0.0;
    for (double x : data) {
        log_likelihood += std::log(k / lambda) + (k - 1) * std::log(x) - std::pow(x / lambda, k);
    }
    result.log_likelihood = log_likelihood;

    // Вычисление ковариационной матрицы через информационную матрицу Фишера
    result.cov_size = 2;
    result.covariance = createMatrix(2, 2);

    // Приближенная ковариационная матрица для Вейбулла
    // Используем формулы из теории (упрощенные)
    double var_lambda = (lambda * lambda) / (n * k * k);  // Var(λ̂)
    double var_k = 1.644 * (k * k) / n;                    // Var(k̂) (приближенно)
    double cov_lambda_k = 0.0;                             // Cov(λ̂, k̂) ≈ 0 для больших n

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
