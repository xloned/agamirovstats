#ifndef MATRIX_OPERATIONS_H
#define MATRIX_OPERATIONS_H

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/io.hpp>

namespace ublas = boost::numeric::ublas;

// Типы для матриц и векторов через Boost.uBLAS
typedef ublas::matrix<double> Matrix;
typedef ublas::vector<double> Vector;

/**
 * Создание матрицы заданного размера, инициализированной нулями
 * @param rows - количество строк
 * @param cols - количество столбцов
 * @return матрица размером rows x cols
 */
Matrix createMatrix(int rows, int cols);

/**
 * Транспонирование матрицы
 * @param a - исходная матрица размером m x n
 * @return транспонированная матрица размером n x m
 */
Matrix TransMatrix(const Matrix& a);

/**
 * Умножение двух матриц
 * @param a - первая матрица
 * @param b - вторая матрица
 * @return произведение матриц a * b
 */
Matrix MultiplyMatrix(const Matrix& a, const Matrix& b);

/**
 * Обращение квадратной матрицы методом Гаусса-Жордана
 * @param a - исходная квадратная матрица
 * @return обратная матрица
 */
Matrix InverseMatrix(const Matrix& a);

/**
 * Вывод матрицы на экран
 * @param a - матрица для вывода
 * @param name - имя матрицы для отображения
 */
void printMatrix(const Matrix& a, const char* name);

/**
 * Устаревшая функция для совместимости со старым кодом
 * Теперь ничего не делает, так как Boost.uBLAS управляет памятью автоматически
 */
inline void clearMemory(double** a, int n) {
    // Пустая функция для обратной совместимости
    // Boost.uBLAS автоматически управляет памятью
}

#endif // MATRIX_OPERATIONS_H
