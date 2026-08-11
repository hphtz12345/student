#include "ScoreEditDialog.h"
#include "../database/DbManager.h"
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QMessageBox>

ScoreEditDialog::ScoreEditDialog(const QVariantMap &data, QWidget *parent) : QDialog(parent) {
    setWindowTitle(data.isEmpty() ? "录入成绩" : "修改成绩");
    auto *form = new QFormLayout(this);

    m_student = new QComboBox;
    m_course = new QComboBox;
    m_semester = new QLineEdit(data.value("semester").toString());
    m_semester->setPlaceholderText("如 2025-2026-1");

    m_score = new QDoubleSpinBox;
    m_score->setRange(0, 100);
    m_score->setDecimals(1);
    m_score->setValue(data.value("score").toDouble());

    QSqlQuery qc = DbManager::instance().execQuery(
        "SELECT id, course_no, course_name FROM courses ORDER BY course_no");
    m_course->addItem("(未指定)", QVariant());
    while (qc.next()) {
        m_course->addItem(qc.value("course_no").toString() + " " + qc.value("course_name").toString(),
                          qc.value("id"));
        if (data.contains("course_id") && qc.value("id") == data.value("course_id"))
            m_course->setCurrentIndex(m_course->count() - 1);
    }

    // 修改模式:直接加载当前学生(不过滤)
    if (!data.isEmpty()) {
        QSqlQuery qs = DbManager::instance().execQuery(
            "SELECT id, student_no, name FROM students WHERE id=?",
            {data.value("student_id")});
        while (qs.next()) {
            m_student->addItem(qs.value("student_no").toString() + " " + qs.value("name").toString(),
                               qs.value("id"));
            m_student->setCurrentIndex(0);
        }
    }

    form->addRow("学生:", m_student);
    form->addRow("课程:", m_course);
    form->addRow("学期:", m_semester);
    form->addRow("分数:", m_score);

    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, this, [this] {
        if (m_student->currentIndex() < 0 || m_course->currentIndex() <= 0) {
            QMessageBox::warning(this, "提示", "请选择学生和课程");
            return;
        }
        if (m_semester->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "提示", "学期不能为空");
            return;
        }
        accept();
    });
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    form->addRow(box);
}

void ScoreEditDialog::setClassFilter(int classId) {
    m_filterClassId = classId;
    m_student->clear();
    QSqlQuery qs = DbManager::instance().execQuery(
        "SELECT id, student_no, name FROM students "
        "WHERE (? <= 0 OR class_id = ?) ORDER BY student_no",
        {classId, classId});
    while (qs.next())
        m_student->addItem(qs.value("student_no").toString() + " " + qs.value("name").toString(),
                           qs.value("id"));
}

QVariantMap ScoreEditDialog::fields() const {
    return {
        {"student_id", m_student->currentData()},
        {"course_id", m_course->currentData()},
        {"semester", m_semester->text().trimmed()},
        {"score", m_score->value()},
    };
}
