#include "nelder_mead.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>

// Глобальная переменная для данных оптимизации
ne_simp nesm;

// Вычисление центроида симплекса (исключая наихудшую точку)
std::vector<double> compute_centroid(const std::vector<std::vector<double>>& simplex, size_t exclude_idx) {
    size_t n = simplex[0].size();
    std::vector<double> centroid(n, 0.0);
    
    for (size_t i = 0; i < simplex.size(); ++i) {
        if (i != exclude_idx) {
            for (size_t j = 0; j < n; ++j) {
                centroid[j] += simplex[i][j];
            }
        }
    }
    
    for (size_t j = 0; j < n; ++j) {
        centroid[j] /= (simplex.size() - 1);
    }
    
    return centroid;
}

// Отражение точки относительно центроида
std::vector<double> reflect_point(const std::vector<double>& point,
                                  const std::vector<double>& centroid, double alpha) {
    size_t n = point.size();
    std::vector<double> reflected(n);
    
    for (size_t i = 0; i < n; ++i) {
        reflected[i] = centroid[i] + alpha * (centroid[i] - point[i]);
    }
    
    return reflected;
}

// Расширение точки
std::vector<double> expand_point(const std::vector<double>& reflected,
                                const std::vector<double>& centroid, double gamma) {
    size_t n = reflected.size();
    std::vector<double> expanded(n);
    
    for (size_t i = 0; i < n; ++i) {
        expanded[i] = centroid[i] + gamma * (reflected[i] - centroid[i]);
    }
    
    return expanded;
}

// Сжатие точки
std::vector<double> contract_point(const std::vector<double>& point,
                                  const std::vector<double>& centroid, double rho) {
    size_t n = point.size();
    std::vector<double> contracted(n);
    
    for (size_t i = 0; i < n; ++i) {
        contracted[i] = centroid[i] + rho * (point[i] - centroid[i]);
    }
    
    return contracted;
}

// Размер симплекса (для проверки сходимости)
double simplex_size(const std::vector<std::vector<double>>& simplex) {
    size_t n = simplex[0].size();
    double max_diff = 0.0;
    
    for (size_t i = 0; i < n; ++i) {
        double min_val = simplex[0][i];
        double max_val = simplex[0][i];
        
        for (size_t j = 1; j < simplex.size(); ++j) {
            min_val = std::min(min_val, simplex[j][i]);
            max_val = std::max(max_val, simplex[j][i]);
        }
        
        max_diff = std::max(max_diff, max_val - min_val);
    }
    
    return max_diff;
}

// Основная функция оптимизации методом Nelder-Mead
std::vector<double> neldermead(std::vector<double>& x0, double eps, 
                               std::function<double(std::vector<double>)> func) {
    const double alpha = 1.0;    // коэффициент отражения
    const double gamma = 2.0;    // коэффициент расширения
    const double rho = 0.5;      // коэффициент сжатия
    const double sigma = 0.5;    // коэффициент уменьшения
    const int max_iter = 1000;   // максимальное число итераций
    
    size_t n = x0.size();
    
    // Инициализация симплекса
    std::vector<std::vector<double>> simplex(n + 1);
    simplex[0] = x0;
    
    for (size_t i = 1; i <= n; ++i) {
        simplex[i] = x0;
        simplex[i][i-1] += 0.1 * (x0[i-1] != 0.0 ? x0[i-1] : 1.0);
    }
    
    // Вычисление значений функции для всех вершин симплекса
    std::vector<double> f_values(n + 1);
    for (size_t i = 0; i <= n; ++i) {
        f_values[i] = func(simplex[i]);
    }
    
    // Основной цикл оптимизации
    for (int iter = 0; iter < max_iter; ++iter) {
        // Сортировка вершин по значению функции
        std::vector<size_t> indices(n + 1);
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(),
                 [&f_values](size_t i, size_t j) { return f_values[i] < f_values[j]; });
        
        // Переупорядочивание симплекса
        std::vector<std::vector<double>> sorted_simplex(n + 1);
        std::vector<double> sorted_f_values(n + 1);
        for (size_t i = 0; i <= n; ++i) {
            sorted_simplex[i] = simplex[indices[i]];
            sorted_f_values[i] = f_values[indices[i]];
        }
        simplex = sorted_simplex;
        f_values = sorted_f_values;
        
        // Проверка сходимости
        if (simplex_size(simplex) < eps) {
            std::cout << "Сходимость достигнута на итерации " << iter << std::endl;
            return simplex[0];
        }
        
        // Вычисление центроида (исключая наихудшую точку)
        std::vector<double> centroid = compute_centroid(simplex, n);
        
        // Отражение
        std::vector<double> reflected = reflect_point(simplex[n], centroid, alpha);
        double f_reflected = func(reflected);
        
        if (f_reflected < f_values[0]) {
            // Расширение
            std::vector<double> expanded = expand_point(reflected, centroid, gamma);
            double f_expanded = func(expanded);
            
            if (f_expanded < f_reflected) {
                simplex[n] = expanded;
                f_values[n] = f_expanded;
            } else {
                simplex[n] = reflected;
                f_values[n] = f_reflected;
            }
        } else if (f_reflected < f_values[n-1]) {
            simplex[n] = reflected;
            f_values[n] = f_reflected;
        } else {
            // Сжатие
            if (f_reflected < f_values[n]) {
                // Внешнее сжатие
                std::vector<double> contracted = contract_point(reflected, centroid, rho);
                double f_contracted = func(contracted);
                
                if (f_contracted < f_reflected) {
                    simplex[n] = contracted;
                    f_values[n] = f_contracted;
                } else {
                    // Уменьшение
                    for (size_t i = 1; i <= n; ++i) {
                        for (size_t j = 0; j < n; ++j) {
                            simplex[i][j] = simplex[0][j] + sigma * (simplex[i][j] - simplex[0][j]);
                        }
                        f_values[i] = func(simplex[i]);
                    }
                }
            } else {
                // Внутреннее сжатие
                std::vector<double> contracted = contract_point(simplex[n], centroid, rho);
                double f_contracted = func(contracted);
                
                if (f_contracted < f_values[n]) {
                    simplex[n] = contracted;
                    f_values[n] = f_contracted;
                } else {
                    // Уменьшение
                    for (size_t i = 1; i <= n; ++i) {
                        for (size_t j = 0; j < n; ++j) {
                            simplex[i][j] = simplex[0][j] + sigma * (simplex[i][j] - simplex[0][j]);
                        }
                        f_values[i] = func(simplex[i]);
                    }
                }
            }
        }
    }
    
    std::cout << "Достигнуто максимальное число итераций: " << max_iter << std::endl;
    return simplex[0];
}

// Функция оптимизации с детальной информацией о результате
NelderMeadResult neldermead_detailed(std::vector<double>& x0, double eps,
                                     std::function<double(std::vector<double>)> func) {
    const double alpha = 1.0;    // коэффициент отражения
    const double gamma = 2.0;    // коэффициент расширения
    const double rho = 0.5;      // коэффициент сжатия
    const double sigma = 0.5;    // коэффициент уменьшения
    const int max_iter = 1000;   // максимальное число итераций

    NelderMeadResult result;
    result.converged = false;
    result.iterations = 0;

    size_t n = x0.size();

    // Инициализация симплекса
    std::vector<std::vector<double>> simplex(n + 1);
    simplex[0] = x0;

    for (size_t i = 1; i <= n; ++i) {
        simplex[i] = x0;
        simplex[i][i-1] += 0.1 * (x0[i-1] != 0.0 ? x0[i-1] : 1.0);
    }

    // Вычисление значений функции для всех вершин симплекса
    std::vector<double> f_values(n + 1);
    for (size_t i = 0; i <= n; ++i) {
        f_values[i] = func(simplex[i]);
    }

    // Основной цикл оптимизации
    for (int iter = 0; iter < max_iter; ++iter) {
        result.iterations = iter + 1;

        // Сортировка вершин по значению функции
        std::vector<size_t> indices(n + 1);
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(),
                 [&f_values](size_t i, size_t j) { return f_values[i] < f_values[j]; });

        // Переупорядочивание симплекса
        std::vector<std::vector<double>> sorted_simplex(n + 1);
        std::vector<double> sorted_f_values(n + 1);
        for (size_t i = 0; i <= n; ++i) {
            sorted_simplex[i] = simplex[indices[i]];
            sorted_f_values[i] = f_values[indices[i]];
        }
        simplex = sorted_simplex;
        f_values = sorted_f_values;

        // Проверка сходимости
        if (simplex_size(simplex) < eps) {
            result.converged = true;
            result.parameters = simplex[0];
            result.final_value = f_values[0];
            return result;
        }

        // Вычисление центроида (исключая наихудшую точку)
        std::vector<double> centroid = compute_centroid(simplex, n);

        // Отражение
        std::vector<double> reflected = reflect_point(simplex[n], centroid, alpha);
        double f_reflected = func(reflected);

        if (f_reflected < f_values[0]) {
            // Расширение
            std::vector<double> expanded = expand_point(reflected, centroid, gamma);
            double f_expanded = func(expanded);

            if (f_expanded < f_reflected) {
                simplex[n] = expanded;
                f_values[n] = f_expanded;
            } else {
                simplex[n] = reflected;
                f_values[n] = f_reflected;
            }
        } else if (f_reflected < f_values[n-1]) {
            simplex[n] = reflected;
            f_values[n] = f_reflected;
        } else {
            // Сжатие
            if (f_reflected < f_values[n]) {
                // Внешнее сжатие
                std::vector<double> contracted = contract_point(reflected, centroid, rho);
                double f_contracted = func(contracted);

                if (f_contracted < f_reflected) {
                    simplex[n] = contracted;
                    f_values[n] = f_contracted;
                } else {
                    // Уменьшение
                    for (size_t i = 1; i <= n; ++i) {
                        for (size_t j = 0; j < n; ++j) {
                            simplex[i][j] = simplex[0][j] + sigma * (simplex[i][j] - simplex[0][j]);
                        }
                        f_values[i] = func(simplex[i]);
                    }
                }
            } else {
                // Внутреннее сжатие
                std::vector<double> contracted = contract_point(simplex[n], centroid, rho);
                double f_contracted = func(contracted);

                if (f_contracted < f_values[n]) {
                    simplex[n] = contracted;
                    f_values[n] = f_contracted;
                } else {
                    // Уменьшение
                    for (size_t i = 1; i <= n; ++i) {
                        for (size_t j = 0; j < n; ++j) {
                            simplex[i][j] = simplex[0][j] + sigma * (simplex[i][j] - simplex[0][j]);
                        }
                        f_values[i] = func(simplex[i]);
                    }
                }
            }
        }
    }

    // Максимальное число итераций достигнуто
    result.converged = false;
    result.iterations = max_iter;
    result.parameters = simplex[0];
    result.final_value = f_values[0];
    return result;
}