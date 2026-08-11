#pragma once
#include <QWidget>

class QComboBox;
class QLineEdit;
class QTableView;
class QPushButton;
class TableModel;

class ScorePage : public QWidget {
    Q_OBJECT
public:
    explicit ScorePage(QWidget *parent = nullptr);
public slots:
    void refresh();
private slots:
    void onAdd();
    void onEdit();
    void onDelete();
private:
    void reloadFilters();
    QComboBox *m_classFilter;
    QComboBox *m_courseFilter;
    QLineEdit *m_semesterFilter;
    QTableView *m_view;
    TableModel *m_model;
};
