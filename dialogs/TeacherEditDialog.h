#pragma once
#include <QDialog>
#include <QVariantMap>

class QLineEdit;
class QComboBox;

class TeacherEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit TeacherEditDialog(const QVariantMap &data, QWidget *parent = nullptr);
    QVariantMap fields() const;
private:
    QLineEdit *m_no, *m_name, *m_title, *m_phone, *m_email;
    QComboBox *m_gender;
};
