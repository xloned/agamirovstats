#ifndef ORDER_H
#define ORDER_H

#include <vector>
#include "matrix_operations.h"

// ========== порядковых статистикские функции для порядковых статистик ==========

/**
 * Вычисление математического ожидания и ковариации порядковых статистик
 * для нормального распределения
 * @param n - размер выборки
 * @param pr, ps - вероятности (обычно i/(n+1), j/(n+1))
 * @param er - математическое ожидание (output)
 * @param vrs - ковариация (output)
 */
void ordern(int n, double pr, double ps, double& er, double& vrs);

/**
 * Вычисление математического ожидания и ковариации порядковых статистик
 * для распределения Вейбулла (в логарифмической шкале)
 */
void orderw(int n, double pr, double ps, double &er, double &vrs);

// ========== Вспомогательные функции для MLS ==========

/**
 * Вычисление эмпирической функции распределения
 */
void cum(int n, const std::vector<double>& x, const std::vector<int>& r, int km,
         std::vector<double>& fcum, std::vector<double>& ycum);

/**
 * Вычисление начальных оценок параметров
 */
void standart(int km, const std::vector<double>& ycum, double& cp, double& cko);

/**
 * Взвешенный МНК (обобщенный метод наименьших квадратов) через Boost.uBLAS
 * @param x - матрица регрессоров (n x k)
 * @param y - вектор наблюдений (n x 1)
 * @param v - ковариационная матрица ошибок (n x n)
 * @param db - выходная ковариационная матрица параметров (k x k)
 * @param b - выходной вектор оценок параметров (k x 1)
 * @param yr - предсказанные значения (n)
 */
void MleastSquare_weight(const Matrix& x, const Matrix& y, const Matrix& v,
                         Matrix& db, Matrix& b, Vector& yr);

#endif // ORDER_H
