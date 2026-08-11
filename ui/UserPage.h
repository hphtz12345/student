#pragma once
#include <QWidget>

class QLineEdit;
class QTableView;
class QPushButton;
class TableModel;

class UserPage : public QWidget {
    Q_OBJECT
public:
    explicit UserPage(QWidget *parent = nullptr);
public slots:
    void refresh();
private slots:
    void onAdd();
    void onEdit();
    void onDelete();
    void onResetPassword();
private:
    QLineEdit *m_search;
    QTableView *m_view;
    TableModel *m_model;
};
