#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chartviewer.h"
#include "dataeditor.h"
#include "statisticsworker.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>
#include <QProcess>
#include <QPixmap>
#include <QDebug>
#include <QCoreApplication>
#include <QRegularExpression>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , worker(nullptr)
    , currentAnalysisType(MLE_NORMAL)
{
    ui->setupUi(this);

    // Очистить папку output при запуске
    QString rootPath = getProjectRootPath();
    QString outputPath = rootPath + "/output";
    QDir outputDir(outputPath);
    if (outputDir.exists()) {
        QStringList files = outputDir.entryList(QStringList() << "*.png" << "*.txt", QDir::Files);
        for (const QString& file : files) {
            outputDir.remove(file);
        }
        qDebug() << "Очищена папка output:" << files.size() << "файлов удалено";
    }

    // Инициализация виджетов
    setupUI();
    setupConnections();

    // Обновить список файлов
    updateFileList();

    // Установить статус
    statusBar()->showMessage("Готов к работе");
}

MainWindow::~MainWindow()
{
    if (worker) {
        worker->quit();
        worker->wait();
        delete worker;
    }
    delete ui;
}

/**
 * @brief Настройка пользовательского интерфейса
 */
void MainWindow::setupUI()
{
    // Создать виджет для просмотра графиков
    chartViewer = new ChartViewer(this);
    chartViewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* chartLayout = new QVBoxLayout(ui->chartContainer);
    chartLayout->setContentsMargins(0, 0, 0, 0);
    chartLayout->setSpacing(0);
    chartLayout->addWidget(chartViewer);
    ui->chartContainer->setLayout(chartLayout);

    // Создать редактор данных
    dataEditor = new DataEditor(this);
    QVBoxLayout* editorLayout = new QVBoxLayout(ui->editorContainer);
    editorLayout->addWidget(dataEditor);
    ui->editorContainer->setLayout(editorLayout);

    // Настроить начальные значения
    ui->alphaSpinBox->setValue(0.05);
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(false);

    // По умолчанию вкладка "График" включена
    // Будет отключаться для Граббса и Фишера в onAnalysisTypeChanged
    ui->tabWidget->setTabEnabled(1, true);

    // Установить шрифт для результатов
    QFont monoFont("Courier", 10);
    ui->resultsText->setFont(monoFont);
    ui->fileContentText->setFont(monoFont);
}

/**
 * @brief Настройка соединений сигналов и слотов
 */
void MainWindow::setupConnections()
{
    // Кнопки управления
    connect(ui->runButton, &QPushButton::clicked, this, &MainWindow::onRunAnalysis);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::onClearResults);
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::onSaveResults);
    connect(ui->loadDataButton, &QPushButton::clicked, this, &MainWindow::onLoadData);

    // Изменение типа анализа
    connect(ui->analysisTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAnalysisTypeChanged);

    // Выбор файлов
    connect(ui->inputFilesList, &QListWidget::currentTextChanged,
            this, &MainWindow::onInputFileSelected);
    connect(ui->outputFilesList, &QListWidget::currentTextChanged,
            this, &MainWindow::onOutputFileSelected);

    // Редактор данных
    connect(dataEditor, &DataEditor::dataChanged, [this]() {
        statusBar()->showMessage("Данные изменены", 2000);
    });

    // Меню
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::onLoadData);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::onSaveResults);
    connect(ui->actionQuit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, [this]() {
        QMessageBox::about(this, "О программе",
            "Статистический анализ - Методы MLS метод\n\n"
            "Версия 1.0\n\n"
            "Реализованы методы:\n"
            "• MLE/MLS оценка параметров\n"
            "• Критерий Граббса\n"
            "• F-критерий Фишера\n"
            "• t-критерий Стьюдента\n"
            "• Доверительные интервалы\n\n"
            "🤖 Generated with Claude Code");
    });
}

/**
 * @brief Запуск статистического анализа
 */
void MainWindow::onRunAnalysis()
{
    // Проверить наличие данных
    if (currentData.empty()) {
        showError("Загрузите данные перед запуском анализа");
        return;
    }

    // Заблокировать кнопку на время выполнения
    ui->runButton->setEnabled(false);
    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);

    statusBar()->showMessage("Выполняется анализ...");

    // Создать рабочий поток
    if (worker) {
        worker->quit();
        worker->wait();
        delete worker;
    }

    worker = new StatisticsWorker(this);

    // Определить тип задачи
    int analysisIndex = ui->analysisTypeCombo->currentIndex();
    currentAnalysisType = static_cast<AnalysisType>(analysisIndex);  // Обновить currentAnalysisType
    StatisticsWorker::TaskType task;

    switch (analysisIndex) {
        case 0: task = StatisticsWorker::TASK_MLE_NORMAL; break;
        case 1: task = StatisticsWorker::TASK_MLE_WEIBULL; break;
        case 2: task = StatisticsWorker::TASK_MLS_NORMAL; break;
        case 3: task = StatisticsWorker::TASK_GRUBBS; break;
        case 4: task = StatisticsWorker::TASK_FISHER; break;
        case 5: task = StatisticsWorker::TASK_STUDENT_AUTO; break;
        case 6: task = StatisticsWorker::TASK_CONFIDENCE_INTERVALS; break;
        case 7: task = StatisticsWorker::TASK_PERCENTILES; break;
        case 8: task = StatisticsWorker::TASK_ANOVA; break;
        case 9: task = StatisticsWorker::TASK_SHAPIRO_WILK; break;
        case 10: task = StatisticsWorker::TASK_WILCOXON_RANKSUM; break;
        default: task = StatisticsWorker::TASK_MLE_NORMAL;
    }

    // ANOVA требует несколько групп
    if (task == StatisticsWorker::TASK_ANOVA) {
        QMessageBox::information(this, "ANOVA",
            "Для дисперсионного анализа выберите несколько файлов данных.\n"
            "Каждый файл будет представлять отдельную группу (минимум 2 файла).");

        QString rootPath = getProjectRootPath();
        QString inputDir = rootPath + "/input";

        // Множественный выбор файлов
        QStringList fileNames = QFileDialog::getOpenFileNames(this,
            "Выберите файлы групп для ANOVA (минимум 2)", inputDir,
            "Data files (*.txt);;All files (*)");

        if (fileNames.isEmpty() || fileNames.size() < 2) {
            showError("Для ANOVA необходимо выбрать минимум 2 файла");
            ui->runButton->setEnabled(true);
            ui->progressBar->setVisible(false);
            return;
        }

        anovaGroups.clear();

        for (int i = 0; i < fileNames.size(); ++i) {
            std::vector<double> groupData = loadDataFromFile(fileNames[i]);
            if (groupData.empty()) {
                showError(QString("Не удалось загрузить данные из файла: %1").arg(fileNames[i]));
                ui->runButton->setEnabled(true);
                ui->progressBar->setVisible(false);
                return;
            }

            anovaGroups.push_back(groupData);
        }

        showSuccess(QString("Загружено %1 групп для ANOVA").arg(anovaGroups.size()));
        worker->setANOVAGroups(anovaGroups);
    }
    // Для Fisher, Student и Wilcoxon тестов нужны две выборки
    else if (task == StatisticsWorker::TASK_FISHER ||
        task == StatisticsWorker::TASK_STUDENT_EQUAL ||
        task == StatisticsWorker::TASK_STUDENT_UNEQUAL ||
        task == StatisticsWorker::TASK_STUDENT_AUTO ||
        task == StatisticsWorker::TASK_WILCOXON_RANKSUM) {

        // ВСЕГДА запрашивать вторую выборку для каждого теста
        // (не использовать повторно предыдущую выборку)
        QString rootPath = getProjectRootPath();
        QString inputDir = rootPath + "/input";
        QString fileName = QFileDialog::getOpenFileName(this,
            "Выберите файл второй выборки", inputDir,
            "Data files (*.txt);;All files (*)");

        if (fileName.isEmpty()) {
            showError("Для этого теста требуется две выборки");
            ui->runButton->setEnabled(true);
            ui->progressBar->setVisible(false);
            return;
        }

        currentData2 = loadDataFromFile(fileName);
        currentInputFile2 = fileName;

        if (currentData2.empty()) {
            showError("Не удалось загрузить вторую выборку");
            ui->runButton->setEnabled(true);
            ui->progressBar->setVisible(false);
            return;
        }

        // Обновить статистику для второй выборки
        double sum2 = std::accumulate(currentData2.begin(), currentData2.end(), 0.0);
        double mean2 = sum2 / currentData2.size();
        double variance2 = 0.0;
        for (double x : currentData2) {
            variance2 += (x - mean2) * (x - mean2);
        }
        variance2 /= currentData2.size();
        double std_dev2 = std::sqrt(variance2);
        double min_val2 = *std::min_element(currentData2.begin(), currentData2.end());
        double max_val2 = *std::max_element(currentData2.begin(), currentData2.end());

        // Обновить статистику для первой выборки
        double sum1 = std::accumulate(currentData.begin(), currentData.end(), 0.0);
        double mean1 = sum1 / currentData.size();
        double variance1 = 0.0;
        for (double x : currentData) {
            variance1 += (x - mean1) * (x - mean1);
        }
        variance1 /= currentData.size();
        double std_dev1 = std::sqrt(variance1);
        double min_val1 = *std::min_element(currentData.begin(), currentData.end());
        double max_val1 = *std::max_element(currentData.begin(), currentData.end());

        QString stats = QString(
            "Выборка 1:\n"
            "Размер: %1\n"
            "Среднее: %2\n"
            "СКО: %3\n"
            "Мин: %4\n"
            "Макс: %5\n\n"
            "Выборка 2:\n"
            "Размер: %6\n"
            "Среднее: %7\n"
            "СКО: %8\n"
            "Мин: %9\n"
            "Макс: %10"
        ).arg(currentData.size())
         .arg(mean1, 0, 'f', 4)
         .arg(std_dev1, 0, 'f', 4)
         .arg(min_val1, 0, 'f', 4)
         .arg(max_val1, 0, 'f', 4)
         .arg(currentData2.size())
         .arg(mean2, 0, 'f', 4)
         .arg(std_dev2, 0, 'f', 4)
         .arg(min_val2, 0, 'f', 4)
         .arg(max_val2, 0, 'f', 4);

        ui->statsText->setPlainText(stats);

        showSuccess(QString("Загружена вторая выборка: %1 значений").arg(currentData2.size()));

        // Передать вторую выборку
        worker->setData2(currentData2);
    }

    worker->setTask(task, currentData, ui->alphaSpinBox->value());

    // Передать флаги цензуры если они есть
    if (!currentCensored.empty()) {
        worker->setCensored(currentCensored);
    }

    // Подключить сигналы
    connect(worker, &StatisticsWorker::progressUpdated, this, &MainWindow::onProgressUpdated);
    connect(worker, &StatisticsWorker::finished, this, &MainWindow::onAnalysisFinished);
    connect(worker, &StatisticsWorker::resultsReady, this, &MainWindow::onResultsReady);

    // Запустить поток
    worker->start();
}

/**
 * @brief Очистка результатов
 */
void MainWindow::onClearResults()
{
    ui->resultsText->clear();
    ui->fileContentText->clear();
    chartViewer->clearChart();
    ui->progressBar->setValue(0);
    statusBar()->showMessage("Результаты очищены", 2000);
}

/**
 * @brief Сохранение результатов
 */
void MainWindow::onSaveResults()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Сохранить результаты", "output",
        "Text files (*.txt);;All files (*)");

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << ui->resultsText->toPlainText();
            file.close();
            showSuccess("Результаты сохранены в " + fileName);
        } else {
            showError("Не удалось сохранить файл");
        }
    }
}

/**
 * @brief Загрузка данных из файла
 */
void MainWindow::onLoadData()
{
    QString rootPath = getProjectRootPath();
    QString inputDir = rootPath + "/input";
    QString fileName = QFileDialog::getOpenFileName(this,
        "Открыть файл данных", inputDir,
        "Data files (*.txt);;All files (*)");

    if (!fileName.isEmpty()) {
        currentData = loadDataFromFile(fileName);
        if (!currentData.empty()) {
            currentInputFile = fileName;

            // Очистить вторую выборку при загрузке новой первой выборки
            currentData2.clear();
            currentInputFile2.clear();

            // Обновить статистику
            double sum = std::accumulate(currentData.begin(), currentData.end(), 0.0);
            double mean = sum / currentData.size();

            double variance = 0.0;
            for (double x : currentData) {
                variance += (x - mean) * (x - mean);
            }
            variance /= currentData.size();
            double std_dev = std::sqrt(variance);

            double min_val = *std::min_element(currentData.begin(), currentData.end());
            double max_val = *std::max_element(currentData.begin(), currentData.end());

            QString stats = QString(
                "Выборка 1:\n"
                "Размер: %1\n"
                "Среднее: %2\n"
                "СКО: %3\n"
                "Мин: %4\n"
                "Макс: %5"
            ).arg(currentData.size())
             .arg(mean, 0, 'f', 4)
             .arg(std_dev, 0, 'f', 4)
             .arg(min_val, 0, 'f', 4)
             .arg(max_val, 0, 'f', 4);

            ui->statsText->setPlainText(stats);

            // Загрузить в редактор
            dataEditor->setData(currentData);

            showSuccess(QString("Загружено %1 значений").arg(currentData.size()));
        }
    }
}

/**
 * @brief Загрузка данных из файла (вспомогательная функция)
 */
std::vector<double> MainWindow::loadDataFromFile(const QString& fileName)
{
    std::vector<double> data;
    currentCensored.clear();  // Очистить старые флаги цензуры

    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty() && !line.startsWith("#")) {
                QStringList parts = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

                if (parts.size() >= 1) {
                    bool ok;
                    double value = parts[0].toDouble(&ok);
                    if (ok) {
                        data.push_back(value);

                        // Если есть второй столбец, это флаг цензуры
                        if (parts.size() >= 2) {
                            int censorFlag = parts[1].toInt(&ok);
                            currentCensored.push_back(ok ? censorFlag : 0);
                        } else {
                            // Если второго столбца нет, данные нецензурированы
                            currentCensored.push_back(0);
                        }
                    }
                }
            }
        }
        file.close();
    } else {
        showError("Не удалось открыть файл: " + fileName);
    }

    return data;
}

/**
 * @brief Обработка завершения анализа
 */
void MainWindow::onAnalysisFinished(bool success)
{
    ui->runButton->setEnabled(true);
    ui->progressBar->setVisible(false);

    if (success) {
        statusBar()->showMessage("Анализ завершен успешно", 3000);
        ui->tabWidget->setCurrentIndex(0); // Переключиться на вкладку результатов
    } else {
        statusBar()->showMessage("Анализ завершен с ошибками", 3000);
    }
}

/**
 * @brief Обновление прогресса
 */
void MainWindow::onProgressUpdated(int value, const QString& message)
{
    ui->progressBar->setValue(value);
    statusBar()->showMessage(message);
}

/**
 * @brief Отображение результатов
 */
void MainWindow::onResultsReady(const QString& results)
{
    ui->resultsText->setPlainText(results);

    // Вызвать Python скрипт для построения профессионального графика
    QString rootPath = getProjectRootPath();
    QString pythonScript;
    QString plotFile;
    QString mode;

    // Извлечь базовое имя входного файла (без пути и расширения)
    QString inputSuffix = "";
    if (!currentInputFile.isEmpty()) {
        QFileInfo fileInfo(currentInputFile);
        inputSuffix = "_" + fileInfo.completeBaseName(); // добавляем префикс с подчеркиванием
    }

    // Определяем какой скрипт и режим использовать
    switch (currentAnalysisType) {
        case MLE_NORMAL:
            pythonScript = rootPath + "/python/plot_normal.py";
            plotFile = rootPath + "/output/plot_mle_normal" + inputSuffix + ".png";
            mode = "mle";
            break;
        case MLE_WEIBULL:
            pythonScript = rootPath + "/python/plot_weibull.py";
            plotFile = rootPath + "/output/plot_mle_weibull" + inputSuffix + ".png";
            mode = "mle";
            break;
        case MLS_NORMAL:
            pythonScript = rootPath + "/python/plot_normal.py";
            plotFile = rootPath + "/output/plot_mls_normal" + inputSuffix + ".png";
            mode = "mls";
            break;
        case CONFIDENCE_INTERVALS:
            // Доверительные интервалы - требует специальный скрипт
            pythonScript = rootPath + "/python/plot_confidence_intervals.py";
            plotFile = rootPath + "/output/plot_confidence_intervals" + inputSuffix + ".png";
            mode = "";  // Не используется для этого скрипта
            break;
        case PERCENTILES:
            // Персентили - требует специальный скрипт
            pythonScript = rootPath + "/python/plot_percentiles.py";
            plotFile = rootPath + "/output/plot_percentiles" + inputSuffix + ".png";
            mode = "";  // Не используется для этого скрипта
            break;
        case GRUBBS_TEST:
        case FISHER_TEST:
            // Граббс и Фишер - только текстовая статистика, без графиков
            updateFileList();
            return;
        case STUDENT_TEST:
            // Критерий Стьюдента - использовать plot_student.py
            // Скрипт создает три графика (auto, equal_var, unequal_var)
            pythonScript = rootPath + "/python/plot_student.py";
            // Будем искать файл после запуска скрипта
            plotFile = rootPath + "/output/plot_student_auto.png";
            mode = "";
            break;
        case ANOVA_TEST:
            // ANOVA - запускаем Python скрипт для графиков
            pythonScript = rootPath + "/python/plot_anova.py";
            plotFile = rootPath + "/output/plot_anova_f_distribution.png";
            mode = "";
            break;
        case SHAPIRO_WILK_TEST:
            // Shapiro-Wilk - запускаем Python скрипт для графиков
            pythonScript = rootPath + "/python/plot_shapiro_wilk.py";
            plotFile = rootPath + "/output/plot_shapiro_wilk_qq.png";
            mode = "";
            break;
        case WILCOXON_RANKSUM_TEST:
            // Wilcoxon - запускаем Python скрипт для графиков
            pythonScript = rootPath + "/python/plot_wilcoxon_ranksum.py";
            plotFile = rootPath + "/output/plot_wilcoxon_normal_approx.png";
            mode = "";
            break;
        default:
            // Для других типов анализа использовать встроенный график
            if (!currentData.empty() && chartViewer) {
                chartViewer->showHistogram(currentData, "Распределение данных");
            }
            updateFileList();
            return;
    }

    // Удаляем старый график если существует
    if (QFile::exists(plotFile)) {
        QFile::remove(plotFile);
        qDebug() << "Deleted old plot file:" << plotFile;
    }

    qDebug() << "=== Python Script Info ===";
    qDebug() << "Analysis type:" << currentAnalysisType;
    qDebug() << "Python script:" << pythonScript;
    qDebug() << "Plot file:" << plotFile;

    // Запускаем Python скрипт
    QString pythonExe = rootPath + "/python/venv/bin/python3";
    if (!QFile::exists(pythonExe)) {
        pythonExe = "python3";
        qDebug() << "Using system python3";
    } else {
        qDebug() << "Using venv python:" << pythonExe;
    }

    QProcess process;
    process.setWorkingDirectory(rootPath);

    // Формируем аргументы в зависимости от типа анализа
    QStringList arguments;
    arguments << pythonScript;

    if (currentAnalysisType == CONFIDENCE_INTERVALS) {
        // Для доверительных интервалов: script input_file output_file
        arguments << (rootPath + "/output/confidence_intervals.txt");
        arguments << plotFile;
    } else if (currentAnalysisType == PERCENTILES) {
        // Для персентилей нужно выбрать правильный файл (normal или weibull)
        QString percentilesFile = rootPath + "/output/percentiles_normal.txt";
        if (!QFile::exists(percentilesFile)) {
            percentilesFile = rootPath + "/output/percentiles_weibull.txt";
        }
        arguments << percentilesFile;
        arguments << plotFile;
    } else if (currentAnalysisType == STUDENT_TEST ||
               currentAnalysisType == ANOVA_TEST ||
               currentAnalysisType == SHAPIRO_WILK_TEST ||
               currentAnalysisType == WILCOXON_RANKSUM_TEST) {
        // Эти скрипты запускаются без аргументов
        // Они читают данные из стандартных файлов результатов
        // Не передаем аргументы
    } else {
        // Для MLE/MLS: script mode
        arguments << mode;
    }

    qDebug() << "Starting Python:";
    qDebug() << "  Executable:" << pythonExe;
    qDebug() << "  Script:" << pythonScript;
    qDebug() << "  Arguments:" << arguments;
    qDebug() << "  Working dir:" << rootPath;

    process.start(pythonExe, arguments);

    if (!process.waitForFinished(10000)) {
        qDebug() << "ERROR: Python process timeout or failed to start";
        qDebug() << "Error:" << process.errorString();
        statusBar()->showMessage("Ошибка генерации графика: timeout", 5000);
        updateFileList();
        return;
    }

    // Получаем вывод Python
    QString output = QString::fromUtf8(process.readAllStandardOutput());
    QString errors = QString::fromUtf8(process.readAllStandardError());

    qDebug() << "Python output:" << output;
    if (!errors.isEmpty()) {
        qDebug() << "Python errors:" << errors;
    }

    int exitCode = process.exitCode();
    qDebug() << "Python exit code:" << exitCode;

    // Для критерия Стьюдента ищем первый существующий файл
    if (currentAnalysisType == STUDENT_TEST) {
        QStringList possiblePlots;
        possiblePlots << (rootPath + "/output/plot_student_auto.png");
        possiblePlots << (rootPath + "/output/plot_student_equal_var.png");
        possiblePlots << (rootPath + "/output/plot_student_unequal_var.png");

        plotFile = "";
        for (const QString& file : possiblePlots) {
            if (QFile::exists(file)) {
                plotFile = file;
                qDebug() << "Found plot file:" << plotFile;
                break;
            }
        }
    }

    // Проверяем что файл был создан
    if (!plotFile.isEmpty() && QFile::exists(plotFile)) {
        qDebug() << "Plot file created successfully:" << plotFile;

        // Загружаем и показываем график
        QPixmap plot(plotFile);
        if (!plot.isNull()) {
            qDebug() << "Plot loaded, size:" << plot.size();
            chartViewer->showImage(plot);
            statusBar()->showMessage("График построен успешно", 3000);
        } else {
            qDebug() << "ERROR: Failed to load plot image";
            statusBar()->showMessage("Ошибка загрузки графика", 5000);
        }
    } else {
        qDebug() << "ERROR: Plot file was not created:" << plotFile;
        statusBar()->showMessage("Ошибка: график не создан", 5000);
    }

    // Обновить список выходных файлов
    updateFileList();
}

/**
 * @brief Изменение типа анализа
 */
void MainWindow::onAnalysisTypeChanged(int index)
{
    currentAnalysisType = static_cast<AnalysisType>(index);

    // Управление доступностью вкладки "График"
    // Отключаем вкладку для Граббса и Фишера (у них нет графиков)
    bool hasChart = true;
    if (currentAnalysisType == GRUBBS_TEST || currentAnalysisType == FISHER_TEST) {
        hasChart = false;
        // Переключаемся на вкладку результатов, если текущая - график
        if (ui->tabWidget->currentIndex() == 1) {
            ui->tabWidget->setCurrentIndex(0);
        }
    }

    // Включаем/отключаем вкладку "График" (индекс 1)
    ui->tabWidget->setTabEnabled(1, hasChart);

    // Для тестов с двумя выборками нужно специальное окно
    if (index == 4 || index == 5) {  // Fisher или Student
        statusBar()->showMessage("Для этого теста требуется две выборки", 3000);
    }
}

/**
 * @brief Выбор входного файла
 */
void MainWindow::onInputFileSelected(const QString& fileName)
{
    if (!fileName.isEmpty()) {
        QString fullPath = getProjectRootPath() + "/input/" + fileName;
        currentData = loadDataFromFile(fullPath);

        if (dataEditor) {
            dataEditor->setData(currentData);
        }

        // Показать статистику
        if (!currentData.empty()) {
            double sum = std::accumulate(currentData.begin(), currentData.end(), 0.0);
            double mean = sum / currentData.size();
            double sq_sum = std::accumulate(currentData.begin(), currentData.end(), 0.0,
                [mean](double acc, double val) { return acc + (val - mean) * (val - mean); });
            double variance = sq_sum / (currentData.size() - 1);
            double std_dev = std::sqrt(variance);

            QString stats = QString("n = %1\nСреднее: %2\nСКО: %3\nМин: %4\nМакс: %5")
                .arg(currentData.size())
                .arg(mean, 0, 'f', 4)
                .arg(std_dev, 0, 'f', 4)
                .arg(*std::min_element(currentData.begin(), currentData.end()), 0, 'f', 4)
                .arg(*std::max_element(currentData.begin(), currentData.end()), 0, 'f', 4);

            ui->statsText->setPlainText(stats);
            statusBar()->showMessage(QString("Загружено %1 значений из %2")
                .arg(currentData.size()).arg(fileName), 3000);
        }
    }
}

/**
 * @brief Выбор выходного файла для просмотра
 */
void MainWindow::onOutputFileSelected(const QString& fileName)
{
    if (!fileName.isEmpty()) {
        QString fullPath = getProjectRootPath() + "/output/" + fileName;

        // Если это PNG файл - показать как изображение
        if (fileName.endsWith(".png", Qt::CaseInsensitive)) {
            QPixmap pixmap(fullPath);
            if (!pixmap.isNull()) {
                // Показать изображение во вкладке графиков
                chartViewer->showImage(pixmap);
                ui->tabWidget->setCurrentIndex(1); // Переключить на вкладку графиков
                ui->fileContentText->setPlainText("Изображение отображается на вкладке 'Графики'");
            } else {
                ui->fileContentText->setPlainText("Ошибка загрузки изображения");
            }
        } else {
            // Текстовый файл - показать содержимое
            QFile file(fullPath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&file);
                ui->fileContentText->setPlainText(in.readAll());
                file.close();
            }
        }
    }
}

/**
 * @brief Обновление списка файлов
 */
QString MainWindow::getProjectRootPath() const
{
    // Получаем путь к исполняемому файлу приложения
    QString appPath = QCoreApplication::applicationDirPath();
    qDebug() << "Application directory:" << appPath;

    QDir appDir(appPath);

    // На macOS: .app/Contents/MacOS -> поднимаемся на 5 уровней
    // На других платформах: build -> поднимаемся на 2 уровня
    if (appPath.contains(".app")) {
        // macOS bundle: .app/Contents/MacOS -> build -> gui -> alg2
        appDir.cdUp();  // Contents
        appDir.cdUp();  // .app
        appDir.cdUp();  // build
        appDir.cdUp();  // gui
        appDir.cdUp();  // alg2
    } else {
        // Direct build
        appDir.cdUp();  // build
        appDir.cdUp();  // gui
        appDir.cdUp();  // alg2
    }

    QString rootPath = appDir.absolutePath();
    qDebug() << "Project root path:" << rootPath;
    return rootPath;
}

void MainWindow::updateFileList()
{
    QString rootPath = getProjectRootPath();
    QString inputPath = rootPath + "/input";

    qDebug() << "Checking input directory:" << inputPath;

    // Обновить список входных файлов
    ui->inputFilesList->clear();
    QDir inputDir(inputPath);

    qDebug() << "Input directory exists:" << inputDir.exists();

    if (inputDir.exists()) {
        QStringList inputFiles = inputDir.entryList(QStringList() << "*.txt", QDir::Files);
        qDebug() << "Found input files:" << inputFiles;
        ui->inputFilesList->addItems(inputFiles);
    } else {
        qDebug() << "ERROR: Input directory does not exist!";
    }

    // Обновить список выходных файлов
    ui->outputFilesList->clear();
    QDir outputDir(rootPath + "/output");
    if (outputDir.exists()) {
        QStringList outputFiles = outputDir.entryList(QStringList() << "*.txt" << "*.png", QDir::Files);

        // Фильтруем файлы: убираем PNG для Граббса и Фишера (у них нет графиков)
        QStringList filteredFiles;
        for (const QString& file : outputFiles) {
            // Исключаем несуществующие PNG файлы для Граббса и Фишера
            if (file.contains("grubbs") && file.endsWith(".png")) {
                continue; // Пропускаем - у Граббса нет графика
            }
            if (file.contains("fisher") && file.endsWith(".png")) {
                continue; // Пропускаем - у Фишера нет графика
            }
            filteredFiles.append(file);
        }

        ui->outputFilesList->addItems(filteredFiles);

        // Показать количество файлов в строке состояния
        if (filteredFiles.isEmpty()) {
            statusBar()->showMessage("Нет выходных файлов", 2000);
        } else {
            statusBar()->showMessage(QString("Найдено %1 выходных файлов").arg(filteredFiles.size()), 2000);
        }
    }
}

/**
 * @brief Показать сообщение об ошибке
 */
void MainWindow::showError(const QString& message)
{
    QMessageBox::critical(this, "Ошибка", message);
    statusBar()->showMessage("Ошибка: " + message, 5000);
}

/**
 * @brief Показать сообщение об успехе
 */
void MainWindow::showSuccess(const QString& message)
{
    statusBar()->showMessage(message, 3000);
}

void MainWindow::onEditData()
{
    // TODO: implement
}

void MainWindow::onShowFiles()
{
    updateFileList();
    statusBar()->showMessage("Список файлов обновлен", 2000);
}

void MainWindow::onShowChart()
{
    chartViewer->showHistogram(currentData, "Распределение данных");
}

void MainWindow::onExportChart()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Экспорт графика", "chart.png", "PNG Images (*.png)");
    if (!fileName.isEmpty()) {
        chartViewer->exportToPNG(fileName);
    }
}

void MainWindow::onDistributionChanged(int)
{
    // TODO: implement
}
