#include "UserEditDialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

UserEditDialog::UserEditDialog(const QVariantMap &user, bool lockRole, QWidget *parent)
    : QDialog(parent), m_edit(!user.isEmpty()) {
    setWindowTitle(m_edit ? "修改用户" : "新增用户");
    setFixedSize(320, 200);

    auto *title = new QLabel(m_edit ? "修改用户" : "新增用户");
    title->setObjectName("pageTitle");
    title->setAlignment(Qt::AlignCenter);

    m_userEdit = new QLineEdit;
    m_userEdit->setPlaceholderText("请输入用户名");
    m_userEdit->setText(user.value("username").toString());
    m_roleCombo = new QComboBox;
    m_roleCombo->addItem("管理员", "admin");
    m_roleCombo->addItem("普通用户", "user");
    // 编辑模式才回填角色;新增模式保持默认 index 0(管理员),避免空角色
    if (m_edit)
        m_roleCombo->setCurrentIndex(m_roleCombo->findData(user.value("role").toString()));
    // 编辑自己时禁止修改角色,防止把自己降级后锁死系统
    if (lockRole)
        m_roleCombo->setEnabled(false);
    m_passEdit = new QLineEdit;
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setPlaceholderText(m_edit ? "留空表示不修改密码" : "请输入初始密码");
    m_statusLabel = new QLabel;
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);

    auto *form = new QFormLayout;
    form->setContentsMargins(24, 8, 24, 8);
    form->setSpacing(10);
    form->addRow("用户名:", m_userEdit);
    form->addRow("角色:", m_roleCombo);
    form->addRow("密码:", m_passEdit);

    auto *btnOk = new QPushButton("确定");
    auto *btnCancel = new QPushButton("取消");
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(title);
    layout->addLayout(form);
    layout->addWidget(m_statusLabel);
    layout->addLayout(btnLayout);

    connect(btnOk, &QPushButton::clicked, this, &UserEditDialog::onOk);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void UserEditDialog::onOk() {
    if (m_userEdit->text().trimmed().isEmpty()) {
        m_statusLabel->setText("用户名不能为空");
        return;
    }
    if (!m_edit && m_passEdit->text().isEmpty()) {
        m_statusLabel->setText("新增用户必须设置密码");
        return;
    }
    accept();
}

QVariantMap UserEditDialog::fields() const {
    QVariantMap f;
    f["username"] = m_userEdit->text().trimmed();
    f["role"] = m_roleCombo->currentData().toString();
    f["password"] = m_passEdit->text();
    return f;
}
