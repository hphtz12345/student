#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QVariantList>
#include <QString>

struct DbConfig {
    QString host = "127.0.0.1";
    int port = 3306;
    QString dbName = "student_manage";
    QString user = "root";
    QString password;
};

class DbManager {
public:
    static DbManager &instance();

    bool open(const DbConfig &cfg, QString *errMsg = nullptr);
    void close();
    bool isOpen() const;
    const DbConfig &config() const;

    QSqlQuery execQuery(const QString &sql, const QVariantList &args = {});
    bool execUpdate(const QString &sql, const QVariantList &args = {}, int *affected = nullptr);
    QVariant queryValue(const QString &sql, const QVariantList &args = {});
    QString lastError() const;

    static QString hashPassword(const QString &password, const QString &salt);
    static QString generateSalt();

private:
    DbManager() = default;
    QSqlDatabase m_db;
    DbConfig m_cfg;
    QString m_lastError;
};
