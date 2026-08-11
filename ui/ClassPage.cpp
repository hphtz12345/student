#include "ClassPage.h"
#include "../models/TableModel.h"
#include "../database/DbManager.h"
#include "../dialogs/ClassEditDialog.h"
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

ClassPage::ClassPage(QWidget *parent) : QWidget(parent) {
    auto *title = new QLabel("班级管理");
    title->setObjectName("pageTitle");
    auto *titleLine = new QFrame;
    titleLine->setObjectName("titleLine");
    titleLine->setFrameShape(QFrame::NoFrame);

    m_search = new QLineEdit;
    m_search->setPlaceholderText("搜索班级名称/专业");

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
        {"class_name", "班级名称"},
        {"grade", "年级"},
        {"major", "专业"},
        {"teacher_name", "班主任"},
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

    connect(m_search, &QLineEdit::textChanged, this, &ClassPage::refresh);
    connect(m_btnAdd, &QPushButton::clicked, this, &ClassPage::onAdd);
    connect(m_btnEdit, &QPushButton::clicked, this, &ClassPage::onEdit);
    connect(m_btnDel, &QPushButton::clicked, this, &ClassPage::onDelete);
    connect(btnRefresh, &QPushButton::clicked, this, &ClassPage::refresh);

    refresh();
}

void ClassPage::setReadOnly(bool ro) {
    m_readOnly = ro;
    m_btnAdd->setEnabled(!ro);
    m_btnEdit->setEnabled(!ro);
    m_btnDel->setEnabled(!ro);
}

void ClassPage::refresh() {
    QString sql = "SELECT c.id, c.class_name, c.grade, c.major, c.head_teacher_id, "
                  "COALESCE(t.name, '') AS teacher_name "
                  "FROM classes c LEFT JOIN teachers t ON t.id = c.head_teacher_id";
    QVariantList args;
    QString kw = m_search->text().trimmed();
    if (!kw.isEmpty()) {
        sql += " WHERE c.class_name LIKE ? OR c.major LIKE ?";
        QString like = "%" + kw + "%";
        args << like << like;
    }
    sql += " ORDER BY c.grade, c.class_name";
    m_model->load(sql, args);
    if (!m_model->lastError().isEmpty())
        QMessageBox::critical(this, "数据库错误", m_model->lastError());
    m_view->resizeColumnsToContents();
}

void ClassPage::onAdd() {
    ClassEditDialog dlg(QVariantMap(), this);
    if (dlg.exec() != QDialog::Accepted) return;
    QVariantMap f = dlg.fields();
    if (!DbManager::instance().execUpdate(
            "INSERT INTO classes (class_name, grade, major, head_teacher_id) "
            "VALUES (?, ?, ?, ?)",
            {f["class_name"], f["grade"], f["major"], f["head_teacher_id"]})) {
        QMessageBox::critical(this, "新增失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void ClassPage::onEdit() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要修改的行"); return; }
    ClassEditDialog dlg(data, this);
    if (dlg.exec() != QDialog::Accepted) return;
    QVariantMap f = dlg.fields();
    if (!DbManager::instance().execUpdate(
            "UPDATE classes SET class_name=?, grade=?, major=?, head_teacher_id=? WHERE id=?",
            {f["class_name"], f["grade"], f["major"], f["head_teacher_id"], data["id"]})) {
        QMessageBox::critical(this, "修改失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void ClassPage::onDelete() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要删除的行"); return; }
    if (QMessageBox::question(this, "确认", QString("确定删除班级 \"%1\" 吗?").arg(data["class_name"].toString()))
            != QMessageBox::Yes) return;
    if (!DbManager::instance().execUpdate("DELETE FROM classes WHERE id=?", {data["id"]})) {
        QMessageBox::critical(this, "删除失败",
            "删除失败(该班级可能仍有学生或关联数据):\n" + DbManager::instance().lastError());
        return;
    }
    refresh();
}
