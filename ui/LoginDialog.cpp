#include "LoginDialog.h"
#include "../database/DbManager.h"
#include <QSqlQuery>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("登录 - 学生管理系统");
    setFixedSize(340, 230);

    auto *title = new QLabel("学生管理系统");
    title->setObjectName("loginTitle");
    title->setAlignment(Qt::AlignCenter);

    m_userEdit = new QLineEdit;
    m_userEdit->setPlaceholderText("请输入用户名");
    m_passEdit = new QLineEdit;
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setPlaceholderText("请输入密码");
    m_statusLabel = new QLabel;
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);

    auto *form = new QFormLayout;
    form->setContentsMargins(24, 8, 24, 8);
    form->setSpacing(10);
    form->addRow("用户名:", m_userEdit);
    form->addRow("密码:", m_passEdit);

    auto *btnLogin = new QPushButton("登录");
    btnLogin->setDefault(true);
    auto *btnCancel = new QPushButton("取消");
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(btnLogin);
    btnLayout->addWidget(btnCancel);

    auto *layout = new QVBoxLayout(this);
    layout->addSpacing(14);
    layout->addWidget(title);
    layout->addSpacing(6);
    layout->addLayout(form);
    layout->addWidget(m_statusLabel);
    layout->addLayout(btnLayout);
    layout->addSpacing(6);

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
    QSqlQuery q = db.execQuery("SELECT salt, role FROM users WHERE username=?", {user});
    if (!q.next()) {
        m_statusLabel->setText("用户不存在");
        return;
    }
    const QString salt = q.value("salt").toString();
    const QString role = q.value("role").toString();
    const QString hash = DbManager::hashPassword(pass, salt);
    const int cnt = db.queryValue(
        "SELECT COUNT(*) FROM users WHERE username=? AND password_hash=?",
        {user, hash}).toInt();
    if (cnt == 1) {
        DbManager::setSession(user, role);
        accept();
    } else {
        m_statusLabel->setText("密码错误");
    }
}
