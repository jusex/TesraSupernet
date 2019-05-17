



#include "walletview.h"

#include "addressbookpage.h"
#include "askpassphrasedialog.h"
#include "bip38tooldialog.h"
#include "bitcoingui.h"
#include "blockexplorer.h"
#include "clientmodel.h"
#include "guiutil.h"
#include "masternodeconfig.h"
#include "multisenddialog.h"
#include "multisigdialog.h"
#include "optionsmodel.h"
#include "overviewpage.h"
#include "createcontractpage.h"
#include "receivecoinsdialog.h"
#include "privacydialog.h"
#include "sendcoinsdialog.h"
#include "signverifymessagedialog.h"
#include "transactiontablemodel.h"
#include "transactionview.h"
#include "walletmodel.h"

#include "ui_interface.h"

#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressDialog>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

WalletView::WalletView(QWidget* parent) : QStackedWidget(parent),
                                          clientModel(0),
                                          walletModel(0)
{
    
    overviewPage = new OverviewPage();
    createContractPage = new CreateContractPage();
    explorerWindow = new BlockExplorer(this);
    transactionsPage = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout();
    QHBoxLayout* hbox_buttons = new QHBoxLayout();
    transactionView = new TransactionView(this);
    vbox->addWidget(transactionView);
    QPushButton* exportButton = new QPushButton(tr("&Export"), this);
    exportButton->setToolTip(tr("Export the data in the current tab to a file"));
#ifndef Q_OS_MAC 
    exportButton->setIcon(QIcon(":/icons/export"));
#endif
    hbox_buttons->addStretch();

    
    QLabel* transactionSumLabel = new QLabel();                
    transactionSumLabel->setObjectName("transactionSumLabel"); 
    transactionSumLabel->setText(tr("Selected amount:"));
    hbox_buttons->addWidget(transactionSumLabel);

    transactionSum = new QLabel();                   
    transactionSum->setObjectName("transactionSum"); 
    transactionSum->setMinimumSize(200, 8);
    transactionSum->setTextInteractionFlags(Qt::TextSelectableByMouse);
    hbox_buttons->addWidget(transactionSum);

    hbox_buttons->addWidget(exportButton);
    vbox->addLayout(hbox_buttons);
    transactionsPage->setLayout(vbox);

    privacyPage = new PrivacyDialog();
    receiveCoinsPage = new ReceiveCoinsDialog();
    sendCoinsPage = new SendCoinsDialog();

    addWidget(overviewPage);
    addWidget(transactionsPage);
    addWidget(privacyPage);
    addWidget(receiveCoinsPage);
    addWidget(sendCoinsPage);
    addWidget(explorerWindow);
    addWidget(createContractPage);

    QSettings settings;
    if (settings.value("fShowMasternodesTab").toBool()) {
        masternodeListPage = new MasternodeList();
        addWidget(masternodeListPage);
    }

    
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), transactionView, SLOT(focusTransaction(QModelIndex)));

    
    connect(transactionView, SIGNAL(doubleClicked(QModelIndex)), transactionView, SLOT(showDetails()));

    
    connect(transactionView, SIGNAL(trxAmount(QString)), this, SLOT(trxAmount(QString)));

    
    connect(exportButton, SIGNAL(clicked()), transactionView, SLOT(exportClicked()));

    
    connect(sendCoinsPage, SIGNAL(message(QString, QString, unsigned int)), this, SIGNAL(message(QString, QString, unsigned int)));

    
    connect(transactionView, SIGNAL(message(QString, QString, unsigned int)), this, SIGNAL(message(QString, QString, unsigned int)));
}

WalletView::~WalletView()
{
}

void WalletView::setBitcoinGUI(BitcoinGUI* gui)
{
    if (gui) {
        
        connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), gui, SLOT(gotoHistoryPage()));

        
        connect(this, SIGNAL(message(QString, QString, unsigned int)), gui, SLOT(message(QString, QString, unsigned int)));

        
        connect(this, SIGNAL(encryptionStatusChanged(int)), gui, SLOT(setEncryptionStatus(int)));

        
        connect(this, SIGNAL(incomingTransaction(QString, int, CAmount, QString, QString)), gui, SLOT(incomingTransaction(QString, int, CAmount, QString, QString)));
    }
}

void WalletView::setClientModel(ClientModel* clientModel)
{
    this->clientModel = clientModel;

    overviewPage->setClientModel(clientModel);
    sendCoinsPage->setClientModel(clientModel);
    QSettings settings;
    if (settings.value("fShowMasternodesTab").toBool()) {
        masternodeListPage->setClientModel(clientModel);
    }
}

void WalletView::setWalletModel(WalletModel* walletModel)
{
    this->walletModel = walletModel;

    
    transactionView->setModel(walletModel);
    overviewPage->setWalletModel(walletModel);
    QSettings settings;
    if (settings.value("fShowMasternodesTab").toBool()) {
        masternodeListPage->setWalletModel(walletModel);
    }
    privacyPage->setModel(walletModel);
    receiveCoinsPage->setModel(walletModel);
    sendCoinsPage->setModel(walletModel);
    createContractPage->setModel(walletModel);

    if (walletModel) {
        
        connect(walletModel, SIGNAL(message(QString, QString, unsigned int)), this, SIGNAL(message(QString, QString, unsigned int)));

        
        connect(walletModel, SIGNAL(encryptionStatusChanged(int)), this, SIGNAL(encryptionStatusChanged(int)));
        updateEncryptionStatus();

        
        connect(walletModel->getTransactionTableModel(), SIGNAL(rowsInserted(QModelIndex, int, int)),
            this, SLOT(processNewTransaction(QModelIndex, int, int)));

        
        connect(walletModel, SIGNAL(requireUnlock()), this, SLOT(unlockWallet()));

        
        connect(walletModel, SIGNAL(showProgress(QString, int)), this, SLOT(showProgress(QString, int)));
    }
}

void WalletView::processNewTransaction(const QModelIndex& parent, int start, int )
{
    
    if (!walletModel || !clientModel || clientModel->inInitialBlockDownload())
        return;

    TransactionTableModel* ttm = walletModel->getTransactionTableModel();
    if (!ttm || ttm->processingQueuedTransactions())
        return;

    QString date = ttm->index(start, TransactionTableModel::Date, parent).data().toString();
    qint64 amount = ttm->index(start, TransactionTableModel::Amount, parent).data(Qt::EditRole).toULongLong();
    QString type = ttm->index(start, TransactionTableModel::Type, parent).data().toString();
    QString address = ttm->index(start, TransactionTableModel::ToAddress, parent).data().toString();

    emit incomingTransaction(date, walletModel->getOptionsModel()->getDisplayUnit(), amount, type, address);
}

void WalletView::gotoOverviewPage()
{
    setCurrentWidget(overviewPage);
    
    walletModel->emitBalanceChanged();
}


void WalletView::gotoCreateContractPage()
{
    setCurrentWidget(createContractPage);
}

void WalletView::gotoHistoryPage()
{
    setCurrentWidget(transactionsPage);
}


void WalletView::gotoBlockExplorerPage()
{
    setCurrentWidget(explorerWindow);
}

void WalletView::gotoMasternodePage()
{
    QSettings settings;
    if (settings.value("fShowMasternodesTab").toBool()) {
        setCurrentWidget(masternodeListPage);
    }
}

void WalletView::gotoReceiveCoinsPage()
{
    setCurrentWidget(receiveCoinsPage);
}

void WalletView::gotoPrivacyPage()
{
    setCurrentWidget(privacyPage);
    
    walletModel->emitBalanceChanged();
}

void WalletView::gotoSendCoinsPage(QString addr)
{
    setCurrentWidget(sendCoinsPage);

    if (!addr.isEmpty())
        sendCoinsPage->setAddress(addr);
}

void WalletView::gotoSignMessageTab(QString addr)
{
    
    SignVerifyMessageDialog* signVerifyMessageDialog = new SignVerifyMessageDialog(this);
    signVerifyMessageDialog->setAttribute(Qt::WA_DeleteOnClose);
    signVerifyMessageDialog->setModel(walletModel);
    signVerifyMessageDialog->showTab_SM(true);

    if (!addr.isEmpty())
        signVerifyMessageDialog->setAddress_SM(addr);
}

void WalletView::gotoVerifyMessageTab(QString addr)
{
    
    SignVerifyMessageDialog* signVerifyMessageDialog = new SignVerifyMessageDialog(this);
    signVerifyMessageDialog->setAttribute(Qt::WA_DeleteOnClose);
    signVerifyMessageDialog->setModel(walletModel);
    signVerifyMessageDialog->showTab_VM(true);

    if (!addr.isEmpty())
        signVerifyMessageDialog->setAddress_VM(addr);
}

void WalletView::gotoBip38Tool()
{
    Bip38ToolDialog* bip38ToolDialog = new Bip38ToolDialog(this);
    
    bip38ToolDialog->setModel(walletModel);
    bip38ToolDialog->showTab_ENC(true);
}

void WalletView::gotoMultiSendDialog()
{
    MultiSendDialog* multiSendDialog = new MultiSendDialog(this);
    multiSendDialog->setModel(walletModel);
    multiSendDialog->show();
}

void WalletView::gotoMultisigDialog(int index)
{
    MultisigDialog* multisig = new MultisigDialog(this);
    multisig->setModel(walletModel);
    multisig->showTab(index);
}

bool WalletView::handlePaymentRequest(const SendCoinsRecipient& recipient)
{
    return sendCoinsPage->handlePaymentRequest(recipient);
}

void WalletView::showOutOfSyncWarning(bool fShow)
{
    overviewPage->showOutOfSyncWarning(fShow);
    privacyPage->showOutOfSyncWarning(fShow);
}

void WalletView::updateEncryptionStatus()
{
    emit encryptionStatusChanged(walletModel->getEncryptionStatus());
}

void WalletView::encryptWallet(bool status)
{
    if (!walletModel)
        return;
    AskPassphraseDialog dlg(status ? AskPassphraseDialog::Encrypt : AskPassphraseDialog::Decrypt, this, walletModel);
    dlg.exec();

    updateEncryptionStatus();
}

void WalletView::backupWallet()
{
    QString filename = GUIUtil::getSaveFileName(this,
        tr("Backup Wallet"), QString(),
        tr("Wallet Data (*.dat)"), NULL);

    if (filename.isEmpty())
        return;

    if (!walletModel->backupWallet(filename)) {
        emit message(tr("Backup Failed"), tr("There was an error trying to save the wallet data to %1.").arg(filename),
            CClientUIInterface::MSG_ERROR);
    } else {
        emit message(tr("Backup Successful"), tr("The wallet data was successfully saved to %1.").arg(filename),
            CClientUIInterface::MSG_INFORMATION);
    }
}

void WalletView::changePassphrase()
{
    AskPassphraseDialog dlg(AskPassphraseDialog::ChangePass, this, walletModel);
    dlg.exec();
}

void WalletView::unlockWallet()
{
    if (!walletModel)
        return;
    

    if (walletModel->getEncryptionStatus() == WalletModel::Locked || walletModel->getEncryptionStatus() == WalletModel::UnlockedForAnonymizationOnly) {
        AskPassphraseDialog dlg(AskPassphraseDialog::UnlockAnonymize, this, walletModel);
        dlg.exec();
    }
}

void WalletView::lockWallet()
{
    if (!walletModel)
        return;

    walletModel->setWalletLocked(true);
}

void WalletView::toggleLockWallet()
{
    if (!walletModel)
        return;

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();

    
    if (encStatus == walletModel->Locked) {
        AskPassphraseDialog dlg(AskPassphraseDialog::UnlockAnonymize, this, walletModel);
        dlg.exec();
    }

    else if (encStatus == walletModel->Unlocked || encStatus == walletModel->UnlockedForAnonymizationOnly) {
            walletModel->setWalletLocked(true);
    }
}

void WalletView::usedSendingAddresses()
{
    if (!walletModel)
        return;
    AddressBookPage* dlg = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::SendingTab, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModel(walletModel->getAddressTableModel());
    dlg->show();
}

void WalletView::usedReceivingAddresses()
{
    if (!walletModel)
        return;
    AddressBookPage* dlg = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::ReceivingTab, this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModel(walletModel->getAddressTableModel());
    dlg->show();
}

void WalletView::showProgress(const QString& title, int nProgress)
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


void WalletView::trxAmount(QString amount)
{
    transactionSum->setText(amount);
}
