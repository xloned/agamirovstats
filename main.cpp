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
    cout << "  1. Нормального распределения (полные данные)" << endl;
    cout << "  2. Распределения Вейбулла (полные данные)" << endl;
    cout << "  3. Нормального распределения (цензурированные данные)" << endl;
    cout << "  4. Распределения Вейбулла (цензурированные данные)" << endl;
    cout << "  5. Доверительные интервалы и распределение Стьюдента" << endl;

    // ==================== 1. НОРМАЛЬНОЕ РАСПРЕДЕЛЕНИЕ (полные данные) ====================
    print_separator("1. НОРМАЛЬНОЕ РАСПРЕДЕЛЕНИЕ - ПОЛНЫЕ ДАННЫЕ");

    string normal_file = "input/data_normal.txt";
    vector<double> normal_data = read_data(normal_file);

    if (!normal_data.empty()) {
        print_data_statistics(normal_data, "нормального распределения");

        cout << "\nВыполняется MLE для нормального распределения..." << endl;
        MLEResult result_normal = mle_normal_complete(normal_data);

        print_mle_result(result_normal, "MLE Нормальное распределение");
        save_mle_result(result_normal, "output/mle_normal_complete.txt", normal_data, vector<int>());

        free_mle_result(result_normal);
        cout << "Результаты сохранены в output/mle_normal_complete.txt" << endl;
    } else {
        cerr << "Ошибка: не удалось загрузить данные для нормального распределения" << endl;
    }

    // ==================== 2. РАСПРЕДЕЛЕНИЕ ВЕЙБУЛЛА (полные данные) ====================
    print_separator("2. РАСПРЕДЕЛЕНИЕ ВЕЙБУЛЛА - ПОЛНЫЕ ДАННЫЕ");

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

    // ==================== 3. НОРМАЛЬНОЕ РАСПРЕДЕЛЕНИЕ (цензурированные данные) ====================
    print_separator("3. НОРМАЛЬНОЕ РАСПРЕДЕЛЕНИЕ - ЦЕНЗУРИРОВАННЫЕ ДАННЫЕ");

    string normal_censored_file = "input/data_censored_normal.txt";
    vector<double> normal_censored_data;
    vector<int> normal_censored_ind;
    read_censored_data(normal_censored_file, normal_censored_data, normal_censored_ind);

    if (!normal_censored_data.empty()) {
        print_data_statistics(normal_censored_data, "цензурированных данных (нормальное)");

        cout << "\nВыполняется MLS для нормального распределения с цензурированием..." << endl;
        MLEResult result_normal_cens = mls_normal_censored(normal_censored_data, normal_censored_ind);

        print_mle_result(result_normal_cens, "MLS Нормальное распределение (цензурированные)");
        save_mle_result(result_normal_cens, "output/mls_normal_censored.txt",
                        normal_censored_data, normal_censored_ind);

        free_mle_result(result_normal_cens);
        cout << "Результаты сохранены в output/mls_normal_censored.txt" << endl;
    } else {
        cerr << "Ошибка: не удалось загрузить цензурированные данные (нормальное)" << endl;
    }

    // ==================== 4. РАСПРЕДЕЛЕНИЕ ВЕЙБУЛЛА (цензурированные данные) ====================
    print_separator("4. РАСПРЕДЕЛЕНИЕ ВЕЙБУЛЛА - ЦЕНЗУРИРОВАННЫЕ ДАННЫЕ");

    string weibull_censored_file = "input/data_censored_weibull.txt";
    vector<double> weibull_censored_data;
    vector<int> weibull_censored_ind;
    read_censored_data(weibull_censored_file, weibull_censored_data, weibull_censored_ind);

    if (!weibull_censored_data.empty()) {
        print_data_statistics(weibull_censored_data, "цензурированных данных (Вейбулл)");

        cout << "\nВыполняется MLS для распределения Вейбулла с цензурированием..." << endl;
        MLEResult result_weibull_cens = mls_weibull_censored(weibull_censored_data, weibull_censored_ind);

        print_mle_result(result_weibull_cens, "MLS Распределение Вейбулла (цензурированные)");
        save_mle_result(result_weibull_cens, "output/mls_weibull_censored.txt",
                        weibull_censored_data, weibull_censored_ind);

        free_mle_result(result_weibull_cens);
        cout << "Результаты сохранены в output/mls_weibull_censored.txt" << endl;
    } else {
        cerr << "Ошибка: не удалось загрузить цензурированные данные (Вейбулл)" << endl;
    }

    // ==================== 5. ДОВЕРИТЕЛЬНЫЕ ИНТЕРВАЛЫ ====================
    print_separator("5. ДОВЕРИТЕЛЬНЫЕ ИНТЕРВАЛЫ И РАСПРЕДЕЛЕНИЕ СТЬЮДЕНТА");

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

    cout << "  - Визуализация MLS для распределения Вейбулла..." << endl;
    run_python_script("plot_weibull.py", "mls");

    cout << "  - Визуализация распределения Стьюдента (3 графика)..." << endl;
    run_python_script("plot_t_distribution.py");

    cout << "\nВизуализация завершена!" << endl;
    cout << "Графики сохранены:" << endl;
    cout << "  - output/plot_mle_normal.png" << endl;
    cout << "  - output/plot_mls_normal.png" << endl;
    cout << "  - output/plot_mle_weibull.png" << endl;
    cout << "  - output/plot_mls_weibull.png" << endl;
    cout << "  - output/plot_t_varying_df.png (неизвестная σ)" << endl;
    cout << "  - output/plot_normal_varying_sigma.png (известная σ)" << endl;
    cout << "  - output/plot_chi_squared.png (неизвестное μ)" << endl;

    // ==================== ЗАВЕРШЕНИЕ ====================
    print_separator("ЗАВЕРШЕНИЕ ПРОГРАММЫ");
    cout << "\nВсе расчеты и визуализация завершены успешно!" << endl;
    cout << "Результаты сохранены в директории output/" << endl;
    print_separator();

    return 0;
}
