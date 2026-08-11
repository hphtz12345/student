#pragma once
#include <QDialog>
#include <QVariantMap>

class QLineEdit;
class QComboBox;
class QDateEdit;
class QSpinBox;

class StudentEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit StudentEditDialog(const QVariantMap &data, QWidget *parent = nullptr);
    QVariantMap fields() const;
private:
    QLineEdit *m_no, *m_name, *m_phone, *m_email;
    QComboBox *m_gender, *m_classCombo, *m_status;
    QDateEdit *m_birth;
    QSpinBox *m_enrollYear;
};
