#include "ClassEditDialog.h"
#include "../database/DbManager.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QMessageBox>

ClassEditDialog::ClassEditDialog(const QVariantMap &data, QWidget *parent) : QDialog(parent) {
    setWindowTitle(data.isEmpty() ? "新增班级" : "修改班级");
    auto *form = new QFormLayout(this);

    m_name = new QLineEdit(data.value("class_name").toString());
    m_grade = new QSpinBox;
    m_grade->setRange(1990, 2035);
    m_grade->setValue(data.value("grade").toInt());
    m_major = new QLineEdit(data.value("major").toString());
    m_headTeacher = new QComboBox;

    // 班主任下拉:来自 teachers 表
    QSqlQuery q = DbManager::instance().execQuery(
        "SELECT id, name FROM teachers ORDER BY name");
    m_headTeacher->addItem("(无)", QVariant());
    while (q.next()) {
        m_headTeacher->addItem(q.value("name").toString(), q.value("id"));
        if (data.contains("head_teacher_id") && q.value("id") == data.value("head_teacher_id"))
            m_headTeacher->setCurrentIndex(m_headTeacher->count() - 1);
    }

    form->addRow("班级名称:", m_name);
    form->addRow("年级:", m_grade);
    form->addRow("专业:", m_major);
    form->addRow("班主任:", m_headTeacher);

    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, this, [this] {
        if (m_name->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "提示", "班级名称不能为空");
            return;
        }
        accept();
    });
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    form->addRow(box);
}

QVariantMap ClassEditDialog::fields() const {
    return {
        {"class_name", m_name->text().trimmed()},
        {"grade", m_grade->value()},
        {"major", m_major->text().trimmed()},
        {"head_teacher_id", m_headTeacher->currentData()},
    };
}
