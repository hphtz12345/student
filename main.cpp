#include <QApplication>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>
#include "database/DbManager.h"

// 数据库自检模式:不启动 GUI,顺序验证连接与 CRUD
static int runDbTest() {
    QTextStream out(stdout);
    auto &db = DbManager::instance();

    DbConfig cfg;
    cfg.host = "127.0.0.1";
    cfg.port = 3306;
    cfg.dbName = "student_manage";
    cfg.user = "root";
    cfg.password = "123456"; // 运行时按需修改,或从 config.ini 读取

    QString err;
    if (!db.open(cfg, &err)) {
        out << "连接失败: " << err << "\n";
        return 1;
    }
    out << "[1/5] 连接成功\n";

    if (!db.execUpdate("INSERT INTO students (student_no, name) VALUES (?, ?)", {"TEST001", "自检学生"})) {
        out << "[2/5] INSERT 失败: " << db.lastError() << "\n";
        return 1;
    }
    out << "[2/5] INSERT 成功\n";

    if (!db.execUpdate("UPDATE students SET name=? WHERE student_no=?", {"自检学生改", "TEST001"})) {
        out << "[3/5] UPDATE 失败: " << db.lastError() << "\n";
        return 1;
    }
    out << "[3/5] UPDATE 成功\n";

    QVariant name = db.queryValue("SELECT name FROM students WHERE student_no=?", {"TEST001"});
    if (name.toString() != "自检学生改") {
        out << "[4/5] SELECT 失败: 取回值不符\n";
        return 1;
    }
    out << "[4/5] SELECT 成功,值=" << name.toString() << "\n";

    if (!db.execUpdate("DELETE FROM students WHERE student_no=?", {"TEST001"})) {
        out << "[5/5] DELETE 失败: " << db.lastError() << "\n";
        return 1;
    }
    out << "[5/5] DELETE 成功\n";
    out << "自检全部通过\n";
    db.close();
    return 0;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QStringList args = QCoreApplication::arguments();
    if (args.contains("--db-test"))
        return runDbTest();
    // TODO(Task 5): 此处替换为:读配置 → open → LoginDialog → MainWindow
    return 0;
}
