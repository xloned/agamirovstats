// Программа 1: MLE для нормального распределения (полные данные)
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
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
    std::cout << "MLE для нормального распределения\n";
    std::cout << "========================================\n\n";
    
    // Чтение данных
    const char* input_file = "input/data_normal.txt";
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
    MLEResult result = mle_normal_complete(data);
    
    // Вывод результатов
    print_mle_result(result, "MLE для нормального распределения");
    
    // Интерпретация результатов
    std::cout << "\n========== Интерпретация ==========\n";
    std::cout << "Оценка среднего: μ = " << result.parameters[0] 
             << " ± " << result.std_errors[0] << "\n";
    std::cout << "Оценка стандартного отклонения: σ = " << result.parameters[1]
             << " ± " << result.std_errors[1] << "\n";
    
    // Доверительные интервалы (95%)
    double z_critical = norm_ppf(0.975);
    double ci_mean_lower = result.parameters[0] - z_critical * result.std_errors[0];
    double ci_mean_upper = result.parameters[0] + z_critical * result.std_errors[0];
    double ci_std_lower = result.parameters[1] - z_critical * result.std_errors[1];
    double ci_std_upper = result.parameters[1] + z_critical * result.std_errors[1];
    
    std::cout << "\n95% доверительные интервалы:\n";
    std::cout << "  μ: [" << ci_mean_lower << ", " << ci_mean_upper << "]\n";
    std::cout << "  σ: [" << ci_std_lower << ", " << ci_std_upper << "]\n";
    
    // Сохранение результатов
    const char* output_file = "output/results_mle_normal.txt";
    std::vector<int> no_censoring(data.size(), 0);
    save_mle_result(result, output_file, data, no_censoring);
    
    // Освобождение памяти
    free_mle_result(result);
    
    std::cout << "\n========================================\n";
    std::cout << "Анализ завершен успешно!\n";
    std::cout << "========================================\n";
    
    return 0;
}