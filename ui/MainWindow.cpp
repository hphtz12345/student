#include "MainWindow.h"
#include "ClassPage.h"
#include "TeacherPage.h"
#include "StudentPage.h"
#include "CoursePage.h"
#include "ScorePage.h"
#include "StatisticsPage.h"
#include "UserPage.h"
#include "../database/DbManager.h"
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

    // 按角色构建导航:管理员 7 项(含"用户管理"),普通用户 6 项
    const bool isAdmin = DbManager::currentRole() == "admin";
    addPage(new StudentPage, "学生管理", "🎓");
    addPage(new TeacherPage, "教师管理", "💼");
    addPage(new ClassPage, "班级管理", "🏫");
    addPage(new CoursePage, "课程管理", "📖");
    addPage(new ScorePage, "成绩管理", "📊");
    addPage(new StatisticsPage, "统计分析", "📈");
    if (isAdmin) {
        m_userPage = new UserPage;
        addPage(m_userPage, "用户管理", "🔑");
    }

    // 普通用户:数据页只读(遍历 stack 中的前 6 页;StatisticsPage 无写操作,无需只读)
    if (!isAdmin) {
        for (int i = 0; i < 6; ++i) {
            if (auto *p = qobject_cast<StudentPage *>(m_stack->widget(i)))
                p->setReadOnly(true);
            else if (auto *p = qobject_cast<TeacherPage *>(m_stack->widget(i)))
                p->setReadOnly(true);
            else if (auto *p = qobject_cast<ClassPage *>(m_stack->widget(i)))
                p->setReadOnly(true);
            else if (auto *p = qobject_cast<CoursePage *>(m_stack->widget(i)))
                p->setReadOnly(true);
            else if (auto *p = qobject_cast<ScorePage *>(m_stack->widget(i)))
                p->setReadOnly(true);
        }
    }

    connect(m_nav, &QListWidget::currentRowChanged,
            m_stack, &QStackedWidget::setCurrentIndex);
    connect(m_nav, &QListWidget::currentRowChanged, this, [this](int row) {
        const QString roleText = DbManager::currentRole() == "admin" ? "管理员" : "普通用户";
        statusBar()->showMessage(QString("当前用户: %1(%2) | 当前页面: %3")
                                     .arg(DbManager::currentUser(), roleText, m_nav->item(row)->text()));
    });
    m_nav->setCurrentRow(0);
}

void MainWindow::addPage(QWidget *page, const QString &title, const QString &icon) {
    m_stack->addWidget(page);
    m_nav->addItem(new QListWidgetItem(icon.isEmpty() ? title : icon + "  " + title));
}
