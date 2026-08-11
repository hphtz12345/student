#include "CoursePage.h"
#include "../models/TableModel.h"
#include "../database/DbManager.h"
#include "../dialogs/CourseEditDialog.h"
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>

CoursePage::CoursePage(QWidget *parent) : QWidget(parent) {
    m_search = new QLineEdit;
    m_search->setPlaceholderText("搜索课程编号/课程名称/教师");

    auto *btnAdd = new QPushButton("新增");
    auto *btnEdit = new QPushButton("修改");
    auto *btnDel = new QPushButton("删除");
    auto *btnRefresh = new QPushButton("刷新");

    auto *bar = new QHBoxLayout;
    bar->addWidget(m_search);
    bar->addWidget(btnAdd);
    bar->addWidget(btnEdit);
    bar->addWidget(btnDel);
    bar->addWidget(btnRefresh);

    m_model = new TableModel(this);
    m_model->setColumns({
        {"course_no", "课程编号"},
        {"course_name", "课程名称"},
        {"credit", "学分"},
        {"teacher_name", "授课教师"},
    });

    m_view = new QTableView;
    m_view->setModel(m_model);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->horizontalHeader()->setStretchLastSection(true);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(bar);
    layout->addWidget(m_view, 1);

    connect(m_search, &QLineEdit::textChanged, this, &CoursePage::refresh);
    connect(btnAdd, &QPushButton::clicked, this, &CoursePage::onAdd);
    connect(btnEdit, &QPushButton::clicked, this, &CoursePage::onEdit);
    connect(btnDel, &QPushButton::clicked, this, &CoursePage::onDelete);
    connect(btnRefresh, &QPushButton::clicked, this, &CoursePage::refresh);

    refresh();
}

void CoursePage::refresh() {
    QString sql = "SELECT c.id, c.course_no, c.course_name, c.credit, "
                  "COALESCE(t.name, '') AS teacher_name "
                  "FROM courses c LEFT JOIN teachers t ON t.id = c.teacher_id";
    QVariantList args;
    QString kw = m_search->text().trimmed();
    if (!kw.isEmpty()) {
        sql += " WHERE c.course_no LIKE ? OR c.course_name LIKE ? OR t.name LIKE ?";
        QString like = "%" + kw + "%";
        args << like << like << like;
    }
    sql += " ORDER BY c.course_no";
    m_model->load(sql, args);
    if (!m_model->lastError().isEmpty())
        QMessageBox::critical(this, "数据库错误", m_model->lastError());
}

void CoursePage::onAdd() {
    CourseEditDialog dlg(QVariantMap(), this);
    if (dlg.exec() != QDialog::Accepted) return;
    QVariantMap f = dlg.fields();
    if (!DbManager::instance().execUpdate(
            "INSERT INTO courses (course_no, course_name, credit, teacher_id) "
            "VALUES (?, ?, ?, ?)",
            {f["course_no"], f["course_name"], f["credit"], f["teacher_id"]})) {
        QMessageBox::critical(this, "新增失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void CoursePage::onEdit() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要修改的行"); return; }
    CourseEditDialog dlg(data, this);
    if (dlg.exec() != QDialog::Accepted) return;
    QVariantMap f = dlg.fields();
    if (!DbManager::instance().execUpdate(
            "UPDATE courses SET course_no=?, course_name=?, credit=?, teacher_id=? WHERE id=?",
            {f["course_no"], f["course_name"], f["credit"], f["teacher_id"], data["id"]})) {
        QMessageBox::critical(this, "修改失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void CoursePage::onDelete() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要删除的行"); return; }
    if (QMessageBox::question(this, "确认", QString("确定删除课程 \"%1\" 吗?").arg(data["course_name"].toString()))
            != QMessageBox::Yes) return;
    if (!DbManager::instance().execUpdate("DELETE FROM courses WHERE id=?", {data["id"]})) {
        QMessageBox::critical(this, "删除失败",
            "删除失败(该课程可能已有成绩记录):\n" + DbManager::instance().lastError());
        return;
    }
    refresh();
}
