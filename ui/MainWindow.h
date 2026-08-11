#pragma once
#include <QMainWindow>

class QListWidget;
class QStackedWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private:
    QListWidget *m_nav;
    QStackedWidget *m_stack;
    void addPage(QWidget *page, const QString &title, const QString &icon = QString());
};
