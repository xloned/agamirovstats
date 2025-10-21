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
double WeibullObjective(std::vector<double> params) {
    double c = params[0];  // параметр формы
    if (c <= 0) return 1e10;

    double n = global_data_weibull.n;
    double s1 = 0.0, s2 = 0.0;

    for (int i = 0; i < global_data_weibull.n; i++) {
        double x = global_data_weibull.x[i];
        s1 += std::pow(x, c);
        s2 += std::pow(x, c) * std::log(x);
    }

    double b = std::pow(s1 / n, 1.0 / c);  // параметр масштаба

    // Уравнение для оценки c
    double eq = s2 / s1 - 1.0 / c - std::accumulate(global_data_weibull.x.begin(),
                                                      global_data_weibull.x.begin() + global_data_weibull.n, 0.0,
                                                      [](double acc, double x) { return acc + std::log(x); }) / n;

    return eq * eq;  // Минимизация квадрата уравнения
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

    double c = optimal[0];  // параметр формы
    double sum_xc = 0.0;
    for (double x : data) {
        sum_xc += std::pow(x, c);
    }
    double b = std::pow(sum_xc / n, 1.0 / c);  // параметр масштаба

    // Сохранение параметров
    result.parameters.push_back(b);  // lambda (масштаб)
    result.parameters.push_back(c);  // k (форма)

    // Вычисление логарифма функции правдоподобия
    double log_likelihood = 0.0;
    for (double x : data) {
        log_likelihood += std::log(c / b) + (c - 1) * std::log(x / b) - std::pow(x / b, c);
    }
    result.log_likelihood = log_likelihood;

    // Вычисление ковариационной матрицы (через информационную матрицу Фишера)
    result.cov_size = 2;
    result.covariance = createMatrix(2, 2);

    // Упрощенная оценка через численное дифференцирование
    double delta = 1e-5;

    // Для точной ковариационной матрицы используем функцию из mle_methods
    std::vector<int> r(n, 0);
    CovMatrixMleW(n, global_data_weibull.x, r, c, b, result.covariance);

    // Стандартные ошибки
    result.std_errors.push_back(std::sqrt(std::abs(result.covariance[0][0])));
    result.std_errors.push_back(std::sqrt(std::abs(result.covariance[1][1])));

    result.iterations = 1;
    result.converged = true;

    return result;
}
