#include "MainWindow.h"
#include "ClassPage.h"
#include "TeacherPage.h"
#include "StudentPage.h"
#include "CoursePage.h"
#include "ScorePage.h"
#include "StatisticsPage.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("学生管理系统");
    resize(1100, 680);

    m_nav = new QListWidget;
    m_nav->setFixedWidth(160);
    m_stack = new QStackedWidget;

    auto *central = new QWidget;
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_nav);
    layout->addWidget(m_stack, 1);
    setCentralWidget(central);

    // 导航项带图标(emoji 由系统字体渲染)
    addPage(new StudentPage, "学生管理", "🎓");
    addPage(new TeacherPage, "教师管理", "💼");
    addPage(new ClassPage, "班级管理", "🏫");
    addPage(new CoursePage, "课程管理", "📖");
    addPage(new ScorePage, "成绩管理", "📊");
    addPage(new StatisticsPage, "统计分析", "📈");

    connect(m_nav, &QListWidget::currentRowChanged,
            m_stack, &QStackedWidget::setCurrentIndex);
    connect(m_nav, &QListWidget::currentRowChanged, this, [this](int row) {
        statusBar()->showMessage(QString("当前页面: %1").arg(m_nav->item(row)->text()));
    });
    m_nav->setCurrentRow(0);
    statusBar()->showMessage("就绪");
}

void MainWindow::addPage(QWidget *page, const QString &title, const QString &icon) {
    m_stack->addWidget(page);
    m_nav->addItem(new QListWidgetItem(icon.isEmpty() ? title : icon + "  " + title));
}
