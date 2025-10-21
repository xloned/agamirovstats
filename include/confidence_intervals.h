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

#endif // CONFIDENCE_INTERVALS_H
