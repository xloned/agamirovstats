// Программа 3: MLS для нормального распределения (цензурированные данные)
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "mle_methods.h"
#include "boost_distributions.h"

// Структура для хранения данных
struct CensoredData {
    std::vector<double> values;
    std::vector<int> censored;  // 0 - наблюдение, 1 - цензура
};

// Чтение цензурированных данных из файла
CensoredData read_censored_data(const char* filename) {
    CensoredData data;
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
        int censored;
        if (iss >> value >> censored) {
            data.values.push_back(value);
            data.censored.push_back(censored);
        }
    }
    
    file.close();
    return data;
}

int main() {
    std::cout << "========================================\n";
    std::cout << "MLS для нормального распределения\n";
    std::cout << "(цензурированные данные)\n";
    std::cout << "========================================\n\n";
    
    // Чтение данных
    const char* input_file = "input/data_censored_normal.txt";
    CensoredData data = read_censored_data(input_file);
    
    if (data.values.empty()) {
        std::cerr << "Ошибка: данные не загружены\n";
        return 1;
    }
    
    // Подсчет цензурированных и полных наблюдений
    int n_complete = 0;
    int n_censored = 0;
    for (int c : data.censored) {
        if (c == 0) n_complete++;
        else n_censored++;
    }
    
    std::cout << "Загружено наблюдений: " << data.values.size() << "\n";
    std::cout << "  Полных наблюдений: " << n_complete << "\n";
    std::cout << "  Цензурированных: " << n_censored << "\n";
    std::cout << "  Процент цензурирования: " 
             << (100.0 * n_censored / data.values.size()) << "%\n\n";
    
    // Описательная статистика для полных наблюдений
    double sum = 0.0;
    double min_val = 1e10, max_val = -1e10;
    for (size_t i = 0; i < data.values.size(); ++i) {
        if (data.censored[i] == 0) {
            sum += data.values[i];
            min_val = std::min(min_val, data.values[i]);
            max_val = std::max(max_val, data.values[i]);
        }
    }
    double mean = sum / n_complete;
    
    std::cout << "Описательная статистика (полные наблюдения):\n";
    std::cout << "  Среднее: " << mean << "\n";
    std::cout << "  Минимум: " << min_val << "\n";
    std::cout << "  Максимум: " << max_val << "\n\n";
    
    // Выполнение MLS
    std::cout << "Выполнение оценки методом максимального правдоподобия\n";
    std::cout << "с учетом цензурирования...\n";
    std::cout << "Оптимизация параметров методом Nelder-Mead...\n\n";
    
    MLEResult result = mls_normal_censored(data.values, data.censored);
    
    // Вывод результатов
    print_mle_result(result, "MLS для нормального распределения");
    
    // Интерпретация результатов
    std::cout << "\n========== Интерпретация ==========\n";
    std::cout << "Оценка среднего (с учетом цензурирования): μ = " << result.parameters[0] 
             << " ± " << result.std_errors[0] << "\n";
    std::cout << "Оценка стандартного отклонения: σ = " << result.parameters[1]
             << " ± " << result.std_errors[1] << "\n";
    
    // Сравнение с наивной оценкой (только полные наблюдения)
    std::vector<double> complete_data;
    for (size_t i = 0; i < data.values.size(); ++i) {
        if (data.censored[i] == 0) {
            complete_data.push_back(data.values[i]);
        }
    }
    MLEResult naive_result = mle_normal_complete(complete_data);
    
    std::cout << "\nСравнение с наивной оценкой (игнорирование цензуры):\n";
    std::cout << "  Наивная оценка μ: " << naive_result.parameters[0] << "\n";
    std::cout << "  MLS оценка μ: " << result.parameters[0] << "\n";
    std::cout << "  Разница: " << (result.parameters[0] - naive_result.parameters[0]) << "\n";
    
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
    const char* output_file = "output/results_mls_normal.txt";
    save_mle_result(result, output_file, data.values, data.censored);
    
    // Освобождение памяти
    free_mle_result(result);
    free_mle_result(naive_result);
    
    std::cout << "\n========================================\n";
    std::cout << "Анализ завершен успешно!\n";
    std::cout << "========================================\n";
    
    return 0;
}