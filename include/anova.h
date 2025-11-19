#ifndef ANOVA_H
#define ANOVA_H

#include <vector>
#include <string>

/**
 * @brief Результат однофакторного дисперсионного анализа (One-way ANOVA)
 *
 * Критерий используется для проверки гипотезы о равенстве средних значений
 * в нескольких группах (порядковые статистики, раздел 3.7, формулы 3.13-3.16)
 */
struct ANOVAResult {
    double f_statistic;        // Значение F-статистики
    double critical_value;     // Критическое значение F-распределения
    double p_value;            // P-значение теста
    bool reject_h0;            // true, если отвергаем H0: μ₁ = μ₂ = ... = μₘ

    double ss_between;         // Межгрупповая сумма квадратов (SS_between)
    double ss_within;          // Внутригрупповая сумма квадратов (SS_within)
    double ss_total;           // Общая сумма квадратов (SS_total)

    double ms_between;         // Межгрупповая дисперсия (MS_between = SS_between / df1)
    double ms_within;          // Внутригрупповая дисперсия (MS_within = SS_within / df2)

    size_t df_between;         // Степени свободы числителя (m - 1)
    size_t df_within;          // Степени свободы знаменателя (N - m)
    size_t df_total;           // Общие степени свободы (N - 1)

    size_t num_groups;         // Количество групп (m)
    size_t total_n;            // Общее количество наблюдений (N)
    std::vector<size_t> group_sizes; // Размеры групп (n₁, n₂, ..., nₘ)
    std::vector<double> group_means; // Средние в группах (x̄₁, x̄₂, ..., x̄ₘ)
    double grand_mean;         // Общее среднее (x̄)

    double alpha;              // Уровень значимости
};

/**
 * @brief Однофакторный дисперсионный анализ (One-way ANOVA)
 *
 * Проверяет гипотезу H0: μ₁ = μ₂ = ... = μₘ (средние во всех группах равны)
 * против H1: хотя бы одно среднее отличается
 *
 * Статистика (формула 3.13 из порядковых статистик):
 * F = MS_between / MS_within = (SS_between / (m-1)) / (SS_within / (N-m))
 *
 * где:
 * - SS_between = Σᵢ nᵢ(x̄ᵢ - x̄)² - межгрупповая сумма квадратов (формула 3.14)
 * - SS_within = ΣᵢΣⱼ(xᵢⱼ - x̄ᵢ)² - внутригрупповая сумма квадратов (формула 3.15)
 * - SS_total = SS_between + SS_within (формула 3.16)
 * - m - количество групп
 * - N - общее количество наблюдений
 * - nᵢ - размер i-той группы
 * - x̄ᵢ - среднее i-той группы
 * - x̄ - общее среднее
 *
 * Распределение под H0: F ~ F(m-1, N-m)
 *
 * @param groups Вектор групп (каждая группа - вектор наблюдений)
 * @param alpha Уровень значимости (по умолчанию 0.05)
 * @return Результат теста с подробной информацией
 *
 * Применимость:
 * - Данные в каждой группе должны быть из нормального распределения
 * - Дисперсии в группах должны быть примерно равны (гомоскедастичность)
 * - Наблюдения независимы
 */
ANOVAResult anova_one_way(const std::vector<std::vector<double>>& groups,
                          double alpha = 0.05);

/**
 * @brief Вывод результатов ANOVA в консоль и файл
 *
 * Выводит подробную таблицу дисперсионного анализа в стандартном формате:
 * Source of Variation | SS | df | MS | F | p-value
 * Between Groups      | ... | ... | ... | ... | ...
 * Within Groups       | ... | ... | ... |
 * Total               | ... | ... |
 *
 * @param result Результат ANOVA
 * @param filename Имя файла для сохранения (если пустое, выводится только в консоль)
 */
void print_anova_result(const ANOVAResult& result, const std::string& filename = "");

#endif // ANOVA_H
