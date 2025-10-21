// Программа 2: MLE для распределения Вейбулла (полные данные)
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
#include "mle_methods.h"
#include "boost_distributions.h"

// Чтение данных из файла
std::vector<double> read_data(const char* filename) {
    std::vector<double> data;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл " << filename << std::endl;
        return data;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        double value;
        if (iss >> value) {
            data.push_back(value);
        }
    }
    
    file.close();
    return data;
}

int main() {
    std::cout << "========================================\n";
    std::cout << "MLE для распределения Вейбулла\n";
    std::cout << "========================================\n\n";
    
    // Чтение данных
    const char* input_file = "input/data_weibull.txt";
    std::vector<double> data = read_data(input_file);
    
    if (data.empty()) {
        std::cerr << "Ошибка: данные не загружены\n";
        return 1;
    }
    
    std::cout << "Загружено наблюдений: " << data.size() << "\n";
    
    // Вычисление описательной статистики
    double sum = 0.0, min_val = data[0], max_val = data[0];
    for (double x : data) {
        sum += x;
        min_val = std::min(min_val, x);
        max_val = std::max(max_val, x);
    }
    double mean = sum / data.size();
    
    std::cout << "Описательная статистика:\n";
    std::cout << "  Среднее: " << mean << "\n";
    std::cout << "  Минимум: " << min_val << "\n";
    std::cout << "  Максимум: " << max_val << "\n\n";
    
    // Выполнение MLE
    std::cout << "Выполнение оценки методом максимального правдоподобия...\n";
    std::cout << "Оптимизация параметра формы методом Nelder-Mead...\n";
    MLEResult result = mle_weibull_complete(data);
    
    // Вывод результатов
    print_mle_result(result, "MLE для распределения Вейбулла");
    
    // Интерпретация результатов
    double lambda = result.parameters[0];  // параметр масштаба
    double k = result.parameters[1];        // параметр формы
    
    std::cout << "\n========== Интерпретация ==========\n";
    std::cout << "Параметр масштаба: λ = " << lambda 
             << " ± " << result.std_errors[0] << "\n";
    std::cout << "Параметр формы: k = " << k
             << " ± " << result.std_errors[1] << "\n";
    
    // Характеристики распределения
    double weibull_mean = lambda * std::tgamma(1.0 + 1.0/k);
    double weibull_variance = std::pow(lambda, 2) * 
                             (std::tgamma(1.0 + 2.0/k) - std::pow(std::tgamma(1.0 + 1.0/k), 2));
    double weibull_std = std::sqrt(weibull_variance);
    
    std::cout << "\nХарактеристики распределения Вейбулла:\n";
    std::cout << "  Математическое ожидание: " << weibull_mean << "\n";
    std::cout << "  Стандартное отклонение: " << weibull_std << "\n";
    std::cout << "  Медиана: " << lambda * std::pow(std::log(2.0), 1.0/k) << "\n";
    
    // Интерпретация параметра формы
    std::cout << "\nИнтерпретация параметра формы k:\n";
    if (k < 1.0) {
        std::cout << "  k < 1: Убывающая интенсивность отказов (детская смертность)\n";
    } else if (std::abs(k - 1.0) < 0.1) {
        std::cout << "  k ≈ 1: Постоянная интенсивность отказов (экспоненциальное распределение)\n";
    } else {
        std::cout << "  k > 1: Возрастающая интенсивность отказов (старение, износ)\n";
    }
    
    // Сохранение результатов
    const char* output_file = "output/results_mle_weibull.txt";
    std::vector<int> no_censoring(data.size(), 0);
    save_mle_result(result, output_file, data, no_censoring);
    
    // Освобождение памяти
    free_mle_result(result);
    
    std::cout << "\n========================================\n";
    std::cout << "Анализ завершен успешно!\n";
    std::cout << "========================================\n";
    
    return 0;
}