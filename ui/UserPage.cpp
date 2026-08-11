#include "UserPage.h"
#include "../models/TableModel.h"
#include "../database/DbManager.h"
#include "../dialogs/UserEditDialog.h"
#include "../dialogs/UserPasswordDialog.h"
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

UserPage::UserPage(QWidget *parent) : QWidget(parent) {
    auto *title = new QLabel("用户管理");
    title->setObjectName("pageTitle");
    auto *titleLine = new QFrame;
    titleLine->setObjectName("titleLine");
    titleLine->setFrameShape(QFrame::NoFrame);

    m_search = new QLineEdit;
    m_search->setPlaceholderText("搜索用户名");

    auto *btnAdd = new QPushButton("新增");
    auto *btnEdit = new QPushButton("修改");
    auto *btnDel = new QPushButton("删除");
    auto *btnPwd = new QPushButton("重置密码");

    auto *bar = new QHBoxLayout;
    bar->addWidget(m_search);
    bar->addWidget(btnAdd);
    bar->addWidget(btnEdit);
    bar->addWidget(btnDel);
    bar->addWidget(btnPwd);

    m_model = new TableModel(this);
    m_model->setColumns({
        {"username", "用户名"},
        {"role", "角色"},
        {"created_at", "创建时间"},
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

    connect(m_search, &QLineEdit::textChanged, this, &UserPage::refresh);
    connect(btnAdd, &QPushButton::clicked, this, &UserPage::onAdd);
    connect(btnEdit, &QPushButton::clicked, this, &UserPage::onEdit);
    connect(btnDel, &QPushButton::clicked, this, &UserPage::onDelete);
    connect(btnPwd, &QPushButton::clicked, this, &UserPage::onResetPassword);

    refresh();
}

void UserPage::refresh() {
    QString sql = "SELECT id, username, "
                  "CASE role WHEN 'admin' THEN '管理员' WHEN 'user' THEN '普通用户' ELSE role END AS role, "
                  "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i') AS created_at FROM users";
    QVariantList args;
    QString kw = m_search->text().trimmed();
    if (!kw.isEmpty()) {
        sql += " WHERE username LIKE ?";
        args << ("%" + kw + "%");
    }
    sql += " ORDER BY id";
    m_model->load(sql, args);
    if (!m_model->lastError().isEmpty())
        QMessageBox::critical(this, "数据库错误", m_model->lastError());
    m_view->resizeColumnsToContents();
}

void UserPage::onAdd() {
    UserEditDialog dlg(QVariantMap(), false, this);
    if (dlg.exec() != QDialog::Accepted) return;
    QVariantMap f = dlg.fields();
    const QString salt = DbManager::generateSalt();
    if (!DbManager::instance().execUpdate(
            "INSERT INTO users (username, password_hash, salt, role) VALUES (?, ?, ?, ?)",
            {f["username"], DbManager::hashPassword(f["password"].toString(), salt),
             salt, f["role"]})) {
        QMessageBox::critical(this, "新增失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void UserPage::onEdit() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要修改的用户"); return; }
    const bool isSelf = data["username"].toString() == DbManager::currentUser();
    UserEditDialog dlg(data, isSelf, this);
    if (dlg.exec() != QDialog::Accepted) return;
    QVariantMap f = dlg.fields();
    QVariantList args = {f["username"], f["role"], data["id"]};
    QString sql = "UPDATE users SET username=?, role=? WHERE id=?";
    if (!f["password"].toString().isEmpty()) {
        const QString salt = DbManager::generateSalt();
        sql = "UPDATE users SET username=?, role=?, password_hash=?, salt=? WHERE id=?";
        args = {f["username"], f["role"], DbManager::hashPassword(f["password"].toString(), salt), salt, data["id"]};
    }
    if (!DbManager::instance().execUpdate(sql, args)) {
        QMessageBox::critical(this, "修改失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void UserPage::onDelete() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要删除的用户"); return; }
    if (data["username"].toString() == DbManager::currentUser()) {
        QMessageBox::warning(this, "无法删除", "不能删除当前登录的用户");
        return;
    }
    if (QMessageBox::question(this, "确认", QString("确定删除用户 \"%1\" 吗?").arg(data["username"].toString()))
            != QMessageBox::Yes) return;
    if (!DbManager::instance().execUpdate("DELETE FROM users WHERE id=?", {data["id"]})) {
        QMessageBox::critical(this, "删除失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void UserPage::onResetPassword() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要重置密码的用户"); return; }
    UserPasswordDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    const QString salt = DbManager::generateSalt();
    if (!DbManager::instance().execUpdate(
            "UPDATE users SET password_hash=?, salt=? WHERE id=?",
            {DbManager::hashPassword(dlg.password(), salt), salt, data["id"]})) {
        QMessageBox::critical(this, "重置失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}
