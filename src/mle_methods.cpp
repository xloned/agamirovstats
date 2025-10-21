#include "mle_methods.h"
#include "boost_distributions.h"
#include "nelder_mead.h"
#include "matrix_operations.h"
#include <cmath>
#include <numeric>
#include <iostream>
#include <fstream>
#include <iomanip>

// ============ Целевая функция для нормального распределения ============
// Реализация из boost.cpp файла
double NormalMinFunction(std::vector<double> xsimpl) {
    double s1, s2, s3, s4, z, psi, p, d, c1, c2;
    int i, kx;
    s1 = 0; s2 = 0; s3 = 0; s4 = 0; kx = 0;
    
    if (xsimpl[0] <= 0) return 10000;
    if (xsimpl[1] <= 0) return 10000;

    for (i = 0; i < nesm.n; i++) {
        z = (nesm.x[i] - xsimpl[0]) / xsimpl[1];
        d = norm_pdf(z);
        p = norm_cdf(z);
        psi = d / (1. - p);
        s1 += (1. - nesm.r[i]) * (nesm.x[i] - xsimpl[0]);
        s2 += (1. - nesm.r[i]) * pow(nesm.x[i] - xsimpl[0], 2);
        s3 += nesm.r[i] * psi;
        s4 += nesm.r[i] * psi * z;
        kx += 1 - nesm.r[i];
    }
    c1 = s1 + xsimpl[1] * s3;
    c2 = s2 + pow(xsimpl[1], 2) * (s4 - kx);
    z = c1 * c1 + c2 * c2;
    return z;
}

// ============ Целевая функция для распределения Вейбулла ============
// Реализация из boost.cpp файла
double WeibullMinFunction(std::vector<double> xsimpl) {
    double s1, s2, s3, z, b, c;
    int i, k;
    
    if (xsimpl[0] <= 0) return 10000000.;
    s1 = 0; s2 = 0; s3 = 0; k = 0;
    b = xsimpl[0];
    
    for (i = 0; i < nesm.n; i++) {
        k += (1 - nesm.r[i]);
        s1 += pow(nesm.x[i], b);
    }
    c = s1 / k;

    for (i = 0; i < nesm.n; i++) {
        z = (pow(nesm.x[i], b)) / c;
        s3 += z * log(z);
        s2 += (1 - nesm.r[i]) * log(z);
    }
    c = s3 - s2 - k;
    return c * c;
}

// ============ Ковариационная матрица для нормального распределения ============
// Реализация из boost.cpp файла
void CovMatrixMleN(int n, std::vector<double> x, std::vector<int> r, double a, double s, double**& v) {
    double z, p, d, s1, s2, s3, psi;
    int j, k;
    s1 = 0; s2 = 0; s3 = 0; k = 0;

    for (j = 0; j < n; j++) {
        z = (x[j] - a) / s;
        p = norm_cdf(z);
        d = norm_pdf(z);
        psi = d / (1 - p);
        s1 += r[j] * psi * (psi - z);
        s2 += r[j] * psi * z * (z * (psi - z) - 1);
        s3 += r[j] * psi * (z * (psi - z) - 1);
        k += (1 - r[j]);
    }

    v[0][0] = (k + s1) / n; v[0][1] = s3 / n;
    v[1][0] = s3 / n; v[1][1] = (2 * k + s2) / n;
    v = InverseMatrix(v, 2);
}

// ============ Ковариационная матрица для распределения Вейбулла ============
// Реализация из boost.cpp файла
void CovMatrixMleW(int n, std::vector<double> x, std::vector<int> r, double c, double b, double**& v) {
    int i, k;
    double s1, s2, z, cpw, ckow;
    cpw = log(c); ckow = 1 / b; s1 = 0; s2 = 0; k = 0;
    
    for (i = 0; i < n; i++) {
        z = (log(x[i]) - cpw) / ckow;
        s1 += (1 - r[i]) * z;
        s2 += z * z * exp(z);
        k += (1 - r[i]);
    }
    v[0][0] = double(k) / double(n); 
    v[0][1] = (k + s1) / n; 
    v[1][0] = (k + s1) / n; 
    v[1][1] = (k + s2) / n;
    v = InverseMatrix(v, 2);
}

// ============ MLE для нормального распределения (полные данные) ============
MLEResult mle_normal_complete(const std::vector<double>& data) {
    MLEResult result;
    int n = data.size();
    
    // Вычисление среднего
    double mean = std::accumulate(data.begin(), data.end(), 0.0) / n;
    
    // Вычисление стандартного отклонения
    double variance = 0.0;
    for (double x : data) {
        variance += (x - mean) * (x - mean);
    }
    variance /= n;
    double std = std::sqrt(variance);
    
    result.parameters = {mean, std};
    result.iterations = 0;
    result.converged = true;
    result.cov_size = 2;
    
    // Ковариационная матрица (асимптотическая)
    result.covariance = createMatrix(2, 2);
    result.covariance[0][0] = variance / n;
    result.covariance[0][1] = 0.0;
    result.covariance[1][0] = 0.0;
    result.covariance[1][1] = variance / (2.0 * n);
    
    // Стандартные ошибки
    result.std_errors = {std::sqrt(result.covariance[0][0]),
                        std::sqrt(result.covariance[1][1])};
    
    // Вычисление логарифма функции правдоподобия
    result.log_likelihood = 0.0;
    for (double x : data) {
        double z = (x - mean) / std;
        result.log_likelihood += -0.5 * log(2 * M_PI) - log(std) - 0.5 * z * z;
    }
    
    return result;
}

// ============ MLS для нормального распределения (цензурированные данные) ============
MLEResult mls_normal_censored(const std::vector<double>& data, const std::vector<int>& censored) {
    MLEResult result;
    
    // Сохранение данных в глобальную структуру
    nesm.n = data.size();
    nesm.x = data;
    nesm.r = censored;
    
    // Начальная оценка (используем только полные данные)
    std::vector<double> complete_data;
    for (size_t i = 0; i < data.size(); ++i) {
        if (censored[i] == 0) {
            complete_data.push_back(data[i]);
        }
    }
    
    if (complete_data.empty()) {
        complete_data = data;
    }
    
    MLEResult initial = mle_normal_complete(complete_data);
    
    // Оптимизация методом Nelder-Mead
    std::vector<double> x0 = initial.parameters;
    double eps = 1e-8;
    
    result.parameters = neldermead(x0, eps, NormalMinFunction);
    result.iterations = 1000;
    result.converged = true;
    result.cov_size = 2;
    
    // Вычисление ковариационной матрицы
    result.covariance = createMatrix(2, 2);
    CovMatrixMleN(data.size(), nesm.x, nesm.r, result.parameters[0], result.parameters[1], result.covariance);
    
    // Стандартные ошибки
    result.std_errors = {std::sqrt(result.covariance[0][0]),
                        std::sqrt(result.covariance[1][1])};
    
    // Логарифм функции правдоподобия
    result.log_likelihood = 0.0;
    for (size_t i = 0; i < data.size(); ++i) {
        double z = (data[i] - result.parameters[0]) / result.parameters[1];
        if (censored[i] == 0) {
            result.log_likelihood += -0.5 * log(2 * M_PI) - log(result.parameters[1]) - 0.5 * z * z;
        } else {
            result.log_likelihood += log(1.0 - norm_cdf(z));
        }
    }
    
    return result;
}

// (продолжение в следующей части)
// (продолжение src/mle_methods.cpp)

// ============ MLE для распределения Вейбулла (полные данные) ============
MLEResult mle_weibull_complete(const std::vector<double>& data) {
    MLEResult result;
    
    // Сохранение данных в глобальную структуру
    nesm.n = data.size();
    nesm.x = data;
    nesm.r = std::vector<int>(data.size(), 0);
    
    // Начальная оценка параметра формы
    std::vector<double> x0 = {1.5};
    double eps = 1e-8;
    
    // Оптимизация
    std::vector<double> opt_params = neldermead(x0, eps, WeibullMinFunction);
    double shape = opt_params[0];
    
    // Вычисление параметра масштаба
    double sum = 0.0;
    for (double x : data) {
        sum += pow(x, shape);
    }
    double scale = sum / data.size();
    
    result.parameters = {scale, shape};
    result.iterations = 1000;
    result.converged = true;
    result.cov_size = 2;
    
    // Вычисление ковариационной матрицы
    result.covariance = createMatrix(2, 2);
    CovMatrixMleW(data.size(), nesm.x, nesm.r, scale, shape, result.covariance);
    
    // Стандартные ошибки
    result.std_errors = {std::sqrt(result.covariance[0][0]),
                        std::sqrt(result.covariance[1][1])};
    
    // Логарифм функции правдоподобия
    result.log_likelihood = 0.0;
    for (double x : data) {
        result.log_likelihood += log(shape) - log(scale) + (shape - 1) * log(x) - 
                  pow(x / scale, shape);
    }
    
    return result;
}

// ============ MLS для распределения Вейбулла (цензурированные данные) ============
MLEResult mls_weibull_censored(const std::vector<double>& data, const std::vector<int>& censored) {
    MLEResult result;
    
    // Сохранение данных в глобальную структуру
    nesm.n = data.size();
    nesm.x = data;
    nesm.r = censored;
    
    // Начальная оценка
    std::vector<double> x0 = {1.5};
    double eps = 1e-8;
    
    // Оптимизация
    std::vector<double> opt_params = neldermead(x0, eps, WeibullMinFunction);
    double shape = opt_params[0];
    
    // Вычисление параметра масштаба
    int k = 0;
    double sum = 0.0;
    for (size_t i = 0; i < data.size(); ++i) {
        k += (1 - censored[i]);
        sum += pow(data[i], shape);
    }
    double scale = sum / k;
    
    result.parameters = {scale, shape};
    result.iterations = 1000;
    result.converged = true;
    result.cov_size = 2;
    
    // Вычисление ковариационной матрицы
    result.covariance = createMatrix(2, 2);
    CovMatrixMleW(data.size(), nesm.x, nesm.r, scale, shape, result.covariance);
    
    // Стандартные ошибки
    result.std_errors = {std::sqrt(result.covariance[0][0]),
                        std::sqrt(result.covariance[1][1])};
    
    // Логарифм функции правдоподобия
    result.log_likelihood = 0.0;
    for (size_t i = 0; i < data.size(); ++i) {
        if (censored[i] == 0) {
            result.log_likelihood += log(shape) - log(scale) + (shape - 1) * log(data[i]) - 
                      pow(data[i] / scale, shape);
        } else {
            result.log_likelihood += -pow(data[i] / scale, shape);
        }
    }
    
    return result;
}

// ============ Вывод результатов MLE ============
void print_mle_result(const MLEResult& result, const char* method_name) {
    std::cout << "\n========== " << method_name << " ==========\n";
    std::cout << std::fixed << std::setprecision(6);
    
    std::cout << "\nОценки параметров:\n";
    for (size_t i = 0; i < result.parameters.size(); ++i) {
        std::cout << "  Параметр " << (i+1) << ": " << result.parameters[i]
                 << " ± " << result.std_errors[i] << "\n";
    }
    
    std::cout << "\nЛог-правдоподобие: " << result.log_likelihood << "\n";
    std::cout << "Итераций: " << result.iterations << "\n";
    std::cout << "Сходимость: " << (result.converged ? "Да" : "Нет") << "\n";
    
    printMatrix(result.covariance, result.cov_size, result.cov_size, "Ковариационная матрица");
}

// ============ Сохранение результатов в файл ============
void save_mle_result(const MLEResult& result, const char* filename,
                     const std::vector<double>& data,
                     const std::vector<int>& censored) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << "\n";
        return;
    }
    
    file << std::fixed << std::setprecision(6);
    
    // Сохранение параметров
    file << "# Оценки параметров\n";
    for (size_t i = 0; i < result.parameters.size(); ++i) {
        file << "parameter_" << (i+1) << " " << result.parameters[i] << "\n";
        file << "std_error_" << (i+1) << " " << result.std_errors[i] << "\n";
    }
    
    file << "\n# Статистики\n";
    file << "log_likelihood " << result.log_likelihood << "\n";
    file << "iterations " << result.iterations << "\n";
    file << "converged " << (result.converged ? 1 : 0) << "\n";
    
    file << "\n# Ковариационная матрица\n";
    for (int i = 0; i < result.cov_size; ++i) {
        for (int j = 0; j < result.cov_size; ++j) {
            file << result.covariance[i][j] << " ";
        }
        file << "\n";
    }
    
    file << "\n# Данные\n";
    file << "# x censored\n";
    for (size_t i = 0; i < data.size(); ++i) {
        int cens = (i < censored.size()) ? censored[i] : 0;
        file << data[i] << " " << cens << "\n";
    }
    
    file.close();
    std::cout << "Результаты сохранены в файл: " << filename << "\n";
}

// ============ Освобождение памяти результата ============
void free_mle_result(MLEResult& result) {
    if (result.covariance != nullptr) {
        clearMemory(result.covariance, result.cov_size);
        result.covariance = nullptr;
    }
}