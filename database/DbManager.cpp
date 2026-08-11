#include "DbManager.h"
#include <QDebug>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QSqlError>

DbManager &DbManager::instance() {
    static DbManager mgr;
    return mgr;
}

bool DbManager::open(const DbConfig &cfg, QString *errMsg) {
    m_db = QSqlDatabase::addDatabase("QMYSQL");
    m_db.setHostName(cfg.host);
    m_db.setPort(cfg.port);
    m_db.setDatabaseName(cfg.dbName);
    m_db.setUserName(cfg.user);
    m_db.setPassword(cfg.password);
    m_db.setConnectOptions("MYSQL_OPT_RECONNECT=1");
    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        if (errMsg) *errMsg = m_lastError;
        return false;
    }
    m_db.exec("SET NAMES utf8mb4");
    m_cfg = cfg;
    m_lastError.clear();
    return true;
}

void DbManager::close() {
    if (m_db.isOpen()) m_db.close();
}

bool DbManager::isOpen() const { return m_db.isOpen(); }

const DbConfig &DbManager::config() const { return m_cfg; }

QSqlQuery DbManager::execQuery(const QString &sql, const QVariantList &args) {
    QSqlQuery q(m_db);
    q.prepare(sql);
    for (const QVariant &v : args) q.addBindValue(v);
    if (!q.exec()) m_lastError = q.lastError().text();
    return q;
}

bool DbManager::execUpdate(const QString &sql, const QVariantList &args, int *affected) {
    QSqlQuery q(m_db);
    q.prepare(sql);
    for (const QVariant &v : args) q.addBindValue(v);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    m_lastError.clear();
    if (affected) *affected = q.numRowsAffected();
    return true;
}

QVariant DbManager::queryValue(const QString &sql, const QVariantList &args) {
    QSqlQuery q = execQuery(sql, args);
    if (!q.isActive()) return QVariant();
    return q.next() ? q.value(0) : QVariant();
}

QString DbManager::lastError() const { return m_lastError; }

QString DbManager::hashPassword(const QString &password, const QString &salt) {
    return QString::fromLatin1(
        QCryptographicHash::hash((password + salt).toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString DbManager::generateSalt() {
    QByteArray bytes(16, 0);
    // Qt 6.11 fillRange 仅接受 >= unsigned int 的无符号类型,uchar 不可用,改用 quint32
    QRandomGenerator::global()->fillRange(reinterpret_cast<quint32 *>(bytes.data()), 4);
    return QString::fromLatin1(bytes.toHex()); // 32 字符 hex
}
