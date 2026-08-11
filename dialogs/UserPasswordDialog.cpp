#include "UserPasswordDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

UserPasswordDialog::UserPasswordDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("重置密码");
    setFixedSize(320, 150);

    auto *title = new QLabel("重置密码");
    title->setObjectName("pageTitle");
    title->setAlignment(Qt::AlignCenter);

    m_passEdit = new QLineEdit;
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setPlaceholderText("请输入新密码");
    m_statusLabel = new QLabel;
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);

    auto *btnOk = new QPushButton("确定");
    auto *btnCancel = new QPushButton("取消");
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(title);
    layout->addWidget(m_passEdit);
    layout->addWidget(m_statusLabel);
    layout->addLayout(btnLayout);

    connect(btnOk, &QPushButton::clicked, this, &UserPasswordDialog::onOk);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void UserPasswordDialog::onOk() {
    if (m_passEdit->text().isEmpty()) {
        m_statusLabel->setText("密码不能为空");
        return;
    }
    accept();
}

QString UserPasswordDialog::password() const { return m_passEdit->text(); }
