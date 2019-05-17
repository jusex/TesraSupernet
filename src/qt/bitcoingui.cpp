





#include "bitcoingui.h"

#include "bitcoinunits.h"
#include "clientmodel.h"
#include "guiconstants.h"
#include "guiutil.h"
#include "miner.h"
#include "networkstyle.h"
#include "notificator.h"
#include "openuridialog.h"
#include "optionsdialog.h"
#include "optionsmodel.h"
#include "rpcconsole.h"
#include "utilitydialog.h"

#ifdef ENABLE_WALLET
#include "blockexplorer.h"
#include "walletframe.h"
#include "walletmodel.h"
#endif 

#ifdef Q_OS_MAC
#include "macdockiconhandler.h"
#endif

#include "init.h"
#include "masternodelist.h"
#include "ui_interface.h"
#include "util.h"

#include <iostream>

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QIcon>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QProgressDialog>
#include <QSettings>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

#if QT_VERSION < 0x050000
#include <QTextDocument>
#include <QUrl>
#else
#include <QUrlQuery>
#endif

const QString BitcoinGUI::DEFAULT_WALLET = "~Default";

BitcoinGUI::BitcoinGUI(const NetworkStyle* networkStyle, QWidget* parent) : QMainWindow(parent),
                                                                            clientModel(0),
                                                                            walletFrame(0),
                                                                            unitDisplayControl(0),
                                                                            labelStakingIcon(0),
                                                                            labelEncryptionIcon(0),
                                                                            labelConnectionsIcon(0),
                                                                            labelBlocksIcon(0),
                                                                            progressBarLabel(0),
                                                                            progressBar(0),
                                                                            progressDialog(0),
                                                                            appMenuBar(0),
                                                                            overviewAction(0),
                                                                            historyAction(0),
                                                                            masternodeAction(0),
                                                                            quitAction(0),
                                                                            sendCoinsAction(0),
                                                                            usedSendingAddressesAction(0),
                                                                            usedReceivingAddressesAction(0),
                                                                            signMessageAction(0),
                                                                            verifyMessageAction(0),
                                                                            bip38ToolAction(0),
                                                                            multisigCreateAction(0),
                                                                            multisigSpendAction(0),
                                                                            multisigSignAction(0),
                                                                            aboutAction(0),
                                                                            receiveCoinsAction(0),
                                                                            privacyAction(0),
                                                                            optionsAction(0),
                                                                            toggleHideAction(0),
                                                                            encryptWalletAction(0),
                                                                            backupWalletAction(0),
                                                                            changePassphraseAction(0),
                                                                            aboutQtAction(0),
                                                                            openRPCConsoleAction(0),
                                                                            openAction(0),
                                                                            showHelpMessageAction(0),
                                                                            multiSendAction(0),
                                                                            trayIcon(0),
                                                                            trayIconMenu(0),
                                                                            notificator(0),
                                                                            rpcConsole(0),
                                                                            explorerWindow(0),
                                                                            prevBlocks(0),
                                                                            spinnerFrame(0)
{
    
    this->setStyleSheet(GUIUtil::loadStyleSheet());

    GUIUtil::restoreWindowGeometry("nWindow", QSize(850, 550), this);

    QString windowTitle = tr("TESRA Core") + " - ";
#ifdef ENABLE_WALLET
    
    enableWallet = !GetBoolArg("-disablewallet", false);
#else
    enableWallet = false;
#endif 
    if (enableWallet) {
        windowTitle += tr("Wallet");
    } else {
        windowTitle += tr("Node");
    }
    QString userWindowTitle = QString::fromStdString(GetArg("-windowtitle", ""));
    if (!userWindowTitle.isEmpty()) windowTitle += " - " + userWindowTitle;
    windowTitle += " " + networkStyle->getTitleAddText();
#ifndef Q_OS_MAC
    QApplication::setWindowIcon(networkStyle->getAppIcon());
    setWindowIcon(networkStyle->getAppIcon());
#else
    MacDockIconHandler::instance()->setIcon(networkStyle->getAppIcon());
#endif
    setWindowTitle(windowTitle);

#if defined(Q_OS_MAC) && QT_VERSION < 0x050000
    
    
    setUnifiedTitleAndToolBarOnMac(true);
#endif

    rpcConsole = new RPCConsole(enableWallet ? this : 0);
#ifdef ENABLE_WALLET
    if (enableWallet) {
        
        walletFrame = new WalletFrame(this);
        explorerWindow = new BlockExplorer(this);
    } else
#endif 
    {
        /* When compiled without wallet or -disablewallet is provided,
         * the central widget is the rpc console.
         */
        setCentralWidget(rpcConsole);
    }

    
    setAcceptDrops(true);

    
    
    createActions(networkStyle);

    
    createMenuBar();

    
    createToolBars();

    
    createTrayIcon(networkStyle);

    
    statusBar();

    
    QFrame* frameBlocks = new QFrame();
    frameBlocks->setContentsMargins(0, 0, 0, 0);
    frameBlocks->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QHBoxLayout* frameBlocksLayout = new QHBoxLayout(frameBlocks);
    frameBlocksLayout->setContentsMargins(3, 0, 3, 0);
    frameBlocksLayout->setSpacing(3);
    unitDisplayControl = new UnitDisplayStatusBarControl();
    labelStakingIcon = new QLabel();
    labelEncryptionIcon = new QPushButton();
    labelEncryptionIcon->setFlat(true); 
    labelEncryptionIcon->setStyleSheet(".QPushButton { background-color: rgba(255, 255, 255, 0);}");
    labelEncryptionIcon->setMaximumSize(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE);
    labelConnectionsIcon = new QPushButton();
    labelConnectionsIcon->setFlat(true); 
    labelConnectionsIcon->setStyleSheet(".QPushButton { background-color: rgba(255, 255, 255, 0);}");
    labelConnectionsIcon->setMaximumSize(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE);
    labelBlocksIcon = new QLabel();

    if (enableWallet) {
        frameBlocksLayout->addStretch();
        frameBlocksLayout->addWidget(unitDisplayControl);
        frameBlocksLayout->addStretch();
        frameBlocksLayout->addWidget(labelEncryptionIcon);
    }
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelStakingIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelConnectionsIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelBlocksIcon);
    frameBlocksLayout->addStretch();

    
    progressBarLabel = new QLabel();
    progressBarLabel->setVisible(true);
    progressBar = new GUIUtil::ProgressBar();
    progressBar->setAlignment(Qt::AlignCenter);
    progressBar->setVisible(true);

    
    
    
    QString curStyle = QApplication::style()->metaObject()->className();
    if (curStyle == "QWindowsStyle" || curStyle == "QWindowsXPStyle") {
        progressBar->setStyleSheet("QProgressBar { background-color: #b4FFEBCD; border: 1px solid grey; border-radius: 7px; padding: 1px; text-align: center; } QProgressBar::chunk { background: QLinearGradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #00CCFF, stop: 1 #33CCFF); border-radius: 7px; margin: 0px; }");
    }

    statusBar()->addWidget(progressBarLabel);
    statusBar()->addWidget(progressBar);
    statusBar()->addPermanentWidget(frameBlocks);

    
    connect(openInfoAction, SIGNAL(triggered()), rpcConsole, SLOT(showInfo()));
    connect(openRPCConsoleAction, SIGNAL(triggered()), rpcConsole, SLOT(showConsole()));
    connect(openNetworkAction, SIGNAL(triggered()), rpcConsole, SLOT(showNetwork()));
    connect(openPeersAction, SIGNAL(triggered()), rpcConsole, SLOT(showPeers()));
    connect(openRepairAction, SIGNAL(triggered()), rpcConsole, SLOT(showRepair()));
    connect(openConfEditorAction, SIGNAL(triggered()), rpcConsole, SLOT(showConfEditor()));
    connect(openMNConfEditorAction, SIGNAL(triggered()), rpcConsole, SLOT(showMNConfEditor()));
    connect(showBackupsAction, SIGNAL(triggered()), rpcConsole, SLOT(showBackups()));
    connect(labelConnectionsIcon, SIGNAL(clicked()), rpcConsole, SLOT(showPeers()));
    connect(labelEncryptionIcon, SIGNAL(clicked()), walletFrame, SLOT(toggleLockWallet()));

    
    connect(rpcConsole, SIGNAL(handleRestart(QStringList)), this, SLOT(handleRestart(QStringList)));

    
    connect(quitAction, SIGNAL(triggered()), rpcConsole, SLOT(hide()));

    connect(openBlockExplorerAction, SIGNAL(triggered()), explorerWindow, SLOT(show()));

    
    connect(quitAction, SIGNAL(triggered()), explorerWindow, SLOT(hide()));

    
    this->installEventFilter(this);

    
    setWalletActionsEnabled(false);

    
    subscribeToCoreSignals();

    QTimer* timerStakingIcon = new QTimer(labelStakingIcon);
    connect(timerStakingIcon, SIGNAL(timeout()), this, SLOT(setStakingStatus()));
    timerStakingIcon->start(10000);
    setStakingStatus();
}

BitcoinGUI::~BitcoinGUI()
{
    
    unsubscribeFromCoreSignals();

    GUIUtil::saveWindowGeometry("nWindow", this);
    if (trayIcon) 
        trayIcon->hide();
#ifdef Q_OS_MAC
    delete appMenuBar;
    MacDockIconHandler::cleanup();
#endif
}

void BitcoinGUI::createActions(const NetworkStyle* networkStyle)
{
    QActionGroup* tabGroup = new QActionGroup(this);

    overviewAction = new QAction(QIcon(":/icons/overview"), tr("&Overview"), this);
    overviewAction->setStatusTip(tr("Show general overview of wallet"));
    overviewAction->setToolTip(overviewAction->statusTip());
    overviewAction->setCheckable(true);
#ifdef Q_OS_MAC
    overviewAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_1));
#else
    overviewAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
#endif
    tabGroup->addAction(overviewAction);

    sendCoinsAction = new QAction(QIcon(":/icons/send"), tr("&Send"), this);
    sendCoinsAction->setStatusTip(tr("Send coins to a TESRA address"));
    sendCoinsAction->setToolTip(sendCoinsAction->statusTip());
    sendCoinsAction->setCheckable(true);
#ifdef Q_OS_MAC
    sendCoinsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_2));
#else
    sendCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_2));
#endif
    tabGroup->addAction(sendCoinsAction);

    receiveCoinsAction = new QAction(QIcon(":/icons/receiving_addresses"), tr("&Receive"), this);
    receiveCoinsAction->setStatusTip(tr("Request payments (generates QR codes and tesra: URIs)"));
    receiveCoinsAction->setToolTip(receiveCoinsAction->statusTip());
    receiveCoinsAction->setCheckable(true);
#ifdef Q_OS_MAC
    receiveCoinsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_3));
#else
    receiveCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_3));
#endif
    tabGroup->addAction(receiveCoinsAction);

    historyAction = new QAction(QIcon(":/icons/history"), tr("&Transactions"), this);
    historyAction->setStatusTip(tr("Browse transaction history"));
    historyAction->setToolTip(historyAction->statusTip());
    historyAction->setCheckable(true);
#ifdef Q_OS_MAC
    historyAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_4));
#else
    historyAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_4));
#endif
    tabGroup->addAction(historyAction);

    privacyAction = new QAction(QIcon(":/icons/privacy"), tr("&Privacy"), this);
    privacyAction->setStatusTip(tr("Privacy Actions for zULO"));
    privacyAction->setToolTip(privacyAction->statusTip());
    privacyAction->setCheckable(true);
#ifdef Q_OS_MAC
    privacyAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_5));
#else
    privacyAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_5));
#endif
    tabGroup->addAction(privacyAction);

#ifdef ENABLE_WALLET

    QSettings settings;
    if (settings.value("fShowMasternodesTab").toBool()) {
        masternodeAction = new QAction(QIcon(":/icons/masternodes"), tr("&Masternodes"), this);
        masternodeAction->setStatusTip(tr("Browse masternodes"));
        masternodeAction->setToolTip(masternodeAction->statusTip());
        masternodeAction->setCheckable(true);
#ifdef Q_OS_MAC
        masternodeAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_6));
#else
        masternodeAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_6));
#endif
        tabGroup->addAction(masternodeAction);
        connect(masternodeAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
        connect(masternodeAction, SIGNAL(triggered()), this, SLOT(gotoMasternodePage()));
    }

    createContractAction = new QAction(QIcon(":/icons/createcontract"), tr("&CreateContract"), this);
    tabGroup->addAction(createContractAction);
    createContractAction->setCheckable(true);

    
    connect(createContractAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(createContractAction, SIGNAL(triggered()), this, SLOT(gotoCreateContractPage()));

    
    
    connect(overviewAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(overviewAction, SIGNAL(triggered()), this, SLOT(gotoOverviewPage()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(gotoSendCoinsPage()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(gotoReceiveCoinsPage()));
    connect(privacyAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(privacyAction, SIGNAL(triggered()), this, SLOT(gotoPrivacyPage()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(gotoHistoryPage()));
#endif 

    quitAction = new QAction(QIcon(":/icons/quit"), tr("E&xit"), this);
    quitAction->setStatusTip(tr("Quit application"));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    quitAction->setMenuRole(QAction::QuitRole);
    aboutAction = new QAction(networkStyle->getAppIcon(), tr("&About TESRA Core"), this);
    aboutAction->setStatusTip(tr("Show information about TESRA Core"));
    aboutAction->setMenuRole(QAction::AboutRole);
#if QT_VERSION < 0x050000
    aboutQtAction = new QAction(QIcon(":/trolltech/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), this);
#else
    aboutQtAction = new QAction(QIcon(":/qt-project.org/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), this);
#endif
    aboutQtAction->setStatusTip(tr("Show information about Qt"));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    optionsAction = new QAction(QIcon(":/icons/options"), tr("&Options..."), this);
    optionsAction->setStatusTip(tr("Modify configuration options for TESRA"));
    optionsAction->setMenuRole(QAction::PreferencesRole);
    toggleHideAction = new QAction(networkStyle->getAppIcon(), tr("&Show / Hide"), this);
    toggleHideAction->setStatusTip(tr("Show or hide the main Window"));

    encryptWalletAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Encrypt Wallet..."), this);
    encryptWalletAction->setStatusTip(tr("Encrypt the private keys that belong to your wallet"));
    encryptWalletAction->setCheckable(true);
    backupWalletAction = new QAction(QIcon(":/icons/filesave"), tr("&Backup Wallet..."), this);
    backupWalletAction->setStatusTip(tr("Backup wallet to another location"));
    changePassphraseAction = new QAction(QIcon(":/icons/key"), tr("&Change Passphrase..."), this);
    changePassphraseAction->setStatusTip(tr("Change the passphrase used for wallet encryption"));
    unlockWalletAction = new QAction(tr("&Unlock Wallet..."), this);
    unlockWalletAction->setToolTip(tr("Unlock wallet"));
    lockWalletAction = new QAction(tr("&Lock Wallet"), this);
    signMessageAction = new QAction(QIcon(":/icons/edit"), tr("Sign &message..."), this);
    signMessageAction->setStatusTip(tr("Sign messages with your TESRA addresses to prove you own them"));
    verifyMessageAction = new QAction(QIcon(":/icons/transaction_0"), tr("&Verify message..."), this);
    verifyMessageAction->setStatusTip(tr("Verify messages to ensure they were signed with specified TESRA addresses"));
    bip38ToolAction = new QAction(QIcon(":/icons/key"), tr("&BIP38 tool"), this);
    bip38ToolAction->setToolTip(tr("Encrypt and decrypt private keys using a passphrase"));
    multiSendAction = new QAction(QIcon(":/icons/edit"), tr("&MultiSend"), this);
    multiSendAction->setToolTip(tr("MultiSend Settings"));
    multiSendAction->setCheckable(true);

    openInfoAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("&Information"), this);
    openInfoAction->setStatusTip(tr("Show diagnostic information"));
    openRPCConsoleAction = new QAction(QIcon(":/icons/debugwindow"), tr("&Debug console"), this);
    openRPCConsoleAction->setStatusTip(tr("Open debugging console"));
    openNetworkAction = new QAction(QIcon(":/icons/connect_4"), tr("&Network Monitor"), this);
    openNetworkAction->setStatusTip(tr("Show network monitor"));
    openPeersAction = new QAction(QIcon(":/icons/connect_4"), tr("&Peers list"), this);
    openPeersAction->setStatusTip(tr("Show peers info"));
    openRepairAction = new QAction(QIcon(":/icons/options"), tr("Wallet &Repair"), this);
    openRepairAction->setStatusTip(tr("Show wallet repair options"));
    openConfEditorAction = new QAction(QIcon(":/icons/edit"), tr("Open Wallet &Configuration File"), this);
    openConfEditorAction->setStatusTip(tr("Open configuration file"));
    openMNConfEditorAction = new QAction(QIcon(":/icons/edit"), tr("Open &Masternode Configuration File"), this);
    openMNConfEditorAction->setStatusTip(tr("Open Masternode configuration file"));
    showBackupsAction = new QAction(QIcon(":/icons/browse"), tr("Show Automatic &Backups"), this);
    showBackupsAction->setStatusTip(tr("Show automatically created wallet backups"));

    usedSendingAddressesAction = new QAction(QIcon(":/icons/address-book"), tr("&Sending addresses..."), this);
    usedSendingAddressesAction->setStatusTip(tr("Show the list of used sending addresses and labels"));
    usedReceivingAddressesAction = new QAction(QIcon(":/icons/address-book"), tr("&Receiving addresses..."), this);
    usedReceivingAddressesAction->setStatusTip(tr("Show the list of used receiving addresses and labels"));

    multisigCreateAction = new QAction(QIcon(":/icons/address-book"), tr("&Multisignature creation..."), this);
    multisigCreateAction->setStatusTip(tr("Create a new multisignature address and add it to this wallet"));
    multisigSpendAction = new QAction(QIcon(":/icons/send"), tr("&Multisignature spending..."), this);
    multisigSpendAction->setStatusTip(tr("Spend from a multisignature address"));
    multisigSignAction = new QAction(QIcon(":/icons/editpaste"), tr("&Multisignature signing..."), this);
    multisigSignAction->setStatusTip(tr("Sign with a multisignature address"));

    openAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_FileIcon), tr("Open &URI..."), this);
    openAction->setStatusTip(tr("Open a TESRA: URI or payment request"));
    openBlockExplorerAction = new QAction(QIcon(":/icons/explorer"), tr("&Blockchain explorer"), this);
    openBlockExplorerAction->setStatusTip(tr("Block explorer window"));

    showHelpMessageAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("&Command-line options"), this);
    showHelpMessageAction->setMenuRole(QAction::NoRole);
    showHelpMessageAction->setStatusTip(tr("Show the TESRA Core help message to get a list with possible TESRA command-line options"));

    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutClicked()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(optionsAction, SIGNAL(triggered()), this, SLOT(optionsClicked()));
    connect(toggleHideAction, SIGNAL(triggered()), this, SLOT(toggleHidden()));
    connect(showHelpMessageAction, SIGNAL(triggered()), this, SLOT(showHelpMessageClicked()));
#ifdef ENABLE_WALLET
    if (walletFrame) {
        connect(encryptWalletAction, SIGNAL(triggered(bool)), walletFrame, SLOT(encryptWallet(bool)));
        connect(backupWalletAction, SIGNAL(triggered()), walletFrame, SLOT(backupWallet()));
        connect(changePassphraseAction, SIGNAL(triggered()), walletFrame, SLOT(changePassphrase()));
        connect(unlockWalletAction, SIGNAL(triggered()), walletFrame, SLOT(unlockWallet()));
        connect(lockWalletAction, SIGNAL(triggered()), walletFrame, SLOT(lockWallet()));
        connect(signMessageAction, SIGNAL(triggered()), this, SLOT(gotoSignMessageTab()));
        connect(verifyMessageAction, SIGNAL(triggered()), this, SLOT(gotoVerifyMessageTab()));
        connect(bip38ToolAction, SIGNAL(triggered()), this, SLOT(gotoBip38Tool()));
        connect(usedSendingAddressesAction, SIGNAL(triggered()), walletFrame, SLOT(usedSendingAddresses()));
        connect(usedReceivingAddressesAction, SIGNAL(triggered()), walletFrame, SLOT(usedReceivingAddresses()));
        connect(openAction, SIGNAL(triggered()), this, SLOT(openClicked()));
        connect(multiSendAction, SIGNAL(triggered()), this, SLOT(gotoMultiSendDialog()));
        connect(multisigCreateAction, SIGNAL(triggered()), this, SLOT(gotoMultisigCreate()));
        connect(multisigSpendAction, SIGNAL(triggered()), this, SLOT(gotoMultisigSpend()));
        connect(multisigSignAction, SIGNAL(triggered()), this, SLOT(gotoMultisigSign()));
    }
#endif 
}

void BitcoinGUI::createMenuBar()
{
#ifdef Q_OS_MAC
    
    appMenuBar = new QMenuBar();
#else
    
    appMenuBar = menuBar();
#endif

    
    QMenu* file = appMenuBar->addMenu(tr("&File"));
    if (walletFrame) {
        file->addAction(openAction);
        file->addAction(backupWalletAction);
        file->addAction(signMessageAction);
        file->addAction(verifyMessageAction);
        file->addSeparator();
        file->addAction(usedSendingAddressesAction);
        file->addAction(usedReceivingAddressesAction);
        file->addSeparator();
        file->addAction(multisigCreateAction);
        file->addAction(multisigSpendAction);
        file->addAction(multisigSignAction);
        file->addSeparator();
    }
    file->addAction(quitAction);

    QMenu* settings = appMenuBar->addMenu(tr("&Settings"));
    if (walletFrame) {
        settings->addAction(encryptWalletAction);
        settings->addAction(changePassphraseAction);
        settings->addAction(unlockWalletAction);
        settings->addAction(lockWalletAction);
        settings->addAction(bip38ToolAction);
        settings->addAction(multiSendAction);
        settings->addSeparator();
    }
    settings->addAction(optionsAction);

    if (walletFrame) {
        QMenu* tools = appMenuBar->addMenu(tr("&Tools"));
        tools->addAction(openInfoAction);
        tools->addAction(openRPCConsoleAction);
        tools->addAction(openNetworkAction);
        tools->addAction(openPeersAction);
        tools->addAction(openRepairAction);
        tools->addSeparator();
        tools->addAction(openConfEditorAction);
        tools->addAction(openMNConfEditorAction);
        tools->addAction(showBackupsAction);
        tools->addAction(openBlockExplorerAction);
    }

    QMenu* help = appMenuBar->addMenu(tr("&Help"));
    help->addAction(showHelpMessageAction);
    help->addSeparator();
    help->addAction(aboutAction);
    help->addAction(aboutQtAction);
}

void BitcoinGUI::createToolBars()
{
    if (walletFrame) {
        QToolBar* toolbar = new QToolBar(tr("Tabs toolbar"));
        toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolbar->addAction(overviewAction);
        toolbar->addAction(sendCoinsAction);
        toolbar->addAction(receiveCoinsAction);
        toolbar->addAction(privacyAction);
        toolbar->addAction(historyAction);
        toolbar->addAction(privacyAction);
        QSettings settings;
        if (settings.value("fShowMasternodesTab").toBool()) {
            toolbar->addAction(masternodeAction);
        }
        toolbar->addAction(createContractAction);

        toolbar->setMovable(false); 
        overviewAction->setChecked(true);

        /** Create additional container for toolbar and walletFrame and make it the central widget.
            This is a workaround mostly for toolbar styling on Mac OS but should work fine for every other OSes too.
        */
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(toolbar);
        layout->addWidget(walletFrame);
        layout->setSpacing(0);
        layout->setContentsMargins(QMargins());
        QWidget* containerWidget = new QWidget();
        containerWidget->setLayout(layout);
        setCentralWidget(containerWidget);
    }
}

void BitcoinGUI::setClientModel(ClientModel* clientModel)
{
    this->clientModel = clientModel;
    if (clientModel) {
        
        
        createTrayIconMenu();

        
        setNumConnections(clientModel->getNumConnections());
        connect(clientModel, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));

        setNumBlocks(clientModel->getNumBlocks());
        connect(clientModel, SIGNAL(numBlocksChanged(int)), this, SLOT(setNumBlocks(int)));

        
        connect(clientModel, SIGNAL(message(QString, QString, unsigned int)), this, SLOT(message(QString, QString, unsigned int)));

        
        connect(clientModel, SIGNAL(showProgress(QString, int)), this, SLOT(showProgress(QString, int)));

        rpcConsole->setClientModel(clientModel);
#ifdef ENABLE_WALLET
        if (walletFrame) {
            walletFrame->setClientModel(clientModel);
        }
#endif 
        unitDisplayControl->setOptionsModel(clientModel->getOptionsModel());

        
        if (trayIcon)
        {
          trayIcon->show();
        }
    } else {
        
        toggleHideAction->setEnabled(false);
        if (trayIconMenu) {
            
            trayIconMenu->clear();
        }
    }
}

#ifdef ENABLE_WALLET
bool BitcoinGUI::addWallet(const QString& name, WalletModel* walletModel)
{
    if (!walletFrame)
        return false;
    setWalletActionsEnabled(true);
    return walletFrame->addWallet(name, walletModel);
}

bool BitcoinGUI::setCurrentWallet(const QString& name)
{
    if (!walletFrame)
        return false;
    return walletFrame->setCurrentWallet(name);
}

void BitcoinGUI::removeAllWallets()
{
    if (!walletFrame)
        return;
    setWalletActionsEnabled(false);
    walletFrame->removeAllWallets();
}
#endif 

void BitcoinGUI::setWalletActionsEnabled(bool enabled)
{
    overviewAction->setEnabled(enabled);
    sendCoinsAction->setEnabled(enabled);
    receiveCoinsAction->setEnabled(enabled);
    privacyAction->setEnabled(enabled);
    historyAction->setEnabled(enabled);
    QSettings settings;
    if (settings.value("fShowMasternodesTab").toBool()) {
        masternodeAction->setEnabled(enabled);
    }
    encryptWalletAction->setEnabled(enabled);
    backupWalletAction->setEnabled(enabled);
    changePassphraseAction->setEnabled(enabled);
    signMessageAction->setEnabled(enabled);
    verifyMessageAction->setEnabled(enabled);
    multisigCreateAction->setEnabled(enabled);
    multisigSpendAction->setEnabled(enabled);
    multisigSignAction->setEnabled(enabled);
    bip38ToolAction->setEnabled(enabled);
    usedSendingAddressesAction->setEnabled(enabled);
    usedReceivingAddressesAction->setEnabled(enabled);
    openAction->setEnabled(enabled);
}

void BitcoinGUI::createTrayIcon(const NetworkStyle* networkStyle)
{
#ifndef Q_OS_MAC
    trayIcon = new QSystemTrayIcon(this);
    QString toolTip = tr("TESRA Core client") + " " + networkStyle->getTitleAddText();
    trayIcon->setToolTip(toolTip);
    trayIcon->setIcon(networkStyle->getAppIcon());
    trayIcon->hide();
#endif

    notificator = new Notificator(QApplication::applicationName(), trayIcon, this);
}

void BitcoinGUI::createTrayIconMenu()
{
#ifndef Q_OS_MAC
    
    if (!trayIcon)
        return;

    trayIconMenu = new QMenu(this);
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
        this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
#else
    
    MacDockIconHandler* dockIconHandler = MacDockIconHandler::instance();
    dockIconHandler->setMainWindow((QMainWindow*)this);
    trayIconMenu = dockIconHandler->dockMenu();
#endif

    
    trayIconMenu->addAction(toggleHideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(sendCoinsAction);
    trayIconMenu->addAction(receiveCoinsAction);
    trayIconMenu->addAction(privacyAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(signMessageAction);
    trayIconMenu->addAction(verifyMessageAction);
    trayIconMenu->addAction(bip38ToolAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(optionsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(openInfoAction);
    trayIconMenu->addAction(openRPCConsoleAction);
    trayIconMenu->addAction(openNetworkAction);
    trayIconMenu->addAction(openPeersAction);
    trayIconMenu->addAction(openRepairAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(openConfEditorAction);
    trayIconMenu->addAction(openMNConfEditorAction);
    trayIconMenu->addAction(showBackupsAction);
    trayIconMenu->addAction(openBlockExplorerAction);
#ifndef Q_OS_MAC 
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
#endif
}

#ifndef Q_OS_MAC
void BitcoinGUI::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        
        toggleHidden();
    }
}
#endif

void BitcoinGUI::optionsClicked()
{
    if (!clientModel || !clientModel->getOptionsModel())
        return;

    OptionsDialog dlg(this, enableWallet);
    dlg.setModel(clientModel->getOptionsModel());
    dlg.exec();
}

void BitcoinGUI::aboutClicked()
{
    if (!clientModel)
        return;

    HelpMessageDialog dlg(this, true);
    dlg.exec();
}

void BitcoinGUI::showHelpMessageClicked()
{
    HelpMessageDialog* help = new HelpMessageDialog(this, false);
    help->setAttribute(Qt::WA_DeleteOnClose);
    help->show();
}

#ifdef ENABLE_WALLET
void BitcoinGUI::openClicked()
{
    OpenURIDialog dlg(this);
    if (dlg.exec()) {
        emit receivedURI(dlg.getURI());
    }
}

void BitcoinGUI::gotoOverviewPage()
{
    overviewAction->setChecked(true);
    if (walletFrame) walletFrame->gotoOverviewPage();
}

void BitcoinGUI::gotoCreateContractPage()
{
    createContractAction->setChecked(true);
    if (walletFrame) walletFrame->gotoCreateContractPage();


}

void BitcoinGUI::gotoHistoryPage()
{
    historyAction->setChecked(true);
    if (walletFrame) walletFrame->gotoHistoryPage();
}

void BitcoinGUI::gotoMasternodePage()
{
    QSettings settings;
    if (settings.value("fShowMasternodesTab").toBool()) {
        masternodeAction->setChecked(true);
        if (walletFrame) walletFrame->gotoMasternodePage();
    }
}

void BitcoinGUI::gotoReceiveCoinsPage()
{
    receiveCoinsAction->setChecked(true);
    if (walletFrame) walletFrame->gotoReceiveCoinsPage();
}

void BitcoinGUI::gotoPrivacyPage()
{
    privacyAction->setChecked(true);
    if (walletFrame) walletFrame->gotoPrivacyPage();
}

void BitcoinGUI::gotoSendCoinsPage(QString addr)
{
    sendCoinsAction->setChecked(true);
    if (walletFrame) walletFrame->gotoSendCoinsPage(addr);
}

void BitcoinGUI::gotoSignMessageTab(QString addr)
{
    if (walletFrame) walletFrame->gotoSignMessageTab(addr);
}

void BitcoinGUI::gotoVerifyMessageTab(QString addr)
{
    if (walletFrame) walletFrame->gotoVerifyMessageTab(addr);
}

void BitcoinGUI::gotoMultisigCreate()
{
    if(walletFrame) walletFrame->gotoMultisigDialog(0);
}

void BitcoinGUI::gotoMultisigSpend()
{
    if(walletFrame) walletFrame->gotoMultisigDialog(1);
}

void BitcoinGUI::gotoMultisigSign()
{
    if(walletFrame) walletFrame->gotoMultisigDialog(2);
}

void BitcoinGUI::gotoBip38Tool()
{
    if (walletFrame) walletFrame->gotoBip38Tool();
}

void BitcoinGUI::gotoMultiSendDialog()
{
    multiSendAction->setChecked(true);
    if (walletFrame)
        walletFrame->gotoMultiSendDialog();
}
void BitcoinGUI::gotoBlockExplorerPage()
{
    if (walletFrame) walletFrame->gotoBlockExplorerPage();
}

#endif 

void BitcoinGUI::setNumConnections(int count)
{
    QString icon;
    switch (count) {
    case 0:
        icon = ":/icons/connect_0";
        break;
    case 1:
    case 2:
    case 3:
        icon = ":/icons/connect_1";
        break;
    case 4:
    case 5:
    case 6:
        icon = ":/icons/connect_2";
        break;
    case 7:
    case 8:
    case 9:
        icon = ":/icons/connect_3";
        break;
    default:
        icon = ":/icons/connect_4";
        break;
    }
    QIcon connectionItem = QIcon(icon).pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE);
    labelConnectionsIcon->setIcon(connectionItem);
    labelConnectionsIcon->setToolTip(tr("%n active connection(s) to TESRA network", "", count));
}

void BitcoinGUI::setNumBlocks(int count)
{
    if (!clientModel)
        return;

    
    statusBar()->clearMessage();

    
    enum BlockSource blockSource = clientModel->getBlockSource();
    switch (blockSource) {
    case BLOCK_SOURCE_NETWORK:
        progressBarLabel->setText(tr("Synchronizing with network..."));
        break;
    case BLOCK_SOURCE_DISK:
        progressBarLabel->setText(tr("Importing blocks from disk..."));
        break;
    case BLOCK_SOURCE_REINDEX:
        progressBarLabel->setText(tr("Reindexing blocks on disk..."));
        break;
    case BLOCK_SOURCE_NONE:
        
        progressBarLabel->setText(tr("No block source available..."));
        break;
    }

    QString tooltip;

    QDateTime lastBlockDate = clientModel->getLastBlockDate();
    QDateTime currentDate = QDateTime::currentDateTime();
    int secs = lastBlockDate.secsTo(currentDate);

    tooltip = tr("Processed %n blocks of transaction history.", "", count);

    
    
    if (masternodeSync.IsBlockchainSynced()) {
        QString strSyncStatus;
        tooltip = tr("Up to date") + QString(".<br>") + tooltip;

        if (masternodeSync.IsSynced()) {
            progressBarLabel->setVisible(false);
            progressBar->setVisible(false);
            labelBlocksIcon->setPixmap(QIcon(":/icons/synced").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
        } else {
            int nAttempt;
            int progress = 0;

            labelBlocksIcon->setPixmap(QIcon(QString(
                                                 ":/movies/spinner-%1")
                                                 .arg(spinnerFrame, 3, 10, QChar('0')))
                                           .pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
            spinnerFrame = (spinnerFrame + 1) % SPINNER_FRAMES;

#ifdef ENABLE_WALLET
            if (walletFrame)
                walletFrame->showOutOfSyncWarning(false);
#endif 

            nAttempt = masternodeSync.RequestedMasternodeAttempt < MASTERNODE_SYNC_THRESHOLD ?
                           masternodeSync.RequestedMasternodeAttempt + 1 :
                           MASTERNODE_SYNC_THRESHOLD;
            progress = nAttempt + (masternodeSync.RequestedMasternodeAssets - 1) * MASTERNODE_SYNC_THRESHOLD;
            progressBar->setMaximum(4 * MASTERNODE_SYNC_THRESHOLD);
            progressBar->setFormat(tr("Synchronizing additional data: %p%"));
            progressBar->setValue(progress);
        }

        strSyncStatus = QString(masternodeSync.GetSyncStatus().c_str());
        progressBarLabel->setText(strSyncStatus);
        tooltip = strSyncStatus + QString("<br>") + tooltip;
    } else {
        
        QString timeBehindText;
        const int HOUR_IN_SECONDS = 60 * 60;
        const int DAY_IN_SECONDS = 24 * 60 * 60;
        const int WEEK_IN_SECONDS = 7 * 24 * 60 * 60;
        const int YEAR_IN_SECONDS = 31556952; 
        if (secs < 2 * DAY_IN_SECONDS) {
            timeBehindText = tr("%n hour(s)", "", secs / HOUR_IN_SECONDS);
        } else if (secs < 2 * WEEK_IN_SECONDS) {
            timeBehindText = tr("%n day(s)", "", secs / DAY_IN_SECONDS);
        } else if (secs < YEAR_IN_SECONDS) {
            timeBehindText = tr("%n week(s)", "", secs / WEEK_IN_SECONDS);
        } else {
            int years = secs / YEAR_IN_SECONDS;
            int remainder = secs % YEAR_IN_SECONDS;
            timeBehindText = tr("%1 and %2").arg(tr("%n year(s)", "", years)).arg(tr("%n week(s)", "", remainder / WEEK_IN_SECONDS));
        }

        progressBarLabel->setVisible(true);
        progressBar->setFormat(tr("%1 behind. Scanning block %2").arg(timeBehindText).arg(count));
        progressBar->setMaximum(1000000000);
        progressBar->setValue(clientModel->getVerificationProgress() * 1000000000.0 + 0.5);
        progressBar->setVisible(true);

        tooltip = tr("Catching up...") + QString("<br>") + tooltip;
        if (count != prevBlocks) {
            labelBlocksIcon->setPixmap(QIcon(QString(
                                                 ":/movies/spinner-%1")
                                                 .arg(spinnerFrame, 3, 10, QChar('0')))
                                           .pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
            spinnerFrame = (spinnerFrame + 1) % SPINNER_FRAMES;
        }
        prevBlocks = count;

#ifdef ENABLE_WALLET
        if (walletFrame)
            walletFrame->showOutOfSyncWarning(true);
#endif 

        tooltip += QString("<br>");
        tooltip += tr("Last received block was generated %1 ago.").arg(timeBehindText);
        tooltip += QString("<br>");
        tooltip += tr("Transactions after this will not yet be visible.");
    }

    
    tooltip = QString("<nobr>") + tooltip + QString("</nobr>");

    labelBlocksIcon->setToolTip(tooltip);
    progressBarLabel->setToolTip(tooltip);
    progressBar->setToolTip(tooltip);
}

void BitcoinGUI::message(const QString& title, const QString& message, unsigned int style, bool* ret)
{
    QString strTitle = tr("TESRA Core"); 
    
    int nMBoxIcon = QMessageBox::Information;
    int nNotifyIcon = Notificator::Information;

    QString msgType;

    
    if (!title.isEmpty()) {
        msgType = title;
    } else {
        switch (style) {
        case CClientUIInterface::MSG_ERROR:
            msgType = tr("Error");
            break;
        case CClientUIInterface::MSG_WARNING:
            msgType = tr("Warning");
            break;
        case CClientUIInterface::MSG_INFORMATION:
            msgType = tr("Information");
            break;
        default:
            break;
        }
    }
    
    if (!msgType.isEmpty())
        strTitle += " - " + msgType;

    
    if (style & CClientUIInterface::ICON_ERROR) {
        nMBoxIcon = QMessageBox::Critical;
        nNotifyIcon = Notificator::Critical;
    } else if (style & CClientUIInterface::ICON_WARNING) {
        nMBoxIcon = QMessageBox::Warning;
        nNotifyIcon = Notificator::Warning;
    }

    
    if (style & CClientUIInterface::MODAL) {
        
        QMessageBox::StandardButton buttons;
        if (!(buttons = (QMessageBox::StandardButton)(style & CClientUIInterface::BTN_MASK)))
            buttons = QMessageBox::Ok;

        showNormalIfMinimized();
        QMessageBox mBox((QMessageBox::Icon)nMBoxIcon, strTitle, message, buttons, this);
        int r = mBox.exec();
        if (ret != NULL)
            *ret = r == QMessageBox::Ok;
    } else
        notificator->notify((Notificator::Class)nNotifyIcon, strTitle, message);
}

void BitcoinGUI::changeEvent(QEvent* e)
{
    QMainWindow::changeEvent(e);
#ifndef Q_OS_MAC 
    if (e->type() == QEvent::WindowStateChange) {
        if (clientModel && clientModel->getOptionsModel() && clientModel->getOptionsModel()->getMinimizeToTray()) {
            QWindowStateChangeEvent* wsevt = static_cast<QWindowStateChangeEvent*>(e);
            if (!(wsevt->oldState() & Qt::WindowMinimized) && isMinimized()) {
                QTimer::singleShot(0, this, SLOT(hide()));
                e->ignore();
            }
        }
    }
#endif
}

void BitcoinGUI::closeEvent(QCloseEvent* event)
{
#ifndef Q_OS_MAC 
    if (clientModel && clientModel->getOptionsModel()) {
        if (!clientModel->getOptionsModel()->getMinimizeOnClose()) {
            QApplication::quit();
        }
    }
#endif
    QMainWindow::closeEvent(event);
}

#ifdef ENABLE_WALLET
void BitcoinGUI::incomingTransaction(const QString& date, int unit, const CAmount& amount, const QString& type, const QString& address)
{
    
    if(!bdisableSystemnotifications){
        
        message((amount) < 0 ? (pwalletMain->fMultiSendNotify == true ? tr("Sent MultiSend transaction") : tr("Sent transaction")) : tr("Incoming transaction"),
            tr("Date: %1\n"
               "Amount: %2\n"
               "Type: %3\n"
               "Address: %4\n")
                .arg(date)
                .arg(BitcoinUnits::formatWithUnit(unit, amount, true))
                .arg(type)
                .arg(address),
            CClientUIInterface::MSG_INFORMATION);

        pwalletMain->fMultiSendNotify = false;
    }
}
#endif 

void BitcoinGUI::dragEnterEvent(QDragEnterEvent* event)
{
    
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void BitcoinGUI::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        foreach (const QUrl& uri, event->mimeData()->urls()) {
            emit receivedURI(uri.toString());
        }
    }
    event->acceptProposedAction();
}

bool BitcoinGUI::eventFilter(QObject* object, QEvent* event)
{
    
    if (event->type() == QEvent::StatusTip) {
        
        if (progressBarLabel->isVisible() || progressBar->isVisible())
            return true;
    }
    return QMainWindow::eventFilter(object, event);
}

void BitcoinGUI::setStakingStatus()
{
    if (pwalletMain)
        fMultiSend = pwalletMain->isMultiSendEnabled();

    if (nLastCoinStakeSearchInterval) {
        labelStakingIcon->show();
        labelStakingIcon->setPixmap(QIcon(":/icons/staking_active").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
        labelStakingIcon->setToolTip(tr("Staking is active\n MultiSend: %1").arg(fMultiSend ? tr("Active") : tr("Not Active")));
    } else {
        labelStakingIcon->show();
        labelStakingIcon->setPixmap(QIcon(":/icons/staking_inactive").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
        labelStakingIcon->setToolTip(tr("Staking is not active\n MultiSend: %1").arg(fMultiSend ? tr("Active") : tr("Not Active")));
    }
}

#ifdef ENABLE_WALLET
bool BitcoinGUI::handlePaymentRequest(const SendCoinsRecipient& recipient)
{
    
    if (walletFrame && walletFrame->handlePaymentRequest(recipient)) {
        showNormalIfMinimized();
        gotoSendCoinsPage();
        return true;
    }
    return false;
}

void BitcoinGUI::setEncryptionStatus(int status)
{
    switch (status) {
    case WalletModel::Unencrypted:
        labelEncryptionIcon->hide();
        encryptWalletAction->setChecked(false);
        changePassphraseAction->setEnabled(false);
        unlockWalletAction->setVisible(false);
        lockWalletAction->setVisible(false);
        encryptWalletAction->setEnabled(true);
        break;
    case WalletModel::Unlocked:
        labelEncryptionIcon->show();
        labelEncryptionIcon->setIcon(QIcon(":/icons/lock_open").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>unlocked</b>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setVisible(false);
        lockWalletAction->setVisible(true);
        encryptWalletAction->setEnabled(false); 
        break;
    case WalletModel::UnlockedForAnonymizationOnly:
        labelEncryptionIcon->show();
        labelEncryptionIcon->setIcon(QIcon(":/icons/lock_open").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>unlocked</b> for anonimization and staking only"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setVisible(true);
        lockWalletAction->setVisible(true);
        encryptWalletAction->setEnabled(false); 
        break;
    case WalletModel::Locked:
        labelEncryptionIcon->show();
        labelEncryptionIcon->setIcon(QIcon(":/icons/lock_closed").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>locked</b>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setVisible(true);
        lockWalletAction->setVisible(false);
        encryptWalletAction->setEnabled(false); 
        break;
    }
}
#endif 

void BitcoinGUI::showNormalIfMinimized(bool fToggleHidden)
{
    if (!clientModel)
        return;

    
    if (isHidden()) {
        show();
        activateWindow();
    } else if (isMinimized()) {
        showNormal();
        activateWindow();
    } else if (GUIUtil::isObscured(this)) {
        raise();
        activateWindow();
    } else if (fToggleHidden)
        hide();
}

void BitcoinGUI::toggleHidden()
{
    showNormalIfMinimized(true);
}

void BitcoinGUI::detectShutdown()
{
    if (ShutdownRequested()) {
        if (rpcConsole)
            rpcConsole->hide();
        qApp->quit();
    }
}

void BitcoinGUI::showProgress(const QString& title, int nProgress)
{
    if (nProgress == 0) {
        progressDialog = new QProgressDialog(title, "", 0, 100);
        progressDialog->setWindowModality(Qt::ApplicationModal);
        progressDialog->setMinimumDuration(0);
        progressDialog->setCancelButton(0);
        progressDialog->setAutoClose(false);
        progressDialog->setValue(0);
    } else if (nProgress == 100) {
        if (progressDialog) {
            progressDialog->close();
            progressDialog->deleteLater();
        }
    } else if (progressDialog)
        progressDialog->setValue(nProgress);
}

static bool ThreadSafeMessageBox(BitcoinGUI* gui, const std::string& message, const std::string& caption, unsigned int style)
{
    bool modal = (style & CClientUIInterface::MODAL);
    
    
    style &= ~CClientUIInterface::SECURE;
    bool ret = false;
    
    QMetaObject::invokeMethod(gui, "message",
        modal ? GUIUtil::blockingGUIThreadConnection() : Qt::QueuedConnection,
        Q_ARG(QString, QString::fromStdString(caption)),
        Q_ARG(QString, QString::fromStdString(message)),
        Q_ARG(unsigned int, style),
        Q_ARG(bool*, &ret));
    return ret;
}

void BitcoinGUI::subscribeToCoreSignals()
{
    
    uiInterface.ThreadSafeMessageBox.connect(boost::bind(ThreadSafeMessageBox, this, _1, _2, _3));
}

void BitcoinGUI::unsubscribeFromCoreSignals()
{
    
    uiInterface.ThreadSafeMessageBox.disconnect(boost::bind(ThreadSafeMessageBox, this, _1, _2, _3));
}


void BitcoinGUI::handleRestart(QStringList args)
{
    if (!ShutdownRequested())
        emit requestedRestart(args);
}

UnitDisplayStatusBarControl::UnitDisplayStatusBarControl() : optionsModel(0),
                                                             menu(0)
{
    createContextMenu();
    setToolTip(tr("Unit to show amounts in. Click to select another unit."));
}


void UnitDisplayStatusBarControl::mousePressEvent(QMouseEvent* event)
{
    onDisplayUnitsClicked(event->pos());
}


void UnitDisplayStatusBarControl::createContextMenu()
{
    menu = new QMenu();
    foreach (BitcoinUnits::Unit u, BitcoinUnits::availableUnits()) {
        QAction* menuAction = new QAction(QString(BitcoinUnits::name(u)), this);
        menuAction->setData(QVariant(u));
        menu->addAction(menuAction);
    }
    connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(onMenuSelection(QAction*)));
}


void UnitDisplayStatusBarControl::setOptionsModel(OptionsModel* optionsModel)
{
    if (optionsModel) {
        this->optionsModel = optionsModel;

        
        connect(optionsModel, SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit(int)));

        
        updateDisplayUnit(optionsModel->getDisplayUnit());
    }
}


void UnitDisplayStatusBarControl::updateDisplayUnit(int newUnits)
{
    if (Params().NetworkID() == CBaseChainParams::MAIN) {
        setPixmap(QIcon(":/icons/unit_" + BitcoinUnits::id(newUnits)).pixmap(39, STATUSBAR_ICONSIZE));
    } else {
        setPixmap(QIcon(":/icons/unit_t" + BitcoinUnits::id(newUnits)).pixmap(39, STATUSBAR_ICONSIZE));
    }
}


void UnitDisplayStatusBarControl::onDisplayUnitsClicked(const QPoint& point)
{
    QPoint globalPos = mapToGlobal(point);
    menu->exec(globalPos);
}


void UnitDisplayStatusBarControl::onMenuSelection(QAction* action)
{
    if (action) {
        optionsModel->setDisplayUnit(action->data());
    }
}
