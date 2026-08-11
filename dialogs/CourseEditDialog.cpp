#include "CourseEditDialog.h"
#include "../database/DbManager.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QMessageBox>

CourseEditDialog::CourseEditDialog(const QVariantMap &data, QWidget *parent) : QDialog(parent) {
    setWindowTitle(data.isEmpty() ? "新增课程" : "修改课程");
    auto *form = new QFormLayout(this);

    m_no = new QLineEdit(data.value("course_no").toString());
    m_name = new QLineEdit(data.value("course_name").toString());
    m_credit = new QDoubleSpinBox;
    m_credit->setRange(0, 20);
    m_credit->setDecimals(1);
    m_credit->setSingleStep(0.5);
    m_credit->setValue(data.value("credit").toDouble());

    m_teacher = new QComboBox;
    QSqlQuery q = DbManager::instance().execQuery(
        "SELECT id, name FROM teachers ORDER BY name");
    m_teacher->addItem("(未指定)", QVariant());
    while (q.next()) {
        m_teacher->addItem(q.value("name").toString(), q.value("id"));
        if (data.contains("teacher_id") && q.value("id") == data.value("teacher_id"))
            m_teacher->setCurrentIndex(m_teacher->count() - 1);
    }

    form->addRow("课程编号:", m_no);
    form->addRow("课程名称:", m_name);
    form->addRow("学分:", m_credit);
    form->addRow("授课教师:", m_teacher);

    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, this, [this] {
        if (m_no->text().trimmed().isEmpty() || m_name->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "提示", "课程编号和课程名称不能为空");
            return;
        }
        accept();
    });
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    form->addRow(box);
}

QVariantMap CourseEditDialog::fields() const {
    return {
        {"course_no", m_no->text().trimmed()},
        {"course_name", m_name->text().trimmed()},
        {"credit", m_credit->value()},
        {"teacher_id", m_teacher->currentData()},
    };
}
