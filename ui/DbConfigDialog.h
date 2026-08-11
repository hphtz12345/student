#pragma once
#include <QDialog>
#include "../database/DbManager.h"

class QLineEdit;
class QLabel;

class DbConfigDialog : public QDialog {
    Q_OBJECT
public:
    explicit DbConfigDialog(const DbConfig &cfg, QWidget *parent = nullptr);
    DbConfig config() const;
private slots:
    void onTestConnection();
private:
    QLineEdit *m_host, *m_port, *m_dbName, *m_user, *m_password;
    QLabel *m_statusLabel;
};
