#include "MainWindow.h"
#include "ClassPage.h"
#include "TeacherPage.h"
#include "StudentPage.h"
#include "CoursePage.h"
#include "ScorePage.h"
#include "StatisticsPage.h"
#include <QListWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("学生管理系统");
    resize(1000, 640);

    m_nav = new QListWidget;
    m_nav->setFixedWidth(140);
    m_stack = new QStackedWidget;

    auto *central = new QWidget;
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_nav);
    layout->addWidget(m_stack, 1);
    setCentralWidget(central);

    addPage(new StudentPage, "学生管理");
    addPage(new TeacherPage, "教师管理");
    addPage(new ClassPage, "班级管理");
    addPage(new CoursePage, "课程管理");
    addPage(new ScorePage, "成绩管理");
    addPage(new StatisticsPage, "统计分析");

    connect(m_nav, &QListWidget::currentRowChanged,
            m_stack, &QStackedWidget::setCurrentIndex);
    m_nav->setCurrentRow(0);
}

void MainWindow::addPage(QWidget *page, const QString &title) {
    m_stack->addWidget(page);
    m_nav->addItem(title);
}
