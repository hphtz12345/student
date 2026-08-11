#pragma once
#include <QAbstractTableModel>
#include <QVariantMap>
#include <QVector>
#include <QPair>
#include <QString>

class TableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit TableModel(QObject *parent = nullptr);

    void setColumns(const QVector<QPair<QString, QString>> &cols);
    void load(const QString &sql, const QVariantList &args = {});
    void clear();
    QString lastError() const { return m_error; }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QVariantMap rowAt(int row) const;

private:
    QVector<QPair<QString, QString>> m_columns; // {字段名, 表头}
    QVector<QVariantMap> m_rows;
    QString m_error;
};
