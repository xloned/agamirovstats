#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>
#include <cstdlib>

#include "include/mle_methods.h"
#include "include/nelder_mead.h"
#include "include/boost_distributions.h"
#include "include/matrix_operations.h"
#include "include/confidence_intervals.h"
#include "include/statistical_tests.h"

using namespace std;

// Кроссплатформенное определение путей
#ifdef _WIN32
    #define PATH_SEPARATOR "\\"
    #define PYTHON_CMD "python\\venv\\Scripts\\python.exe"
    #define NULL_DEVICE "NUL"
#else
    #define PATH_SEPARATOR "/"
    #define PYTHON_CMD "python/venv/bin/python"
    #define NULL_DEVICE "/dev/null"
#endif

// Функция для кроссплатформенного выполнения Python скриптов
int run_python_script(const string& script, const string& args = "") {
    string command;

#ifdef _WIN32
    // Windows: используем cd /d для смены диска и директории
    command = "cd /d python && .." PATH_SEPARATOR PYTHON_CMD " " + script;
    if (!args.empty()) {
        command += " " + args;
    }
    command += " 2>" NULL_DEVICE;
#else
    // Unix/Linux/macOS
    command = "cd python && ../" PYTHON_CMD " " + script;
    if (!args.empty()) {
        command += " " + args;
    }
    command += " 2>" NULL_DEVICE;
#endif

    return system(command.c_str());
}

// Функция для чтения данных из файла (только значения)
vector<double> read_data(const string& filename) {
    vector<double> data;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Ошибка: не удалось открыть файл " << filename << endl;
        return data;
    }

    double value;
    while (file >> value) {
        data.push_back(value);
    }

    file.close();
    cout << "Загружено " << data.size() << " значений из " << filename << endl;
    return data;
}

// Функция для чтения цензурированных данных (значение и индикатор)
void read_censored_data(const string& filename, vector<double>& data, vector<int>& censored) {
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Ошибка: не удалось открыть файл " << filename << endl;
        return;
    }

    double value;
    int cens;
    while (file >> value >> cens) {
        data.push_back(value);
        censored.push_back(cens);
    }

    file.close();
    cout << "Загружено " << data.size() << " значений из " << filename;
    cout << " (цензурировано: " << count(censored.begin(), censored.end(), 1) << ")" << endl;
}

// Функция для вывода разделителя
void print_separator(const string& title = "") {
    cout << "\n" << string(80, '=') << endl;
    if (!title.empty()) {
        cout << "  " << title << endl;
        cout << string(80, '=') << endl;
    }
}

// Функция для вывода статистики данных
void print_data_statistics(const vector<double>& data, const string& name) {
    if (data.empty()) return;

    double sum = 0;
    for (double x : data) sum += x;
    double mean = sum / data.size();

    double variance = 0;
    for (double x : data) {
        variance += (x - mean) * (x - mean);
    }
    variance /= data.size();
    double std_dev = sqrt(variance);

    double min_val = data[0], max_val = data[0];
    for (double x : data) {
        if (x < min_val) min_val = x;
        if (x > max_val) max_val = x;
    }

    cout << "\nСтатистика для " << name << ":" << endl;
    cout << "  Размер выборки: " << data.size() << endl;
    cout << "  Среднее:        " << fixed << setprecision(4) << mean << endl;
    cout << "  Ст. отклонение: " << std_dev << endl;
    cout << "  Минимум:        " << min_val << endl;
    cout << "  Максимум:       " << max_val << endl;
}

// Главная функция
int main() {
    cout << "\n";
    print_separator("СИСТЕМА АВТОМАТИЧЕСКОЙ ОЦЕНКИ ПАРАМЕТРОВ РАСПРЕДЕЛЕНИЙ");
    cout << "\nПрограмма выполняет оценку параметров для:" << endl;
    cout << "  1. Нормального распределения - MLE (полные данные)" << endl;
    cout << "  2. Нормального распределения - MLS через метод Дэйвида (полные данные)" << endl;
    cout << "  3. Распределения Вейбулла - MLE (полные данные)" << endl;
    cout << "  4. Статистические критерии (Граббса, Фишера, Стьюдента)" << endl;
    cout << "  5. Доверительные интервалы и персентили" << endl;

    // ==================== 1. НОРМАЛЬНОЕ РАСПРЕДЕЛЕНИЕ MLE (полные данные) ====================
    print_separator("1. НОРМАЛЬНОЕ РАСПРЕДЕЛЕНИЕ - MLE (ПОЛНЫЕ ДАННЫЕ)");

    string normal_file = "input/data_normal.txt";
    vector<double> normal_data = read_data(normal_file);

    if (!normal_data.empty()) {
        print_data_statistics(normal_data, "нормального распределения");

        cout << "\nВыполняется MLE для нормального распределения..." << endl;
        MLEResult result_normal_mle = mle_normal_complete(normal_data);

        print_mle_result(result_normal_mle, "MLE Нормальное распределение");
        save_mle_result(result_normal_mle, "output/mle_normal_complete.txt", normal_data, vector<int>());

        free_mle_result(result_normal_mle);
        cout << "Результаты сохранены в output/mle_normal_complete.txt" << endl;
    } else {
        cerr << "Ошибка: не удалось загрузить данные для нормального распределения" << endl;
    }

    // ==================== 2. НОРМАЛЬНОЕ РАСПРЕДЕЛЕНИЕ MLS (полные данные) ====================
    print_separator("2. НОРМАЛЬНОЕ РАСПРЕДЕЛЕНИЕ - MLS МЕТОД (ПОЛНЫЕ ДАННЫЕ)");

    if (!normal_data.empty()) {
        cout << "\nВыполняется MLS (метод Дэйвида - ordern) для нормального распределения..." << endl;
        MLEResult result_normal_mls = mls_normal_complete(normal_data);

        print_mle_result(result_normal_mls, "MLS Нормальное распределение (метод Дэйвида)");
        save_mle_result(result_normal_mls, "output/mls_normal_complete.txt", normal_data, vector<int>());

        free_mle_result(result_normal_mls);
        cout << "Результаты сохранены в output/mls_normal_complete.txt" << endl;
    }

    // ==================== 3. РАСПРЕДЕЛЕНИЕ ВЕЙБУЛЛА (полные данные) ====================
    print_separator("3. РАСПРЕДЕЛЕНИЕ ВЕЙБУЛЛА - MLE (ПОЛНЫЕ ДАННЫЕ)");

    string weibull_file = "input/data_weibull.txt";
    vector<double> weibull_data = read_data(weibull_file);

    if (!weibull_data.empty()) {
        print_data_statistics(weibull_data, "распределения Вейбулла");

        cout << "\nВыполняется MLE для распределения Вейбулла..." << endl;
        MLEResult result_weibull = mle_weibull_complete(weibull_data);

        print_mle_result(result_weibull, "MLE Распределение Вейбулла");
        save_mle_result(result_weibull, "output/mle_weibull_complete.txt", weibull_data, vector<int>());

        free_mle_result(result_weibull);
        cout << "Результаты сохранены в output/mle_weibull_complete.txt" << endl;
    } else {
        cerr << "Ошибка: не удалось загрузить данные для распределения Вейбулла" << endl;
    }

    // ==================== 4. СТАТИСТИЧЕСКИЕ КРИТЕРИИ ====================
    print_separator("4. СТАТИСТИЧЕСКИЕ КРИТЕРИИ");

    // 4.1 Критерий Граббса для выявления выбросов
    cout << "\n4.1 КРИТЕРИЙ ГРАББСА ДЛЯ ВЫЯВЛЕНИЯ ВЫБРОСОВ" << endl;
    cout << string(60, '-') << endl;

    if (!normal_data.empty()) {
        cout << "\nПроверка данных нормального распределения на наличие выбросов..." << endl;

        // Двусторонний критерий (проверяет оба экстремума)
        GrubbsTestResult grubbs_result = grubbs_test(normal_data, 0.05);
        print_grubbs_result(grubbs_result, "output/grubbs_test_normal.txt");

        if (grubbs_result.is_outlier) {
            cout << "\n⚠️  ВНИМАНИЕ: Обнаружен потенциальный выброс!" << endl;
            cout << "   Рекомендуется проверить значение x[" << grubbs_result.outlier_index
                 << "] = " << grubbs_result.outlier_value << endl;
        } else {
            cout << "\n✓ Выбросы не обнаружены (α = 0.05)" << endl;
        }
    }

    // 4.2 F-критерий Фишера и t-критерий Стьюдента
    cout << "\n\n4.2 СРАВНЕНИЕ ДВУХ ВЫБОРОК" << endl;
    cout << string(60, '-') << endl;

    // Создадим две подвыборки для демонстрации (первая и вторая половины)
    if (normal_data.size() >= 10) {
        size_t mid = normal_data.size() / 2;
        vector<double> sample1(normal_data.begin(), normal_data.begin() + mid);
        vector<double> sample2(normal_data.begin() + mid, normal_data.end());

        cout << "\nДля демонстрации разделим данные на две подвыборки:" << endl;
        cout << "  Выборка 1: первые " << sample1.size() << " наблюдений" << endl;
        cout << "  Выборка 2: последние " << sample2.size() << " наблюдений" << endl;

        // F-критерий для сравнения дисперсий
        cout << "\n--- F-критерий Фишера (сравнение дисперсий) ---" << endl;
        FisherTestResult fisher_result = fisher_test(sample1, sample2, 0.05);
        print_fisher_result(fisher_result, "output/fisher_test.txt");

        // t-критерий для равных дисперсий
        cout << "\n--- t-критерий Стьюдента для РАВНЫХ дисперсий ---" << endl;
        StudentTestResult student_equal = student_test_equal_var(sample1, sample2, 0.05);
        print_student_result(student_equal, "output/student_test_equal_var.txt");

        // t-критерий для неравных дисперсий (Уэлча)
        cout << "\n--- t-критерий Стьюдента для НЕРАВНЫХ дисперсий (Уэлч) ---" << endl;
        StudentTestResult student_unequal = student_test_unequal_var(sample1, sample2, 0.05);
        print_student_result(student_unequal, "output/student_test_unequal_var.txt");

        // Автоматический выбор критерия
        cout << "\n--- АВТОМАТИЧЕСКИЙ ВЫБОР (с предварительным F-тестом) ---" << endl;
        StudentTestResult student_auto = student_test_auto(sample1, sample2, 0.05);
        print_student_result(student_auto, "output/student_test_auto.txt");

        // Итоговые рекомендации
        cout << "\n📊 ИТОГОВЫЕ ВЫВОДЫ:" << endl;
        if (fisher_result.reject_h0) {
            cout << "  • Дисперсии различаются → используйте критерий Уэлча" << endl;
            cout << "  • Рекомендуемый результат: " << (student_unequal.reject_h0 ? "средние различаются" : "средние не различаются") << endl;
        } else {
            cout << "  • Дисперсии равны → используйте классический t-критерий" << endl;
            cout << "  • Рекомендуемый результат: " << (student_equal.reject_h0 ? "средние различаются" : "средние не различаются") << endl;
        }
    }

    // ==================== 5. ДОВЕРИТЕЛЬНЫЕ ИНТЕРВАЛЫ ====================
    print_separator("5. ДОВЕРИТЕЛЬНЫЕ ИНТЕРВАЛЫ И ПЕРСЕНТИЛИ");

    // Используем данные нормального распределения для вычисления доверительных интервалов
    if (!normal_data.empty()) {
        cout << "\nВычисление доверительных интервалов для нормального распределения..." << endl;
        cout << "Используются данные из: " << normal_file << endl;

        // Вычисляем все доверительные интервалы
        // Первый случай: известная σ (для демонстрации используем выборочную σ как "известную")
        // Второй случай: неизвестная σ (стандартный случай)
        // Третий случай: неизвестное μ (доверительный интервал для σ²)
        ConfidenceIntervals ci = compute_all_confidence_intervals(normal_data);

        // Вывод результатов на экран
        print_confidence_intervals(ci);

        // Сохранение результатов в файл
        save_confidence_intervals(ci, "output/confidence_intervals.txt", normal_data);

        cout << "\nДоверительные интервалы сохранены в output/confidence_intervals.txt" << endl;

        // Вычисление персентилей для нормального распределения
        cout << "\nВычисление персентилей для нормального распределения..." << endl;

        // Вычисляем выборочные характеристики
        double mean = 0.0;
        for (double x : normal_data) mean += x;
        mean /= normal_data.size();

        double variance = 0.0;
        for (double x : normal_data) {
            variance += (x - mean) * (x - mean);
        }
        double sigma = sqrt(variance / (normal_data.size() - 1));

        vector<double> p_levels = {0.01, 0.05, 0.10, 0.25, 0.50, 0.75, 0.90, 0.95, 0.99};
        Percentiles normal_perc = compute_normal_percentiles(mean, sigma, normal_data.size(), p_levels);

        print_percentiles(normal_perc);
        save_percentiles(normal_perc, "output/percentiles_normal.txt");

    } else {
        cerr << "Ошибка: нет данных для вычисления доверительных интервалов" << endl;
    }

    // Вычисление персентилей для распределения Вейбулла
    if (!weibull_data.empty()) {
        cout << "\nВычисление персентилей для распределения Вейбулла..." << endl;

        // Получаем параметры из результата MLE
        MLEResult result_weibull = mle_weibull_complete(weibull_data);
        double lambda = result_weibull.parameters[0];
        double k = result_weibull.parameters[1];

        vector<double> p_levels = {0.01, 0.05, 0.10, 0.25, 0.50, 0.75, 0.90, 0.95, 0.99};
        Percentiles weibull_perc = compute_weibull_percentiles(lambda, k, weibull_data.size(), p_levels);

        print_percentiles(weibull_perc);
        save_percentiles(weibull_perc, "output/percentiles_weibull.txt");

        free_mle_result(result_weibull);
    }

    // ==================== ГЕНЕРАЦИЯ ВИЗУАЛИЗАЦИИ ====================
    print_separator("ГЕНЕРАЦИЯ ВИЗУАЛИЗАЦИИ");
    cout << "\nСоздание графиков..." << endl;
    cout << "  - Визуализация MLE для нормального распределения..." << endl;
    run_python_script("plot_normal.py", "mle");

    cout << "  - Визуализация MLS для нормального распределения..." << endl;
    run_python_script("plot_normal.py", "mls");

    cout << "  - Визуализация MLE для распределения Вейбулла..." << endl;
    run_python_script("plot_weibull.py", "mle");

    cout << "  - Визуализация распределения Стьюдента (3 графика)..." << endl;
    run_python_script("plot_t_distribution.py");

    cout << "  - Визуализация результатов t-критерия Стьюдента..." << endl;
    run_python_script("plot_student.py");

    cout << "\nВизуализация завершена!" << endl;
    cout << "Графики сохранены:" << endl;
    cout << "  - output/plot_mle_normal.png" << endl;
    cout << "  - output/plot_mls_normal.png" << endl;
    cout << "  - output/plot_mle_weibull.png" << endl;
    cout << "  - output/plot_t_varying_df.png (неизвестная σ)" << endl;
    cout << "  - output/plot_normal_varying_sigma.png (известная σ)" << endl;
    cout << "  - output/plot_chi_squared.png (неизвестное μ)" << endl;
    cout << "  - output/plot_student_equal_var.png (t-тест, равные дисперсии)" << endl;
    cout << "  - output/plot_student_unequal_var.png (t-тест, неравные дисперсии)" << endl;
    cout << "  - output/plot_student_auto.png (t-тест, автоматический выбор)" << endl;

    // ==================== ЗАВЕРШЕНИЕ ====================
    print_separator("ЗАВЕРШЕНИЕ ПРОГРАММЫ");
    cout << "\nВсе расчеты и визуализация завершены успешно!" << endl;
    cout << "Результаты сохранены в директории output/" << endl;
    print_separator();

    return 0;
}
