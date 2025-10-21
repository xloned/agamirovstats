#include "matrix_operations.h"
#include <iostream>
#include <iomanip>

// Создание матрицы
double** createMatrix(int rows, int cols) {
    double** matrix = new double*[rows];
    for (int i = 0; i < rows; i++) {
        matrix[i] = new double[cols];
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = 0.0;
        }
    }
    return matrix;
}

// Транспонирование матрицы
double** TransMatrix(int m, int n, double** a) {
    int i, j;
    double** b;
    b = new double*[n];
    for (i = 0; i < n; i++) b[i] = new double[m];
    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) b[i][j] = a[j][i];
    }
    return b;
}

// Умножение матриц
double** MultiplyMatrix(int rowsa, int colsa, int rowsb, int colsb, double** a, double** b) {
    int i, j, k;
    double t;
    double** c;
    c = new double*[rowsa];
    for (i = 0; i < rowsa; i++) c[i] = new double[colsb];

    if (colsa != rowsb) return 0;
    for (k = 0; k < colsb; k++) {
        for (i = 0; i < rowsa; i++) {
            t = 0;
            for (j = 0; j < rowsb; j++) t += a[i][j] * b[j][k];
            c[i][k] = t;
        }
    }
    return c;
}

// Освобождение памяти
void clearMemory(double** a, int n) {
    for (int i = 0; i < n; i++) {
        delete[] a[i];
    }
    delete[] a;
}

// Обращение матрицы (метод Гаусса)
double** InverseMatrix(double** a, int n) {
    double temp;
    int i, j, k;

    double** e;
    e = new double*[n];
    for (i = 0; i < n; i++) e[i] = new double[n];
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++) {
            e[i][j] = 0;
            if (i == j)
                e[i][j] = 1;
        }

    for (k = 0; k < n; k++) {
        temp = a[k][k];
        for (j = 0; j < n; j++) {
            a[k][j] /= temp;
            e[k][j] /= temp;
        }

        for (i = k + 1; i < n; i++) {
            temp = a[i][k];
            for (j = 0; j < n; j++) {
                a[i][j] -= a[k][j] * temp;
                e[i][j] -= e[k][j] * temp;
            }
        }
    }
    for (k = n - 1; k > 0; k--) {
        for (i = k - 1; i >= 0; i--) {
            temp = a[i][k];
            for (j = 0; j < n; j++) {
                a[i][j] -= a[k][j] * temp;
                e[i][j] -= e[k][j] * temp;
            }
        }
    }
    return e;
}

// Вывод матрицы
void printMatrix(double** a, int rows, int cols, const char* name) {
    std::cout << "\n" << name << " (" << rows << "x" << cols << "):\n";
    std::cout << std::fixed << std::setprecision(6);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            std::cout << std::setw(12) << a[i][j] << " ";
        }
        std::cout << "\n";
    }
}