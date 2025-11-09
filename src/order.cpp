#include "order.h"
#include "boost_distributions.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>

// порядковых статистикские функции для вычисления математического ожидания и ковариации
// порядковых статистик нормального распределения

// ============ ordern - для нормального распределения ============
// Вычисляет математическое ожидание (er) и ковариацию (vrs) порядковых статистик
// n - размер выборки
// pr, ps - вероятности (r/(n+1), s/(n+1))
void ordern(int n, double pr, double ps, double& er, double& vrs) {
    double p, pr1, ps1, xr, xr1, xr2, xr3, xr4, xr5, xr6, dr, qr, qs;
    double xs1, xs2, xs3, xs4, xs5, xs6, ds, xs;
    double z1, z2, z3, z4, z5, z6, z7;

    p = 1;
    xr = norm_ppf(pr);
    xs = norm_ppf(ps);
    qr = 1. - pr;
    qs = 1. - ps;
    pr1 = pr * p;
    ps1 = ps * p;

    dr = norm_pdf(xr);
    ds = norm_pdf(xs);

    xr1 = p / dr;
    xr2 = xr * xr1 * xr1;
    xr3 = (2. * xr * xr + 1.) * pow((p / dr), 3);
    xr4 = (6. * xr * xr * xr + 7. * xr) * pow((p / dr), 4);
    xr5 = (24. * pow(xr, 4) + 46. * xr * xr + 7.) * pow((p / dr), 5);
    xr6 = (120. * pow(xr, 5) + 326. * xr * xr * xr + 127. * xr) * pow((p / dr), 6);

    xs1 = p / ds;
    xs2 = xs * xs1 * xs1;
    xs3 = (2. * xs * xs + 1.) * pow((p / ds), 3);
    xs4 = (6. * xs * xs * xs + 7. * xs) * pow((p / ds), 4);
    xs5 = (24. * pow(xs, 4) + 46. * xs * xs + 7.) * pow((p / ds), 5);
    xs6 = (120. * pow(xs, 5) + 326. * xs * xs * xs + 127. * xs) * pow(p / ds, 6);

    // Математическое ожидание r-ой порядковой статистики
    er = xr + pr * qr * xr2 / (2. * (n + 2.)) +
         pr * qr * ((qr - pr) * xr3 / 3. + pr * qr * xr4 / 8.) / pow((n + 2.), 2) +
         pr * qr * (-(qr - pr) * xr3 / 3. + (pow((qr - pr), 2) - pr * qr) * xr4 / 4. +
                    qr * pr * (qr - pr) * xr5 / 6. + pow((qr * pr), 2) * xr6 / 48.) / pow((n + 2.), 3);

    // Ковариация r-ой и s-ой порядковых статистик
    z1 = (qr - pr) * xr2 * xs1 + (qs - ps) * xr1 * xs2 + pr * qr * xr3 * xs1 / 2. +
         ps * qs * xr1 * xs3 / 2. + pr * qs * xr2 * xs2 / 2.;
    z1 = z1 * pr * qs / pow((n + 2.), 2);

    z2 = -(qr - pr) * xr2 * xs1 - (qs - ps) * xr1 * xs2 + (pow((qr - pr), 2) - pr * qr) * xr3 * xs1;
    z3 = (pow((qs - ps), 2) - ps * qs) * xr1 * xs3 + (1.5 * (qr - pr) * (qs - ps) + 0.5 * ps * qr - 2. * pr * qs) * xr2 * xs2;
    z4 = (5. / 6.) * pr * qr * (qr - pr) * xr4 * xs1 + (5. / 6.) * ps * qs * (qs - ps) * xr1 * xs4 +
         (pr * qs * (qr - pr) + .5 * pr * qr * (qs - ps)) * xr3 * xs2;
    z5 = (pr * qs * (qs - ps) + 0.5 * ps * qs * (qr - pr)) * xr2 * xs3 + (1. / 8.) * pow((pr * qr), 2) * xr5 * xs1 +
         (1. / 8.) * pow((ps * qs), 2) * xr1 * xs5;
    z6 = 0.25 * pr * pr * qr * qs * xr4 * xs2 + 0.25 * pr * ps * qs * qs * xr2 * xs4 +
         (2. * (pr * pr * qs * qs) + 3. * pr * qr * ps * qs) * xr3 * xs3 / 12.;
    z7 = z2 + z3 + z4 + z5 + z6;

    vrs = z1 + pr * qs * z7 / pow((n + 2.), 3) + pr * qs * xr1 * xs1 / (n + 2.);
}

// ============ orderw - для распределения Вейбулла ============
// Вычисляет математическое ожидание и ковариацию порядковых статистик
// для распределения Вейбулла (в логарифмической шкале)
void orderw(int n, double pr, double ps, double &er, double &vrs) {
    double  qr, qs, xr, xr1, xr2, xr3, xr4, xr5, xr55, xr6;
    double xs1, xs2, xs3, xs4, xs5;
    double z1, z2, z3, z4, z5, z6, z7, a1, b1, c1, d1;

    xr = log(log(1. / (1. - pr)));
    qr = 1. - pr;
    qs = 1. - ps;
    xr1 = 1. / (log(1. / (1. - pr)) * (1. - pr));
    xr2 = xr1 * (1. / (1. - pr) - xr1);
    xr3 = xr2 * xr2 / xr1 + xr1 * (1. / pow((1. - pr), 2) - xr2);
    xr4 = (3. * xr1 * xr2 * xr3 - 2. * pow(xr2, 3)) / pow(xr1, 2) + xr1 * (2. / pow((1. - pr), 3) - xr3);
    xr55 = (-12. * xr1 * xr2 * xr2 * xr3 + 3. * xr1 * xr1 * xr3 * xr3 + 4. * xr1 * xr1 * xr2 * xr4 + 6. * pow(xr2, 4));
    xr5 = xr55 / pow(xr1, 3) + xr1 * (6. / pow(1. - pr, 4) - xr4);
    a1 = -12. * pow(xr2, 3) * xr3 - 12. * xr1 * (2. * xr2 * xr3 * xr3 + xr2 * xr2 * xr4);
    b1 = 6. * xr1 * xr2 * xr3 * xr3 + 6. * xr1 * xr1 * xr3 * xr4;
    c1 = 8. * xr1 * xr2 * xr2 * xr4 + 4. * xr1 * xr1 * (xr3 * xr4 + xr2 * xr5);
    d1 = 24. * pow(xr2, 3) * xr3;
    xr6 = (pow(xr1, 3) * (a1 + b1 + c1 + d1) - 3. * xr1 * xr1 * xr2 * xr55) / pow(xr1, 6) +
          xr2 * (6. / pow((1. - pr), 4) - xr4) + xr1 * (24. / pow((1. - pr), 5) - xr5);

    xs1 = 1. / (log(1. / (1. - ps)) * (1. - ps));
    xs2 = xs1 * (1. / (1. - ps) - xs1);
    xs3 = xs2 * xs2 / xs1 + xs1 * (1. / pow((1. - ps), 2) - xs2);
    xs4 = (3. * xs1 * xs2 * xs3 - 2. * pow(xs2, 3)) / (xs1 * xs1) + xs1 * (2. / pow((1. - ps), 3) - xs3);
    xs5 = (-12. * xs1 * xs2 * xs2 * xs3 + 3. * xs1 * xs1 * xs3 * xs3 + 4. * xs1 * xs1 * xs2 * xs4 + 6. * pow(xs2, 4)) /
          pow(xs1, 3) + xs1 * (6. / pow((1. - ps), 4) - xs4);

    z1 = pr * qr * xr2 / (2. * (n + 2.));
    z2 = pr * qr * ((qr - pr) * xr3 / 3. + pr * qr * xr4 / 8.) / pow((n + 2.), 2);
    z3 = -(qr - pr) * xr3 / 3. + (pow((qr - pr), 2) - pr * qr) * xr4 / 4.;
    z4 = qr * pr * (qr - pr) * xr5 / 6. + qr * qr * pr * pr * xr6 / 48.;
    er = xr + z1 + z2 + pr * qr * (z3 + z4) / pow((n + 2.), 3);

    z1 = (qr - pr) * xr2 * xs1 + (qs - ps) * xr1 * xs2 + pr * qr * xr3 * xs1 / 2. +
         ps * qs * xr1 * xs3 / 2. + pr * qs * xr2 * xs2 / 2.;
    z1 = z1 * pr * qs / pow((n + 2.), 2);
    z2 = -(qr - pr) * xr2 * xs1 - (qs - ps) * xr1 * xs2 + (pow((qr - pr), 2) - pr * qr) * xr3 * xs1;
    z3 = (pow((qs - ps), 2) - ps * qs) * xr1 * xs3 + (1.5 * (qr - pr) * (qs - ps) + 0.5 * ps * qr - 2. * pr * qs) * xr2 * xs2;
    z4 = (5. / 6.) * pr * qr * (qr - pr) * xr4 * xs1 + (5. / 6.) * ps * qs * (qs - ps) * xr1 * xs4 +
         (pr * qs * (qr - pr) + .5 * pr * qr * (qs - ps)) * xr3 * xs2;
    z5 = (pr * qs * (qs - ps) + .5 * ps * qs * (qr - pr)) * xr2 * xs3 + (1. / 8.) * pr * pr * qr * qr * xr5 * xs1 +
         (1. / 8.) * ps * ps * qs * qs * xr1 * xs5;
    z6 = 0.25 * pr * pr * qr * qs * xr4 * xs2 + 0.25 * pr * ps * qs * qs * xr2 * xs4 +
         (2. * pr * pr * qs * qs + 3. * pr * qr * ps * qs) * xr3 * xs3 / 12.;
    z7 = z2 + z3 + z4 + z5 + z6;
    vrs = z1 + pr * qs * z7 / pow((n + 2.), 3) + pr * qs * xr1 * xs1 / (n + 2.);
}

// ============ Вспомогательные функции для MLS ============

// Вычисление эмпирической функции распределения для полных данных
void cum(int n, const std::vector<double>& x, const std::vector<int>& r, int km,
         std::vector<double>& fcum, std::vector<double>& ycum) {
    std::vector<double> sorted_data;
    for (int i = 0; i < n; i++) {
        if (r[i] == 0) {  // только полные наблюдения
            sorted_data.push_back(x[i]);
        }
    }
    std::sort(sorted_data.begin(), sorted_data.end());

    for (int i = 0; i < km; i++) {
        ycum[i] = sorted_data[i];
        fcum[i] = (i + 1.0) / (km + 1.0);  // эмпирическая вероятность
    }
}

// Вычисление начальных оценок параметров
void standart(int km, const std::vector<double>& ycum, double& cp, double& cko) {
    // Простая оценка через выборочные моменты
    cp = 0.0;  // среднее
    for (int i = 0; i < km; i++) {
        cp += ycum[i];
    }
    cp /= km;

    cko = 0.0;  // стандартное отклонение
    for (int i = 0; i < km; i++) {
        cko += (ycum[i] - cp) * (ycum[i] - cp);
    }
    cko = std::sqrt(cko / (km - 1));
}

/**
 * Взвешенный МНК (обобщенный метод наименьших квадратов) через Boost.uBLAS
 * Формула: b = (X^T V^{-1} X)^{-1} X^T V^{-1} y
 * где V - ковариационная матрица ошибок
 */
void MleastSquare_weight(const Matrix& x, const Matrix& y, const Matrix& v,
                         Matrix& db, Matrix& b, Vector& yr) {
    // V^{-1}
    Matrix v_inv = InverseMatrix(v);

    // X^T
    Matrix x_t = TransMatrix(x);

    // X^T V^{-1}
    Matrix xt_v_inv = MultiplyMatrix(x_t, v_inv);

    // X^T V^{-1} X
    Matrix xt_v_inv_x = MultiplyMatrix(xt_v_inv, x);

    // (X^T V^{-1} X)^{-1} - ковариационная матрица параметров
    db = InverseMatrix(xt_v_inv_x);

    // X^T V^{-1} y
    Matrix xt_v_inv_y = MultiplyMatrix(xt_v_inv, y);

    // b = (X^T V^{-1} X)^{-1} X^T V^{-1} y
    b = MultiplyMatrix(db, xt_v_inv_y);

    // Предсказанные значения: yr = X * b
    size_t n = x.size1();
    size_t k = x.size2();
    yr.resize(n);

    for (size_t i = 0; i < n; i++) {
        yr(i) = 0.0;
        for (size_t j = 0; j < k; j++) {
            yr(i) += x(i, j) * b(j, 0);
        }
    }
}
