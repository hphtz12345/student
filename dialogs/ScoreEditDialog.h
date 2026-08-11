#pragma once
#include <QDialog>
#include <QVariantMap>

class QComboBox;
class QLineEdit;
class QDoubleSpinBox;

class ScoreEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit ScoreEditDialog(const QVariantMap &data, QWidget *parent = nullptr);
    QVariantMap fields() const;
    void setClassFilter(int classId); // 新增时按班级筛学生;data 非空(修改)时忽略
private:
    QComboBox *m_student;  // itemData 存 student_id,显示 学号+姓名
    QComboBox *m_course;   // itemData 存 course_id
    QLineEdit *m_semester;
    QDoubleSpinBox *m_score;
    int m_filterClassId = 0;
};
