#include "TeacherPage.h"
#include "../models/TableModel.h"
#include "../database/DbManager.h"
#include "../dialogs/TeacherEditDialog.h"
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>

TeacherPage::TeacherPage(QWidget *parent) : QWidget(parent) {
    auto *title = new QLabel("教师管理");
    title->setObjectName("pageTitle");
    auto *titleLine = new QFrame;
    titleLine->setObjectName("titleLine");
    titleLine->setFrameShape(QFrame::NoFrame);

    m_search = new QLineEdit;
    m_search->setPlaceholderText("搜索教师编号/姓名/职称");

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
        {"teacher_no", "教师编号"},
        {"name", "姓名"},
        {"gender", "性别"},
        {"title", "职称"},
        {"phone", "电话"},
        {"email", "邮箱"},
    });

    m_view = new QTableView;
    m_view->setModel(m_model);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->horizontalHeader()->setStretchLastSection(true);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->addWidget(title);
    layout->addWidget(titleLine);
    layout->addSpacing(6);
    layout->addLayout(bar);
    layout->addWidget(m_view, 1);

    connect(m_search, &QLineEdit::textChanged, this, &TeacherPage::refresh);
    connect(btnAdd, &QPushButton::clicked, this, &TeacherPage::onAdd);
    connect(btnEdit, &QPushButton::clicked, this, &TeacherPage::onEdit);
    connect(btnDel, &QPushButton::clicked, this, &TeacherPage::onDelete);
    connect(btnRefresh, &QPushButton::clicked, this, &TeacherPage::refresh);

    refresh();
}

void TeacherPage::refresh() {
    QString sql = "SELECT id, teacher_no, name, gender, title, phone, email "
                  "FROM teachers";
    QVariantList args;
    QString kw = m_search->text().trimmed();
    if (!kw.isEmpty()) {
        sql += " WHERE teacher_no LIKE ? OR name LIKE ? OR title LIKE ?";
        QString like = "%" + kw + "%";
        args << like << like << like;
    }
    sql += " ORDER BY teacher_no";
    m_model->load(sql, args);
    if (!m_model->lastError().isEmpty())
        QMessageBox::critical(this, "数据库错误", m_model->lastError());
    m_view->resizeColumnsToContents();
}

void TeacherPage::onAdd() {
    TeacherEditDialog dlg(QVariantMap(), this);
    if (dlg.exec() != QDialog::Accepted) return;
    QVariantMap f = dlg.fields();
    if (!DbManager::instance().execUpdate(
            "INSERT INTO teachers (teacher_no, name, gender, title, phone, email) "
            "VALUES (?, ?, ?, ?, ?, ?)",
            {f["teacher_no"], f["name"], f["gender"], f["title"], f["phone"], f["email"]})) {
        QMessageBox::critical(this, "新增失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void TeacherPage::onEdit() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要修改的行"); return; }
    TeacherEditDialog dlg(data, this);
    if (dlg.exec() != QDialog::Accepted) return;
    QVariantMap f = dlg.fields();
    if (!DbManager::instance().execUpdate(
            "UPDATE teachers SET teacher_no=?, name=?, gender=?, title=?, phone=?, email=? WHERE id=?",
            {f["teacher_no"], f["name"], f["gender"], f["title"], f["phone"], f["email"], data["id"]})) {
        QMessageBox::critical(this, "修改失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void TeacherPage::onDelete() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要删除的行"); return; }
    if (QMessageBox::question(this, "确认", QString("确定删除教师 \"%1\" 吗?").arg(data["name"].toString()))
            != QMessageBox::Yes) return;
    if (!DbManager::instance().execUpdate("DELETE FROM teachers WHERE id=?", {data["id"]})) {
        QMessageBox::critical(this, "删除失败",
            "删除失败(该教师可能被班级或课程引用):\n" + DbManager::instance().lastError());
        return;
    }
    refresh();
}
