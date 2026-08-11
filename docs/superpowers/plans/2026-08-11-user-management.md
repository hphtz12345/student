# 用户管理功能实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 增加用户管理页面,实现管理员/普通用户双角色:普通用户只读(按钮禁用),用户管理页仅管理员可见。

**Architecture:** 登录成功时把用户名+角色存入 DbManager 静态会话;MainWindow 按角色构建导航(管理员 7 页含用户管理,普通用户 6 页)并给 5 个数据页面调用 setReadOnly(true);UserPage 复用 TableModel + UserEditDialog/UserPasswordDialog 完成 CRUD。

**Tech Stack:** Qt 6.11.1 mingw_64(QtSql、QtWidgets)、MySQL 8.0.29、qmake + mingw32-make

## Global Constraints

- 所有注释、日志、界面文案使用中文
- 密码存储:`password_hash = SHA256(password + salt)`,salt 为 32 字符 hex,复用 `DbManager::hashPassword` / `DbManager::generateSalt()`
- 角色取值:`role` 字段 'admin' = 管理员、'user' = 普通用户;界面显示中文
- 编译命令:
  ```bash
  export PATH="/c/Qt/6.11.1/mingw_64/bin:/c/Qt/Tools/mingw1310_64/bin:$PATH"
  cd /c/Users/ME/StudentManager && qmake StudentManager.pro && mingw32-make -j4
  ```
- 部署命令(先杀进程再复制):
  ```bash
  powershell -Command "Get-Process StudentManager -ErrorAction SilentlyContinue | Stop-Process -Force"
  cp release/StudentManager.exe dist/ && cp resources/style.qss dist/resources/style.qss
  ```
- git 提交用:`git -C /c/Users/ME/StudentManager -c user.name="Claude" -c user.email="claude@local" commit -m "..."`(当前 shell 工作目录不在仓库内)
- **seed*.sql 不提交**(用户约定);推送需要代理:git -C 仓库 -c http.proxy=http://127.0.0.1:65532 push -q origin main(用户手动开代理)
- 数据库自检:`dist/StudentManager.exe --db-test` 输出"自检全部通过"即数据库层正常
- 快验证:`dist/StudentManager.exe --stats-dump` 输出"统计图已导出"即 GUI 无崩溃

---

### Task 1: 会话存储 + 登录读取角色

**Files:**
- Modify: `database/DbManager.h`(加静态成员与存取方法)
- Modify: `database/DbManager.cpp`
- Modify: `ui/LoginDialog.cpp`(查询带 role)

**Interfaces:**
- Produces: `DbManager::setSession(const QString &user, const QString &role)`、`DbManager::currentUser() -> QString`、`DbManager::currentRole() -> QString`(静态);登录成功后 role 已存入会话
- Consumes: 现有 `DbManager::instance()`、`execQuery`

- [ ] **Step 1: DbManager.h 加静态会话成员与存取方法**

在 `class DbManager` 的 public 段(static generateSalt 之后)加:

```cpp
    static void setSession(const QString &user, const QString &role);
    static QString currentUser();
    static QString currentRole();
```

在 private 段加:

```cpp
    static QString m_sessionUser;
    static QString m_sessionRole;
```

- [ ] **Step 2: DbManager.cpp 实现静态成员与存取方法**

在文件顶部 include 之后加静态成员定义:

```cpp
QString DbManager::m_sessionUser;
QString DbManager::m_sessionRole;
```

在 `generateSalt()` 定义后加:

```cpp
void DbManager::setSession(const QString &user, const QString &role) {
    m_sessionUser = user;
    m_sessionRole = role;
}

QString DbManager::currentUser() { return m_sessionUser; }
QString DbManager::currentRole() { return m_sessionRole; }
```

- [ ] **Step 3: LoginDialog.cpp 登录查询带 role,成功后写入会话**

把 `onLogin` 中的

```cpp
    const QString salt = db.queryValue(
        "SELECT salt FROM users WHERE username=?", {user}).toString();
```

改为:

```cpp
    QSqlQuery q = db.execQuery("SELECT salt, role FROM users WHERE username=?", {user});
    if (!q.next()) {
        m_statusLabel->setText("用户不存在");
        return;
    }
    const QString salt = q.value("salt").toString();
    const QString role = q.value("role").toString();
```

把后面的

```cpp
    if (cnt == 1) {
        accept();
    } else {
```

改为:

```cpp
    if (cnt == 1) {
        DbManager::setSession(user, role);
        accept();
    } else {
```

在文件头部 `#include "../database/DbManager.h"` 之后加 `#include <QSqlQuery>`(原代码依赖 queryValue 无此 include;若已有则跳过)。

- [ ] **Step 4: 编译验证**

运行 Global Constraints 的编译命令。预期:无错误,`release/StudentManager.exe` 生成。

- [ ] **Step 5: 提交**

```bash
git -C /c/Users/ME/StudentManager add database/DbManager.h database/DbManager.cpp ui/LoginDialog.cpp
git -C /c/Users/ME/StudentManager -c user.name="Claude" -c user.email="claude@local" commit -m "feat: 登录会话保存用户名和角色" -q
```

---

### Task 2: 五个数据页面支持只读模式

**Files:**
- Modify: `ui/StudentPage.h`、`ui/StudentPage.cpp`
- Modify: `ui/TeacherPage.h`、`ui/TeacherPage.cpp`
- Modify: `ui/ClassPage.h`、`ui/ClassPage.cpp`
- Modify: `ui/CoursePage.h`、`ui/CoursePage.cpp`
- Modify: `ui/ScorePage.h`、`ui/ScorePage.cpp`

**Interfaces:**
- Produces: 每个页面 public 方法 `void setReadOnly(bool ro)`,以及成员 `bool m_readOnly`;禁用新增/修改/删除按钮(ScorePage 为"录入"),搜索/刷新保持可用
- Consumes: 无(独立)

- [ ] **Step 1: StudentPage 头文件加方法声明和成员**

`ui/StudentPage.h` 的 public 段(构造函数之后)加:

```cpp
    void setReadOnly(bool ro);
```

private 段加:

```cpp
    bool m_readOnly = false;
```

- [ ] **Step 2: StudentPage.cpp 按钮存为成员并实现 setReadOnly**

构造函数中 `auto *btnAdd = new QPushButton("新增");` 改为成员创建(其余三个按钮相同):

```cpp
    m_btnAdd = new QPushButton("新增");
    m_btnEdit = new QPushButton("修改");
    m_btnDel = new QPushButton("删除");
    auto *btnRefresh = new QPushButton("刷新");
```

`StudentPage` 构造函数末尾(connect 之后、refresh() 之前)加 `m_btnAdd->setEnabled(!m_readOnly);` 行可省略(默认 enabled),直接实现方法,放在 `refresh()` 定义之前:

```cpp
void StudentPage::setReadOnly(bool ro) {
    m_readOnly = ro;
    m_btnAdd->setEnabled(!ro);
    m_btnEdit->setEnabled(!ro);
    m_btnDel->setEnabled(!ro);
}
```

`ui/StudentPage.h` 顶部前向声明加 `class QPushButton;`(已有该声明,复用成员 `QPushButton *m_btnAdd;` 等需在头文件 private 段加):

```cpp
    QPushButton *m_btnAdd;
    QPushButton *m_btnEdit;
    QPushButton *m_btnDel;
```

- [ ] **Step 3: TeacherPage / ClassPage / CoursePage 相同改动**

按 Step 1-2 模式逐页处理:
- 头文件:public 加 `void setReadOnly(bool ro);`,private 加 `bool m_readOnly = false;` 和 `QPushButton *m_btnAdd; *m_btnEdit; *m_btnDel;`
- cpp:三个按钮改成员,新增 `setReadOnly` 实现(与 Step 2 完全一致)

- [ ] **Step 4: ScorePage 相同改动(按钮为 录入/修改/删除)**

与 Step 2 相同,唯一区别:第一个按钮文本是 "录入":

```cpp
    m_btnAdd = new QPushButton("录入");
```

- [ ] **Step 5: 编译验证**

运行编译命令。预期:无错误。

- [ ] **Step 6: 提交**

```bash
git -C /c/Users/ME/StudentManager add ui/StudentPage.h ui/StudentPage.cpp ui/TeacherPage.h ui/TeacherPage.cpp ui/ClassPage.h ui/ClassPage.cpp ui/CoursePage.h ui/CoursePage.cpp ui/ScorePage.h ui/ScorePage.cpp
git -C /c/Users/ME/StudentManager -c user.name="Claude" -c user.email="claude@local" commit -m "feat: 数据页面支持只读模式(普通用户)" -q
```

---

### Task 3: MainWindow 按角色构建导航 + 状态栏显示用户

**Files:**
- Modify: `ui/MainWindow.cpp`

**Interfaces:**
- Consumes: `DbManager::currentRole()` / `DbManager::currentUser()`(Task 1);`setReadOnly(bool)`(Task 2)
- Produces: 管理员 7 个导航项(含"用户管理" 🔑),普通用户 6 个导航项且数据页只读

- [ ] **Step 1: 构造函数改为按角色构建**

`ui/MainWindow.cpp` 构造函数,把固定的 addPage 序列改为:

```cpp
    const bool isAdmin = DbManager::currentRole() == "admin";
    addPage(new StudentPage, "学生管理", "🎓");
    addPage(new TeacherPage, "教师管理", "💼");
    addPage(new ClassPage, "班级管理", "🏫");
    addPage(new CoursePage, "课程管理", "📖");
    addPage(new ScorePage, "成绩管理", "📊");
    addPage(new StatisticsPage, "统计分析", "📈");
    if (isAdmin) {
        m_userPage = new UserPage;
        addPage(m_userPage, "用户管理", "🔑");
    }

    // 普通用户:只读模式(遍历 stack 中的前 6 页)
    if (!isAdmin) {
        for (int i = 0; i < 6; ++i) {
            if (auto *p = qobject_cast<StudentPage *>(m_stack->widget(i)))
                p->setReadOnly(true);
            else if (auto *p = qobject_cast<TeacherPage *>(m_stack->widget(i)))
                p->setReadOnly(true);
            else if (auto *p = qobject_cast<ClassPage *>(m_stack->widget(i)))
                p->setReadOnly(true);
            else if (auto *p = qobject_cast<CoursePage *>(m_stack->widget(i)))
                p->setReadOnly(true);
            else if (auto *p = qobject_cast<ScorePage *>(m_stack->widget(i)))
                p->setReadOnly(true);
        }
    }
```

(注:StatisticsPage 无需只读,无写操作。)

- [ ] **Step 2: 状态栏显示当前用户和角色**

构造函数末尾,把

```cpp
    statusBar()->showMessage("就绪");
```

改为:

```cpp
    const QString roleText = isAdmin ? "管理员" : "普通用户";
    statusBar()->showMessage(QString("当前用户: %1(%2)").arg(DbManager::currentUser(), roleText));
```

- [ ] **Step 3: include UserPage 与 qobject_cast 所需头文件**

`ui/MainWindow.cpp` 顶部 include 区加:

```cpp
#include "UserPage.h"
#include <QPushButton>
```

(在 `#include "StatisticsPage.h"` 之后。)

- [ ] **Step 4: MainWindow.h 加 m_userPage 成员**

`ui/MainWindow.h` private 段加:

```cpp
    class UserPage;   // 头文件前向声明区已有 class 列表,加此项
```

在头文件前向声明区(`class QStackedWidget;` 之后)加 `class UserPage;`,private 成员区加:

```cpp
    UserPage *m_userPage = nullptr;
```

- [ ] **Step 5: 编译**

本任务引用了 `UserPage`(Task 5 才创建)且 UserPage 未注册进 `StudentManager.pro`(Task 6),因此**本任务不单独编译**。验证统一放在 Task 6 Step 2 的全量编译:预期无错误(此时 MainWindow、UserPage、对话框均已就位)。

- [ ] **Step 6: 提交**

```bash
git -C /c/Users/ME/StudentManager add ui/MainWindow.cpp ui/MainWindow.h
git -C /c/Users/ME/StudentManager -c user.name="Claude" -c user.email="claude@local" commit -m "feat: 主窗口按角色构建导航,状态栏显示当前用户" -q
```

---

### Task 4: 用户编辑对话框(UserEditDialog + UserPasswordDialog)

**Files:**
- Create: `dialogs/UserEditDialog.h`、`dialogs/UserEditDialog.cpp`
- Create: `dialogs/UserPasswordDialog.h`、`dialogs/UserPasswordDialog.cpp`

**Interfaces:**
- Produces:
  - `UserEditDialog(const QVariantMap &user = QVariantMap(), bool lockRole = false, QWidget *parent = nullptr)` — `user` 为空表示新增;`lockRole` 为真时角色下拉禁用(编辑自己时用);`bool isEdit() const`、`QVariantMap fields() const`(keys: username、role、password 可能为空)
  - `UserPasswordDialog(QWidget *parent = nullptr)` — `QString password() const`
  - 密码字段留空表示"不修改"
- Consumes: 无

- [ ] **Step 1: UserEditDialog.h**

```cpp
#pragma once
#include <QDialog>
#include <QVariantMap>

class QLineEdit;
class QComboBox;
class QLabel;

class UserEditDialog : public QDialog {
    Q_OBJECT
public:
    explicit UserEditDialog(const QVariantMap &user = QVariantMap(), bool lockRole = false,
                            QWidget *parent = nullptr);
    bool isEdit() const { return m_edit; }
    QVariantMap fields() const;
private slots:
    void onOk();
private:
    bool m_edit;
    QLineEdit *m_userEdit;
    QComboBox *m_roleCombo;
    QLineEdit *m_passEdit;
    QLabel *m_statusLabel;
};
```

- [ ] **Step 2: UserEditDialog.cpp**

```cpp
#include "UserEditDialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

UserEditDialog::UserEditDialog(const QVariantMap &user, bool lockRole, QWidget *parent)
    : QDialog(parent), m_edit(!user.isEmpty()) {
    setWindowTitle(m_edit ? "修改用户" : "新增用户");
    setFixedSize(320, 200);

    auto *title = new QLabel(m_edit ? "修改用户" : "新增用户");
    title->setObjectName("pageTitle");
    title->setAlignment(Qt::AlignCenter);

    m_userEdit = new QLineEdit;
    m_userEdit->setPlaceholderText("请输入用户名");
    m_userEdit->setText(user.value("username").toString());
    m_roleCombo = new QComboBox;
    m_roleCombo->addItem("管理员", "admin");
    m_roleCombo->addItem("普通用户", "user");
    m_roleCombo->setCurrentIndex(m_roleCombo->findData(user.value("role").toString()));
    // 编辑自己时禁止修改角色,防止把自己降级后锁死系统
    if (lockRole)
        m_roleCombo->setEnabled(false);
    m_passEdit = new QLineEdit;
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setPlaceholderText(m_edit ? "留空表示不修改密码" : "请输入初始密码");
    m_statusLabel = new QLabel;
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);

    auto *form = new QFormLayout;
    form->setContentsMargins(24, 8, 24, 8);
    form->setSpacing(10);
    form->addRow("用户名:", m_userEdit);
    form->addRow("角色:", m_roleCombo);
    form->addRow("密码:", m_passEdit);

    auto *btnOk = new QPushButton("确定");
    auto *btnCancel = new QPushButton("取消");
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(title);
    layout->addLayout(form);
    layout->addWidget(m_statusLabel);
    layout->addLayout(btnLayout);

    connect(btnOk, &QPushButton::clicked, this, &UserEditDialog::onOk);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void UserEditDialog::onOk() {
    if (m_userEdit->text().trimmed().isEmpty()) {
        m_statusLabel->setText("用户名不能为空");
        return;
    }
    if (!m_edit && m_passEdit->text().isEmpty()) {
        m_statusLabel->setText("新增用户必须设置密码");
        return;
    }
    accept();
}

QVariantMap UserEditDialog::fields() const {
    QVariantMap f;
    f["username"] = m_userEdit->text().trimmed();
    f["role"] = m_roleCombo->currentData().toString();
    f["password"] = m_passEdit->text();
    return f;
}
```

- [ ] **Step 3: UserPasswordDialog.h**

```cpp
#pragma once
#include <QDialog>

class QLineEdit;
class QLabel;

class UserPasswordDialog : public QDialog {
    Q_OBJECT
public:
    explicit UserPasswordDialog(QWidget *parent = nullptr);
    QString password() const;
private slots:
    void onOk();
private:
    QLineEdit *m_passEdit;
    QLabel *m_statusLabel;
};
```

- [ ] **Step 4: UserPasswordDialog.cpp**

```cpp
#include "UserPasswordDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

UserPasswordDialog::UserPasswordDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("重置密码");
    setFixedSize(320, 150);

    auto *title = new QLabel("重置密码");
    title->setObjectName("pageTitle");
    title->setAlignment(Qt::AlignCenter);

    m_passEdit = new QLineEdit;
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setPlaceholderText("请输入新密码");
    m_statusLabel = new QLabel;
    m_statusLabel->setObjectName("statusLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);

    auto *btnOk = new QPushButton("确定");
    auto *btnCancel = new QPushButton("取消");
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(title);
    layout->addWidget(m_passEdit);
    layout->addWidget(m_statusLabel);
    layout->addLayout(btnLayout);

    connect(btnOk, &QPushButton::clicked, this, &UserPasswordDialog::onOk);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
}

void UserPasswordDialog::onOk() {
    if (m_passEdit->text().isEmpty()) {
        m_statusLabel->setText("密码不能为空");
        return;
    }
    accept();
}

QString UserPasswordDialog::password() const { return m_passEdit->text(); }
```

- [ ] **Step 5: 编译**

运行编译命令。预期:无错误(两个对话框不引用未定义类型)。

- [ ] **Step 6: 提交**

```bash
git -C /c/Users/ME/StudentManager add dialogs/UserEditDialog.h dialogs/UserEditDialog.cpp dialogs/UserPasswordDialog.h dialogs/UserPasswordDialog.cpp
git -C /c/Users/ME/StudentManager -c user.name="Claude" -c user.email="claude@local" commit -m "feat: 用户编辑与重置密码对话框" -q
```

---

### Task 5: 用户管理页面 UserPage

**Files:**
- Create: `ui/UserPage.h`、`ui/UserPage.cpp`

**Interfaces:**
- Consumes: `TableModel`、`DbManager::instance()`、`UserEditDialog`、`UserPasswordDialog`(Task 4)、`DbManager::hashPassword` / `generateSalt`、`DbManager::currentUser()`(Task 1)
- Produces: `UserPage`(Task 3 已引用,此任务补齐使其可编译)

- [ ] **Step 1: UserPage.h**

```cpp
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
```

- [ ] **Step 2: UserPage.cpp**

```cpp
#include "UserPage.h"
#include "../models/TableModel.h"
#include "../database/DbManager.h"
#include "../dialogs/UserEditDialog.h"
#include "../dialogs/UserPasswordDialog.h"
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>

UserPage::UserPage(QWidget *parent) : QWidget(parent) {
    auto *title = new QLabel("用户管理");
    title->setObjectName("pageTitle");
    auto *titleLine = new QFrame;
    titleLine->setObjectName("titleLine");
    titleLine->setFrameShape(QFrame::NoFrame);

    m_search = new QLineEdit;
    m_search->setPlaceholderText("搜索用户名");

    auto *btnAdd = new QPushButton("新增");
    auto *btnEdit = new QPushButton("修改");
    auto *btnDel = new QPushButton("删除");
    auto *btnPwd = new QPushButton("重置密码");

    auto *bar = new QHBoxLayout;
    bar->addWidget(m_search);
    bar->addWidget(btnAdd);
    bar->addWidget(btnEdit);
    bar->addWidget(btnDel);
    bar->addWidget(btnPwd);

    m_model = new TableModel(this);
    m_model->setColumns({
        {"username", "用户名"},
        {"role", "角色"},
        {"created_at", "创建时间"},
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

    connect(m_search, &QLineEdit::textChanged, this, &UserPage::refresh);
    connect(btnAdd, &QPushButton::clicked, this, &UserPage::onAdd);
    connect(btnEdit, &QPushButton::clicked, this, &UserPage::onEdit);
    connect(btnDel, &QPushButton::clicked, this, &UserPage::onDelete);
    connect(btnPwd, &QPushButton::clicked, this, &UserPage::onResetPassword);

    refresh();
}

void UserPage::refresh() {
    QString sql = "SELECT id, username, "
                  "CASE role WHEN 'admin' THEN '管理员' WHEN 'user' THEN '普通用户' ELSE role END AS role, "
                  "DATE_FORMAT(created_at, '%Y-%m-%d %H:%i') AS created_at FROM users";
    QVariantList args;
    QString kw = m_search->text().trimmed();
    if (!kw.isEmpty()) {
        sql += " WHERE username LIKE ?";
        args << ("%" + kw + "%");
    }
    sql += " ORDER BY id";
    m_model->load(sql, args);
    if (!m_model->lastError().isEmpty())
        QMessageBox::critical(this, "数据库错误", m_model->lastError());
    m_view->resizeColumnsToContents();
}

void UserPage::onAdd() {
    UserEditDialog dlg(QVariantMap(), this);
    if (dlg.exec() != QDialog::Accepted) return;
    QVariantMap f = dlg.fields();
    const QString salt = DbManager::generateSalt();
    if (!DbManager::instance().execUpdate(
            "INSERT INTO users (username, password_hash, salt, role) VALUES (?, ?, ?, ?)",
            {f["username"], DbManager::hashPassword(f["password"].toString(), salt),
             salt, f["role"]})) {
        QMessageBox::critical(this, "新增失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void UserPage::onEdit() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要修改的用户"); return; }
    const bool isSelf = data["username"].toString() == DbManager::currentUser();
    UserEditDialog dlg(data, isSelf, this);
    if (dlg.exec() != QDialog::Accepted) return;
    QVariantMap f = dlg.fields();
    QVariantList args = {f["username"], f["role"], data["id"]};
    QString sql = "UPDATE users SET username=?, role=? WHERE id=?";
    if (!f["password"].toString().isEmpty()) {
        const QString salt = DbManager::generateSalt();
        sql = "UPDATE users SET username=?, role=?, password_hash=?, salt=? WHERE id=?";
        args = {f["username"], f["role"], DbManager::hashPassword(f["password"].toString(), salt), salt, data["id"]};
    }
    if (!DbManager::instance().execUpdate(sql, args)) {
        QMessageBox::critical(this, "修改失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void UserPage::onDelete() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要删除的用户"); return; }
    if (data["username"].toString() == DbManager::currentUser()) {
        QMessageBox::warning(this, "无法删除", "不能删除当前登录的用户");
        return;
    }
    if (QMessageBox::question(this, "确认", QString("确定删除用户 \"%1\" 吗?").arg(data["username"].toString()))
            != QMessageBox::Yes) return;
    if (!DbManager::instance().execUpdate("DELETE FROM users WHERE id=?", {data["id"]})) {
        QMessageBox::critical(this, "删除失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}

void UserPage::onResetPassword() {
    int row = m_view->currentIndex().row();
    QVariantMap data = m_model->rowAt(row);
    if (data.isEmpty()) { QMessageBox::information(this, "提示", "请先选择要重置密码的用户"); return; }
    UserPasswordDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    const QString salt = DbManager::generateSalt();
    if (!DbManager::instance().execUpdate(
            "UPDATE users SET password_hash=?, salt=? WHERE id=?",
            {DbManager::hashPassword(dlg.password(), salt), salt, data["id"]})) {
        QMessageBox::critical(this, "重置失败", DbManager::instance().lastError());
        return;
    }
    refresh();
}
```

- [ ] **Step 3: 检查表头字段映射**

`m_model->rowAt(row)` 返回的 QVariantMap 以 SQL 列名为 key(含 id)。若 TableModel::rowAt 只包含 setColumns 声明的列(无 id),则在 setColumns 中追加隐藏 id 列不可行——检查 `models/TableModel.cpp` 的 `rowAt` 实现:

- 若 rowAt 返回全部 SQL 列:直接用 `data["id"]`,本任务代码无需改
- 若 rowAt 只返回展示列:在 `setColumns` 列表末尾追加 `{"id", "ID"}`,并在 `refresh()` 中 `m_view->hideColumn(m_view->model()->columnCount() - 1);` 隐藏

- [ ] **Step 4: 编译**

本任务与 Task 3(MainWindow 引用 UserPage)、Task 6(pro 注册)构成构建单元:**统一在 Task 6 Step 2 编译**,预期无错误。

- [ ] **Step 5: 提交**

```bash
git -C /c/Users/ME/StudentManager add ui/UserPage.h ui/UserPage.cpp
git -C /c/Users/ME/StudentManager -c user.name="Claude" -c user.email="claude@local" commit -m "feat: 用户管理页面(增删改查/重置密码)" -q
```

---

### Task 6: 注册到工程并部署验证

**Files:**
- Modify: `StudentManager.pro`

- [ ] **Step 1: pro 文件注册 UserPage 和对话框**

`SOURCES +=` 中 `ui/StatisticsPage.cpp` 之后加:

```
    ui/UserPage.cpp \
    dialogs/UserEditDialog.cpp \
    dialogs/UserPasswordDialog.cpp
```

`HEADERS +=` 中 `ui/StatisticsPage.h` 之后加:

```
    ui/UserPage.h \
    dialogs/UserEditDialog.h \
    dialogs/UserPasswordDialog.h
```

- [ ] **Step 2: 全量编译**

运行编译命令。预期:无错误,exe 生成。

- [ ] **Step 3: 部署到 dist**

运行 Global Constraints 的部署命令。

- [ ] **Step 4: 数据库与 GUI 快速验证**

```bash
cd /c/Users/ME/StudentManager/dist && ./StudentManager.exe --db-test
cd /c/Users/ME/StudentManager/dist && ./StudentManager.exe --stats-dump
```

预期:两行都成功(自检全部通过 / 统计图已导出)。

- [ ] **Step 5: 提交**

```bash
git -C /c/Users/ME/StudentManager add StudentManager.pro
git -C /c/Users/ME/StudentManager -c user.name="Claude" -c user.email="claude@local" commit -m "build: 注册用户管理页面与对话框" -q
```

---

### Task 7: UIA 自动化验证双角色

**Files:**
- Create: `C:\Users\ME\user_mgmt_test.ps1`(临时验证脚本,不提交)

- [ ] **Step 1: 编写验证脚本**

完整脚本(参照 `C:\Users\ME\stats_dump5.ps1` 的登录段与 UIA 用法,脚本路径 `C:\Users\ME\user_mgmt_test.ps1`):

```powershell
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName UIAutomationClient
Add-Type -AssemblyName UIAutomationTypes
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class M7 {
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, uint d, UIntPtr e);
}
"@
[M7]::SetProcessDPIAware() | Out-Null
$root = [System.Windows.Automation.AutomationElement]::RootElement
$exe = "C:\Users\ME\StudentManager\dist\StudentManager.exe"
$fails = 0
function Assert($cond, $msg) {
  if ($cond) { Write-Host "PASS: $msg" }
  else { Write-Host "FAIL: $msg"; $script:fails++ }
}
function Start-App {
  Get-Process StudentManager -ErrorAction SilentlyContinue | Stop-Process -Force
  Start-Sleep -Milliseconds 500
  Start-Process $exe | Out-Null
  Start-Sleep -Seconds 4
}
function Login($user, $pass) {
  $lcond = New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ClassNameProperty, "LoginDialog")
  $w = $root.FindFirst([System.Windows.Automation.TreeScope]::Children, $lcond)
  if (-not $w) { throw "登录框未找到" }
  $econd = New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ControlTypeProperty, [System.Windows.Automation.ControlType]::Edit)
  $edits = $w.FindAll([System.Windows.Automation.TreeScope]::Descendants, $econd)
  $edits[0].GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern).SetValue($user)
  $edits[1].GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern).SetValue($pass)
  $bcond = New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ControlTypeProperty, [System.Windows.Automation.ControlType]::Button)
  $btns = $w.FindAll([System.Windows.Automation.TreeScope]::Descendants, $bcond)
  $btns[0].GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
  Start-Sleep -Seconds 3
  $wcond = New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ClassNameProperty, "MainWindow")
  return $root.FindFirst([System.Windows.Automation.TreeScope]::Children, $wcond)
}
function Get-Nav($win) {
  $cond = New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ControlTypeProperty, [System.Windows.Automation.ControlType]::ListItem)
  return $win.FindAll([System.Windows.Automation.TreeScope]::Descendants, $cond)
}
function Click-Item($el) {
  $r = $el.Current.BoundingRectangle
  [M7]::SetCursorPos([int]($r.X + $r.Width/2), [int]($r.Y + $r.Height/2)) | Out-Null
  [M7]::mouse_event(2,0,0,0,[UIntPtr]::Zero); [M7]::mouse_event(4,0,0,0,[UIntPtr]::Zero)
  Start-Sleep -Seconds 2
}

# --- 阶段 1:admin 登录,断言 7 个导航项 ---
Start-App
$win = Login "admin" "admin123"
Assert ($win -ne $null) "admin 登录成功进入主窗口"
$nav = Get-Nav $win
Assert ($nav.Count -eq 7) "导航项数 = 7(实际 $($nav.Count))"
Assert (($nav | Where-Object { $_.Current.Name -match "用户管理" }).Count -eq 1) "导航含'用户管理'"

# --- 阶段 2:进入用户管理,新增普通用户 testuser ---
$navU = $nav | Where-Object { $_.Current.Name -match "用户管理" }
Click-Item $navU
$bcond = New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ControlTypeProperty, [System.Windows.Automation.ControlType]::Button)
$btnAdd = $win.FindFirst([System.Windows.Automation.TreeScope]::Descendants, (New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::NameProperty, "新增")))
Assert ($btnAdd -ne $null) "用户管理页'新增'按钮存在"
$btnAdd.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
Start-Sleep -Seconds 2
$dcond = New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ClassNameProperty, "UserEditDialog")
$dlg = $root.FindFirst([System.Windows.Automation.TreeScope]::Children, $dcond)
Assert ($dlg -ne $null) "新增用户对话框弹出"
$edits = $dlg.FindAll([System.Windows.Automation.TreeScope]::Descendants, $econd)
$edits[0].GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern).SetValue("testuser")
$edits[1].GetCurrentPattern([System.Windows.Automation.ValuePattern]::Pattern).SetValue("test1234")
# 角色下拉展开选"普通用户"(第 2 项)
$ccond = New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ControlTypeProperty, [System.Windows.Automation.ControlType]::ComboBox)
$combo = $dlg.FindFirst([System.Windows.Automation.TreeScope]::Descendants, $ccond)
$combo.GetCurrentPattern([System.Windows.Automation.ExpandCollapsePattern]::Pattern).Expand()
Start-Sleep -Milliseconds 800
$listItem = $dlg.FindAll([System.Windows.Automation.TreeScope]::Descendants, (New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ControlTypeProperty, [System.Windows.Automation.ControlType]::ListItem)))
$normal = $listItem | Where-Object { $_.Current.Name -eq "普通用户" }
$normal.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
Start-Sleep -Milliseconds 500
$dlg.FindFirst([System.Windows.Automation.TreeScope]::Descendants, (New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::NameProperty, "确定"))).GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
Start-Sleep -Seconds 2
Assert ($root.FindFirst([System.Windows.Automation.TreeScope]::Children, (New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ClassNameProperty, "UserEditDialog"))) -eq $null) "新增对话框已关闭"
# 表格中应出现 testuser
$tcond = New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::ControlTypeProperty, [System.Windows.Automation.ControlType]::Text)
$txts = $win.FindAll([System.Windows.Automation.TreeScope]::Descendants, $tcond)
Assert (($txts | Where-Object { $_.Current.Name -eq "testuser" }).Count -ge 1) "表格出现 testuser 行"

# --- 阶段 3:testuser 登录,断言 6 个导航项且按钮禁用 ---
Start-App
$win = Login "testuser" "test1234"
Assert ($win -ne $null) "testuser 登录成功(密码哈希验证通过)"
$nav = Get-Nav $win
Assert ($nav.Count -eq 6) "导航项数 = 6(实际 $($nav.Count))"
Assert (($nav | Where-Object { $_.Current.Name -match "用户管理" }).Count -eq 0) "导航不含'用户管理'"
# 学生管理页"新增"按钮应为禁用
Click-Item ($nav | Select-Object -First 1)
$btnAdd = $win.FindFirst([System.Windows.Automation.TreeScope]::Descendants, (New-Object System.Windows.Automation.PropertyCondition([System.Windows.Automation.AutomationElement]::NameProperty, "新增")))
Assert ($btnAdd -ne $null -and -not $btnAdd.Current.IsEnabled) "学生页'新增'按钮为禁用态"

Write-Host ("FAILED: {0}" -f $fails)
exit ($fails -gt 0)
```

- [ ] **Step 2: 运行脚本并确认 PASS**

```bash
powershell -ExecutionPolicy Bypass -File /c/Users/ME/user_mgmt_test.ps1
```

预期:所有断言 PASS。

- [ ] **Step 3: 清理测试用户**

用 SQL 删除 testuser(保留数据库干净):

```bash
"/d/MySQL/mysql-8.0.29-winx64/bin/mysql.exe" -u root -p123456 student_manage -e "DELETE FROM users WHERE username='testuser';"
```

- [ ] **Step 4: 推送**

```bash
git -C /c/Users/ME/StudentManager -c http.proxy=http://127.0.0.1:65532 push -q origin main
```

预期:push 成功(需用户开启代理)。
