



#ifndef BITCOIN_QT_BITCOINAMOUNTFIELD_H
#define BITCOIN_QT_BITCOINAMOUNTFIELD_H

#include "amount.h"

#include <QWidget>

class AmountSpinBox;

QT_BEGIN_NAMESPACE
class QValueComboBox;
QT_END_NAMESPACE

/** Widget for entering bitcoin amounts.
  */
class BitcoinAmountField : public QWidget
{
    Q_OBJECT

    
    
    Q_PROPERTY(qint64 value READ value WRITE setValue NOTIFY valueChanged USER true)

public:
    explicit BitcoinAmountField(QWidget* parent = 0);

    CAmount value(bool* value = 0) const;
    void setValue(const CAmount& value);

    
    void setSingleStep(const CAmount& step);

    
    void setReadOnly(bool fReadOnly);

    
    void setValid(bool valid);
    
    bool validate();

    
    void setDisplayUnit(int unit);

    
    void clear();

    
    void setEnabled(bool fEnabled);

    /** Qt messes up the tab chain by default in some cases (issue https:
        in these cases we have to set it up manually.
    */
    QWidget* setupTabChain(QWidget* prev);

signals:
    void valueChanged();

protected:
    
    bool eventFilter(QObject* object, QEvent* event);

private:
    AmountSpinBox* amount;
    QValueComboBox* unit;

private slots:
    void unitChanged(int idx);
};

#endif 
