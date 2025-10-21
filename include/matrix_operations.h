#ifndef MATRIX_OPERATIONS_H
#define MATRIX_OPERATIONS_H

// Операции с матрицами используя двумерные массивы

// Обращение матрицы (метод Гаусса)
double** InverseMatrix(double** a, int n);

// Транспонирование матрицы
double** TransMatrix(int m, int n, double** a);

// Умножение матриц
double** MultiplyMatrix(int rowsa, int colsa, int rowsb, int colsb, double** a, double** b);

// Освобождение памяти
void clearMemory(double** a, int n);

// Вывод матрицы
void printMatrix(double** a, int rows, int cols, const char* name);

// Создание матрицы
double** createMatrix(int rows, int cols);

#endif // MATRIX_OPERATIONS_H