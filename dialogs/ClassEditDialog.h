#pragma once
#include <QDialog>
#include <QVariantMap>

class QLineEdit;
class QSpinBox;
class QComboBox;

class ClassEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit ClassEditDialog(const QVariantMap &data, QWidget *parent = nullptr);
    QVariantMap fields() const;
private:
    QLineEdit *m_name;
    QSpinBox *m_grade;
    QLineEdit *m_major;
    QComboBox *m_headTeacher; // 存教师 id 到 itemData
};
