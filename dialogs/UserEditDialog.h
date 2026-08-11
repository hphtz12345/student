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
