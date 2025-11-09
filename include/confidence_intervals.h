#ifndef CONFIDENCE_INTERVALS_H
#define CONFIDENCE_INTERVALS_H

#include <vector>

// Структура для хранения доверительного интервала
struct ConfidenceInterval {
    double lower;        // Нижняя граница
    double upper;        // Верхняя граница
    double point_est;    // Точечная оценка
    double confidence;   // Уровень доверия (например, 0.95)
    double std_error;    // Стандартная ошибка
};

// Структура для хранения всех доверительных интервалов
struct ConfidenceIntervals {
    ConfidenceInterval mean_known_sigma;     // ДИ для μ при известной σ
    ConfidenceInterval mean_unknown_sigma;   // ДИ для μ при неизвестной σ
    ConfidenceInterval variance;             // ДИ для σ² при неизвестном μ
    ConfidenceInterval sigma;                // ДИ для σ
};

// Структура для персентилей (квантилей)
struct Percentile {
    double p;              // Уровень персентиля (0-1)
    double value;          // Значение персентиля
    double lower;          // Нижняя граница ДИ
    double upper;          // Верхняя граница ДИ
    double confidence;     // Уровень доверия
};

// Структура для набора персентилей
struct Percentiles {
    std::vector<Percentile> percentiles;
    std::string distribution_type;  // "normal" или "weibull"
};

/**
 * Доверительный интервал для среднего при ИЗВЕСТНОЙ σ
 * Использует нормальное распределение N(0,1)
 * Формула: μ ± z_{α/2} × σ/√n
 *
 * @param mean Выборочное среднее
 * @param sigma Известное стандартное отклонение генеральной совокупности
 * @param n Размер выборки
 * @param confidence Уровень доверия (по умолчанию 0.95)
 * @return Доверительный интервал
 */
ConfidenceInterval ci_mean_known_sigma(double mean, double sigma, int n,
                                       double confidence = 0.95);

/**
 * Доверительный интервал для среднего при НЕИЗВЕСТНОЙ σ
 * Использует распределение Стьюдента t(n-1)
 * Формула: μ ± t_{α/2}(n-1) × s/√n
 *
 * @param mean Выборочное среднее
 * @param sigma Выборочное стандартное отклонение
 * @param n Размер выборки
 * @param confidence Уровень доверия (по умолчанию 0.95)
 * @return Доверительный интервал
 */
ConfidenceInterval ci_mean_unknown_sigma(double mean, double sigma, int n,
                                         double confidence = 0.95);

/**
 * Доверительный интервал для дисперсии при НЕИЗВЕСТНОМ μ
 * Использует распределение χ²(n-1)
 * Формула (2.83) из PDF: [(n-1)s²/χ²_{α/2}, (n-1)s²/χ²_{1-α/2}]
 *
 * @param sigma Выборочное стандартное отклонение
 * @param n Размер выборки
 * @param confidence Уровень доверия (по умолчанию 0.95)
 * @return Доверительный интервал для σ²
 */
ConfidenceInterval ci_variance_unknown_mu(double sigma, int n,
                                          double confidence = 0.95);

/**
 * Доверительный интервал для стандартного отклонения при неизвестном μ
 * Вычисляется как sqrt от границ ДИ для дисперсии
 *
 * @param sigma Выборочное стандартное отклонение
 * @param n Размер выборки
 * @param confidence Уровень доверия (по умолчанию 0.95)
 * @return Доверительный интервал для σ
 */
ConfidenceInterval ci_sigma_unknown_mu(double sigma, int n,
                                       double confidence = 0.95);

/**
 * Вычисление всех доверительных интервалов для нормального распределения
 *
 * @param data Выборка данных
 * @param known_sigma Известное σ (если <= 0, то считается неизвестным)
 * @param confidence Уровень доверия (по умолчанию 0.95)
 * @return Структура со всеми доверительными интервалами
 */
ConfidenceIntervals compute_all_confidence_intervals(
    const std::vector<double>& data,
    double known_sigma = -1.0,
    double confidence = 0.95);

/**
 * Вывод доверительных интервалов на экран
 */
void print_confidence_intervals(const ConfidenceIntervals& ci);

/**
 * Сохранение доверительных интервалов в файл для визуализации
 */
void save_confidence_intervals(const ConfidenceIntervals& ci,
                               const char* filename,
                               const std::vector<double>& data,
                               double known_sigma = -1.0);

/**
 * Вычисление персентилей для нормального распределения с доверительными интервалами
 * Использует формулы (2.79), (2.80) из PDF порядковых статистик
 *
 * @param mean Среднее
 * @param sigma Стандартное отклонение
 * @param n Размер выборки
 * @param p_levels Уровни персентилей (например {0.05, 0.1, 0.25, 0.5, 0.75, 0.9, 0.95})
 * @param confidence Уровень доверия (по умолчанию 0.95)
 * @return Структура с персентилями и их ДИ
 */
Percentiles compute_normal_percentiles(double mean, double sigma, int n,
                                       const std::vector<double>& p_levels,
                                       double confidence = 0.95);

/**
 * Вычисление персентилей для распределения Вейбулла с доверительными интервалами
 *
 * @param lambda Параметр масштаба
 * @param k Параметр формы
 * @param n Размер выборки
 * @param p_levels Уровни персентилей
 * @param confidence Уровень доверия (по умолчанию 0.95)
 * @return Структура с персентилями и их ДИ
 */
Percentiles compute_weibull_percentiles(double lambda, double k, int n,
                                        const std::vector<double>& p_levels,
                                        double confidence = 0.95);

/**
 * Вывод персентилей на экран
 */
void print_percentiles(const Percentiles& percentiles);

/**
 * Сохранение персентилей в файл
 */
void save_percentiles(const Percentiles& percentiles, const char* filename);

#endif // CONFIDENCE_INTERVALS_H
