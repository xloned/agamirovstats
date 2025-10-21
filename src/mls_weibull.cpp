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

    double c = optimal[0];  // параметр формы

    // Вычисление параметра масштаба
    double sum_xc = 0.0;
    for (int i = 0; i < n; i++) {
        sum_xc += std::pow(data[i], c);
    }
    double b = std::pow(sum_xc / n_complete, 1.0 / c);

    // Сохранение параметров
    result.parameters.push_back(b);  // lambda (масштаб)
    result.parameters.push_back(c);  // k (форма)

    // Вычисление логарифма функции правдоподобия
    double log_likelihood = 0.0;
    for (int i = 0; i < n; i++) {
        if (censored[i] == 0) {
            // Полное наблюдение
            log_likelihood += std::log(c / b) + (c - 1) * std::log(data[i] / b) - std::pow(data[i] / b, c);
        } else {
            // Цензурированное наблюдение (вероятность выживания)
            log_likelihood += -std::pow(data[i] / b, c);
        }
    }
    result.log_likelihood = log_likelihood;

    // Вычисление ковариационной матрицы
    result.cov_size = 2;
    result.covariance = createMatrix(2, 2);

    std::vector<int> r_vec = censored;
    CovMatrixMleW(n, data, r_vec, c, b, result.covariance);

    // Стандартные ошибки
    result.std_errors.push_back(std::sqrt(std::abs(result.covariance[0][0])));
    result.std_errors.push_back(std::sqrt(std::abs(result.covariance[1][1])));

    result.iterations = 1;
    result.converged = true;

    return result;
}
