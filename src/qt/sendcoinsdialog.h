



#ifndef BITCOIN_QT_SENDCOINSDIALOG_H
#define BITCOIN_QT_SENDCOINSDIALOG_H

#include "walletmodel.h"

#include <QDialog>
#include <QString>
#include <QAbstractButton>
#include <QTimer>
#include <QMessageBox>
static const int MAX_SEND_POPUP_ENTRIES = 10;

class ClientModel;
class OptionsModel;
class SendCoinsEntry;
class SendCoinsRecipient;

namespace Ui
{
class SendCoinsDialog;
}

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE


class SendCoinsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SendCoinsDialog(QWidget* parent = 0);
    ~SendCoinsDialog();

    void setClientModel(ClientModel* clientModel);
    void setModel(WalletModel* model);

    /** Set up the tab chain manually, as Qt messes up the tab chain by default in some cases (issue https:
     */
    QWidget* setupTabChain(QWidget* prev);

    void setAddress(const QString& address);
    void pasteEntry(const SendCoinsRecipient& rv);
    bool handlePaymentRequest(const SendCoinsRecipient& recipient);
    bool fSplitBlock;

public slots:
    void clear();
    void reject();
    void accept();
    SendCoinsEntry* addEntry();
    void updateTabsAndLabels();
    void setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance, 
                    const CAmount& zerocoinBalance, const CAmount& unconfirmedZerocoinBalance, const CAmount& immatureZerocoinBalance,
                    const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance);

private:
    Ui::SendCoinsDialog* ui;
    ClientModel* clientModel;
    WalletModel* model;
    bool fNewRecipientAllowed;
    void send(QList<SendCoinsRecipient> recipients, QString strFee, QStringList formatted);
    bool fFeeMinimized;

    
    
    
    void processSendCoinsReturn(const WalletModel::SendCoinsReturn& sendCoinsReturn, const QString& msgArg = QString(), bool fPrepare = false);
    void minimizeFeeSection(bool fMinimize);
    void updateFeeMinimizedLabel();

private slots:
    void on_sendButton_clicked();
    void on_buttonChooseFee_clicked();
    void on_buttonMinimizeFee_clicked();
    void removeEntry(SendCoinsEntry* entry);
    void updateDisplayUnit();
    void updateSwiftTX();
    void coinControlFeatureChanged(bool);
    void coinControlButtonClicked();
    void coinControlChangeChecked(int);
    void coinControlChangeEdited(const QString&);
    void coinControlUpdateLabels();
    void coinControlClipboardQuantity();
    void coinControlClipboardAmount();
    void coinControlClipboardFee();
    void coinControlClipboardAfterFee();
    void coinControlClipboardBytes();
    void coinControlClipboardPriority();
    void coinControlClipboardLowOutput();
    void coinControlClipboardChange();
    void splitBlockChecked(int);
    void splitBlockLineEditChanged(const QString& text);
    void setMinimumFee();
    void updateFeeSectionControls();
    void updateMinFeeLabel();
    void updateSmartFeeLabel();
    void updateGlobalFeeVariables();

signals:
    
    void message(const QString& title, const QString& message, unsigned int style);
};

#define SEND_CONFIRM_DELAY   3

class SendConfirmationDialog : public QMessageBox
{
    Q_OBJECT

public:
    SendConfirmationDialog(const QString &title, const QString &text, int secDelay = SEND_CONFIRM_DELAY, QWidget *parent = 0);
    int exec();

private Q_SLOTS:
    void countDown();
    void updateYesButton();

private:
    QAbstractButton *yesButton;
    QTimer countDownTimer;
    int secDelay;
};


#endif 
