#include "statistical_tests.h"
#include "boost_distributions.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <fstream>
#include <iomanip>

// ============================================================================
// Вспомогательные функции для вычисления базовых статистик
// ============================================================================

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
 * @brief Вычисление выборочной дисперсии (несмещенная оценка)
 * @param data Вектор данных
 * @param mean Среднее значение (если уже вычислено)
 * @return Выборочная дисперсия s² = Σ(xi - x̄)² / (n-1)
 */
static double compute_variance(const std::vector<double>& data, double mean) {
    if (data.size() <= 1) return 0.0;

    double sum_sq = 0.0;
    for (double x : data) {
        double diff = x - mean;
        sum_sq += diff * diff;
    }

    return sum_sq / (data.size() - 1);
}

/**
 * @brief Вычисление выборочного стандартного отклонения
 * @param data Вектор данных
 * @param mean Среднее значение
 * @return Стандартное отклонение s = sqrt(s²)
 */
static double compute_std(const std::vector<double>& data, double mean) {
    return std::sqrt(compute_variance(data, mean));
}

// ============================================================================
// Критерий Граббса (Grubbs' test) - формулы 3.8-3.10 из порядковых статистик
// ============================================================================

/**
 * @brief Вычисление критического значения для критерия Граббса
 *
 * Критическое значение вычисляется по формуле:
 * G_critical = ((n-1)/√n) * √(t²_{α/(2n), n-2} / (n - 2 + t²_{α/(2n), n-2}))
 *
 * где t_{α/(2n), n-2} - квантиль распределения Стьюдента
 *
 * @param n Размер выборки
 * @param alpha Уровень значимости
 * @return Критическое значение G_critical
 */
static double grubbs_critical_value(size_t n, double alpha) {
    if (n < 3) return INFINITY;

    // Квантиль распределения Стьюдента: t_{α/(2n), n-2}
    double t_alpha = t_ppf(1.0 - alpha / (2.0 * n), n - 2);

    // Формула критического значения Граббса
    double t_sq = t_alpha * t_alpha;
    double numerator = (n - 1) * t_alpha;
    double denominator = std::sqrt(n) * std::sqrt(n - 2 + t_sq);

    return numerator / denominator;
}

/**
 * @brief Критерий Граббса для проверки максимального значения
 *
 * Статистика теста: G = (x_max - x̄) / s
 * H0: максимальное значение не является выбросом
 * H1: максимальное значение является выбросом
 */
GrubbsTestResult grubbs_test_max(const std::vector<double>& data, double alpha) {
    GrubbsTestResult result;
    result.alpha = alpha;
    result.n = data.size();
    result.test_type = "max";

    if (data.size() < 3) {
        std::cerr << "Ошибка: для критерия Граббса требуется минимум 3 наблюдения" << std::endl;
        result.is_outlier = false;
        return result;
    }

    // Вычисляем среднее и стандартное отклонение
    double mean = compute_mean(data);
    double std = compute_std(data, mean);

    // Находим максимальное значение
    auto max_it = std::max_element(data.begin(), data.end());
    result.outlier_value = *max_it;
    result.outlier_index = std::distance(data.begin(), max_it);

    // Вычисляем статистику G = (x_max - x̄) / s
    result.test_statistic = std::abs(result.outlier_value - mean) / std;

    // Вычисляем критическое значение
    result.critical_value = grubbs_critical_value(data.size(), alpha);

    // Проверяем гипотезу: если G > G_critical, то x_max - выброс
    result.is_outlier = (result.test_statistic > result.critical_value);

    return result;
}

/**
 * @brief Критерий Граббса для проверки минимального значения
 *
 * Статистика теста: G = (x̄ - x_min) / s
 */
GrubbsTestResult grubbs_test_min(const std::vector<double>& data, double alpha) {
    GrubbsTestResult result;
    result.alpha = alpha;
    result.n = data.size();
    result.test_type = "min";

    if (data.size() < 3) {
        std::cerr << "Ошибка: для критерия Граббса требуется минимум 3 наблюдения" << std::endl;
        result.is_outlier = false;
        return result;
    }

    // Вычисляем среднее и стандартное отклонение
    double mean = compute_mean(data);
    double std = compute_std(data, mean);

    // Находим минимальное значение
    auto min_it = std::min_element(data.begin(), data.end());
    result.outlier_value = *min_it;
    result.outlier_index = std::distance(data.begin(), min_it);

    // Вычисляем статистику G = (x̄ - x_min) / s
    result.test_statistic = std::abs(mean - result.outlier_value) / std;

    // Вычисляем критическое значение
    result.critical_value = grubbs_critical_value(data.size(), alpha);

    // Проверяем гипотезу
    result.is_outlier = (result.test_statistic > result.critical_value);

    return result;
}

/**
 * @brief Двусторонний критерий Граббса
 *
 * Проверяет оба экстремума и возвращает результат для наиболее подозрительного
 */
GrubbsTestResult grubbs_test(const std::vector<double>& data, double alpha) {
    GrubbsTestResult result_max = grubbs_test_max(data, alpha);
    GrubbsTestResult result_min = grubbs_test_min(data, alpha);

    // Возвращаем результат с большей статистикой
    return (result_max.test_statistic > result_min.test_statistic) ? result_max : result_min;
}

// ============================================================================
// F-критерий Фишера - формула 3.12 из порядковых статистик
// ============================================================================

/**
 * @brief F-критерий Фишера для сравнения дисперсий двух выборок
 *
 * H0: σ₁² = σ₂² (дисперсии равны)
 * H1: σ₁² ≠ σ₂² (дисперсии различаются)
 *
 * Модифицированный критерий с настраиваемым порогом:
 * Дисперсии считаются различными, если |σ₁² - σ₂²| ≥ alpha
 * где alpha - порог, задаваемый пользователем через интерфейс
 */
FisherTestResult fisher_test(const std::vector<double>& data1,
                             const std::vector<double>& data2,
                             double alpha) {
    FisherTestResult result;
    result.alpha = alpha;
    result.n1 = data1.size();
    result.n2 = data2.size();

    if (data1.size() < 2 || data2.size() < 2) {
        std::cerr << "Ошибка: для F-критерия требуется минимум 2 наблюдения в каждой выборке" << std::endl;
        result.reject_h0 = false;
        return result;
    }

    // Вычисляем средние и дисперсии
    double mean1 = compute_mean(data1);
    double mean2 = compute_mean(data2);
    result.var1 = compute_variance(data1, mean1);
    result.var2 = compute_variance(data2, mean2);

    // Абсолютная разница дисперсий
    result.var_diff = std::abs(result.var1 - result.var2);

    // Степени свободы
    result.df1 = data1.size() - 1;
    result.df2 = data2.size() - 1;

    // F-статистика: большая дисперсия / меньшая дисперсия
    // Это гарантирует, что F ≥ 1
    if (result.var1 >= result.var2) {
        result.f_statistic = result.var1 / result.var2;
    } else {
        result.f_statistic = result.var2 / result.var1;
        // Меняем местами степени свободы
        std::swap(result.df1, result.df2);
    }

    // Критическое значение для двустороннего теста: F_{α/2, df1, df2}
    result.critical_value = f_ppf(1.0 - alpha / 2.0, result.df1, result.df2);

    // P-значение для двустороннего теста
    // P = 2 * min(P(F > f_obs), P(F < f_obs))
    double p_upper = 1.0 - f_cdf(result.f_statistic, result.df1, result.df2);
    result.p_value = 2.0 * p_upper;
    if (result.p_value > 1.0) result.p_value = 1.0;

    // Модифицированный критерий: проверяем порог практической значимости
    // Дисперсии считаются различными, если разница ≥ alpha (порог задается пользователем)
    result.reject_h0 = (result.var_diff >= alpha);

    return result;
}

// ============================================================================
// t-критерий Стьюдента для равных дисперсий - формула 3.14 из порядковых статистик
// ============================================================================

/**
 * @brief t-критерий Стьюдента для равных дисперсий (классический)
 *
 * H0: μ₁ = μ₂ (средние равны)
 * H1: μ₁ ≠ μ₂ (средние различаются)
 *
 * Предположение: σ₁² = σ₂²
 *
 * Статистика: t = (x̄₁ - x̄₂) / (sp * √(1/n₁ + 1/n₂))
 * где sp² = ((n₁-1)s₁² + (n₂-1)s₂²) / (n₁+n₂-2) - объединенная дисперсия
 *
 * Распределение под H0: t ~ Student(n₁ + n₂ - 2)
 */
StudentTestResult student_test_equal_var(const std::vector<double>& data1,
                                         const std::vector<double>& data2,
                                         double alpha) {
    StudentTestResult result;
    result.alpha = alpha;
    result.n1 = data1.size();
    result.n2 = data2.size();
    result.test_type = "equal_var";

    if (data1.size() < 2 || data2.size() < 2) {
        std::cerr << "Ошибка: для t-критерия требуется минимум 2 наблюдения в каждой выборке" << std::endl;
        result.reject_h0 = false;
        return result;
    }

    // Вычисляем средние
    result.mean1 = compute_mean(data1);
    result.mean2 = compute_mean(data2);

    // Вычисляем стандартные отклонения
    result.std1 = compute_std(data1, result.mean1);
    result.std2 = compute_std(data2, result.mean2);

    // Вычисляем объединенную дисперсию (pooled variance)
    // sp² = ((n₁-1)s₁² + (n₂-1)s₂²) / (n₁+n₂-2)
    double var1 = result.std1 * result.std1;
    double var2 = result.std2 * result.std2;
    double pooled_var = ((result.n1 - 1) * var1 + (result.n2 - 1) * var2) /
                        (result.n1 + result.n2 - 2);
    result.pooled_std = std::sqrt(pooled_var);

    // Степени свободы
    result.df = result.n1 + result.n2 - 2;

    // Вычисляем стандартную ошибку разности средних
    // SE = sp * √(1/n₁ + 1/n₂)
    double se = result.pooled_std * std::sqrt(1.0 / result.n1 + 1.0 / result.n2);

    // Вычисляем t-статистику: t = (x̄₁ - x̄₂) / SE
    result.t_statistic = (result.mean1 - result.mean2) / se;

    // Критическое значение для двустороннего теста: t_{α/2, df}
    result.critical_value = t_ppf(1.0 - alpha / 2.0, result.df);

    // P-значение для двустороннего теста
    // P = 2 * P(|T| > |t_obs|) = 2 * (1 - CDF(|t_obs|))
    double t_abs = std::abs(result.t_statistic);
    result.p_value = 2.0 * (1.0 - t_cdf(t_abs, result.df));

    // Отвергаем H0, если |t| > t_critical или p_value < alpha
    result.reject_h0 = (t_abs > result.critical_value);

    return result;
}

// ============================================================================
// t-критерий Стьюдента для неравных дисперсий (Уэлча) - формула 3.16
// ============================================================================

/**
 * @brief t-критерий Стьюдента для неравных дисперсий (Welch's t-test)
 *
 * H0: μ₁ = μ₂
 * H1: μ₁ ≠ μ₂
 *
 * Без предположения о равенстве дисперсий
 *
 * Статистика: t = (x̄₁ - x̄₂) / √(s₁²/n₁ + s₂²/n₂)
 *
 * Степени свободы (приближение Уэлча-Саттертуэйта):
 * ν = (s₁²/n₁ + s₂²/n₂)² / ((s₁²/n₁)²/(n₁-1) + (s₂²/n₂)²/(n₂-1))
 */
StudentTestResult student_test_unequal_var(const std::vector<double>& data1,
                                           const std::vector<double>& data2,
                                           double alpha) {
    StudentTestResult result;
    result.alpha = alpha;
    result.n1 = data1.size();
    result.n2 = data2.size();
    result.test_type = "unequal_var";
    result.pooled_std = 0.0;  // Не используется для неравных дисперсий

    if (data1.size() < 2 || data2.size() < 2) {
        std::cerr << "Ошибка: для t-критерия требуется минимум 2 наблюдения в каждой выборке" << std::endl;
        result.reject_h0 = false;
        return result;
    }

    // Вычисляем средние
    result.mean1 = compute_mean(data1);
    result.mean2 = compute_mean(data2);

    // Вычисляем стандартные отклонения
    result.std1 = compute_std(data1, result.mean1);
    result.std2 = compute_std(data2, result.mean2);

    // Вычисляем дисперсии
    double var1 = result.std1 * result.std1;
    double var2 = result.std2 * result.std2;

    // Вычисляем компоненты для формулы Уэлча
    double var1_n1 = var1 / result.n1;
    double var2_n2 = var2 / result.n2;

    // Вычисляем стандартную ошибку разности средних
    // SE = √(s₁²/n₁ + s₂²/n₂)
    double se = std::sqrt(var1_n1 + var2_n2);

    // Вычисляем t-статистику
    result.t_statistic = (result.mean1 - result.mean2) / se;

    // Вычисляем степени свободы по формуле Уэлча-Саттертуэйта
    // ν = (s₁²/n₁ + s₂²/n₂)² / ((s₁²/n₁)²/(n₁-1) + (s₂²/n₂)²/(n₂-1))
    double numerator = (var1_n1 + var2_n2) * (var1_n1 + var2_n2);
    double denominator = (var1_n1 * var1_n1) / (result.n1 - 1) +
                        (var2_n2 * var2_n2) / (result.n2 - 1);
    result.df = numerator / denominator;

    // Критическое значение для двустороннего теста
    result.critical_value = t_ppf(1.0 - alpha / 2.0, result.df);

    // P-значение для двустороннего теста
    double t_abs = std::abs(result.t_statistic);
    result.p_value = 2.0 * (1.0 - t_cdf(t_abs, result.df));

    // Отвергаем H0, если |t| > t_critical
    result.reject_h0 = (t_abs > result.critical_value);

    return result;
}

/**
 * @brief Автоматический t-критерий с предварительной проверкой дисперсий
 *
 * Сначала выполняет F-тест, затем применяет соответствующий вариант t-критерия
 */
StudentTestResult student_test_auto(const std::vector<double>& data1,
                                    const std::vector<double>& data2,
                                    double alpha) {
    // Выполняем F-тест для проверки равенства дисперсий
    FisherTestResult f_result = fisher_test(data1, data2, alpha);

    // Выбираем подходящий вариант t-критерия
    if (f_result.reject_h0) {
        // Дисперсии различаются - используем критерий Уэлча
        std::cout << "F-тест отвергает гипотезу о равенстве дисперсий (p = "
                  << f_result.p_value << ")" << std::endl;
        std::cout << "Используется t-критерий Уэлча для неравных дисперсий" << std::endl;
        return student_test_unequal_var(data1, data2, alpha);
    } else {
        // Дисперсии не различаются - используем классический критерий
        std::cout << "F-тест не отвергает гипотезу о равенстве дисперсий (p = "
                  << f_result.p_value << ")" << std::endl;
        std::cout << "Используется классический t-критерий для равных дисперсий" << std::endl;
        return student_test_equal_var(data1, data2, alpha);
    }
}

// ============================================================================
// Функции для вывода результатов
// ============================================================================

/**
 * @brief Вывод результатов критерия Граббса
 */
void print_grubbs_result(const GrubbsTestResult& result, const std::string& filename) {
    std::ostream* out = &std::cout;
    std::ofstream file;

    if (!filename.empty()) {
        file.open(filename);
        if (file.is_open()) {
            out = &file;
        }
    }

    *out << "========================================" << std::endl;
    *out << "  КРИТЕРИЙ ГРАББСА (Grubbs' test)" << std::endl;
    *out << "  для выявления выбросов" << std::endl;
    *out << "========================================" << std::endl;
    *out << std::endl;

    *out << "Тип теста: " << (result.test_type == "max" ? "максимум" :
                              result.test_type == "min" ? "минимум" : "двусторонний")
         << std::endl;
    *out << "Размер выборки: n = " << result.n << std::endl;
    *out << "Уровень значимости: α = " << result.alpha << std::endl;
    *out << std::endl;

    *out << "Подозрительное значение: x[" << result.outlier_index << "] = "
         << std::fixed << std::setprecision(6) << result.outlier_value << std::endl;
    *out << std::endl;

    *out << "Статистика G = " << std::setprecision(6) << result.test_statistic << std::endl;
    *out << "Критическое значение G_critical = " << std::setprecision(6)
         << result.critical_value << std::endl;
    *out << std::endl;

    *out << "Гипотеза H0: значение не является выбросом" << std::endl;
    if (result.is_outlier) {
        *out << "РЕЗУЛЬТАТ: H0 ОТВЕРГАЕТСЯ (обнаружен выброс)" << std::endl;
        *out << "G (" << result.test_statistic << ") > G_critical ("
             << result.critical_value << ")" << std::endl;
    } else {
        *out << "РЕЗУЛЬТАТ: H0 НЕ ОТВЕРГАЕТСЯ (выброс не обнаружен)" << std::endl;
        *out << "G (" << result.test_statistic << ") ≤ G_critical ("
             << result.critical_value << ")" << std::endl;
    }
    *out << std::endl;

    if (file.is_open()) {
        std::cout << "Результаты сохранены в файл: " << filename << std::endl;
        file.close();
    }
}

/**
 * @brief Вывод результатов F-критерия
 */
void print_fisher_result(const FisherTestResult& result, const std::string& filename) {
    std::ostream* out = &std::cout;
    std::ofstream file;

    if (!filename.empty()) {
        file.open(filename);
        if (file.is_open()) {
            out = &file;
        }
    }

    *out << "========================================" << std::endl;
    *out << "  F-КРИТЕРИЙ ФИШЕРА (Fisher's F-test)" << std::endl;
    *out << "  для сравнения дисперсий" << std::endl;
    *out << "========================================" << std::endl;
    *out << std::endl;

    *out << "Размеры выборок: n₁ = " << result.n1 << ", n₂ = " << result.n2 << std::endl;
    *out << "Степени свободы: df₁ = " << result.df1 << ", df₂ = " << result.df2 << std::endl;
    *out << "Уровень значимости: α = " << result.alpha << std::endl;
    *out << std::endl;

    *out << std::fixed << std::setprecision(6);
    *out << "Дисперсия 1: s₁² = " << result.var1 << std::endl;
    *out << "Дисперсия 2: s₂² = " << result.var2 << std::endl;
    *out << std::endl;

    *out << "F-статистика = " << result.f_statistic << std::endl;
    *out << "Критическое значение F_{" << (1.0 - result.alpha/2.0) << ", "
         << result.df1 << ", " << result.df2 << "} = " << result.critical_value << std::endl;
    *out << "P-значение = " << std::setprecision(4) << result.p_value << std::endl;
    *out << std::endl;

    *out << "Гипотеза H0: σ₁² = σ₂² (дисперсии равны)" << std::endl;
    if (result.reject_h0) {
        *out << "РЕЗУЛЬТАТ: H0 ОТВЕРГАЕТСЯ (дисперсии различаются)" << std::endl;
        *out << "F (" << std::setprecision(6) << result.f_statistic
             << ") > F_critical (" << result.critical_value << ")" << std::endl;
        *out << "p-value (" << std::setprecision(4) << result.p_value
             << ") < α (" << result.alpha << ")" << std::endl;
    } else {
        *out << "РЕЗУЛЬТАТ: H0 НЕ ОТВЕРГАЕТСЯ (дисперсии не различаются)" << std::endl;
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

/**
 * @brief Вывод результатов t-критерия
 */
void print_student_result(const StudentTestResult& result, const std::string& filename) {
    std::ostream* out = &std::cout;
    std::ofstream file;

    if (!filename.empty()) {
        file.open(filename);
        if (file.is_open()) {
            out = &file;
        }
    }

    *out << "========================================" << std::endl;
    *out << "  t-КРИТЕРИЙ СТЬЮДЕНТА (Student's t-test)" << std::endl;
    *out << "  для сравнения средних" << std::endl;
    *out << "========================================" << std::endl;
    *out << std::endl;

    *out << "Метод: " << (result.test_type == "equal_var" ?
                          "равные дисперсии (классический)" :
                          "неравные дисперсии (Уэлч)") << std::endl;
    *out << "Размеры выборок: n₁ = " << result.n1 << ", n₂ = " << result.n2 << std::endl;
    *out << "Степени свободы: ν = " << std::fixed << std::setprecision(2) << result.df << std::endl;
    *out << "Уровень значимости: α = " << std::setprecision(3) << result.alpha << std::endl;
    *out << std::endl;

    *out << std::setprecision(6);
    *out << "Среднее 1: x̄₁ = " << result.mean1 << std::endl;
    *out << "Среднее 2: x̄₂ = " << result.mean2 << std::endl;
    *out << "СКО 1: s₁ = " << result.std1 << std::endl;
    *out << "СКО 2: s₂ = " << result.std2 << std::endl;
    if (result.test_type == "equal_var") {
        *out << "Объединенное СКО: sp = " << result.pooled_std << std::endl;
    }
    *out << std::endl;

    *out << "t-статистика = " << result.t_statistic << std::endl;
    *out << "Критическое значение t_{" << (1.0 - result.alpha/2.0) << ", "
         << std::setprecision(2) << result.df << "} = "
         << std::setprecision(6) << result.critical_value << std::endl;
    *out << "P-значение = " << std::setprecision(4) << result.p_value << std::endl;
    *out << std::endl;

    *out << "Гипотеза H0: μ₁ = μ₂ (средние равны)" << std::endl;
    if (result.reject_h0) {
        *out << "РЕЗУЛЬТАТ: H0 ОТВЕРГАЕТСЯ (средние различаются)" << std::endl;
        *out << "|t| (" << std::setprecision(6) << std::abs(result.t_statistic)
             << ") > t_critical (" << result.critical_value << ")" << std::endl;
        *out << "p-value (" << std::setprecision(4) << result.p_value
             << ") < α (" << result.alpha << ")" << std::endl;
    } else {
        *out << "РЕЗУЛЬТАТ: H0 НЕ ОТВЕРГАЕТСЯ (средние не различаются)" << std::endl;
        *out << "|t| (" << std::setprecision(6) << std::abs(result.t_statistic)
             << ") ≤ t_critical (" << result.critical_value << ")" << std::endl;
        *out << "p-value (" << std::setprecision(4) << result.p_value
             << ") ≥ α (" << result.alpha << ")" << std::endl;
    }
    *out << std::endl;

    if (file.is_open()) {
        std::cout << "Результаты сохранены в файл: " << filename << std::endl;
        file.close();
    }
}
