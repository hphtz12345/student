#include "DbConfigDialog.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>

DbConfigDialog::DbConfigDialog(const DbConfig &cfg, QWidget *parent) : QDialog(parent) {
    setWindowTitle("数据库连接设置");
    auto *form = new QFormLayout(this);

    m_host = new QLineEdit(cfg.host);
    m_port = new QLineEdit(QString::number(cfg.port));
    m_dbName = new QLineEdit(cfg.dbName);
    m_user = new QLineEdit(cfg.user);
    m_password = new QLineEdit(cfg.password);
    m_password->setEchoMode(QLineEdit::Password);
    m_statusLabel = new QLabel;

    form->addRow("主机:", m_host);
    form->addRow("端口:", m_port);
    form->addRow("数据库:", m_dbName);
    form->addRow("账号:", m_user);
    form->addRow("密码:", m_password);
    form->addRow(m_statusLabel);

    auto *btnTest = new QPushButton("测试连接");
    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnTest, &QPushButton::clicked, this, &DbConfigDialog::onTestConnection);
    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(btnTest);
    btnLayout->addStretch();
    btnLayout->addWidget(btnBox);
    form->addRow(btnLayout);
}

DbConfig DbConfigDialog::config() const {
    DbConfig cfg;
    cfg.host = m_host->text().trimmed();
    cfg.port = m_port->text().trimmed().toInt();
    cfg.dbName = m_dbName->text().trimmed();
    cfg.user = m_user->text().trimmed();
    cfg.password = m_password->text();
    return cfg;
}

void DbConfigDialog::onTestConnection() {
    DbConfig cfg = config();
    QString err;
    DbManager &db = DbManager::instance();
    bool wasOpen = db.isOpen();
    if (wasOpen) db.close();
    if (db.open(cfg, &err)) {
        m_statusLabel->setText("连接成功");
        db.close(); // 正式连接由 main 打开
    } else {
        m_statusLabel->setText("连接失败: " + err);
    }
    if (wasOpen) {
        QString e2;
        db.open(DbManager::instance().config(), &e2); // 恢复原连接
    }
}
