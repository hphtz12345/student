#include "LoginDialog.h"
#include "../database/DbManager.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("登录 - 学生管理系统");
    setFixedSize(320, 180);
    auto *form = new QFormLayout(this);

    m_userEdit = new QLineEdit;
    m_passEdit = new QLineEdit;
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_statusLabel = new QLabel;

    form->addRow("用户名:", m_userEdit);
    form->addRow("密码:", m_passEdit);
    form->addRow(m_statusLabel);

    auto *btnLogin = new QPushButton("登录");
    auto *btnCancel = new QPushButton("取消");
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(btnLogin);
    btnLayout->addWidget(btnCancel);
    form->addRow(btnLayout);

    connect(btnLogin, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_passEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLogin);
}

void LoginDialog::onLogin() {
    const QString user = m_userEdit->text().trimmed();
    const QString pass = m_passEdit->text();
    if (user.isEmpty() || pass.isEmpty()) {
        m_statusLabel->setText("请输入用户名和密码");
        return;
    }
    auto &db = DbManager::instance();
    const QString salt = db.queryValue(
        "SELECT salt FROM users WHERE username=?", {user}).toString();
    if (salt.isEmpty()) {
        m_statusLabel->setText("用户不存在");
        return;
    }
    const QString hash = DbManager::hashPassword(pass, salt);
    const int cnt = db.queryValue(
        "SELECT COUNT(*) FROM users WHERE username=? AND password_hash=?",
        {user, hash}).toInt();
    if (cnt == 1) {
        accept();
    } else {
        m_statusLabel->setText("密码错误");
    }
}
