#include "ScorePage.h"
#include "../models/TableModel.h"
#include "../database/DbManager.h"
#include "../dialogs/ScoreEditDialog.h"
#include <QLabel>
#include <QFrame>
#include <QComboBox>
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>

ScorePage::ScorePage(QWidget *parent) : QWidget(parent) {
    auto *title = new QLabel("成绩管理");
    title->setObjectName("pageTitle");
    auto *titleLine = new QFrame;
    titleLine->setObjectName("titleLine");
    titleLine->setFrameShape(QFrame::NoFrame);

    m_classFilter = new QComboBox;
    m_courseFilter = new QComboBox;
    m_semesterFilter = new QLineEdit;
    m_semesterFilter->setPlaceholderText("学期(留空为全部)");

    m_btnAdd = new QPushButton("录入");
    m_btnEdit = new QPushButton("修改");
    m_btnDel = new QPushButton("删除");
    auto *btnRefresh = new QPushButton("刷新");

    auto *bar = new QHBoxLayout;
    bar->addWidget(new QLabel("班级:"));
    bar->addWidget(m_classFilter);
    bar->addWidget(new QLabel("课程:"));
    bar->addWidget(m_courseFilter);
    bar->addWidget(m_semesterFilter);
    bar->addWidget(m_btnAdd);
    bar->addWidget(m_btnEdit);
    bar->addWidget(m_btnDel);
    bar->addWidget(btnRefresh);

    m_model = new TableModel(this);
    m_model->setColumns({
        {"student_no", "学号"},
        {"student_name", "姓名"},
        {"class_name", "班级"},
        {"course_name", "课程"},
        {"semester", "学期"},
        {"score", "分数"},
    });

    m_view = new QTableView;
    m_view->setModel(m_model);
    m_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_view->setSelectionMode(QAbstractItemView::SingleSelection);
    m_view->horizontalHeader()->setStretchLastSection(true);
    m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->addWidget(title);
    layout->addWidget(titleLine);
    layout->addSpacing(6);
    layout->addLayout(bar);
    layout->addWidget(m_view, 1);

    connect(m_btnAdd, &QPushButton::clicked, this, &ScorePage::onAdd);
    connect(m_btnEdit, &QPushButton::clicked, this, &ScorePage::onEdit);
    connect(m_btnDel, &QPushButton::clicked, this, &ScorePage::onDelete);
    connect(btnRefresh, &QPushButton::clicked, this, &ScorePage::refresh);
    connect(m_classFilter, &QComboBox::currentIndexChanged, this, &ScorePage::refresh);
    connect(m_courseFilter, &QComboBox::currentIndexChanged, this, &ScorePage::refresh);
    connect(m_semesterFilter, &QLineEdit::textChanged, this, &ScorePage::refresh);

    reloadFilters();
    refresh();
}

void ScorePage::reloadFilters() {
    // 重新填充筛选下拉,保留当前选中;填充期间屏蔽信号避免触发 refresh
    QVariant curClass = m_classFilter->currentData();
    m_classFilter->blockSignals(true);
    m_classFilter->clear();
    m_classFilter->addItem("全部班级", QVariant());
    QSqlQuery qc = DbManager::instance().execQuery("SELECT id, class_name FROM classes ORDER BY class_name");
    while (qc.next()) {
        m_classFilter->addItem(qc.value("class_name").toString(), qc.value("id"));
        if (!curClass.isNull() && qc.value("id") == curClass)
            m_classFilter->setCurrentIndex(m_classFilter->count() - 1);
    }
    m_classFilter->blockSignals(false);

    QVariant curCourse = m_courseFilter->currentData();
    m_courseFilter->blockSignals(true);
    m_courseFilter->clear();
    m_courseFilter->addItem("全部课程", QVariant());
    QSqlQuery qk = DbManager::instance().execQuery("SELECT id, course_no, course_name FROM courses ORDER BY course_no");
    while (qk.next()) {
        m_courseFilter->addItem(qk.value("course_no").toString() + " " + qk.value("course_name").toString(), qk.value("id"));
        if (!curCourse.isNull() && qk.value("id") == curCourse)
            m_courseFilter->setCurrentIndex(m_courseFilter->count() - 1);
    }
    m_courseFilter->blockSignals(false);
}

void ScorePage::setReadOnly(bool ro) {
    m_readOnly = ro;
    m_btnAdd->setEnabled(!ro);
    m_btnEdit->setEnabled(!ro);
    m_btnDel->setEnabled(!ro);
}

void ScorePage::refresh() {
    reloadFilters();
    // 注意:SELECT 需包含 sc.student_id / sc.course_id 隐藏列,
    // 供 ScoreEditDialog 修改模式回填学生与课程(setColumns 不含则不影响显示)
    QString sql = "SELECT sc.id, sc.student_id, sc.course_id, "
                  "st.student_no, st.name AS student_name, "
                  "COALESCE(c.class_name, '') AS class_name, "
                  "k.course_name, sc.semester, sc.score "
                  "FROM scores sc "
                  "JOIN students st ON st.id = sc.student_id "
                  "LEFT JOIN classes c ON c.id = st.class_id "
                  "JOIN courses k ON k.id = sc.course_id "
                  "WHERE 1=1";
    QVariantList args;
    if (m_classFilter->currentIndex() > 0) {
        sql += " AND st.class_id = ?";
        args << m_classFilter->currentData();
    }
    if (m_courseFilter->currentIndex() > 0) {
        sql += " AND sc.course_id = ?";
        args << m_courseFilter->currentData();
    }
    QString sem = m_semesterFilter->text().trimmed();
    if (!sem.isEmpty()) {
        sql += " AND sc.semester LIKE ?";
        args << "%" + sem + "%";
    }
    sql += " ORDER BY sc.id DESC";
    m_model->load(sql, args);
    if (!m_model->lastError().isEmpty())
        QMessageBox::critical(this, "数据库错误", m_model->lastError());
    m_view->resizeColumnsToContents();
}

void ScorePage::onAdd() {
    ScoreEditDialog dlg(QVariantMap(), this);
    dlg.setClassFilter(m_classFilter->currentIndex() > 0 ? m_classFilter->currentData().toInt() : 0);
    if (dlg.exec() != QDialog::Accepted) return;
    QVariantMap f = dlg.fields();
    if (!DbManager::instance().execUpdate(
            "INSERT INTO scores (student_id, course_id, semester, score) VALUES (?, ?, ?, ?)",
            {f["student_id"], f["course_id"], f["semester"], f["score"]})) {
        QMessageBox::critical(this, "录入失败",
            "录入失败(该学生此课程学期可能已录过):\n" + DbManager::instance().lastError());
        return;
    }
    refresh();
}

void ScorePage::onEdit() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要修改的行"); return; }
    ScoreEditDialog dlg(data, this);
    if (dlg.exec() != QDialog::Accepted) return;
    QVariantMap f = dlg.fields();
    if (!DbManager::instance().execUpdate(
            "UPDATE scores SET student_id=?, course_id=?, semester=?, score=? WHERE id=?",
            {f["student_id"], f["course_id"], f["semester"], f["score"], data["id"]})) {
        QMessageBox::critical(this, "修改失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void ScorePage::onDelete() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要删除的行"); return; }
    if (QMessageBox::question(this, "确认", "确定删除该条成绩吗?") != QMessageBox::Yes) return;
    if (!DbManager::instance().execUpdate("DELETE FROM scores WHERE id=?", {data["id"]})) {
        QMessageBox::critical(this, "删除失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}
