#include "StatisticsPage.h"
#include "../database/DbManager.h"
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QHorizontalBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QSqlQuery>
#include <QSqlError>

// 注:Qt 6 中 QtCharts 类位于全局命名空间(QT_BEGIN_NAMESPACE 展开为空),无需 using

StatisticsPage::StatisticsPage(QWidget *parent) : QWidget(parent) {
    auto *title = new QLabel("统计分析");
    title->setStyleSheet("font-size: 16px; font-weight: bold;");

    m_courseCombo = new QComboBox;
    reloadFilters();

    auto *btnRefresh = new QPushButton("刷新图表");

    auto *bar = new QHBoxLayout;
    bar->addWidget(title);
    bar->addStretch();
    bar->addWidget(new QLabel("课程:"));
    bar->addWidget(m_courseCombo);
    bar->addWidget(btnRefresh);

    // 二级菜单:选项卡切换查看不同类型的统计图表
    m_tabs = new QTabWidget;
    for (const QString &tabName : {"各班平均分", "成绩分数段分布", "各课程平均分"}) {
        auto *page = new QWidget;
        page->setLayout(new QVBoxLayout);
        m_tabs->addTab(page, tabName);
    }

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(bar);
    layout->addWidget(m_tabs, 1);

    connect(btnRefresh, &QPushButton::clicked, this, &StatisticsPage::refreshCharts);
    connect(m_courseCombo, &QComboBox::currentIndexChanged, this, &StatisticsPage::refreshCharts);

    refreshCharts();
}

void StatisticsPage::reloadFilters() {
    // 保留当前选中,重建期间屏蔽信号避免触发 refreshCharts
    QVariant cur = m_courseCombo->currentData();
    m_courseCombo->blockSignals(true);
    m_courseCombo->clear();
    m_courseCombo->addItem("全部课程", QVariant());
    QSqlQuery qc = DbManager::instance().execQuery(
        "SELECT id, course_no, course_name FROM courses ORDER BY course_no");
    while (qc.next()) {
        m_courseCombo->addItem(qc.value("course_no").toString() + " " + qc.value("course_name").toString(),
                               qc.value("id"));
        if (!cur.isNull() && qc.value("id") == cur)
            m_courseCombo->setCurrentIndex(m_courseCombo->count() - 1);
    }
    m_courseCombo->blockSignals(false);
}

void StatisticsPage::clearChartArea() {
    for (int i = 0; i < m_tabs->count(); ++i) {
        QLayout *l = m_tabs->widget(i)->layout();
        while (QLayoutItem *item = l->takeAt(0)) {
            if (item->widget()) { item->widget()->deleteLater(); }
            delete item;
        }
    }
}

QChart *StatisticsPage::buildClassAvgChart() {
    QSqlQuery q = DbManager::instance().execQuery(
        "SELECT c.class_name, ROUND(AVG(sc.score), 1) AS avg_score "
        "FROM scores sc JOIN students st ON st.id = sc.student_id "
        "JOIN classes c ON c.id = st.class_id "
        "GROUP BY c.id ORDER BY avg_score DESC");
    if (!q.isActive()) {
        QMessageBox::critical(this, "数据库错误", DbManager::instance().lastError());
    }
    auto *chart = new QChart;
    chart->setTitle("各班平均分");
    auto *series = new QBarSeries;
    auto *set = new QBarSet("平均分");
    QStringList cats;
    while (q.next()) {
        cats << q.value("class_name").toString();
        *set << q.value("avg_score").toDouble();
    }
    series->append(set);
    chart->addSeries(series);
    auto *axisX = new QBarCategoryAxis;
    axisX->append(cats);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    auto *axisY = new QValueAxis;
    axisY->setRange(0, 100);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    chart->legend()->hide();
    return chart;
}

QChart *StatisticsPage::buildScoreDistChart() {
    int courseId = m_courseCombo->currentIndex() >= 0
                   ? m_courseCombo->currentData().toInt() : 0;
    QSqlQuery q = DbManager::instance().execQuery(
        "SELECT CASE WHEN score >= 90 THEN '90-100' WHEN score >= 80 THEN '80-89' "
        "WHEN score >= 70 THEN '70-79' WHEN score >= 60 THEN '60-69' "
        "ELSE '不及格' END AS band, COUNT(*) AS cnt "
        "FROM scores WHERE (? <= 0 OR course_id = ?) GROUP BY band",
        {courseId, courseId});
    if (!q.isActive()) {
        QMessageBox::critical(this, "数据库错误", DbManager::instance().lastError());
    }
    auto *chart = new QChart;
    chart->setTitle("成绩分数段分布");
    auto *series = new QPieSeries;
    while (q.next()) {
        series->append(q.value("band").toString() + " (" + q.value("cnt").toString() + ")",
                       q.value("cnt").toInt());
    }
    chart->addSeries(series);
    chart->legend()->setAlignment(Qt::AlignRight);
    return chart;
}

QChart *StatisticsPage::buildCourseAvgChart() {
    QSqlQuery q = DbManager::instance().execQuery(
        "SELECT c.course_name, ROUND(AVG(sc.score), 1) AS avg_score "
        "FROM scores sc JOIN courses c ON c.id = sc.course_id "
        "GROUP BY c.id ORDER BY avg_score DESC");
    if (!q.isActive()) {
        QMessageBox::critical(this, "数据库错误", DbManager::instance().lastError());
    }
    auto *chart = new QChart;
    chart->setTitle("各课程平均分");
    // 规格 §5.4:各课程平均分为横向条形图
    auto *series = new QHorizontalBarSeries;
    auto *set = new QBarSet("平均分");
    QStringList cats;
    while (q.next()) {
        cats << q.value("course_name").toString();
        *set << q.value("avg_score").toDouble();
    }
    series->append(set);
    chart->addSeries(series);
    // 横向条形图:类别轴在左侧,数值轴在下侧
    auto *axisY = new QBarCategoryAxis;
    axisY->append(cats);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    auto *axisX = new QValueAxis;
    axisX->setRange(0, 100);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    chart->legend()->hide();
    return chart;
}

void StatisticsPage::refreshCharts() {
    reloadFilters();
    clearChartArea();
    QChartView *v1 = new QChartView(buildClassAvgChart());
    QChartView *v2 = new QChartView(buildScoreDistChart());
    QChartView *v3 = new QChartView(buildCourseAvgChart());
    for (QChartView *v : {v1, v2, v3}) {
        v->setRenderHint(QPainter::Antialiasing);
        v->setMinimumSize(320, 260);
    }
    // 每张图放进对应的选项卡页,单图显示空间更大
    m_tabs->widget(0)->layout()->addWidget(v1);
    m_tabs->widget(1)->layout()->addWidget(v2);
    m_tabs->widget(2)->layout()->addWidget(v3);
}
