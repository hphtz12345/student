#pragma once
#include <QDialog>

class QLineEdit;
class QLabel;

class UserPasswordDialog : public QDialog {
    Q_OBJECT
public:
    explicit UserPasswordDialog(QWidget *parent = nullptr);
    QString password() const;
private slots:
    void onOk();
private:
    QLineEdit *m_passEdit;
    QLabel *m_statusLabel;
};
