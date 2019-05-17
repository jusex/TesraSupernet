



#ifndef BITCOIN_QT_WALLETVIEW_H
#define BITCOIN_QT_WALLETVIEW_H

#include "amount.h"
#include "masternodelist.h"

#include <QStackedWidget>

class BitcoinGUI;
class ClientModel;
class OverviewPage;
class CreateContractPage;
class ReceiveCoinsDialog;
class PrivacyDialog;
class SendCoinsDialog;
class SendCoinsRecipient;
class TransactionView;
class WalletModel;
class BlockExplorer;

QT_BEGIN_NAMESPACE
class QLabel;
class QModelIndex;
class QProgressDialog;
QT_END_NAMESPACE

/*
  WalletView class. This class represents the view to a single wallet.
  It was added to support multiple wallet functionality. Each wallet gets its own WalletView instance.
  It communicates with both the client and the wallet models to give the user an up-to-date view of the
  current core state.
*/
class WalletView : public QStackedWidget
{
    Q_OBJECT

public:
    explicit WalletView(QWidget* parent);
    ~WalletView();

    void setBitcoinGUI(BitcoinGUI* gui);
    /** Set the client model.
        The client model represents the part of the core that communicates with the P2P network, and is wallet-agnostic.
    */
    void setClientModel(ClientModel* clientModel);
    /** Set the wallet model.
        The wallet model represents a bitcoin wallet, and offers access to the list of transactions, address book and sending
        functionality.
    */
    void setWalletModel(WalletModel* walletModel);

    bool handlePaymentRequest(const SendCoinsRecipient& recipient);

    void showOutOfSyncWarning(bool fShow);

private:
    ClientModel* clientModel;
    WalletModel* walletModel;

    OverviewPage* overviewPage;

    CreateContractPage* createContractPage;
    QWidget* transactionsPage;
    ReceiveCoinsDialog* receiveCoinsPage;
    PrivacyDialog* privacyPage;
    SendCoinsDialog* sendCoinsPage;
    BlockExplorer* explorerWindow;
    MasternodeList* masternodeListPage;

    TransactionView* transactionView;

    QProgressDialog* progressDialog;
    QLabel* transactionSum;

public slots:
    
    void gotoOverviewPage();
    
    void gotoCreateContractPage();
    
    void gotoHistoryPage();
    
    void gotoMasternodePage();
    
    void gotoBlockExplorerPage();
    
    void gotoPrivacyPage();
    
    void gotoReceiveCoinsPage();
    
    void gotoSendCoinsPage(QString addr = "");

    
    void gotoSignMessageTab(QString addr = "");
    
    void gotoVerifyMessageTab(QString addr = "");
    
    void gotoMultiSendDialog();
    
    void gotoMultisigDialog(int index);
    
    void gotoBip38Tool();

    /** Show incoming transaction notification for new transactions.

        The new items are those between start and end inclusive, under the given parent item.
    */
    void processNewTransaction(const QModelIndex& parent, int start, int );
    
    void encryptWallet(bool status);
    
    void backupWallet();
    
    void changePassphrase();
    
    void unlockWallet();
    
    void lockWallet();
    
    void toggleLockWallet();

    
    void usedSendingAddresses();
    
    void usedReceivingAddresses();

    
    void updateEncryptionStatus();

    
    void showProgress(const QString& title, int nProgress);

    
    void trxAmount(QString amount);

signals:
    
    void showNormalIfMinimized();
    
    void message(const QString& title, const QString& message, unsigned int style);
    
    void encryptionStatusChanged(int status);
    
    void incomingTransaction(const QString& date, int unit, const CAmount& amount, const QString& type, const QString& address);
};

#endif 
