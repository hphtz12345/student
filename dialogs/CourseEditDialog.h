#pragma once
#include <QDialog>
#include <QVariantMap>

class QLineEdit;
class QDoubleSpinBox;
class QComboBox;

class CourseEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit CourseEditDialog(const QVariantMap &data, QWidget *parent = nullptr);
    QVariantMap fields() const;
private:
    QLineEdit *m_no, *m_name;
    QDoubleSpinBox *m_credit;
    QComboBox *m_teacher; // itemData 存教师 id
};
