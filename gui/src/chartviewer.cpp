#include "chartviewer.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QResizeEvent>
#include <QTimer>
#include <cmath>
#include <algorithm>

ChartViewer::ChartViewer(QWidget *parent)
    : QWidget(parent)
    , imageLabel(nullptr)
    , originalPixmap()
{
    setupChart();
}

ChartViewer::~ChartViewer()
{
}

void ChartViewer::setupChart()
{
    chart = new QChart();
    chart->setTitle("График");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);

    // Создать label для изображений высокого разрешения
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setScaledContents(true); // ИСПОЛЬЗОВАТЬ автоматическое масштабирование на весь размер
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored); // Игнорировать размер pixmap
    imageLabel->setMinimumSize(200, 200);
    imageLabel->hide();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(chartView);
    layout->addWidget(imageLabel);
    setLayout(layout);
}

void ChartViewer::showHistogram(const std::vector<double>& data, const QString& title)
{
    // Скрыть изображение, показать график
    imageLabel->hide();
    chartView->show();

    clearChart();
    if (data.empty()) return;

    double min_val = *std::min_element(data.begin(), data.end());
    double max_val = *std::max_element(data.begin(), data.end());
    int bins = std::min(20, static_cast<int>(std::sqrt(data.size())));
    double bin_width = (max_val - min_val) / bins;

    std::vector<int> frequencies(bins, 0);
    for (double x : data) {
        int bin = std::min(static_cast<int>((x - min_val) / bin_width), bins - 1);
        frequencies[bin]++;
    }

    QBarSet* barSet = new QBarSet("Частота");
    QStringList categories;

    for (int i = 0; i < bins; i++) {
        *barSet << frequencies[i];
        double bin_center = min_val + (i + 0.5) * bin_width;
        categories << QString::number(bin_center, 'f', 2);
    }

    QBarSeries* series = new QBarSeries();
    series->append(barSet);

    chart->addSeries(series);
    chart->setTitle(title);

    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Частота");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
}

void ChartViewer::showQQPlot(const std::vector<double>&, const QString&)
{
    // TODO: implement
}

void ChartViewer::showScatterPlot(const std::vector<double>&, const std::vector<double>&)
{
    // TODO: implement
}

void ChartViewer::showDistributionFit(const std::vector<double>& data,
                                     const QString&,
                                     double, double)
{
    clearChart();
    if (data.empty()) return;

    QScatterSeries* scatterSeries = new QScatterSeries();
    scatterSeries->setName("Данные");
    scatterSeries->setMarkerSize(8.0);

    for (size_t i = 0; i < data.size(); i++) {
        scatterSeries->append(i, data[i]);
    }

    chart->addSeries(scatterSeries);
    chart->createDefaultAxes();
}

void ChartViewer::showConfidenceIntervals(const std::vector<double>&,
                                         const std::vector<double>&,
                                         const std::vector<double>&,
                                         const QStringList&)
{
    // TODO: implement
}

void ChartViewer::clearChart()
{
    chart->removeAllSeries();

    // Удалить все оси
    for (auto axis : chart->axes()) {
        chart->removeAxis(axis);
    }
}

void ChartViewer::exportToPNG(const QString& fileName)
{
    QPixmap pixmap = chartView->grab();
    pixmap.save(fileName, "PNG");
}

void ChartViewer::setTitle(const QString& title)
{
    chart->setTitle(title);
}

void ChartViewer::showImage(const QPixmap& pixmap)
{
    // Сохраняем оригинал для последующего перемасштабирования
    originalPixmap = pixmap;

    // Скрыть график, показать изображение
    chartView->hide();

    // Просто устанавливаем pixmap - setScaledContents(true) сделает автомасштабирование
    imageLabel->setPixmap(pixmap);
    qDebug() << "ChartViewer: Image set, original size:" << pixmap.size();

    imageLabel->show();
}

void ChartViewer::updateImageDisplay()
{
    if (originalPixmap.isNull()) {
        qDebug() << "ChartViewer: originalPixmap is null";
        return;
    }

    // Получаем размер виджета
    QSize widgetSize = this->size();
    qDebug() << "ChartViewer: widget size:" << widgetSize;

    // Проверяем что размер валидный
    if (widgetSize.width() <= 100 || widgetSize.height() <= 100) {
        qDebug() << "ChartViewer: widget size too small, scheduling retry...";
        // Виджет ещё не инициализирован, попробуем позже
        QTimer::singleShot(200, this, [this]() {
            updateImageDisplay();
        });
        return;
    }

    // Учитываем margins layout
    QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(this->layout());
    if (layout) {
        QMargins margins = layout->contentsMargins();
        widgetSize -= QSize(margins.left() + margins.right(),
                           margins.top() + margins.bottom());
    }

    qDebug() << "ChartViewer: scaling to size:" << widgetSize;

    // Масштабируем изображение на ВЕСЬ доступный размер
    // Qt::SmoothTransformation использует билинейную фильтрацию для высокого качества
    QPixmap scaled = originalPixmap.scaled(
        widgetSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    qDebug() << "ChartViewer: scaled pixmap size:" << scaled.size();

    // Для Retina дисплеев устанавливаем правильный devicePixelRatio
    scaled.setDevicePixelRatio(this->devicePixelRatio());

    imageLabel->setPixmap(scaled);
    qDebug() << "ChartViewer: pixmap set to label";
}

void ChartViewer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    // Если отображается изображение, перемасштабируем его
    if (!originalPixmap.isNull() && imageLabel->isVisible()) {
        updateImageDisplay();
    }
}

std::vector<double> ChartViewer::generateNormalPDF(double mean, double std, int points)
{
    std::vector<double> pdf(points);
    double step = (6 * std) / points;

    for (int i = 0; i < points; i++) {
        double x = mean - 3 * std + i * step;
        double exponent = -0.5 * std::pow((x - mean) / std, 2);
        pdf[i] = (1.0 / (std * std::sqrt(2 * M_PI))) * std::exp(exponent);
    }

    return pdf;
}

std::vector<double> ChartViewer::generateWeibullPDF(double lambda, double k, int points)
{
    std::vector<double> pdf(points);
    double step = (3 * lambda) / points;

    for (int i = 0; i < points; i++) {
        double x = i * step;
        if (x > 0) {
            pdf[i] = (k / lambda) * std::pow(x / lambda, k - 1) * std::exp(-std::pow(x / lambda, k));
        } else {
            pdf[i] = 0;
        }
    }

    return pdf;
}
