# 用户管理功能设计

日期:2026-08-11
状态:已批准

## 背景与目标

系统目前只有 admin 一个账号,`users` 表已有 `role` 字段(默认 'admin')但代码中从未使用,所有用户登录后权限完全相同。本功能增加用户管理页面,并实现管理员 / 普通用户两种角色:

- **管理员**:全部功能 + 用户管理页面
- **普通用户**:只读模式(可查看全部数据页面,增删改按钮禁用)

## 需求确认(已与用户确认)

1. 普通用户权限:只读模式,按钮禁用
2. 密码:新增时管理员填写初始密码;重置密码也由管理员输入新密码
3. 用户管理页面入口:仅管理员可见;状态栏显示当前登录用户和角色

## 方案:按钮禁用 + 按角色构建导航

### 1. 会话与登录

- `DbManager` 增加静态字段 `m_currentUser` / `m_currentRole` 及静态存取方法 `currentUser()` / `currentRole()` / `setSession(user, role)`
- `LoginDialog::onLogin` 查询改为 `SELECT salt, role FROM users WHERE username=?`,验证通过后调用 `setSession(user, role)` 再 `accept()`

### 2. 用户管理页面(新增)

- `ui/UserPage.h/.cpp`:表格列(用户名、角色、创建时间)+ 按钮(新增、修改、删除、重置密码),搜索框
- `ui/UserEditDialog.h/.cpp`:新增/修改对话框
  - 字段:用户名、角色下拉(QComboBox,选项 管理员/普通用户)、密码(新增必填;修改时留空表示不改)
  - 密码加密复用 `DbManager::hashPassword(password, salt)` + `DbManager::generateSalt()`
- 重置密码:`UserEditDialog` 以"仅密码"模式复用,或独立小对话框——采用独立 `UserPasswordDialog`(只输入新密码),职责单一
- 保护规则:
  - 删除时若选中自己 → 弹窗拒绝
  - 修改时若选中自己 → 角色下拉禁用(防止把自己降级后锁死系统)
  - 删除前 QMessageBox 确认
- 角色显示中文映射:admin → 管理员,user → 普通用户

### 3. 角色权限

- `MainWindow`:
  - 登录后根据 `DbManager::currentRole()` 决定导航:管理员 7 项(含"用户管理" 🔑),普通用户 6 项
  - 普通用户:对 5 个数据页面调用 `setReadOnly(true)`
  - 状态栏:显示 `当前用户: <用户名>(管理员/普通用户)`
- 5 个数据页面(Student/Teacher/Class/Course/Score)各增加:
  - `void setReadOnly(bool ro)`:禁用 新增/修改/删除(成绩页为"录入")按钮,搜索/刷新保留可用
  - 成员 `bool m_readOnly` 记录状态

### 4. 不改动项

- 普通用户自助改密、登录失败次数限制、会话超时:不做
- users 表结构:不需要迁移(role 字段已存在)

## 验证

1. 编译通过 + 部署 dist
2. UIA 自动化:
   - admin 登录 → 导航 7 项,用户管理页存在
   - 新增普通用户 → 登录 → 导航 6 项,无用户管理;学生页新增按钮 IsEnabled=false
   - 删除/修改自己的防护逻辑人工核对
3. 密码验证:用普通用户登录成功即证明 salt/hash 正确写入
