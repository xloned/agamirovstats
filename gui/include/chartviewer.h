#ifndef CHARTVIEWER_H
#define CHARTVIEWER_H

#include <QWidget>
#include <QLabel>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <vector>

class ChartViewer : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ChartViewer)

public:
    explicit ChartViewer(QWidget *parent = nullptr);
    ~ChartViewer();

    void showHistogram(const std::vector<double>& data, const QString& title);
    void showQQPlot(const std::vector<double>& data, const QString& distribution);
    void showScatterPlot(const std::vector<double>& x, const std::vector<double>& y);
    void showDistributionFit(const std::vector<double>& data,
                            const QString& distribution,
                            double param1, double param2);
    void showConfidenceIntervals(const std::vector<double>& means,
                                const std::vector<double>& lower,
                                const std::vector<double>& upper,
                                const QStringList& labels);

    void clearChart();
    void exportToPNG(const QString& fileName);
    void setTitle(const QString& title);
    void showImage(const QPixmap& pixmap);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupChart();
    void updateImageDisplay();
    std::vector<double> generateNormalPDF(double mean, double std, int points = 100);
    std::vector<double> generateWeibullPDF(double lambda, double k, int points = 100);

    QChartView* chartView;
    QChart* chart;
    QLabel* imageLabel;
    QPixmap originalPixmap;  // Сохраняем оригинал для перемасштабирования
};

#endif // CHARTVIEWER_H
