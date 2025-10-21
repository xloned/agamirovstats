#ifndef MLE_METHODS_H
#define MLE_METHODS_H

#include <vector>

// Структура для хранения результатов MLE
struct MLEResult {
    std::vector<double> parameters;  // оценки параметров
    double** covariance;             // ковариационная матрица
    std::vector<double> std_errors;  // стандартные ошибки
    double log_likelihood;           // логарифм функции правдоподобия
    int iterations;                  // число итераций
    bool converged;                  // флаг сходимости
    int cov_size;                    // размер ковариационной матрицы
};

// Целевая функция для оптимизации (нормальное распределение)
double NormalMinFunction(std::vector<double> xsimpl);

// Целевая функция для оптимизации (распределение Вейбулла)
double WeibullMinFunction(std::vector<double> xsimpl);

// Вычисление ковариационной матрицы для нормального распределения
void CovMatrixMleN(int n, std::vector<double> x, std::vector<int> r, 
                   double a, double s, double**& v);

// Вычисление ковариационной матрицы для распределения Вейбулла
void CovMatrixMleW(int n, std::vector<double> x, std::vector<int> r, 
                   double c, double b, double**& v);

// MLE для нормального распределения (полные данные)
MLEResult mle_normal_complete(const std::vector<double>& data);

// MLE для распределения Вейбулла (полные данные)
MLEResult mle_weibull_complete(const std::vector<double>& data);

// MLS для нормального распределения (цензурированные данные)
MLEResult mls_normal_censored(const std::vector<double>& data, 
                               const std::vector<int>& censored);

// MLS для распределения Вейбулла (цензурированные данные)
MLEResult mls_weibull_censored(const std::vector<double>& data,
                                const std::vector<int>& censored);

// Вывод результатов MLE
void print_mle_result(const MLEResult& result, const char* method_name);

// Сохранение результатов в файл
void save_mle_result(const MLEResult& result, const char* filename,
                     const std::vector<double>& data,
                     const std::vector<int>& censored);

// Освобождение памяти результата
void free_mle_result(MLEResult& result);

#endif // MLE_METHODS_H