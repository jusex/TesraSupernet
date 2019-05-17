



#ifndef BITCOIN_QT_QVALUECOMBOBOX_H
#define BITCOIN_QT_QVALUECOMBOBOX_H

#include <QComboBox>
#include <QVariant>


class QValueComboBox : public QComboBox
{
    Q_OBJECT

    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged USER true)

public:
    explicit QValueComboBox(QWidget* parent = 0);

    QVariant value() const;
    void setValue(const QVariant& value);

    
    void setRole(int role);

signals:
    void valueChanged();

private:
    int role;

private slots:
    void handleSelectionChanged(int idx);
};

#endif 
