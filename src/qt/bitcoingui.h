



#ifndef BITCOIN_QT_BITCOINGUI_H
#define BITCOIN_QT_BITCOINGUI_H

#if defined(HAVE_CONFIG_H)
#include "config/tesra-config.h"
#endif

#include "amount.h"

#include <QLabel>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QPoint>
#include <QPushButton>
#include <QSystemTrayIcon>

class ClientModel;
class NetworkStyle;
class Notificator;
class OptionsModel;
class BlockExplorer;
class RPCConsole;
class SendCoinsRecipient;
class UnitDisplayStatusBarControl;
class WalletFrame;
class WalletModel;
class MasternodeList;

class CWallet;

QT_BEGIN_NAMESPACE
class QAction;
class QProgressBar;
class QProgressDialog;
QT_END_NAMESPACE

/**
  Bitcoin GUI main class. This class represents the main window of the Bitcoin UI. It communicates with both the client and
  wallet models to give the user an up-to-date view of the current core state.
*/
class BitcoinGUI : public QMainWindow
{
    Q_OBJECT

public:
    static const QString DEFAULT_WALLET;

    explicit BitcoinGUI(const NetworkStyle* networkStyle, QWidget* parent = 0);
    ~BitcoinGUI();

    /** Set the client model.
        The client model represents the part of the core that communicates with the P2P network, and is wallet-agnostic.
    */
    void setClientModel(ClientModel* clientModel);

#ifdef ENABLE_WALLET
    /** Set the wallet model.
        The wallet model represents a bitcoin wallet, and offers access to the list of transactions, address book and sending
        functionality.
    */
    bool addWallet(const QString& name, WalletModel* walletModel);
    bool setCurrentWallet(const QString& name);
    void removeAllWallets();
#endif 
    bool enableWallet;
    bool fMultiSend = false;

protected:
    void changeEvent(QEvent* e);
    void closeEvent(QCloseEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    bool eventFilter(QObject* object, QEvent* event);

private:
    ClientModel* clientModel;
    WalletFrame* walletFrame;

    UnitDisplayStatusBarControl* unitDisplayControl;
    QLabel* labelStakingIcon;
    QPushButton* labelEncryptionIcon;
    QPushButton* labelConnectionsIcon;
    QLabel* labelBlocksIcon;
    QLabel* progressBarLabel;
    QProgressBar* progressBar;
    QProgressDialog* progressDialog;

    QMenuBar* appMenuBar;
    QAction* overviewAction;
    QAction* historyAction;
    QAction* masternodeAction;
    QAction* createContractAction;

    QAction* quitAction;
    QAction* sendCoinsAction;
    QAction* usedSendingAddressesAction;
    QAction* usedReceivingAddressesAction;
    QAction* signMessageAction;
    QAction* verifyMessageAction;
    QAction* bip38ToolAction;
    QAction* multisigCreateAction;
    QAction* multisigSpendAction;
    QAction* multisigSignAction;
    QAction* aboutAction;
    QAction* receiveCoinsAction;
    QAction* privacyAction;
    QAction* optionsAction;
    QAction* toggleHideAction;
    QAction* encryptWalletAction;
    QAction* backupWalletAction;
    QAction* changePassphraseAction;
    QAction* unlockWalletAction;
    QAction* lockWalletAction;
    QAction* aboutQtAction;
    QAction* openInfoAction;
    QAction* openRPCConsoleAction;
    QAction* openNetworkAction;
    QAction* openPeersAction;
    QAction* openRepairAction;
    QAction* openConfEditorAction;
    QAction* openMNConfEditorAction;
    QAction* showBackupsAction;
    QAction* openAction;
    QAction* openBlockExplorerAction;
    QAction* showHelpMessageAction;
    QAction* multiSendAction;

    QSystemTrayIcon* trayIcon;
    QMenu* trayIconMenu;
    Notificator* notificator;
    RPCConsole* rpcConsole;
    BlockExplorer* explorerWindow;

    
    int prevBlocks;
    int spinnerFrame;

    
    void createActions(const NetworkStyle* networkStyle);
    
    void createMenuBar();
    
    void createToolBars();
    
    void createTrayIcon(const NetworkStyle* networkStyle);
    
    void createTrayIconMenu();

    
    void setWalletActionsEnabled(bool enabled);

    
    void subscribeToCoreSignals();
    
    void unsubscribeFromCoreSignals();

signals:
    
    void receivedURI(const QString& uri);
    
    void requestedRestart(QStringList args);

public slots:
    
    void setNumConnections(int count);
    
    void setNumBlocks(int count);
    
    void handleRestart(QStringList args);

    /** Notify the user of an event from the core network or transaction handling code.
       @param[in] title     the message box / notification title
       @param[in] message   the displayed text
       @param[in] style     modality and style definitions (icon and used buttons - buttons only for message boxes)
                            @see CClientUIInterface::MessageBoxFlags
       @param[in] ret       pointer to a bool that will be modified to whether Ok was clicked (modal only)
    */
    void message(const QString& title, const QString& message, unsigned int style, bool* ret = NULL);

    void setStakingStatus();

#ifdef ENABLE_WALLET
    /** Set the encryption status as shown in the UI.
       @param[in] status            current encryption status
       @see WalletModel::EncryptionStatus
    */
    void setEncryptionStatus(int status);

    bool handlePaymentRequest(const SendCoinsRecipient& recipient);

    
    void incomingTransaction(const QString& date, int unit, const CAmount& amount, const QString& type, const QString& address);
#endif 

private slots:
#ifdef ENABLE_WALLET
    
    void gotoOverviewPage();
    
    void gotoCreateContractPage();
    
    void gotoHistoryPage();
    
    void gotoBlockExplorerPage();
    
    void gotoMasternodePage();
    
    void gotoReceiveCoinsPage();
    
    void gotoPrivacyPage();
    
    void gotoSendCoinsPage(QString addr = "");

    
    void gotoSignMessageTab(QString addr = "");
    
    void gotoVerifyMessageTab(QString addr = "");
    
    void gotoMultiSendDialog();
    
    void gotoMultisigCreate();
    void gotoMultisigSpend();
    void gotoMultisigSign();
    
    void gotoBip38Tool();

    
    void openClicked();

#endif 
    
    void optionsClicked();
    
    void aboutClicked();
    
    void showHelpMessageClicked();
#ifndef Q_OS_MAC
    
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
#endif

    
    void showNormalIfMinimized(bool fToggleHidden = false);
    
    void toggleHidden();

    
    void detectShutdown();

    
    void showProgress(const QString& title, int nProgress);
};

class UnitDisplayStatusBarControl : public QLabel
{
    Q_OBJECT

public:
    explicit UnitDisplayStatusBarControl();
    
    void setOptionsModel(OptionsModel* optionsModel);

protected:
    
    void mousePressEvent(QMouseEvent* event);

private:
    OptionsModel* optionsModel;
    QMenu* menu;

    
    void onDisplayUnitsClicked(const QPoint& point);
    
    void createContextMenu();

private slots:
    
    void updateDisplayUnit(int newUnits);
    
    void onMenuSelection(QAction* action);
};

#endif 
