



#ifndef BITCOIN_QT_ASKPASSPHRASEDIALOG_H
#define BITCOIN_QT_ASKPASSPHRASEDIALOG_H

#include <QDialog>

class WalletModel;

namespace Ui
{
class AskPassphraseDialog;
}

/** Multifunctional dialog to ask for passphrases. Used for encryption, unlocking, and changing the passphrase.
 */
class AskPassphraseDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        Encrypt,         
        UnlockAnonymize, 
        Unlock,          
        ChangePass,      
        Decrypt          
    };

    explicit AskPassphraseDialog(Mode mode, QWidget* parent, WalletModel* model);
    ~AskPassphraseDialog();

    void accept();

private:
    Ui::AskPassphraseDialog* ui;
    Mode mode;
    WalletModel* model;
    bool fCapsLock;

private slots:
    void textChanged();

protected:
    bool event(QEvent* event);
    bool eventFilter(QObject* object, QEvent* event);
};

#endif 
