# 学生管理系统 (StudentManager)

基于 **Qt 6 Widgets + MySQL 8** 的桌面学生管理系统,面向日常实际使用场景:界面美观、数据可靠、功能完整。

## 功能特性

- **登录验证**:用户名 + 密码,密码加盐 SHA-256 加密存储,默认管理员 `admin / admin123`
- **学生管理**:学号/姓名/班级搜索,增删改查,学号格式与唯一性校验
- **教师管理**:编号/姓名/职称搜索,增删改查
- **班级管理**:按名称/专业搜索,可指定班主任
- **课程管理**:按编号/名称/教师搜索,学分录入
- **成绩管理**:班级→学生级联筛选,按班级/课程/学期组合过滤,分数 0-100 校验,一人一学期一科唯一
- **统计图表** (QtCharts):
  - 各班平均分柱状图
  - 课程成绩分数段分布饼图
  - 各课程平均分横向条形图
- **数据安全**:外键 `ON DELETE RESTRICT` 防止误删(有成绩的学生、有学生的班级等不可直接删除);utf8mb4 中文无乱码;连接可配置(config.ini)

## 技术栈

| 组件 | 版本 | 说明 |
|------|------|------|
| Qt | 6.11.1 (mingw_64) | QtSql + QtCharts,qmake 构建 |
| MySQL | 8.0.29 | 本地服务,数据库 `student_manage` |
| 驱动 | 自编译 qsqlmysql | Qt 6 不自带 MySQL 驱动,已编译入 Qt 插件目录 |

## 快速开始

### 方式一:使用发布包(推荐)

`dist/` 目录已自包含全部运行库(约 75MB):

1. 将整个 `dist/` 文件夹复制到目标电脑
2. 运行 `StudentManager.exe`
3. 首次运行弹出数据库连接设置:填入 MySQL 主机/端口/账号/密码(本机默认 `127.0.0.1:3306`),点"测试连接",保存后登录

### 方式二:源码构建

**环境要求**:Qt 6.11.1 mingw_64、MySQL 8.0.29、qsqlmysql 驱动(见下文)

```bash
cd StudentManager
export PATH="/c/Qt/6.11.1/mingw_64/bin:/c/Qt/Tools/mingw1310_64/bin:$PATH"
qmake StudentManager.pro
mingw32-make -j4
./release/StudentManager.exe   # 需 PATH 含 MySQL lib 目录(libmysql.dll)
```

### 数据库初始化

```bash
mysql -u root -p < database/schema.sql
```

自动创建 `student_manage` 库、6 张表(users/teachers/classes/students/courses/scores)和默认管理员。

## 项目结构

```
StudentManager/
├── StudentManager.pro          # qmake 工程
├── main.cpp                    # 入口(支持 --db-test 自检)
├── config.ini                  # 数据库连接配置(自动生成)
├── database/
│   ├── schema.sql              # 建库建表脚本
│   └── DbManager.h/.cpp        # 数据访问层(唯一数据库入口)
├── models/
│   └── TableModel.h/.cpp       # 通用只读表格模型
├── ui/
│   ├── LoginDialog / MainWindow
│   ├── StudentPage / TeacherPage / ClassPage
│   ├── CoursePage / ScorePage / StatisticsPage
│   └── DbConfigDialog
├── dialogs/                    # 各模块新增/编辑对话框
├── resources/style.qss         # 界面样式(构建后需复制到发布目录)
└── dist/                       # 可分发布(完整运行库)
```

## 常见问题

| 问题 | 解决 |
|------|------|
| `Driver not loaded` | 运行环境缺 `libmysql.dll`(在 MySQL `lib/` 目录),加入 PATH 或用发布包 |
| 中文乱码 | 确保连接字符集 utf8mb4(DbManager 已自动 `SET NAMES utf8mb4`) |
| 重新构建后界面没样式 | 把 `resources/style.qss` 复制到 exe 同目录 `resources/` |
| 删除被引用数据失败 | 属于外键保护,先删除关联数据(成绩→课程/学生等) |
| 修改学生/课程/班级后下拉不回填 | 对应管理页 SELECT 必须包含编辑对话框回填所需的隐藏列(如 class_id、teacher_id),这是本项目踩过的坑 |

## 自检

```bash
./release/StudentManager.exe --db-test   # 依次验证 连接→插入→更新→查询→删除
```
