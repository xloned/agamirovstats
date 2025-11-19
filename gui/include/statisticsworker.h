#ifndef STATISTICSWORKER_H
#define STATISTICSWORKER_H

#include <QThread>
#include <QString>
#include <QTextStream>
#include <vector>

// Forward declarations
struct MLEResult;
struct GrubbsTestResult;
struct FisherTestResult;
struct StudentTestResult;
struct ANOVAResult;
struct ShapiroWilkResult;
struct WilcoxonRankSumResult;

class StatisticsWorker : public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY(StatisticsWorker)

public:
    enum TaskType {
        TASK_MLE_NORMAL,
        TASK_MLE_WEIBULL,
        TASK_MLS_NORMAL,
        TASK_GRUBBS,
        TASK_FISHER,
        TASK_STUDENT_EQUAL,
        TASK_STUDENT_UNEQUAL,
        TASK_STUDENT_AUTO,
        TASK_ANOVA,
        TASK_SHAPIRO_WILK,
        TASK_WILCOXON_RANKSUM,
        TASK_CONFIDENCE_INTERVALS,
        TASK_PERCENTILES
    };

    explicit StatisticsWorker(QObject *parent = nullptr);
    ~StatisticsWorker();

    void setTask(TaskType task, const std::vector<double>& d, double a = 0.05);
    void setCensored(const std::vector<int>& c);  // Установить флаги цензуры
    void setData2(const std::vector<double>& d);
    void setANOVAGroups(const std::vector<std::vector<double>>& groups);  // Установить группы для ANOVA

signals:
    void progressUpdated(int value, const QString& message);
    void finished(bool success);
    void resultsReady(const QString& results);

protected:
    void run() override;

private:
    QString runMLENormal();
    QString runMLEWeibull();
    QString runMLSNormal();
    QString runGrubbsTest();
    QString runFisherTest();
    QString runStudentTestEqual();
    QString runStudentTestUnequal();
    QString runStudentTestAuto();
    QString runANOVA();
    QString runShapiroWilk();
    QString runWilcoxonRankSum();
    QString runConfidenceIntervals();
    QString runPercentiles();

    QString formatMLEResult(const MLEResult& result, const QString& title);
    QString formatStudentResult(const StudentTestResult& result);
    QString formatANOVAResult(const ANOVAResult& result);
    QString formatShapiroWilkResult(const ShapiroWilkResult& result);
    QString formatWilcoxonRankSumResult(const WilcoxonRankSumResult& result);

private:
    TaskType currentTask;
    std::vector<double> data;
    std::vector<int> censored;  // Флаги цензуры
    std::vector<double> data2;
    std::vector<std::vector<double>> anovaGroups;  // Группы для ANOVA
    double alpha;
};

#endif // STATISTICSWORKER_H
