// MLS для нормального распределения (цензурированные данные)
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "mle_methods.h"
#include "nelder_mead.h"
#include "boost_distributions.h"
#include "matrix_operations.h"

// Реализация MLS для нормального распределения с цензурированными данными
MLEResult mls_normal_censored(const std::vector<double>& data, const std::vector<int>& censored) {
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
    double std_init = std::sqrt(variance_init);

    // Оптимизация методом Nelder-Mead
    std::vector<double> x0 = {mean_init, std_init};
    double eps = 1e-6;
    std::vector<double> optimal = neldermead(x0, eps, NormalMinFunction);

    double a = optimal[0];  // среднее
    double s = optimal[1];  // стандартное отклонение

    // Сохранение параметров
    result.parameters.push_back(a);
    result.parameters.push_back(s);

    // Вычисление логарифма функции правдоподобия
    double log_likelihood = 0.0;
    for (int i = 0; i < n; i++) {
        double z = (data[i] - a) / s;
        if (censored[i] == 0) {
            // Полное наблюдение
            log_likelihood += std::log(norm_pdf(z) / s);
        } else {
            // Цензурированное наблюдение
            log_likelihood += std::log(1.0 - norm_cdf(z));
        }
    }
    result.log_likelihood = log_likelihood;

    // Вычисление ковариационной матрицы
    result.cov_size = 2;
    result.covariance = createMatrix(2, 2);

    std::vector<int> r_vec = censored;
    CovMatrixMleN(n, data, r_vec, a, s, result.covariance);

    // Стандартные ошибки
    result.std_errors.push_back(std::sqrt(std::abs(result.covariance[0][0])));
    result.std_errors.push_back(std::sqrt(std::abs(result.covariance[1][1])));

    result.iterations = 1;
    result.converged = true;

    return result;
}
