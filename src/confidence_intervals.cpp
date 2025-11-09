#include "confidence_intervals.h"
#include "boost_distributions.h"
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <numeric>

// Вычисление среднего
static double compute_mean(const std::vector<double>& data) {
    return std::accumulate(data.begin(), data.end(), 0.0) / data.size();
}

// Вычисление выборочного стандартного отклонения
static double compute_std(const std::vector<double>& data) {
    double mean = compute_mean(data);
    double sum_sq = 0.0;
    for (double x : data) {
        sum_sq += (x - mean) * (x - mean);
    }
    return std::sqrt(sum_sq / (data.size() - 1));
}

ConfidenceInterval ci_mean_known_sigma(double mean, double sigma, int n,
                                       double confidence) {
    ConfidenceInterval ci;
    ci.point_est = mean;
    ci.confidence = confidence;

    // Стандартная ошибка среднего
    ci.std_error = sigma / std::sqrt(n);

    // Квантиль стандартного нормального распределения
    double alpha = 1.0 - confidence;
    double z = norm_ppf(1.0 - alpha / 2.0);  // z_{α/2}

    // Доверительный интервал: μ ± z_{α/2} × σ/√n
    ci.lower = mean - z * ci.std_error;
    ci.upper = mean + z * ci.std_error;

    return ci;
}

ConfidenceInterval ci_mean_unknown_sigma(double mean, double sigma, int n,
                                         double confidence) {
    ConfidenceInterval ci;
    ci.point_est = mean;
    ci.confidence = confidence;

    // Стандартная ошибка среднего
    ci.std_error = sigma / std::sqrt(n);

    // Квантиль распределения Стьюдента с (n-1) степенями свободы
    double alpha = 1.0 - confidence;
    int df = n - 1;
    double t = t_ppf(1.0 - alpha / 2.0, df);  // t_{α/2}(n-1)

    // Доверительный интервал: μ ± t_{α/2}(n-1) × s/√n
    ci.lower = mean - t * ci.std_error;
    ci.upper = mean + t * ci.std_error;

    return ci;
}

ConfidenceInterval ci_variance_unknown_mu(double sigma, int n,
                                          double confidence) {
    ConfidenceInterval ci;
    double variance = sigma * sigma;
    ci.point_est = variance;
    ci.confidence = confidence;
    ci.std_error = 0.0;  // Для дисперсии стандартная ошибка вычисляется сложнее

    // Квантили распределения χ² с (n-1) степенями свободы
    double alpha = 1.0 - confidence;
    int df = n - 1;

    // Формула (2.83) из PDF: [(n-1)s²/χ²_{α/2}, (n-1)s²/χ²_{1-α/2}]
    double chi_upper = chi_ppf(1.0 - alpha / 2.0, df);  // χ²_{α/2}
    double chi_lower = chi_ppf(alpha / 2.0, df);         // χ²_{1-α/2}

    // Доверительный интервал для дисперсии
    ci.lower = (df * variance) / chi_upper;
    ci.upper = (df * variance) / chi_lower;

    return ci;
}

ConfidenceInterval ci_sigma_unknown_mu(double sigma, int n,
                                       double confidence) {
    // Сначала вычисляем ДИ для дисперсии
    ConfidenceInterval ci_var = ci_variance_unknown_mu(sigma, n, confidence);

    // Затем берем квадратный корень от границ
    ConfidenceInterval ci;
    ci.point_est = sigma;
    ci.confidence = confidence;
    ci.std_error = 0.0;
    ci.lower = std::sqrt(ci_var.lower);
    ci.upper = std::sqrt(ci_var.upper);

    return ci;
}

ConfidenceIntervals compute_all_confidence_intervals(
    const std::vector<double>& data,
    double known_sigma,
    double confidence) {

    ConfidenceIntervals result;
    int n = data.size();

    // Вычисляем выборочные характеристики
    double mean = compute_mean(data);
    double sigma = compute_std(data);

    // 1. ДИ для среднего при известной σ
    if (known_sigma > 0) {
        result.mean_known_sigma = ci_mean_known_sigma(mean, known_sigma, n, confidence);
    } else {
        // Если σ неизвестна, используем выборочную оценку для демонстрации
        result.mean_known_sigma = ci_mean_known_sigma(mean, sigma, n, confidence);
    }

    // 2. ДИ для среднего при неизвестной σ
    result.mean_unknown_sigma = ci_mean_unknown_sigma(mean, sigma, n, confidence);

    // 3. ДИ для дисперсии при неизвестном μ
    result.variance = ci_variance_unknown_mu(sigma, n, confidence);

    // 4. ДИ для σ при неизвестном μ
    result.sigma = ci_sigma_unknown_mu(sigma, n, confidence);

    return result;
}

void print_confidence_intervals(const ConfidenceIntervals& ci) {
    std::cout << "\n========================================\n";
    std::cout << "ДОВЕРИТЕЛЬНЫЕ ИНТЕРВАЛЫ\n";
    std::cout << "========================================\n";
    std::cout << std::fixed << std::setprecision(4);

    std::cout << "\n1. ДИ для среднего μ при ИЗВЕСТНОЙ σ:\n";
    std::cout << "   Оценка: " << ci.mean_known_sigma.point_est << "\n";
    std::cout << "   " << (ci.mean_known_sigma.confidence * 100) << "% ДИ: ["
              << ci.mean_known_sigma.lower << ", "
              << ci.mean_known_sigma.upper << "]\n";
    std::cout << "   Ширина: " << (ci.mean_known_sigma.upper - ci.mean_known_sigma.lower) << "\n";
    std::cout << "   Метод: Нормальное распределение N(0,1)\n";

    std::cout << "\n2. ДИ для среднего μ при НЕИЗВЕСТНОЙ σ:\n";
    std::cout << "   Оценка: " << ci.mean_unknown_sigma.point_est << "\n";
    std::cout << "   " << (ci.mean_unknown_sigma.confidence * 100) << "% ДИ: ["
              << ci.mean_unknown_sigma.lower << ", "
              << ci.mean_unknown_sigma.upper << "]\n";
    std::cout << "   Ширина: " << (ci.mean_unknown_sigma.upper - ci.mean_unknown_sigma.lower) << "\n";
    std::cout << "   Метод: Распределение Стьюдента t(df)\n";

    std::cout << "\n3. ДИ для дисперсии σ² при НЕИЗВЕСТНОМ μ:\n";
    std::cout << "   Оценка: " << ci.variance.point_est << "\n";
    std::cout << "   " << (ci.variance.confidence * 100) << "% ДИ: ["
              << ci.variance.lower << ", "
              << ci.variance.upper << "]\n";
    std::cout << "   Ширина: " << (ci.variance.upper - ci.variance.lower) << "\n";
    std::cout << "   Метод: Распределение χ²(df)\n";

    std::cout << "\n4. ДИ для стандартного отклонения σ:\n";
    std::cout << "   Оценка: " << ci.sigma.point_est << "\n";
    std::cout << "   " << (ci.sigma.confidence * 100) << "% ДИ: ["
              << ci.sigma.lower << ", "
              << ci.sigma.upper << "]\n";
    std::cout << "   Ширина: " << (ci.sigma.upper - ci.sigma.lower) << "\n";

    std::cout << "\n========================================\n";

    // Сравнение ширины интервалов
    double width_known = ci.mean_known_sigma.upper - ci.mean_known_sigma.lower;
    double width_unknown = ci.mean_unknown_sigma.upper - ci.mean_unknown_sigma.lower;
    double ratio = width_unknown / width_known;

    std::cout << "\nСРАВНЕНИЕ:\n";
    std::cout << "Ширина ДИ (известная σ):   " << width_known << "\n";
    std::cout << "Ширина ДИ (неизвестная σ): " << width_unknown << "\n";
    std::cout << "Отношение: " << ratio << " раз\n";
    std::cout << "\nИнтервал при неизвестной σ шире на "
              << ((ratio - 1.0) * 100) << "%\n";
    std::cout << "========================================\n\n";
}

void save_confidence_intervals(const ConfidenceIntervals& ci,
                               const char* filename,
                               const std::vector<double>& data,
                               double known_sigma) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << "\n";
        return;
    }

    file << std::fixed << std::setprecision(6);

    // Заголовок
    file << "# Доверительные интервалы для нормального распределения\n";
    file << "# Уровень доверия: " << (ci.mean_known_sigma.confidence * 100) << "%\n";
    file << "# Размер выборки: " << data.size() << "\n";
    file << "#\n";

    // Выборочные статистики
    double mean = compute_mean(data);
    double sigma = compute_std(data);
    file << "sample_mean " << mean << "\n";
    file << "sample_std " << sigma << "\n";
    file << "sample_size " << data.size() << "\n";

    if (known_sigma > 0) {
        file << "known_sigma " << known_sigma << "\n";
    }
    file << "\n";

    // ДИ для среднего при известной σ
    file << "# Доверительный интервал для μ при известной σ\n";
    file << "ci_mean_known_sigma_lower " << ci.mean_known_sigma.lower << "\n";
    file << "ci_mean_known_sigma_upper " << ci.mean_known_sigma.upper << "\n";
    file << "ci_mean_known_sigma_width "
         << (ci.mean_known_sigma.upper - ci.mean_known_sigma.lower) << "\n";
    file << "\n";

    // ДИ для среднего при неизвестной σ
    file << "# Доверительный интервал для μ при неизвестной σ\n";
    file << "ci_mean_unknown_sigma_lower " << ci.mean_unknown_sigma.lower << "\n";
    file << "ci_mean_unknown_sigma_upper " << ci.mean_unknown_sigma.upper << "\n";
    file << "ci_mean_unknown_sigma_width "
         << (ci.mean_unknown_sigma.upper - ci.mean_unknown_sigma.lower) << "\n";
    file << "\n";

    // ДИ для дисперсии
    file << "# Доверительный интервал для σ² при неизвестном μ\n";
    file << "ci_variance_lower " << ci.variance.lower << "\n";
    file << "ci_variance_upper " << ci.variance.upper << "\n";
    file << "ci_variance_point " << ci.variance.point_est << "\n";
    file << "\n";

    // ДИ для σ
    file << "# Доверительный интервал для σ\n";
    file << "ci_sigma_lower " << ci.sigma.lower << "\n";
    file << "ci_sigma_upper " << ci.sigma.upper << "\n";
    file << "ci_sigma_point " << ci.sigma.point_est << "\n";
    file << "\n";

    // Данные для визуализации распределений Стьюдента
    file << "# Параметры для визуализации t-распределения\n";
    file << "df " << (data.size() - 1) << "\n";
    file << "confidence " << ci.mean_known_sigma.confidence << "\n";

    file.close();
    std::cout << "Доверительные интервалы сохранены: " << filename << "\n";
}

// ============ ПЕРСЕНТИЛИ (КВАНТИЛИ) ============

Percentiles compute_normal_percentiles(double mean, double sigma, int n,
                                       const std::vector<double>& p_levels,
                                       double confidence) {
    Percentiles result;
    result.distribution_type = "normal";

    double alpha = 1.0 - confidence;
    int df = n - 1;

    // Для каждого уровня персентиля
    for (double p : p_levels) {
        Percentile perc;
        perc.p = p;
        perc.confidence = confidence;

        // Квантиль нормального распределения
        double z_p = norm_ppf(p);
        perc.value = mean + z_p * sigma;

        // Доверительный интервал для персентиля при неизвестной σ
        // Используем формулы (2.79), (2.80) из PDF порядковых статистик
        // x_p = μ + z_p*σ
        // ДИ: x̄_p ± t_{α/2}(n-1) * σ̄ * sqrt(1 + z_p²/(2*(n-1)))

        double t_crit = t_ppf(1.0 - alpha / 2.0, df);
        double se = sigma * std::sqrt(1.0 / n + z_p * z_p / (2.0 * df));

        perc.lower = perc.value - t_crit * se;
        perc.upper = perc.value + t_crit * se;

        result.percentiles.push_back(perc);
    }

    return result;
}

Percentiles compute_weibull_percentiles(double lambda, double k, int n,
                                        const std::vector<double>& p_levels,
                                        double confidence) {
    Percentiles result;
    result.distribution_type = "weibull";

    double alpha = 1.0 - confidence;

    // Для каждого уровня персентиля
    for (double p : p_levels) {
        Percentile perc;
        perc.p = p;
        perc.confidence = confidence;

        // Квантиль распределения Вейбулла
        // x_p = λ * (-ln(1-p))^(1/k)
        perc.value = lambda * std::pow(-std::log(1.0 - p), 1.0 / k);

        // Доверительный интервал для персентиля Вейбулла
        // Используем дельта-метод для приближенного ДИ
        // Var(x_p) ≈ (∂x_p/∂λ)² * Var(λ) + (∂x_p/∂k)² * Var(k)

        double var_lambda = (lambda * lambda) / (n * k * k);
        double var_k = 1.644 * (k * k) / n;

        // Частные производные
        double w = -std::log(1.0 - p);
        double w_pow = std::pow(w, 1.0 / k);

        // ∂x_p/∂λ = w^(1/k)
        double dx_dlambda = w_pow;

        // ∂x_p/∂k = -λ * w^(1/k) * ln(w) / k²
        double dx_dk = -lambda * w_pow * std::log(w) / (k * k);

        // Variance of percentile
        double var_xp = dx_dlambda * dx_dlambda * var_lambda +
                        dx_dk * dx_dk * var_k;

        double se_xp = std::sqrt(var_xp);

        // Используем нормальное приближение для ДИ
        double z_crit = norm_ppf(1.0 - alpha / 2.0);
        perc.lower = perc.value - z_crit * se_xp;
        perc.upper = perc.value + z_crit * se_xp;

        // Ограничение: персентиль не может быть отрицательным
        if (perc.lower < 0) perc.lower = 0;

        result.percentiles.push_back(perc);
    }

    return result;
}

void print_percentiles(const Percentiles& percentiles) {
    std::cout << "\n========================================\n";
    std::cout << "ПЕРСЕНТИЛИ (КВАНТИЛИ) - " << percentiles.distribution_type << "\n";
    std::cout << "========================================\n";
    std::cout << std::fixed << std::setprecision(4);

    std::cout << "\n" << std::setw(10) << "Уровень"
              << std::setw(12) << "Значение"
              << std::setw(25) << "95% ДИ"
              << std::setw(12) << "Ширина" << "\n";
    std::cout << std::string(60, '-') << "\n";

    for (const auto& p : percentiles.percentiles) {
        std::cout << std::setw(9) << (p.p * 100) << "%"
                  << std::setw(12) << p.value
                  << "   [" << std::setw(8) << p.lower << ", " << std::setw(8) << p.upper << "]"
                  << std::setw(12) << (p.upper - p.lower) << "\n";
    }

    std::cout << "========================================\n\n";
}

void save_percentiles(const Percentiles& percentiles, const char* filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << "\n";
        return;
    }

    file << std::fixed << std::setprecision(6);

    file << "# Персентили (квантили) для " << percentiles.distribution_type << "\n";
    file << "# Уровень доверия: 95%\n";
    file << "#\n";
    file << "distribution_type " << percentiles.distribution_type << "\n";
    file << "n_percentiles " << percentiles.percentiles.size() << "\n";
    file << "\n";

    file << "# p value lower upper width\n";
    for (const auto& p : percentiles.percentiles) {
        file << p.p << " "
             << p.value << " "
             << p.lower << " "
             << p.upper << " "
             << (p.upper - p.lower) << "\n";
    }

    file.close();
    std::cout << "Персентили сохранены: " << filename << "\n";
}
