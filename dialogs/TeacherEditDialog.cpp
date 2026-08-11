#include "TeacherEditDialog.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QMessageBox>

TeacherEditDialog::TeacherEditDialog(const QVariantMap &data, QWidget *parent) : QDialog(parent) {
    setWindowTitle(data.isEmpty() ? "新增教师" : "修改教师");
    auto *form = new QFormLayout(this);

    m_no = new QLineEdit(data.value("teacher_no").toString());
    m_name = new QLineEdit(data.value("name").toString());
    m_gender = new QComboBox;
    m_gender->addItems({"男", "女"});
    if (data.value("gender").toString() == "女") m_gender->setCurrentIndex(1);
    m_title = new QLineEdit(data.value("title").toString());
    m_phone = new QLineEdit(data.value("phone").toString());
    m_email = new QLineEdit(data.value("email").toString());

    form->addRow("教师编号:", m_no);
    form->addRow("姓名:", m_name);
    form->addRow("性别:", m_gender);
    form->addRow("职称:", m_title);
    form->addRow("电话:", m_phone);
    form->addRow("邮箱:", m_email);

    auto *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, this, [this] {
        if (m_no->text().trimmed().isEmpty() || m_name->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "提示", "编号和姓名不能为空");
            return;
        }
        accept();
    });
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    form->addRow(box);
}

QVariantMap TeacherEditDialog::fields() const {
    return {
        {"teacher_no", m_no->text().trimmed()},
        {"name", m_name->text().trimmed()},
        {"gender", m_gender->currentText()},
        {"title", m_title->text().trimmed()},
        {"phone", m_phone->text().trimmed()},
        {"email", m_email->text().trimmed()},
    };
}
