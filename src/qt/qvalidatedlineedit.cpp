



#include <qt/qvalidatedlineedit.h>

#include <qt/bitcoinaddressvalidator.h>


QValidatedLineEdit::QValidatedLineEdit(QWidget *parent) :
    QLineEdit(parent),
    valid(true),
    checkValidator(0),
    emptyIsValid(true)
{
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(markValid()));
}

void QValidatedLineEdit::setValid(bool _valid)
{
    if(_valid == this->valid)
    {
        return;
    }

    if(_valid)
    {
        QWidget *widget = this->parentWidget();
        if(widget && widget->inherits("QComboBox"))
        {
            widget->setStyleSheet("");
        }
        else
        {
            setStyleSheet("");
        }
    }
    else
    {
        QWidget *widget = this->parentWidget();
        if(widget && widget->inherits("QComboBox"))
        {
           
        }
        else
        {
           
        }
    }
    this->valid = _valid;
}

void QValidatedLineEdit::focusInEvent(QFocusEvent *evt)
{
    
    setValid(true);

    QLineEdit::focusInEvent(evt);
}

void QValidatedLineEdit::focusOutEvent(QFocusEvent *evt)
{
    checkValidity();

    QLineEdit::focusOutEvent(evt);
}

bool QValidatedLineEdit::getEmptyIsValid() const
{
    return emptyIsValid;
}

void QValidatedLineEdit::setEmptyIsValid(bool value)
{
    emptyIsValid = value;
}

void QValidatedLineEdit::markValid()
{
    
    setValid(true);
}

void QValidatedLineEdit::clear()
{
    setValid(true);
    QLineEdit::clear();
}

void QValidatedLineEdit::setEnabled(bool enabled)
{
    if (!enabled)
    {
        
        setValid(true);
    }
    else
    {
        
        checkValidity();
    }

    QLineEdit::setEnabled(enabled);
}

void QValidatedLineEdit::checkValidity()
{
    if (emptyIsValid && text().isEmpty())
    {
        setValid(true);
    }
    else if (hasAcceptableInput())
    {
        setValid(true);

        
        if (checkValidator)
        {
            QString address = text();
            int pos = 0;
            if (checkValidator->validate(address, pos) == QValidator::Acceptable)
                setValid(true);
            else
                setValid(false);
        }
    }
    else
        setValid(false);

    Q_EMIT validationDidChange(this);
}

void QValidatedLineEdit::setCheckValidator(const QValidator *v)
{
    checkValidator = v;
}

bool QValidatedLineEdit::isValid()
{
    
    if (checkValidator)
    {
        QString address = text();
        int pos = 0;
        if (checkValidator->validate(address, pos) == QValidator::Acceptable)
            return true;
    }

    return valid;
}
