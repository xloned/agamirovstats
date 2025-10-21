#ifndef BOOST_DISTRIBUTIONS_H
#define BOOST_DISTRIBUTIONS_H

// Функции для работы со статистическими распределениями через Boost

// Нормальное распределение
double norm_cdf(double x);
double norm_ppf(double p);
double norm_pdf(double x);

// Распределение Стьюдента
double t_cdf(double x, double f);
double t_ppf(double p, double f);
double t_pdf(double x, double f);

// Распределение Хи-квадрат
double chi_cdf(double x, double f);
double chi_ppf(double p, double f);
double chi_pdf(double x, double f);

// F-распределение
double f_cdf(double x, double f1, double f2);
double f_ppf(double p, double f1, double f2);
double f_pdf(double x, double f1, double f2);

// Нецентральное распределение Стьюдента
double nct_cdf(double x, double f, double delta);
double nct_ppf(double p, double f, double delta);
double nct_pdf(double x, double f, double delta);

// Нецентральное распределение Хи-квадрат
double nchi_cdf(double x, double f, double delta);
double nchi_ppf(double p, double f, double delta);
double nchi_pdf(double x, double f, double delta);

// Нецентральное F-распределение
double ncf_cdf(double x, double f1, double f2, double delta);
double ncf_ppf(double p, double f1, double f2, double delta);
double ncf_pdf(double x, double f1, double f2, double delta);

// Биномиальное распределение
double binom_cdf(double k, double n, double p);
double binom_ppf(double prob, double n, double p);
double binom_pdf(double k, double n, double p);

#endif // BOOST_DISTRIBUTIONS_H