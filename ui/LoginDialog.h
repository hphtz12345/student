#pragma once
#include <QDialog>

class QLineEdit;
class QLabel;

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
private slots:
    void onLogin();
private:
    QLineEdit *m_userEdit;
    QLineEdit *m_passEdit;
    QLabel *m_statusLabel;
};
