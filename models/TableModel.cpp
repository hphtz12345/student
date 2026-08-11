#include "TableModel.h"
#include "../database/DbManager.h"
#include <QSqlQuery>
#include <QSqlRecord>

TableModel::TableModel(QObject *parent) : QAbstractTableModel(parent) {}

void TableModel::setColumns(const QVector<QPair<QString, QString>> &cols) {
    beginResetModel();
    m_columns = cols;
    endResetModel();
}

void TableModel::load(const QString &sql, const QVariantList &args) {
    beginResetModel();
    m_rows.clear();
    m_error.clear();
    QSqlQuery q = DbManager::instance().execQuery(sql, args);
    if (!q.isActive()) {
        m_error = DbManager::instance().lastError();
    } else {
        QSqlRecord rec = q.record();
        while (q.next()) {
            QVariantMap row;
            for (int i = 0; i < rec.count(); ++i)
                row[rec.fieldName(i)] = q.value(i);
            m_rows.append(row);
        }
    }
    endResetModel();
}

void TableModel::clear() {
    beginResetModel();
    m_rows.clear();
    m_error.clear();
    endResetModel();
}

int TableModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_rows.size();
}

int TableModel::columnCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_columns.size();
}

QVariant TableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) return QVariant();
    const QString field = m_columns[index.column()].first;
    return m_rows[index.row()].value(field);
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return QVariant();
    if (orientation == Qt::Horizontal) return m_columns[section].second;
    return section + 1;
}

QVariantMap TableModel::rowAt(int row) const {
    return (row >= 0 && row < m_rows.size()) ? m_rows[row] : QVariantMap();
}
