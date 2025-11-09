#include "mle_methods.h"
#include "boost_distributions.h"
#include "nelder_mead.h"
#include "matrix_operations.h"
#include "order.h"
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

    // Обращение через Boost.uBLAS
    Matrix v_matrix(2, 2);
    v_matrix(0, 0) = v[0][0]; v_matrix(0, 1) = v[0][1];
    v_matrix(1, 0) = v[1][0]; v_matrix(1, 1) = v[1][1];
    Matrix v_inv = InverseMatrix(v_matrix);
    v[0][0] = v_inv(0, 0); v[0][1] = v_inv(0, 1);
    v[1][0] = v_inv(1, 0); v[1][1] = v_inv(1, 1);
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

    // Обращение через Boost.uBLAS
    Matrix v_matrix(2, 2);
    v_matrix(0, 0) = v[0][0]; v_matrix(0, 1) = v[0][1];
    v_matrix(1, 0) = v[1][0]; v_matrix(1, 1) = v[1][1];
    Matrix v_inv = InverseMatrix(v_matrix);
    v[0][0] = v_inv(0, 0); v[0][1] = v_inv(0, 1);
    v[1][0] = v_inv(1, 0); v[1][1] = v_inv(1, 1);
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
    
    // Начальные параметры (для нормального распределения совпадают с финальными,
    // т.к. есть аналитическое решение)
    result.initial_parameters = {mean, std};

    // Начальный log-likelihood
    result.initial_log_likelihood = 0.0;
    for (double x : data) {
        double z = (x - mean) / std;
        result.initial_log_likelihood += -0.5 * log(2 * M_PI) - log(std) - 0.5 * z * z;
    }

    result.parameters = {mean, std};
    result.iterations = 0;
    result.converged = true;
    result.cov_size = 2;

    // Ковариационная матрица (асимптотическая) - используем обычный массив для совместимости
    result.covariance = new double*[2];
    for (int i = 0; i < 2; i++) {
        result.covariance[i] = new double[2]();
    }
    result.covariance[0][0] = variance / n;
    result.covariance[0][1] = 0.0;
    result.covariance[1][0] = 0.0;
    result.covariance[1][1] = variance / (2.0 * n);

    // Стандартные ошибки
    result.std_errors = {std::sqrt(result.covariance[0][0]),
                        std::sqrt(result.covariance[1][1])};

    // Вычисление логарифма функции правдоподобия
    result.log_likelihood = result.initial_log_likelihood;
    
    return result;
}

// ============ MLS для нормального распределения (ТОЛЬКО полные данные) ============
// Использует взвешенный МНК через порядковые статистики (метод Дэйвида - ordern)
MLEResult mls_normal_complete(const std::vector<double>& data) {
    MLEResult result;
    int n = data.size();

    // Начальная оценка - используем MLE
    MLEResult initial_mle = mle_normal_complete(data);
    result.initial_parameters = initial_mle.parameters;
    result.initial_log_likelihood = initial_mle.log_likelihood;

    // Подготовка данных: все данные полные (r[i] = 0)
    std::vector<int> r(n, 0);  // все наблюдения полные
    std::vector<double> fcum(n);
    std::vector<double> ycum(n);

    // Вычисление эмпирической функции распределения
    cum(n, data, r, n, fcum, ycum);

    // Построение ковариационной матрицы порядковых статистик через Boost.uBLAS
    Matrix v = createMatrix(n, n);
    Matrix x = createMatrix(n, 2);
    Matrix y = createMatrix(n, 1);
    Matrix b = createMatrix(2, 1);
    Matrix db = createMatrix(2, 2);
    Vector yr(n);

    // Заполнение матриц для взвешенного МНК
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            double er_i, vrs;
            ordern(n, fcum[i], fcum[j], er_i, vrs);
            v(j, i) = vrs;
            v(i, j) = vrs;
        }
        double er, vrs;
        ordern(n, fcum[i], fcum[i], er, vrs);
        x(i, 0) = 1.0;      // столбец для μ
        x(i, 1) = er;       // столбец для σ (математическое ожидание порядковой статистики)
        y(i, 0) = ycum[i];  // наблюдаемые значения
    }

    // Взвешенный МНК через Boost
    MleastSquare_weight(x, y, v, db, b, yr);

    // Результаты: b(0,0) = μ, b(1,0) = σ
    result.parameters.push_back(b(0, 0));  // μ
    result.parameters.push_back(b(1, 0));  // σ

    // Вычисление логарифма функции правдоподобия
    result.log_likelihood = 0.0;
    for (double x_val : data) {
        double z = (x_val - b(0, 0)) / b(1, 0);
        result.log_likelihood += -0.5 * log(2 * M_PI) - log(b(1, 0)) - 0.5 * z * z;
    }

    // Ковариационная матрица параметров (из взвешенного МНК)
    // Теперь используем обычный массив для совместимости с MLEResult
    result.cov_size = 2;
    result.covariance = new double*[2];
    for (int i = 0; i < 2; i++) {
        result.covariance[i] = new double[2];
        for (int j = 0; j < 2; j++) {
            result.covariance[i][j] = db(i, j);
        }
    }

    // Стандартные ошибки
    result.std_errors.push_back(std::sqrt(std::abs(db(0, 0))));
    result.std_errors.push_back(std::sqrt(std::abs(db(1, 1))));

    result.iterations = 0;  // Прямое вычисление
    result.converged = true;

    // Boost.uBLAS автоматически управляет памятью для Matrix и Vector

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
    result.initial_parameters.push_back(x0[0]); // начальное k

    // Вычисление начального λ для k=1.5
    double sum_initial = 0.0;
    int n = data.size();
    for (double x : data) {
        sum_initial += pow(x, x0[0]);
    }
    double scale_initial = pow(sum_initial / n, 1.0 / x0[0]);
    result.initial_parameters.push_back(scale_initial);

    // Начальный log-likelihood
    result.initial_log_likelihood = 0.0;
    for (double x : data) {
        result.initial_log_likelihood += log(x0[0] / scale_initial) + (x0[0] - 1) * log(x) -
                  pow(x / scale_initial, x0[0]);
    }

    // Оптимизация
    double eps = 1e-8;
    NelderMeadResult nm_result = neldermead_detailed(x0, eps, WeibullMinFunction);
    double shape = nm_result.parameters[0];

    // Вычисление параметра масштаба λ
    // λ = (1/n * Σ x_i^k)^(1/k)
    double sum = 0.0;
    for (double x : data) {
        sum += pow(x, shape);
    }
    double scale = pow(sum / n, 1.0 / shape);

    result.parameters = {scale, shape};
    result.iterations = nm_result.iterations;
    result.converged = nm_result.converged;
    result.cov_size = 2;

    // Вычисление ковариационной матрицы (приближенная формула)
    result.covariance = new double*[2];
    for (int i = 0; i < 2; i++) {
        result.covariance[i] = new double[2]();
    }
    double var_lambda = (scale * scale) / (n * shape * shape);
    double var_k = 1.644 * (shape * shape) / n;
    result.covariance[0][0] = var_lambda;
    result.covariance[0][1] = 0.0;
    result.covariance[1][0] = 0.0;
    result.covariance[1][1] = var_k;

    // Стандартные ошибки
    result.std_errors = {std::sqrt(var_lambda), std::sqrt(var_k)};

    // Логарифм функции правдоподобия
    // log L = n*log(k/λ) + (k-1)*Σlog(x_i) - Σ(x_i/λ)^k
    result.log_likelihood = 0.0;
    for (double x : data) {
        result.log_likelihood += log(shape / scale) + (shape - 1) * log(x) -
                  pow(x / scale, shape);
    }

    return result;
}

// MLS для Вейбулла не реализован - используется только MLE

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

    // Вывод ковариационной матрицы (конвертируем в Boost.uBLAS для вывода)
    Matrix cov_matrix(result.cov_size, result.cov_size);
    for (int i = 0; i < result.cov_size; i++) {
        for (int j = 0; j < result.cov_size; j++) {
            cov_matrix(i, j) = result.covariance[i][j];
        }
    }
    printMatrix(cov_matrix, "Ковариационная матрица");
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

    // Сохранение начальных оценок (если есть)
    if (!result.initial_parameters.empty()) {
        file << "# Начальные оценки параметров\n";
        for (size_t i = 0; i < result.initial_parameters.size(); ++i) {
            file << "initial_parameter_" << (i+1) << " " << result.initial_parameters[i] << "\n";
        }
        file << "initial_log_likelihood " << result.initial_log_likelihood << "\n\n";
    }

    // Сохранение финальных параметров
    file << "# Финальные оценки параметров\n";
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