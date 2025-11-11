/**
 * @file statisticsworker.cpp
 * @brief Рабочий поток для выполнения статистического анализа
 *
 * Использует существующие модули из корневой папки проекта:
 * - mle_methods.h для MLE/MLS анализа
 * - statistical_tests.h для статистических тестов
 */

#include "statisticsworker.h"
#include "mle_methods.h"
#include "statistical_tests.h"
#include "confidence_intervals.h"

#include <QDir>
#include <QProcess>
#include <QFile>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <numeric>

StatisticsWorker::StatisticsWorker(QObject *parent)
    : QThread(parent)
    , currentTask(TASK_MLE_NORMAL)
    , alpha(0.05)
{
}

StatisticsWorker::~StatisticsWorker()
{
}

void StatisticsWorker::setTask(TaskType task, const std::vector<double>& d, double a)
{
    currentTask = task;
    data = d;
    alpha = a;
}

void StatisticsWorker::setCensored(const std::vector<int>& c)
{
    censored = c;
}

void StatisticsWorker::setData2(const std::vector<double>& d)
{
    data2 = d;
}

void StatisticsWorker::run()
{
    emit progressUpdated(10, "Начало анализа...");

    QString results;
    bool success = false;

    try {
        switch (currentTask) {
            case TASK_MLE_NORMAL:
                results = runMLENormal();
                success = true;
                break;

            case TASK_MLE_WEIBULL:
                results = runMLEWeibull();
                success = true;
                break;

            case TASK_MLS_NORMAL:
                results = runMLSNormal();
                success = true;
                break;

            case TASK_GRUBBS:
                results = runGrubbsTest();
                success = true;
                break;

            case TASK_FISHER:
                results = runFisherTest();
                success = true;
                break;

            case TASK_STUDENT_EQUAL:
                results = runStudentTestEqual();
                success = true;
                break;

            case TASK_STUDENT_UNEQUAL:
                results = runStudentTestUnequal();
                success = true;
                break;

            case TASK_STUDENT_AUTO:
                results = runStudentTestAuto();
                success = true;
                break;

            case TASK_CONFIDENCE_INTERVALS:
                results = runConfidenceIntervals();
                success = true;
                break;

            case TASK_PERCENTILES:
                results = runPercentiles();
                success = true;
                break;

            default:
                results = "Неизвестный тип анализа";
                success = false;
        }

        emit progressUpdated(100, "Анализ завершен");
        emit resultsReady(results);
        emit finished(success);

    } catch (const std::exception& e) {
        results = QString("Ошибка: %1").arg(e.what());
        emit resultsReady(results);
        emit finished(false);
    }
}

QString StatisticsWorker::runMLENormal()
{
    emit progressUpdated(30, "Выполнение MLE для нормального распределения...");

    // Вызов реального модуля MLE
    MLEResult result = mle_normal_complete(data);

    emit progressUpdated(60, "Сохранение результатов...");

    // Сохранение в файл (используем существующую функцию)
    QDir rootDir = QDir::current();
    rootDir.cdUp();
    rootDir.cdUp();
    QString outputPath = rootDir.absolutePath() + "/output/mle_normal_complete.txt";

    // Использовать переданные флаги цензуры или создать нецензурированные
    std::vector<int> censorFlags = censored.empty() ? std::vector<int>(data.size(), 0) : censored;
    save_mle_result(result, outputPath.toStdString().c_str(), data, censorFlags);

    emit progressUpdated(80, "Построение графика...");

    // Вызов Python скрипта для графика
    QString pythonScript = rootDir.absolutePath() + "/python/plot_normal.py";
    // Используем venv python
    QString pythonExe = rootDir.absolutePath() + "/python/venv/bin/python3";
    if (!QFile::exists(pythonExe)) pythonExe = "python3";

    // Запускаем Python с правильной рабочей директорией
    QProcess process;
    process.setWorkingDirectory(rootDir.absolutePath());
    process.start(pythonExe, QStringList() << pythonScript << "mle");
    process.waitForFinished(10000); // ждем до 10 секунд

    // Формирование текстового отчета
    QString report = formatMLEResult(result, "MLE - Нормальное распределение");

    free_mle_result(result);

    return report;
}

QString StatisticsWorker::runMLEWeibull()
{
    emit progressUpdated(30, "Выполнение MLE для распределения Вейбулла...");

    MLEResult result = mle_weibull_complete(data);

    emit progressUpdated(60, "Сохранение результатов...");

    QDir rootDir = QDir::current();
    rootDir.cdUp();
    rootDir.cdUp();
    QString outputPath = rootDir.absolutePath() + "/output/mle_weibull_complete.txt";

    // Использовать переданные флаги цензуры или создать нецензурированные
    std::vector<int> censorFlags = censored.empty() ? std::vector<int>(data.size(), 0) : censored;
    save_mle_result(result, outputPath.toStdString().c_str(), data, censorFlags);

    emit progressUpdated(80, "Построение графика...");

    QString pythonScript = rootDir.absolutePath() + "/python/plot_weibull.py";
    // Используем venv python
    QString pythonExe = rootDir.absolutePath() + "/python/venv/bin/python3";
    if (!QFile::exists(pythonExe)) pythonExe = "python3";

    // Запускаем Python с правильной рабочей директорией
    QProcess process;
    process.setWorkingDirectory(rootDir.absolutePath());
    process.start(pythonExe, QStringList() << pythonScript << "mle");
    process.waitForFinished(10000);

    QString report = formatMLEResult(result, "MLE - Распределение Вейбулла");

    free_mle_result(result);

    return report;
}

QString StatisticsWorker::runMLSNormal()
{
    emit progressUpdated(30, "Выполнение MLS для нормального распределения...");

    MLEResult result = mls_normal_complete(data);

    emit progressUpdated(60, "Сохранение результатов...");

    QDir rootDir = QDir::current();
    rootDir.cdUp();
    rootDir.cdUp();
    QString outputPath = rootDir.absolutePath() + "/output/mls_normal_complete.txt";

    // Использовать переданные флаги цензуры или создать нецензурированные
    std::vector<int> censorFlags = censored.empty() ? std::vector<int>(data.size(), 0) : censored;
    save_mle_result(result, outputPath.toStdString().c_str(), data, censorFlags);

    emit progressUpdated(80, "Построение графика...");

    QString pythonScript = rootDir.absolutePath() + "/python/plot_normal.py";
    // Используем venv python
    QString pythonExe = rootDir.absolutePath() + "/python/venv/bin/python3";
    if (!QFile::exists(pythonExe)) pythonExe = "python3";

    // Запускаем Python с правильной рабочей директорией
    QProcess process;
    process.setWorkingDirectory(rootDir.absolutePath());
    process.start(pythonExe, QStringList() << pythonScript << "mls");
    process.waitForFinished(10000);

    QString report = formatMLEResult(result, "MLS - Метод MLS метод (Нормальное)");

    free_mle_result(result);

    return report;
}

QString StatisticsWorker::runGrubbsTest()
{
    emit progressUpdated(50, "Выполнение критерия Граббса...");

    GrubbsTestResult result = grubbs_test(data, alpha);

    emit progressUpdated(70, "Сохранение результатов...");

    // Сохранить результаты в файл для визуализации
    QDir rootDir = QDir::current();
    rootDir.cdUp();
    rootDir.cdUp();
    QString outputPath = rootDir.absolutePath() + "/output/grubbs_test_normal.txt";

    QFile file(outputPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream fileOut(&file);

        fileOut << "# Данные\n";
        for (size_t i = 0; i < data.size(); ++i) {
            fileOut << data[i] << "\n";
        }

        fileOut << "\n# Критерий Граббса\n";
        fileOut << "G-статистика: " << QString::number(result.test_statistic, 'f', 4) << "\n";
        fileOut << "Критическое значение: " << QString::number(result.critical_value, 'f', 4) << "\n";
        fileOut << "Подозрительное значение: " << QString::number(result.outlier_value, 'f', 4) << "\n";
        fileOut << "Вывод: " << (result.is_outlier ? "Выброс обнаружен" : "Выброс не обнаружен") << "\n";

        file.close();
    }

    QString report;
    QTextStream out(&report);

    out << "=== Критерий Граббса (выявление выбросов) ===\n\n";
    out << "Размер выборки: n = " << result.n << "\n";
    out << "Уровень значимости: α = " << result.alpha << "\n\n";
    out << "Тестовая статистика G = " << QString::number(result.test_statistic, 'f', 4) << "\n";
    out << "Критическое значение = " << QString::number(result.critical_value, 'f', 4) << "\n\n";
    out << "Подозрительное значение: " << QString::number(result.outlier_value, 'f', 4);
    out << " (индекс " << result.outlier_index << ")\n";
    out << "Тип теста: " << QString::fromStdString(result.test_type) << "\n\n";

    if (result.is_outlier) {
        out << "РЕЗУЛЬТАТ: ВЫБРОС ОБНАРУЖЕН\n";
        out << "H0 отвергается: данное наблюдение является выбросом\n";
    } else {
        out << "РЕЗУЛЬТАТ: ВЫБРОС НЕ ОБНАРУЖЕН\n";
        out << "H0 принимается: данное наблюдение не является выбросом\n";
    }

    return report;
}

QString StatisticsWorker::runFisherTest()
{
    emit progressUpdated(50, "Выполнение F-критерия Фишера...");

    if (data2.empty()) {
        return "Ошибка: для F-критерия требуется две выборки";
    }

    FisherTestResult result = fisher_test(data, data2, alpha);

    QString report;
    QTextStream out(&report);

    out << "=== F-критерий Фишера (сравнение дисперсий) ===\n\n";
    out << "Выборка 1: n1 = " << result.n1 << ", s1^2 = " << QString::number(result.var1, 'f', 4) << "\n";
    out << "Выборка 2: n2 = " << result.n2 << ", s2^2 = " << QString::number(result.var2, 'f', 4) << "\n\n";
    out << "F-статистика = " << QString::number(result.f_statistic, 'f', 4) << "\n";
    out << "Критическое значение = " << QString::number(result.critical_value, 'f', 4) << "\n";
    out << "P-значение = " << QString::number(result.p_value, 'f', 4) << "\n\n";

    if (result.reject_h0) {
        out << "РЕЗУЛЬТАТ: ДИСПЕРСИИ РАЗЛИЧАЮТСЯ\n";
        out << "H0 отвергается\n";
    } else {
        out << "РЕЗУЛЬТАТ: ДИСПЕРСИИ РАВНЫ\n";
        out << "H0 принимается\n";
    }

    return report;
}

QString StatisticsWorker::runStudentTestEqual()
{
    emit progressUpdated(50, "Выполнение t-критерия Стьюдента (равные дисперсии)...");

    if (data2.empty()) {
        return "Ошибка: для t-критерия требуется две выборки";
    }

    StudentTestResult result = student_test_equal_var(data, data2, alpha);

    emit progressUpdated(70, "Сохранение результатов...");

    // Сохранить результаты в файл используя существующую функцию
    QDir rootDir = QDir::current();
    rootDir.cdUp();
    rootDir.cdUp();
    QString outputPath = rootDir.absolutePath() + "/output/student_test_equal_var.txt";
    print_student_result(result, outputPath.toStdString());

    emit progressUpdated(85, "Построение графика...");

    // Вызов Python скрипта для графика
    QString pythonScript = rootDir.absolutePath() + "/python/plot_student.py";
    QString pythonExe = rootDir.absolutePath() + "/python/venv/bin/python3";
    if (!QFile::exists(pythonExe)) pythonExe = "python3";

    QProcess process;
    process.setWorkingDirectory(rootDir.absolutePath());
    process.start(pythonExe, QStringList() << pythonScript);
    process.waitForFinished(10000);

    return formatStudentResult(result);
}

QString StatisticsWorker::runStudentTestUnequal()
{
    emit progressUpdated(50, "Выполнение t-критерия Стьюдента (неравные дисперсии)...");

    if (data2.empty()) {
        return "Ошибка: для t-критерия требуется две выборки";
    }

    StudentTestResult result = student_test_unequal_var(data, data2, alpha);

    emit progressUpdated(70, "Сохранение результатов...");

    // Сохранить результаты в файл используя существующую функцию
    QDir rootDir = QDir::current();
    rootDir.cdUp();
    rootDir.cdUp();
    QString outputPath = rootDir.absolutePath() + "/output/student_test_unequal_var.txt";
    print_student_result(result, outputPath.toStdString());

    emit progressUpdated(85, "Построение графика...");

    // Вызов Python скрипта для графика
    QString pythonScript = rootDir.absolutePath() + "/python/plot_student.py";
    QString pythonExe = rootDir.absolutePath() + "/python/venv/bin/python3";
    if (!QFile::exists(pythonExe)) pythonExe = "python3";

    QProcess process;
    process.setWorkingDirectory(rootDir.absolutePath());
    process.start(pythonExe, QStringList() << pythonScript);
    process.waitForFinished(10000);

    return formatStudentResult(result);
}

QString StatisticsWorker::runStudentTestAuto()
{
    emit progressUpdated(50, "Выполнение t-критерия Стьюдента (автовыбор метода)...");

    if (data2.empty()) {
        return "Ошибка: для t-критерия требуется две выборки";
    }

    StudentTestResult result = student_test_auto(data, data2, alpha);

    emit progressUpdated(70, "Сохранение результатов...");

    // Сохранить результаты в файл используя существующую функцию
    QDir rootDir = QDir::current();
    rootDir.cdUp();
    rootDir.cdUp();
    QString outputPath = rootDir.absolutePath() + "/output/student_test_auto.txt";
    print_student_result(result, outputPath.toStdString());

    emit progressUpdated(85, "Построение графика...");

    // Вызов Python скрипта для графика
    QString pythonScript = rootDir.absolutePath() + "/python/plot_student.py";
    QString pythonExe = rootDir.absolutePath() + "/python/venv/bin/python3";
    if (!QFile::exists(pythonExe)) pythonExe = "python3";

    QProcess process;
    process.setWorkingDirectory(rootDir.absolutePath());
    process.start(pythonExe, QStringList() << pythonScript);
    process.waitForFinished(10000);

    return formatStudentResult(result);
}

QString StatisticsWorker::formatMLEResult(const MLEResult& result, const QString& title)
{
    QString report;
    QTextStream out(&report);

    out << "=== " << title << " ===\n\n";
    out << "Количество наблюдений: n = " << data.size() << "\n\n";

    out << "Оценки параметров:\n";
    for (size_t i = 0; i < result.parameters.size(); ++i) {
        out << "  Параметр " << (i+1) << " = " << QString::number(result.parameters[i], 'f', 6);
        if (i < result.std_errors.size()) {
            out << " +/- " << QString::number(result.std_errors[i], 'f', 6);
        }
        out << "\n";
    }

    out << "\nLog-likelihood = " << QString::number(result.log_likelihood, 'f', 4) << "\n";
    out << "Итераций: " << result.iterations << "\n";
    out << "Сходимость: " << (result.converged ? "ДА" : "НЕТ") << "\n";

    return report;
}

QString StatisticsWorker::formatStudentResult(const StudentTestResult& result)
{
    QString report;
    QTextStream out(&report);

    out << "=== t-критерий Стьюдента (" << QString::fromStdString(result.test_type) << ") ===\n\n";
    out << "Выборка 1: n1 = " << result.n1 << ", среднее = " << QString::number(result.mean1, 'f', 4);
    out << ", СКО = " << QString::number(result.std1, 'f', 4) << "\n";
    out << "Выборка 2: n2 = " << result.n2 << ", среднее = " << QString::number(result.mean2, 'f', 4);
    out << ", СКО = " << QString::number(result.std2, 'f', 4) << "\n\n";

    out << "t-статистика = " << QString::number(result.t_statistic, 'f', 4) << "\n";
    out << "Степени свободы = " << QString::number(result.df, 'f', 2) << "\n";
    out << "Критическое значение = " << QString::number(result.critical_value, 'f', 4) << "\n";
    out << "P-значение = " << QString::number(result.p_value, 'f', 4) << "\n\n";

    if (result.reject_h0) {
        out << "РЕЗУЛЬТАТ: СРЕДНИЕ РАЗЛИЧАЮТСЯ\n";
        out << "H0 отвергается\n";
    } else {
        out << "РЕЗУЛЬТАТ: СРЕДНИЕ РАВНЫ\n";
        out << "H0 принимается\n";
    }

    return report;
}

QString StatisticsWorker::runConfidenceIntervals()
{
    emit progressUpdated(30, "Вычисление доверительных интервалов...");

    // Вычислить все доверительные интервалы
    ConfidenceIntervals ci = compute_all_confidence_intervals(data, -1.0, 1.0 - alpha);

    emit progressUpdated(60, "Сохранение результатов...");

    // Сохранить в файл
    QDir rootDir = QDir::current();
    rootDir.cdUp();
    rootDir.cdUp();
    QString outputPath = rootDir.absolutePath() + "/output/confidence_intervals.txt";

    save_confidence_intervals(ci, outputPath.toStdString().c_str(), data, -1.0);

    // Сформировать отчет
    QString report;
    QTextStream out(&report);

    out << "=== Доверительные интервалы (уровень доверия: "
        << QString::number((1.0 - alpha) * 100, 'f', 1) << "%) ===\n\n";

    out << "Размер выборки: n = " << data.size() << "\n\n";

    out << "1. ДИ для μ (при известной σ):\n";
    out << "   Точечная оценка: " << QString::number(ci.mean_known_sigma.point_est, 'f', 4) << "\n";
    out << "   Интервал: [" << QString::number(ci.mean_known_sigma.lower, 'f', 4)
        << ", " << QString::number(ci.mean_known_sigma.upper, 'f', 4) << "]\n\n";

    out << "2. ДИ для μ (при неизвестной σ):\n";
    out << "   Точечная оценка: " << QString::number(ci.mean_unknown_sigma.point_est, 'f', 4) << "\n";
    out << "   Интервал: [" << QString::number(ci.mean_unknown_sigma.lower, 'f', 4)
        << ", " << QString::number(ci.mean_unknown_sigma.upper, 'f', 4) << "]\n\n";

    out << "3. ДИ для σ²:\n";
    out << "   Точечная оценка: " << QString::number(ci.variance.point_est, 'f', 4) << "\n";
    out << "   Интервал: [" << QString::number(ci.variance.lower, 'f', 4)
        << ", " << QString::number(ci.variance.upper, 'f', 4) << "]\n\n";

    out << "4. ДИ для σ:\n";
    out << "   Точечная оценка: " << QString::number(ci.sigma.point_est, 'f', 4) << "\n";
    out << "   Интервал: [" << QString::number(ci.sigma.lower, 'f', 4)
        << ", " << QString::number(ci.sigma.upper, 'f', 4) << "]\n";

    return report;
}

QString StatisticsWorker::runPercentiles()
{
    emit progressUpdated(30, "Вычисление персентилей...");

    // Вычислить среднее и стандартное отклонение
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    double mean = sum / data.size();

    double variance = 0.0;
    for (double x : data) {
        variance += (x - mean) * (x - mean);
    }
    variance /= (data.size() - 1);
    double sigma = std::sqrt(variance);

    // Уровни персентилей
    std::vector<double> p_levels = {0.01, 0.05, 0.10, 0.25, 0.50, 0.75, 0.90, 0.95, 0.99};

    // Вычислить персентили для нормального распределения
    Percentiles percentiles = compute_normal_percentiles(mean, sigma, data.size(), p_levels, 1.0 - alpha);

    emit progressUpdated(60, "Сохранение результатов...");

    // Сохранить в файл
    QDir rootDir = QDir::current();
    rootDir.cdUp();
    rootDir.cdUp();
    QString outputPath = rootDir.absolutePath() + "/output/percentiles_normal.txt";

    save_percentiles(percentiles, outputPath.toStdString().c_str());

    // Сформировать отчет
    QString report;
    QTextStream out(&report);

    out << "=== Персентили (Нормальное распределение) ===\n\n";
    out << "Параметры: μ = " << QString::number(mean, 'f', 4)
        << ", σ = " << QString::number(sigma, 'f', 4) << "\n";
    out << "Размер выборки: n = " << data.size() << "\n";
    out << "Уровень доверия: " << QString::number((1.0 - alpha) * 100, 'f', 1) << "%\n\n";

    out << QString("%1  %2  %3\n")
           .arg("P", -8)
           .arg("Значение", -12)
           .arg("95% ДИ", -25);
    out << QString("-").repeated(50) << "\n";

    for (const auto& p : percentiles.percentiles) {
        out << QString("%1  %2  [%3, %4]\n")
               .arg(QString::number(p.p * 100, 'f', 1) + "%", -8)
               .arg(QString::number(p.value, 'f', 4), -12)
               .arg(QString::number(p.lower, 'f', 4), -10)
               .arg(QString::number(p.upper, 'f', 4), -10);
    }

    return report;
}
