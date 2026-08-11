#pragma once
#include <QWidget>

class QComboBox;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
// Qt 6 中 QtCharts 类位于全局命名空间(QT_BEGIN_NAMESPACE 展开为空)
class QChart;
class QChartView;
class QBarSeries;
class QPieSeries;

class StatisticsPage : public QWidget {
    Q_OBJECT
public:
    explicit StatisticsPage(QWidget *parent = nullptr);
private slots:
    void refreshCharts();
private:
    QComboBox *m_courseCombo;
    QHBoxLayout *m_chartLayout;
    void reloadFilters();
    void clearChartArea();
    QChart *buildClassAvgChart();
    QChart *buildScoreDistChart();
    QChart *buildCourseAvgChart();
};
