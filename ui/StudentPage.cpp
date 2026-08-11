#include "StudentPage.h"
#include "../models/TableModel.h"
#include "../database/DbManager.h"
#include "../dialogs/StudentEditDialog.h"
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

StudentPage::StudentPage(QWidget *parent) : QWidget(parent) {
    auto *title = new QLabel("学生管理");
    title->setObjectName("pageTitle");
    auto *titleLine = new QFrame;
    titleLine->setObjectName("titleLine");
    titleLine->setFrameShape(QFrame::NoFrame);

    m_search = new QLineEdit;
    m_search->setPlaceholderText("搜索学号/姓名/班级");

    m_btnAdd = new QPushButton("新增");
    m_btnEdit = new QPushButton("修改");
    m_btnDel = new QPushButton("删除");
    auto *btnRefresh = new QPushButton("刷新");

    auto *bar = new QHBoxLayout;
    bar->addWidget(m_search);
    bar->addWidget(m_btnAdd);
    bar->addWidget(m_btnEdit);
    bar->addWidget(m_btnDel);
    bar->addWidget(btnRefresh);

    m_model = new TableModel(this);
    m_model->setColumns({
        {"student_no", "学号"},
        {"name", "姓名"},
        {"gender", "性别"},
        {"birth_date", "出生日期"},
        {"class_name", "班级"},
        {"enroll_year", "入学年份"},
        {"status", "状态"},
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

    connect(m_search, &QLineEdit::textChanged, this, &StudentPage::refresh);
    connect(m_btnAdd, &QPushButton::clicked, this, &StudentPage::onAdd);
    connect(m_btnEdit, &QPushButton::clicked, this, &StudentPage::onEdit);
    connect(m_btnDel, &QPushButton::clicked, this, &StudentPage::onDelete);
    connect(btnRefresh, &QPushButton::clicked, this, &StudentPage::refresh);

    refresh();
}

void StudentPage::setReadOnly(bool ro) {
    m_readOnly = ro;
    m_btnAdd->setEnabled(!ro);
    m_btnEdit->setEnabled(!ro);
    m_btnDel->setEnabled(!ro);
}

void StudentPage::refresh() {
    QString sql = "SELECT s.id, s.student_no, s.name, s.gender, "
                  "DATE_FORMAT(s.birth_date, '%Y-%m-%d') AS birth_date, "
                  "COALESCE(c.class_name, '') AS class_name, "
                  "s.class_id, s.enroll_year, s.status, s.phone, s.email "
                  "FROM students s LEFT JOIN classes c ON c.id = s.class_id";
    QVariantList args;
    QString kw = m_search->text().trimmed();
    if (!kw.isEmpty()) {
        sql += " WHERE s.student_no LIKE ? OR s.name LIKE ? OR c.class_name LIKE ?";
        QString like = "%" + kw + "%";
        args << like << like << like;
    }
    sql += " ORDER BY s.student_no";
    m_model->load(sql, args);
    if (!m_model->lastError().isEmpty())
        QMessageBox::critical(this, "数据库错误", m_model->lastError());
    m_view->resizeColumnsToContents();
}

void StudentPage::onAdd() {
    StudentEditDialog dlg(QVariantMap(), this);
    if (dlg.exec() != QDialog::Accepted) return;
    QVariantMap f = dlg.fields();
    if (!DbManager::instance().execUpdate(
            "INSERT INTO students (student_no, name, gender, birth_date, phone, email, "
            "class_id, enroll_year, status) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
            {f["student_no"], f["name"], f["gender"], f["birth_date"], f["phone"],
             f["email"], f["class_id"], f["enroll_year"], f["status"]})) {
        QMessageBox::critical(this, "新增失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void StudentPage::onEdit() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要修改的行"); return; }
    StudentEditDialog dlg(data, this);
    if (dlg.exec() != QDialog::Accepted) return;
    QVariantMap f = dlg.fields();
    if (!DbManager::instance().execUpdate(
            "UPDATE students SET student_no=?, name=?, gender=?, birth_date=?, phone=?, email=?, "
            "class_id=?, enroll_year=?, status=? WHERE id=?",
            {f["student_no"], f["name"], f["gender"], f["birth_date"], f["phone"],
             f["email"], f["class_id"], f["enroll_year"], f["status"], data["id"]})) {
        QMessageBox::critical(this, "修改失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void StudentPage::onDelete() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要删除的行"); return; }
    if (QMessageBox::question(this, "确认", QString("确定删除学生 \"%1\" 吗?").arg(data["name"].toString()))
            != QMessageBox::Yes) return;
    if (!DbManager::instance().execUpdate("DELETE FROM students WHERE id=?", {data["id"]})) {
        QMessageBox::critical(this, "删除失败",
            "删除失败(该学生可能已有成绩记录):\n" + DbManager::instance().lastError());
        return;
    }
    refresh();
}
