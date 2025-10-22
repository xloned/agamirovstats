#ifndef NELDER_MEAD_H
#define NELDER_MEAD_H

#include <vector>
#include <functional>

// Структура для хранения данных оптимизации
struct ne_simp {
    int n;                          // размер выборки
    std::vector<double> p;          // параметры
    std::vector<double> x;          // данные
    std::vector<int> r;             // индикаторы цензурирования (0 - наблюдение, 1 - цензура)
    std::vector<int> nsample;       // размеры подвыборок
};

// Структура для результата оптимизации Nelder-Mead
struct NelderMeadResult {
    std::vector<double> parameters; // оптимальные параметры
    int iterations;                 // количество итераций
    bool converged;                 // флаг сходимости
    double final_value;            // финальное значение целевой функции
};

// Глобальная переменная для передачи данных в функцию оптимизации
extern ne_simp nesm;

// Функция оптимизации методом Nelder-Mead
// Возвращает вектор оптимальных параметров
std::vector<double> neldermead(
    std::vector<double>& x0,                              // начальная точка
    double eps,                                           // точность
    std::function<double(std::vector<double>)> func      // целевая функция
);

// Функция оптимизации методом Nelder-Mead с детальной информацией
NelderMeadResult neldermead_detailed(
    std::vector<double>& x0,                              // начальная точка
    double eps,                                           // точность
    std::function<double(std::vector<double>)> func      // целевая функция
);

#endif // NELDER_MEAD_H