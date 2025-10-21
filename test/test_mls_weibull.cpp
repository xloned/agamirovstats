// Программа 4: MLS для распределения Вейбулла (цензурированные данные)
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <cmath>
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
    std::cout << "MLS для распределения Вейбулла\n";
    std::cout << "(цензурированные данные)\n";
    std::cout << "========================================\n\n";
    
    // Чтение данных
    const char* input_file = "input/data_censored_weibull.txt";
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
    
    MLEResult result = mls_weibull_censored(data.values, data.censored);
    
    // Вывод результатов
    print_mle_result(result, "MLS для распределения Вейбулла");
    
    // Интерпретация результатов
    double lambda = result.parameters[0];  // параметр масштаба
    double k = result.parameters[1];        // параметр формы
    
    std::cout << "\n========== Интерпретация ==========\n";
    std::cout << "Параметр масштаба (с учетом цензурирования): λ = " << lambda 
             << " ± " << result.std_errors[0] << "\n";
    std::cout << "Параметр формы: k = " << k
             << " ± " << result.std_errors[1] << "\n";
    
    // Характеристики распределения
    double weibull_mean = lambda * std::tgamma(1.0 + 1.0/k);
    double weibull_variance = std::pow(lambda, 2) * 
                             (std::tgamma(1.0 + 2.0/k) - std::pow(std::tgamma(1.0 + 1.0/k), 2));
    double weibull_std = std::sqrt(weibull_variance);
    double weibull_median = lambda * std::pow(std::log(2.0), 1.0/k);
    
    std::cout << "\nХарактеристики распределения Вейбулла:\n";
    std::cout << "  Математическое ожидание: " << weibull_mean << "\n";
    std::cout << "  Стандартное отклонение: " << weibull_std << "\n";
    std::cout << "  Медиана: " << weibull_median << "\n";
    
    // Интерпретация параметра формы
    std::cout << "\nИнтерпретация параметра формы k:\n";
    if (k < 1.0) {
        std::cout << "  k < 1 (" << k << "): Убывающая интенсивность отказов\n";
        std::cout << "    (детская смертность, ранние дефекты)\n";
    } else if (std::abs(k - 1.0) < 0.1) {
        std::cout << "  k ≈ 1 (" << k << "): Постоянная интенсивность отказов\n";
        std::cout << "    (случайные отказы, экспоненциальное распределение)\n";
    } else {
        std::cout << "  k > 1 (" << k << "): Возрастающая интенсивность отказов\n";
        std::cout << "    (старение, износ, усталость материала)\n";
    }
    
    // Сравнение с наивной оценкой
    std::vector<double> complete_data;
    for (size_t i = 0; i < data.values.size(); ++i) {
        if (data.censored[i] == 0) {
            complete_data.push_back(data.values[i]);
        }
    }
    MLEResult naive_result = mle_weibull_complete(complete_data);
    
    std::cout << "\nСравнение с наивной оценкой (игнорирование цензуры):\n";
    std::cout << "  Наивная оценка λ: " << naive_result.parameters[0] << "\n";
    std::cout << "  MLS оценка λ: " << lambda << "\n";
    std::cout << "  Относительная разница: " 
             << (100.0 * (lambda - naive_result.parameters[0]) / naive_result.parameters[0]) 
             << "%\n";
    std::cout << "  Наивная оценка k: " << naive_result.parameters[1] << "\n";
    std::cout << "  MLS оценка k: " << k << "\n";
    
    // Надежность в важных точках
    std::cout << "\nОценка надежности (вероятность выживания):\n";
    std::vector<double> time_points = {weibull_median * 0.5, weibull_median, 
                                       weibull_median * 1.5, weibull_median * 2.0};
    for (double t : time_points) {
        double reliability = std::exp(-std::pow(t / lambda, k));
        std::cout << "  P(T > " << t << ") = " << reliability << "\n";
    }
    
    // Сохранение результатов
    const char* output_file = "output/results_mls_weibull.txt";
    save_mle_result(result, output_file, data.values, data.censored);
    
    // Освобождение памяти
    free_mle_result(result);
    free_mle_result(naive_result);
    
    std::cout << "\n========================================\n";
    std::cout << "Анализ завершен успешно!\n";
    std::cout << "========================================\n";
    
    return 0;
}