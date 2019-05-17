





#include "bitcoinaddressvalidator.h"

#include "base58.h"

/* Base58 characters are:
     "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"

  This is:
  - All numbers except for '0'
  - All upper-case letters except for 'I' and 'O'
  - All lower-case letters except for 'l'
*/

BitcoinAddressEntryValidator::BitcoinAddressEntryValidator(QObject* parent) : QValidator(parent)
{
}

QValidator::State BitcoinAddressEntryValidator::validate(QString& input, int& pos) const
{
    Q_UNUSED(pos);

    
    if (input.isEmpty())
        return QValidator::Intermediate;

    
    for (int idx = 0; idx < input.size();) {
        bool removeChar = false;
        QChar ch = input.at(idx);
        
        
        
        switch (ch.unicode()) {
        
        case 0x200B: 
        case 0xFEFF: 
            removeChar = true;
            break;
        default:
            break;
        }

        
        if (ch.isSpace())
            removeChar = true;

        
        if (removeChar)
            input.remove(idx, 1);
        else
            ++idx;
    }

    
    QValidator::State state = QValidator::Acceptable;
    for (int idx = 0; idx < input.size(); ++idx) {
        int ch = input.at(idx).unicode();

        if (((ch >= '0' && ch <= '9') ||
                (ch >= 'a' && ch <= 'z') ||
                (ch >= 'A' && ch <= 'Z')) &&
            ch != 'l' && ch != 'I' && ch != '0' && ch != 'O') {
            
        } else {
            state = QValidator::Invalid;
        }
    }

    return state;
}

BitcoinAddressCheckValidator::BitcoinAddressCheckValidator(QObject* parent) : QValidator(parent)
{
}

QValidator::State BitcoinAddressCheckValidator::validate(QString& input, int& pos) const
{
    Q_UNUSED(pos);
    
    CBitcoinAddress addr(input.toStdString());
    if (addr.IsValid())
        return QValidator::Acceptable;

    return QValidator::Invalid;
}
