#include "boost_distributions.h"
#include <boost/math/distributions/normal.hpp>
#include <boost/math/distributions/students_t.hpp>
#include <boost/math/distributions/chi_squared.hpp>
#include <boost/math/distributions/fisher_f.hpp>
#include <boost/math/distributions/non_central_t.hpp>
#include <boost/math/distributions/non_central_chi_squared.hpp>
#include <boost/math/distributions/non_central_f.hpp>
#include <boost/math/distributions/binomial.hpp>

using namespace boost::math;

// ============ Нормальное распределение ============
double norm_cdf(double x) {
    normal_distribution<> d(0, 1);
    return cdf(d, x);
}

double norm_ppf(double p) {
    if (p <= 0 || p >= 1) return 0;
    normal_distribution<> d(0, 1);
    return quantile(d, p);
}

double norm_pdf(double x) {
    normal_distribution<> d(0, 1);
    return pdf(d, x);
}

// ============ Распределение Стьюдента ============
double t_cdf(double x, double f) {
    students_t_distribution<> d(f);
    return cdf(d, x);
}

double t_ppf(double p, double f) {
    if (p <= 0 || p >= 1) return 0;
    students_t_distribution<> d(f);
    return quantile(d, p);
}

double t_pdf(double x, double f) {
    students_t_distribution<> d(f);
    return pdf(d, x);
}

// ============ Распределение Хи-квадрат ============
double chi_cdf(double x, double f) {
    chi_squared_distribution<> d(f);
    return cdf(d, x);
}

double chi_ppf(double p, double f) {
    if (p <= 0 || p >= 1) return 0;
    chi_squared_distribution<> d(f);
    return quantile(d, p);
}

double chi_pdf(double x, double f) {
    chi_squared_distribution<> d(f);
    return pdf(d, x);
}

// ============ F-распределение ============
double f_cdf(double x, double f1, double f2) {
    fisher_f_distribution<> d(f1, f2);
    return cdf(d, x);
}

double f_ppf(double p, double f1, double f2) {
    if (p <= 0 || p >= 1) return 0;
    fisher_f_distribution<> d(f1, f2);
    return quantile(d, p);
}

double f_pdf(double x, double f1, double f2) {
    fisher_f_distribution<> d(f1, f2);
    return pdf(d, x);
}

// ============ Нецентральное распределение Стьюдента ============
double nct_cdf(double x, double f, double delta) {
    non_central_t_distribution<> d(f, delta);
    return cdf(d, x);
}

double nct_ppf(double p, double f, double delta) {
    if (p <= 0 || p >= 1) return 0;
    non_central_t_distribution<> d(f, delta);
    return quantile(d, p);
}

double nct_pdf(double x, double f, double delta) {
    non_central_t_distribution<> d(f, delta);
    return pdf(d, x);
}

// ============ Нецентральное распределение Хи-квадрат ============
double nchi_cdf(double x, double f, double delta) {
    non_central_chi_squared_distribution<> d(f, delta);
    return cdf(d, x);
}

double nchi_ppf(double p, double f, double delta) {
    if (p <= 0 || p >= 1) return 0;
    non_central_chi_squared_distribution<> d(f, delta);
    return quantile(d, p);
}

double nchi_pdf(double x, double f, double delta) {
    non_central_chi_squared_distribution<> d(f, delta);
    return pdf(d, x);
}

// ============ Нецентральное F-распределение ============
double ncf_cdf(double x, double f1, double f2, double delta) {
    non_central_f_distribution<> d(f1, f2, delta);
    return cdf(d, x);
}

double ncf_ppf(double p, double f1, double f2, double delta) {
    if (p <= 0 || p >= 1) return 0;
    non_central_f_distribution<> d(f1, f2, delta);
    return quantile(d, p);
}

double ncf_pdf(double x, double f1, double f2, double delta) {
    non_central_f_distribution<> d(f1, f2, delta);
    return pdf(d, x);
}

// ============ Биномиальное распределение ============
double binom_cdf(double k, double n, double p) {
    binomial_distribution<> d(n, p);
    return cdf(d, k);
}

double binom_ppf(double prob, double n, double p) {
    if (prob <= 0 || prob >= 1) return 0;
    binomial_distribution<> d(n, p);
    return quantile(d, prob);
}

double binom_pdf(double k, double n, double p) {
    binomial_distribution<> d(n, p);
    return pdf(d, k);
}