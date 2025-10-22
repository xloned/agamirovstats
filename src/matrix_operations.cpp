#include "matrix_operations.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <boost/numeric/ublas/lu.hpp>

using namespace boost::numeric::ublas;

/**
 * Создание матрицы заданного размера, инициализированной нулями
 */
Matrix createMatrix(int rows, int cols) {
    return Matrix(rows, cols, 0.0);
}

/**
 * Транспонирование матрицы через Boost.uBLAS
 */
Matrix TransMatrix(const Matrix& a) {
    return trans(a);
}

/**
 * Умножение двух матриц через Boost.uBLAS
 */
Matrix MultiplyMatrix(const Matrix& a, const Matrix& b) {
    if (a.size2() != b.size1()) {
        throw std::runtime_error("Несовместимые размеры матриц для умножения");
    }
    return prod(a, b);
}

/**
 * Обращение матрицы методом LU-разложения через Boost.uBLAS
 * Более устойчивый численно метод, чем Гаусс-Жордан
 */
Matrix InverseMatrix(const Matrix& input) {
    using namespace boost::numeric::ublas;

    // Копируем входную матрицу (LU-разложение изменяет матрицу)
    Matrix a(input);
    size_t n = a.size1();

    if (n != a.size2()) {
        throw std::runtime_error("Матрица должна быть квадратной для обращения");
    }

    // Создаем единичную матрицу для результата
    Matrix inverse = identity_matrix<double>(n);

    // Вектор перестановок для LU-разложения
    permutation_matrix<std::size_t> pm(n);

    // Выполняем LU-разложение
    int res = lu_factorize(a, pm);
    if (res != 0) {
        throw std::runtime_error("Матрица вырожденная, обращение невозможно");
    }

    // Вычисляем обратную матрицу
    lu_substitute(a, pm, inverse);

    return inverse;
}

/**
 * Вывод матрицы на экран
 */
void printMatrix(const Matrix& a, const char* name) {
    std::cout << "\n" << name << " (" << a.size1() << "x" << a.size2() << "):\n";
    std::cout << std::fixed << std::setprecision(6);

    for (size_t i = 0; i < a.size1(); i++) {
        for (size_t j = 0; j < a.size2(); j++) {
            std::cout << std::setw(12) << a(i, j) << " ";
        }
        std::cout << "\n";
    }
}
