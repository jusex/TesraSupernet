



#ifndef BITCOIN_QT_WALLETFRAME_H
#define BITCOIN_QT_WALLETFRAME_H

#include <QFrame>
#include <QMap>

class BitcoinGUI;
class ClientModel;
class SendCoinsRecipient;
class WalletModel;
class WalletView;
class TradingDialog;
class BlockExplorer;

QT_BEGIN_NAMESPACE
class QStackedWidget;
QT_END_NAMESPACE

class WalletFrame : public QFrame
{
    Q_OBJECT

public:
    explicit WalletFrame(BitcoinGUI* _gui = 0);
    ~WalletFrame();

    void setClientModel(ClientModel* clientModel);

    bool addWallet(const QString& name, WalletModel* walletModel);
    bool setCurrentWallet(const QString& name);
    bool removeWallet(const QString& name);
    void removeAllWallets();

    bool handlePaymentRequest(const SendCoinsRecipient& recipient);

    void showOutOfSyncWarning(bool fShow);

private:
    QStackedWidget* walletStack;
    BitcoinGUI* gui;
    ClientModel* clientModel;
    QMap<QString, WalletView*> mapWalletViews;

    bool bOutOfSync;

    WalletView* currentWalletView();

public slots:
    
    void gotoOverviewPage();
    
    void gotoCreateContractPage();
    
    void gotoHistoryPage();
    
    void gotoMasternodePage();
    
    void gotoReceiveCoinsPage();
    
    void gotoPrivacyPage();
    
    void gotoSendCoinsPage(QString addr = "");
    
    void gotoBlockExplorerPage();
    
    void gotoSignMessageTab(QString addr = "");
    
    void gotoVerifyMessageTab(QString addr = "");
    
    void gotoMultiSendDialog();
    
    void gotoMultisigDialog(int index);
    
    void gotoBip38Tool();

    
    void encryptWallet(bool status);
    
    void backupWallet();
    
    void changePassphrase();
    
    void unlockWallet();
    
    void lockWallet();
    
    void toggleLockWallet();

    
    void usedSendingAddresses();
    
    void usedReceivingAddresses();
};

#endif 
