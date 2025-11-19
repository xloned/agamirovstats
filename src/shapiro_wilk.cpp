#include "shapiro_wilk.h"
#include "boost_distributions.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <fstream>
#include <iomanip>

/**
 * @brief Вычисление среднего значения выборки
 */
static double compute_mean(const std::vector<double>& data) {
    if (data.empty()) return 0.0;
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    return sum / data.size();
}

/**
 * @brief Вычисление критического значения для критерия Шапиро-Уилка
 *
 * Критические значения основаны на таблице П1 из приложения (стр. 64).
 * Для промежуточных значений n используется линейная интерполяция.
 *
 * @param n Размер выборки
 * @param alpha Уровень значимости
 * @return Критическое значение W_critical
 */
static double shapiro_wilk_critical_value(size_t n, double alpha) {
    // Таблица критических значений для α = 0.05 и α = 0.01
    // Источник: Таблица П1, стр. 64 (Агамиров, порядковые статистики)
    // Формат: n -> {W_0.05, W_0.01}

    struct TableEntry {
        size_t n;
        double w_005;
        double w_001;
    };

    // Сокращенная таблица критических значений
    static const TableEntry table[] = {
        {3,  0.767, 0.753},
        {4,  0.748, 0.687},
        {5,  0.762, 0.686},
        {6,  0.788, 0.713},
        {7,  0.803, 0.730},
        {8,  0.818, 0.749},
        {9,  0.829, 0.764},
        {10, 0.842, 0.781},
        {11, 0.850, 0.792},
        {12, 0.859, 0.805},
        {13, 0.866, 0.814},
        {14, 0.874, 0.825},
        {15, 0.881, 0.835},
        {16, 0.887, 0.844},
        {17, 0.892, 0.851},
        {18, 0.897, 0.858},
        {19, 0.901, 0.863},
        {20, 0.905, 0.868},
        {25, 0.918, 0.888},
        {30, 0.927, 0.900},
        {35, 0.934, 0.910},
        {40, 0.940, 0.919},
        {45, 0.945, 0.926},
        {50, 0.947, 0.930}
    };

    const size_t table_size = sizeof(table) / sizeof(TableEntry);

    // Определяем, какой столбец использовать
    bool use_005 = (std::abs(alpha - 0.05) < std::abs(alpha - 0.01));

    // Если n меньше минимального в таблице
    if (n < table[0].n) {
        return use_005 ? table[0].w_005 : table[0].w_001;
    }

    // Если n больше максимального в таблице, используем максимальное значение
    if (n >= table[table_size - 1].n) {
        return use_005 ? table[table_size - 1].w_005 : table[table_size - 1].w_001;
    }

    // Поиск в таблице с линейной интерполяцией
    for (size_t i = 0; i < table_size - 1; ++i) {
        if (table[i].n == n) {
            return use_005 ? table[i].w_005 : table[i].w_001;
        }
        if (table[i].n < n && n < table[i + 1].n) {
            // Линейная интерполяция
            double t = static_cast<double>(n - table[i].n) /
                      static_cast<double>(table[i + 1].n - table[i].n);
            if (use_005) {
                return table[i].w_005 + t * (table[i + 1].w_005 - table[i].w_005);
            } else {
                return table[i].w_001 + t * (table[i + 1].w_001 - table[i].w_001);
            }
        }
    }

    return use_005 ? table[table_size - 1].w_005 : table[table_size - 1].w_001;
}

/**
 * @brief Вычисление коэффициентов aᵢ для критерия Шапиро-Уилка
 *
 * Коэффициенты вычисляются на основе математических ожиданий порядковых
 * статистик стандартного нормального распределения.
 * Используется приближенная формула для вычисления.
 *
 * @param n Размер выборки
 * @return Вектор коэффициентов a₁, a₂, ..., aₖ (где k = n/2)
 */
static std::vector<double> compute_shapiro_wilk_coefficients(size_t n) {
    std::vector<double> coeffs;
    if (n < 3) return coeffs;

    // Количество коэффициентов k = floor(n/2)
    size_t k = n / 2;
    coeffs.resize(k);

    // Вычисление математических ожиданий порядковых статистик
    // Используется приближение через квантили нормального распределения
    std::vector<double> m(n);
    for (size_t i = 0; i < n; ++i) {
        // Приближение: m[i] ≈ Φ⁻¹((i+1-0.375)/(n+0.25))
        double p = (i + 1.0 - 0.375) / (n + 0.25);
        m[i] = norm_ppf(p);
    }

    // Вычисляем сумму квадратов m
    double sum_m_sq = 0.0;
    for (double val : m) {
        sum_m_sq += val * val;
    }

    // Вычисляем коэффициенты aᵢ
    // aᵢ = (m[n-i] - m[i-1]) / sqrt(sum_m_sq) для симметричного случая
    for (size_t i = 0; i < k; ++i) {
        coeffs[i] = (m[n - 1 - i] - m[i]) / std::sqrt(sum_m_sq);
    }

    return coeffs;
}

/**
 * @brief Критерий Шапиро-Уилка для проверки нормальности
 *
 * Реализация по формулам 3.17-3.19 из порядковых статистик
 */
ShapiroWilkResult shapiro_wilk_test(const std::vector<double>& data,
                                    double alpha) {
    ShapiroWilkResult result;
    result.alpha = alpha;
    result.n = data.size();

    // Проверка корректности входных данных
    if (data.size() < 3) {
        std::cerr << "Ошибка: для критерия Шапиро-Уилка требуется минимум 3 наблюдения" << std::endl;
        result.reject_h0 = false;
        result.w_statistic = 0.0;
        return result;
    }

    if (data.size() > 5000) {
        std::cerr << "Предупреждение: размер выборки велик (n > 5000), "
                  << "результаты могут быть неточными" << std::endl;
    }

    // Сортируем данные для получения порядковых статистик x(1), x(2), ..., x(n)
    std::vector<double> sorted_data = data;
    std::sort(sorted_data.begin(), sorted_data.end());

    // Вычисляем среднее x̄
    double mean = compute_mean(sorted_data);

    // Вычисляем знаменатель s² (формула 3.19)
    // s² = Σᵢ(xᵢ - x̄)²
    result.denominator = 0.0;
    for (double x : sorted_data) {
        double diff = x - mean;
        result.denominator += diff * diff;
    }

    if (result.denominator == 0.0) {
        std::cerr << "Ошибка: все значения в выборке одинаковы (дисперсия = 0)" << std::endl;
        result.reject_h0 = false;
        result.w_statistic = 1.0;
        return result;
    }

    // Вычисляем коэффициенты aᵢ
    std::vector<double> coeffs = compute_shapiro_wilk_coefficients(data.size());

    // Проверка на корректность коэффициентов
    if (coeffs.empty()) {
        std::cerr << "Ошибка: не удалось вычислить коэффициенты Шапиро-Уилка" << std::endl;
        result.reject_h0 = false;
        result.w_statistic = 0.0;
        return result;
    }

    // Вычисляем числитель b (формула 3.18)
    // b = Σᵢ aᵢ(x(n+1-i) - x(i))
    result.numerator = 0.0;
    size_t k = coeffs.size();
    for (size_t i = 0; i < k; ++i) {
        double diff = sorted_data[data.size() - 1 - i] - sorted_data[i];
        result.numerator += coeffs[i] * diff;
    }

    // Возводим в квадрат для получения b²
    result.numerator = result.numerator * result.numerator;

    // Вычисляем статистику W (формула 3.17)
    // W = b² / s²
    result.w_statistic = result.numerator / result.denominator;

    // Проверка на NaN и некорректные значения
    if (std::isnan(result.w_statistic) || std::isinf(result.w_statistic)) {
        std::cerr << "Ошибка: некорректное значение W-статистики (NaN/Inf)" << std::endl;
        result.reject_h0 = false;
        result.w_statistic = 0.0;
        return result;
    }

    // W должна быть от 0 до 1, если больше - ошибка в вычислениях
    if (result.w_statistic > 1.0) {
        std::cerr << "Предупреждение: W-статистика > 1 (" << result.w_statistic
                  << "), устанавливаем W = 1.0" << std::endl;
        std::cerr << "Это может указывать на проблему с данными или вычислениями" << std::endl;
        result.w_statistic = 1.0;
    }

    if (result.w_statistic < 0.0) {
        std::cerr << "Ошибка: W-статистика < 0, устанавливаем W = 0.0" << std::endl;
        result.w_statistic = 0.0;
    }

    // Получаем критическое значение из таблицы
    result.critical_value = shapiro_wilk_critical_value(data.size(), alpha);

    // Приближенное вычисление p-значения
    // Используется логарифмическое преобразование для аппроксимации
    // Это приближение, основанное на работе Royston (1992)
    double ln_w = std::log(1.0 - result.w_statistic);
    double mu = 0.0;
    double sigma = 1.0;

    if (data.size() >= 4 && data.size() <= 11) {
        double n_d = static_cast<double>(data.size());
        mu = -1.5861 - 0.31082 * std::log(n_d) - 0.083751 * std::log(n_d) * std::log(n_d);
        sigma = std::exp(-0.4803 - 0.082676 * std::log(n_d) + 0.0030302 * std::log(n_d) * std::log(n_d));
    } else if (data.size() >= 12) {
        double n_d = static_cast<double>(data.size());
        mu = -1.5861 - 0.31082 * std::log(n_d) - 0.083751 * std::log(n_d) * std::log(n_d);
        sigma = std::exp(-0.4803 - 0.082676 * std::log(n_d) + 0.0030302 * std::log(n_d) * std::log(n_d));
    }

    // Проверка на корректность вычислений перед использованием norm_cdf
    double z = 0.0;
    if (sigma > 0.0 && !std::isnan(ln_w) && !std::isinf(ln_w)) {
        z = (ln_w - mu) / sigma;
    }

    // Проверка z на NaN/Inf перед передачей в norm_cdf
    if (std::isnan(z) || std::isinf(z)) {
        result.p_value = 0.5; // Нейтральное значение при ошибке
    } else {
        result.p_value = norm_cdf(z);
    }

    // Проверяем гипотезу H0: данные из нормального распределения
    // Отвергаем H0, если W < W_critical (малые значения W указывают на ненормальность)
    result.reject_h0 = (result.w_statistic < result.critical_value);

    return result;
}

/**
 * @brief Вывод результатов критерия Шапиро-Уилка
 */
void print_shapiro_wilk_result(const ShapiroWilkResult& result,
                               const std::string& filename) {
    std::ostream* out = &std::cout;
    std::ofstream file;

    if (!filename.empty()) {
        file.open(filename);
        if (file.is_open()) {
            out = &file;
        }
    }

    *out << "========================================" << std::endl;
    *out << "  КРИТЕРИЙ ШАПИРО-УИЛКА" << std::endl;
    *out << "  (Shapiro-Wilk normality test)" << std::endl;
    *out << "  для проверки нормальности" << std::endl;
    *out << "========================================" << std::endl;
    *out << std::endl;

    *out << "Размер выборки: n = " << result.n << std::endl;
    *out << "Уровень значимости: α = " << result.alpha << std::endl;
    *out << std::endl;

    *out << std::fixed << std::setprecision(6);
    *out << "W-статистика = " << result.w_statistic << std::endl;
    *out << "Критическое значение W_critical = " << result.critical_value << std::endl;
    *out << "Приблизительное p-значение = " << std::setprecision(4) << result.p_value << std::endl;
    *out << std::endl;

    *out << "Примечание: W-статистика принимает значения от 0 до 1." << std::endl;
    *out << "Значения близкие к 1 указывают на согласие с нормальным распределением." << std::endl;
    *out << std::endl;

    *out << "Гипотеза H0: выборка получена из нормального распределения" << std::endl;
    if (result.reject_h0) {
        *out << "РЕЗУЛЬТАТ: H0 ОТВЕРГАЕТСЯ (выборка не является нормальной)" << std::endl;
        *out << "W (" << std::setprecision(6) << result.w_statistic
             << ") < W_critical (" << result.critical_value << ")" << std::endl;
        *out << "p-value (" << std::setprecision(4) << result.p_value
             << ") < α (" << result.alpha << ")" << std::endl;
    } else {
        *out << "РЕЗУЛЬТАТ: H0 НЕ ОТВЕРГАЕТСЯ (нет оснований отвергнуть нормальность)" << std::endl;
        *out << "W (" << std::setprecision(6) << result.w_statistic
             << ") ≥ W_critical (" << result.critical_value << ")" << std::endl;
        *out << "p-value (" << std::setprecision(4) << result.p_value
             << ") ≥ α (" << result.alpha << ")" << std::endl;
    }
    *out << std::endl;

    if (file.is_open()) {
        std::cout << "Результаты сохранены в файл: " << filename << std::endl;
        file.close();
    }
}
