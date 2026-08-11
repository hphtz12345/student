#include "StudentEditDialog.h"
#include "../database/DbManager.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDate>
#include <QRegularExpression>

StudentEditDialog::StudentEditDialog(const QVariantMap &data, QWidget *parent) : QDialog(parent) {
    setWindowTitle(data.isEmpty() ? "新增学生" : "修改学生");
    auto *form = new QFormLayout(this);

    m_no = new QLineEdit(data.value("student_no").toString());
    m_name = new QLineEdit(data.value("name").toString());
    m_gender = new QComboBox;
    m_gender->addItems({"男", "女"});
    if (data.value("gender").toString() == "女") m_gender->setCurrentIndex(1);

    m_birth = new QDateEdit;
    m_birth->setCalendarPopup(true);
    m_birth->setDisplayFormat("yyyy-MM-dd");
    QDate birth = data.value("birth_date").toDate();
    m_birth->setDate(birth.isValid() ? birth : QDate(2000, 1, 1));

    m_phone = new QLineEdit(data.value("phone").toString());
    m_email = new QLineEdit(data.value("email").toString());

    m_classCombo = new QComboBox;
    QSqlQuery q = DbManager::instance().execQuery(
        "SELECT id, class_name FROM classes ORDER BY class_name");
    m_classCombo->addItem("(未分班)", QVariant());
    while (q.next()) {
        m_classCombo->addItem(q.value("class_name").toString(), q.value("id"));
        if (data.contains("class_id") && q.value("id") == data.value("class_id"))
            m_classCombo->setCurrentIndex(m_classCombo->count() - 1);
    }

    m_enrollYear = new QSpinBox;
    m_enrollYear->setRange(1990, 2035);
    m_enrollYear->setValue(data.value("enroll_year").toInt());

    m_status = new QComboBox;
    m_status->addItems({"在读", "休学", "毕业"});
    int idx = m_status->findText(data.value("status").toString());
    if (idx >= 0) m_status->setCurrentIndex(idx);

    form->addRow("学号:", m_no);
    form->addRow("姓名:", m_name);
    form->addRow("性别:", m_gender);
    form->addRow("出生日期:", m_birth);
    form->addRow("电话:", m_phone);
    form->addRow("邮箱:", m_email);
    form->addRow("班级:", m_classCombo);
    form->addRow("入学年份:", m_enrollYear);
    form->addRow("状态:", m_status);

    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, this, [this] {
        if (m_no->text().trimmed().isEmpty() || m_name->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "提示", "学号和姓名不能为空");
            return;
        }
        const QString no = m_no->text().trimmed();
        if (!no.contains(QRegularExpression("^[A-Za-z0-9]{6,20}$"))) {
            QMessageBox::warning(this, "提示", "学号须为 6-20 位数字或字母");
            return;
        }
        accept();
    });
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    form->addRow(box);
}

QVariantMap StudentEditDialog::fields() const {
    return {
        {"student_no", m_no->text().trimmed()},
        {"name", m_name->text().trimmed()},
        {"gender", m_gender->currentText()},
        {"birth_date", m_birth->date().toString("yyyy-MM-dd")},
        {"phone", m_phone->text().trimmed()},
        {"email", m_email->text().trimmed()},
        {"class_id", m_classCombo->currentData()},
        {"enroll_year", m_enrollYear->value()},
        {"status", m_status->currentText()},
    };
}
