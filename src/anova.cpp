#include "anova.h"
#include "boost_distributions.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <fstream>
#include <iomanip>

/**
 * @brief Вычисление среднего значения выборки
 * @param data Вектор данных
 * @return Среднее арифметическое
 */
static double compute_mean(const std::vector<double>& data) {
    if (data.empty()) return 0.0;
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    return sum / data.size();
}

/**
 * @brief Однофакторный дисперсионный анализ (One-way ANOVA)
 *
 * Реализация по формулам 3.13-3.16 из порядковых статистик
 */
ANOVAResult anova_one_way(const std::vector<std::vector<double>>& groups,
                          double alpha) {
    ANOVAResult result;
    result.alpha = alpha;
    result.num_groups = groups.size();

    // Проверка корректности входных данных
    if (groups.size() < 2) {
        std::cerr << "Ошибка: для ANOVA требуется минимум 2 группы" << std::endl;
        result.reject_h0 = false;
        return result;
    }

    // Проверка, что все группы непустые
    for (const auto& group : groups) {
        if (group.empty()) {
            std::cerr << "Ошибка: обнаружена пустая группа" << std::endl;
            result.reject_h0 = false;
            return result;
        }
    }

    // Вычисляем размеры групп и общее количество наблюдений
    result.total_n = 0;
    result.group_sizes.resize(groups.size());
    for (size_t i = 0; i < groups.size(); ++i) {
        result.group_sizes[i] = groups[i].size();
        result.total_n += groups[i].size();
    }

    // Проверка минимального количества наблюдений
    if (result.total_n <= groups.size()) {
        std::cerr << "Ошибка: недостаточно наблюдений для ANOVA" << std::endl;
        result.reject_h0 = false;
        return result;
    }

    // Вычисляем средние в каждой группе (x̄ᵢ)
    result.group_means.resize(groups.size());
    for (size_t i = 0; i < groups.size(); ++i) {
        result.group_means[i] = compute_mean(groups[i]);
    }

    // Вычисляем общее среднее (x̄)
    // x̄ = (Σᵢ nᵢ x̄ᵢ) / N
    double sum_weighted_means = 0.0;
    for (size_t i = 0; i < groups.size(); ++i) {
        sum_weighted_means += result.group_sizes[i] * result.group_means[i];
    }
    result.grand_mean = sum_weighted_means / result.total_n;

    // Вычисляем межгрупповую сумму квадратов (SS_between)
    // Формула 3.14: SS_between = Σᵢ nᵢ(x̄ᵢ - x̄)²
    result.ss_between = 0.0;
    for (size_t i = 0; i < groups.size(); ++i) {
        double diff = result.group_means[i] - result.grand_mean;
        result.ss_between += result.group_sizes[i] * diff * diff;
    }

    // Вычисляем внутригрупповую сумму квадратов (SS_within)
    // Формула 3.15: SS_within = ΣᵢΣⱼ(xᵢⱼ - x̄ᵢ)²
    result.ss_within = 0.0;
    for (size_t i = 0; i < groups.size(); ++i) {
        for (double value : groups[i]) {
            double diff = value - result.group_means[i];
            result.ss_within += diff * diff;
        }
    }

    // Вычисляем общую сумму квадратов (SS_total)
    // Формула 3.16: SS_total = SS_between + SS_within
    result.ss_total = result.ss_between + result.ss_within;

    // Вычисляем степени свободы
    result.df_between = groups.size() - 1;           // m - 1
    result.df_within = result.total_n - groups.size(); // N - m
    result.df_total = result.total_n - 1;            // N - 1

    // Вычисляем средние квадраты (дисперсии)
    result.ms_between = result.ss_between / result.df_between;
    result.ms_within = result.ss_within / result.df_within;

    // Вычисляем F-статистику
    // Формула 3.13: F = MS_between / MS_within
    if (result.ms_within > 0) {
        result.f_statistic = result.ms_between / result.ms_within;
    } else {
        std::cerr << "Ошибка: внутригрупповая дисперсия равна нулю" << std::endl;
        result.f_statistic = INFINITY;
    }

    // Вычисляем критическое значение F-распределения
    // F_{1-α, df_between, df_within}
    result.critical_value = f_ppf(1.0 - alpha, result.df_between, result.df_within);

    // Вычисляем p-значение
    // P = P(F > F_obs) = 1 - CDF(F_obs)
    result.p_value = 1.0 - f_cdf(result.f_statistic, result.df_between, result.df_within);

    // Проверяем гипотезу H0: если F > F_critical или p < alpha, отвергаем H0
    result.reject_h0 = (result.f_statistic > result.critical_value) ||
                       (result.p_value < alpha);

    return result;
}

/**
 * @brief Вывод результатов ANOVA в консоль и файл
 */
void print_anova_result(const ANOVAResult& result, const std::string& filename) {
    std::ostream* out = &std::cout;
    std::ofstream file;

    if (!filename.empty()) {
        file.open(filename);
        if (file.is_open()) {
            out = &file;
        }
    }

    *out << "============================================================" << std::endl;
    *out << "  ОДНОФАКТОРНЫЙ ДИСПЕРСИОННЫЙ АНАЛИЗ (One-way ANOVA)" << std::endl;
    *out << "============================================================" << std::endl;
    *out << std::endl;

    *out << "Количество групп: m = " << result.num_groups << std::endl;
    *out << "Общее количество наблюдений: N = " << result.total_n << std::endl;
    *out << "Уровень значимости: α = " << result.alpha << std::endl;
    *out << std::endl;

    // Выводим информацию о группах
    *out << "Информация о группах:" << std::endl;
    *out << std::fixed << std::setprecision(6);
    for (size_t i = 0; i < result.num_groups; ++i) {
        *out << "  Группа " << (i + 1) << ": n = " << std::setw(4) << result.group_sizes[i]
             << ", x̄ = " << result.group_means[i] << std::endl;
    }
    *out << "  Общее среднее: x̄ = " << result.grand_mean << std::endl;
    *out << std::endl;

    // Таблица дисперсионного анализа (ANOVA table)
    *out << "Таблица дисперсионного анализа:" << std::endl;
    *out << "------------------------------------------------------------" << std::endl;
    *out << std::left << std::setw(20) << "Источник вариации"
         << std::right << std::setw(12) << "SS"
         << std::setw(8) << "df"
         << std::setw(12) << "MS"
         << std::setw(12) << "F"
         << std::setw(12) << "p-value" << std::endl;
    *out << "------------------------------------------------------------" << std::endl;

    // Межгрупповая вариация
    *out << std::left << std::setw(20) << "Между группами"
         << std::right << std::fixed << std::setprecision(4)
         << std::setw(12) << result.ss_between
         << std::setw(8) << result.df_between
         << std::setw(12) << result.ms_between
         << std::setw(12) << result.f_statistic
         << std::setw(12) << std::setprecision(6) << result.p_value << std::endl;

    // Внутригрупповая вариация
    *out << std::left << std::setw(20) << "Внутри групп"
         << std::right << std::fixed << std::setprecision(4)
         << std::setw(12) << result.ss_within
         << std::setw(8) << result.df_within
         << std::setw(12) << result.ms_within
         << std::setw(12) << ""
         << std::setw(12) << "" << std::endl;

    // Общая вариация
    *out << std::left << std::setw(20) << "Всего"
         << std::right << std::fixed << std::setprecision(4)
         << std::setw(12) << result.ss_total
         << std::setw(8) << result.df_total
         << std::setw(12) << ""
         << std::setw(12) << ""
         << std::setw(12) << "" << std::endl;

    *out << "------------------------------------------------------------" << std::endl;
    *out << std::endl;

    // Результат проверки гипотезы
    *out << std::fixed << std::setprecision(6);
    *out << "F-статистика = " << result.f_statistic << std::endl;
    *out << "Критическое значение F_{" << (1.0 - result.alpha) << ", "
         << result.df_between << ", " << result.df_within << "} = "
         << result.critical_value << std::endl;
    *out << "P-значение = " << std::setprecision(4) << result.p_value << std::endl;
    *out << std::endl;

    *out << "Гипотеза H0: μ₁ = μ₂ = ... = μₘ (средние во всех группах равны)" << std::endl;
    if (result.reject_h0) {
        *out << "РЕЗУЛЬТАТ: H0 ОТВЕРГАЕТСЯ (средние различаются)" << std::endl;
        *out << "F (" << std::setprecision(6) << result.f_statistic
             << ") > F_critical (" << result.critical_value << ")" << std::endl;
        *out << "p-value (" << std::setprecision(4) << result.p_value
             << ") < α (" << result.alpha << ")" << std::endl;
    } else {
        *out << "РЕЗУЛЬТАТ: H0 НЕ ОТВЕРГАЕТСЯ (нет оснований отвергнуть гипотезу о равенстве средних)" << std::endl;
        *out << "F (" << std::setprecision(6) << result.f_statistic
             << ") ≤ F_critical (" << result.critical_value << ")" << std::endl;
        *out << "p-value (" << std::setprecision(4) << result.p_value
             << ") ≥ α (" << result.alpha << ")" << std::endl;
    }
    *out << std::endl;

    if (file.is_open()) {
        std::cout << "Результаты сохранены в файл: " << filename << std::endl;
        file.close();
    }
}
