#pragma once
#include <QMainWindow>

class QListWidget;
class QStackedWidget;
class UserPage;   // 前向声明,避免头文件引入 UserPage.h

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
private:
    QListWidget *m_nav;
    QStackedWidget *m_stack;
    UserPage *m_userPage = nullptr;   // 仅管理员登录时创建(成员指针 + 前向声明,不直接包含 UserPage.h)
    void addPage(QWidget *page, const QString &title, const QString &icon = QString());
};
