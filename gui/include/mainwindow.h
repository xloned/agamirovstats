#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QTabWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QFileSystemModel>
#include <QListWidget>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class ChartViewer;
class DataEditor;
class StatisticsWorker;

/**
 * @brief Главное окно приложения статистического анализа
 *
 * Предоставляет визуальный интерфейс для:
 * - Выбора типа анализа (MLE, MLS, статистические тесты)
 * - Редактирования входных данных
 * - Просмотра результатов и графиков
 * - Управления параметрами анализа
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Слоты для кнопок управления
    void onRunAnalysis();
    void onClearResults();
    void onShowFiles();
    void onSaveResults();

    // Слоты для работы с данными
    void onLoadData();
    void onEditData();

    // Слоты для выбора типа анализа
    void onAnalysisTypeChanged(int index);
    void onDistributionChanged(int index);

    // Слоты для отображения результатов
    void onAnalysisFinished(bool success);
    void onProgressUpdated(int value, const QString& message);
    void onResultsReady(const QString& results);

    // Слоты для работы с файлами
    void onInputFileSelected(const QString& fileName);
    void onOutputFileSelected(const QString& fileName);

    // Слоты для графиков
    void onShowChart();
    void onExportChart();

private:
    // Методы инициализации
    void setupUI();
    void setupConnections();
    void setupInputPanel();
    void setupOutputPanel();
    void setupControlPanel();

    // Методы для работы с данными
    std::vector<double> loadDataFromFile(const QString& fileName);
    void displayResults(const QString& results);
    void displayChart(const QString& chartType);

    // Методы запуска анализа
    void runMLEAnalysis();
    void runMLSAnalysis();
    void runStatisticalTests();
    void runConfidenceIntervals();

    // Утилиты
    void showError(const QString& message);
    void showSuccess(const QString& message);
    void updateFileList();
    QString getProjectRootPath() const;

private:
    Ui::MainWindow *ui;

    // Виджеты для параметров
    QComboBox* analysisTypeCombo;
    QComboBox* distributionCombo;
    QDoubleSpinBox* alphaSpinBox;
    QCheckBox* useCensoredDataCheck;
    QListWidget* inputFilesList;
    QListWidget* outputFilesList;

    // Виджеты для отображения
    QTextEdit* resultsText;
    QTabWidget* tabWidget;
    ChartViewer* chartViewer;
    DataEditor* dataEditor;

    // Кнопки управления
    QPushButton* runButton;
    QPushButton* clearButton;
    QPushButton* loadDataButton;
    QPushButton* saveButton;

    // Прогресс и статус
    QProgressBar* progressBar;
    QLabel* statusLabel;

    // Рабочий поток для вычислений
    StatisticsWorker* worker;

    // Текущие данные
    std::vector<double> currentData;
    std::vector<double> currentData2;  // Вторая выборка для Fisher/Student тестов
    std::vector<int> currentCensored;  // Флаги цензуры (0 = нецензурировано, 1 = цензурировано)
    QString currentInputFile;
    QString currentInputFile2;  // Файл второй выборки
    QString currentOutputFile;

    // Параметры анализа
    enum AnalysisType {
        MLE_NORMAL,
        MLE_WEIBULL,
        MLS_NORMAL,
        GRUBBS_TEST,
        FISHER_TEST,
        STUDENT_TEST,
        ANOVA_TEST,
        SHAPIRO_WILK_TEST,
        WILCOXON_RANKSUM_TEST,
        CONFIDENCE_INTERVALS,
        PERCENTILES
    };

    AnalysisType currentAnalysisType;

    // Дополнительные данные для ANOVA (несколько групп)
    std::vector<std::vector<double>> anovaGroups;
};

#endif // MAINWINDOW_H
