



#include "privacydialog.h"
#include "ui_privacydialog.h"

#include "addressbookpage.h"
#include "addresstablemodel.h"
#include "bitcoinunits.h"
#include "coincontroldialog.h"
#include "libzerocoin/Denominations.h"
#include "optionsmodel.h"
#include "sendcoinsentry.h"
#include "walletmodel.h"
#include "coincontrol.h"
#include "zulocontroldialog.h"
#include "spork.h"

#include <QClipboard>
#include <QSettings>
#include <utilmoneystr.h>
#include <QtWidgets>

PrivacyDialog::PrivacyDialog(QWidget* parent) : QDialog(parent),
                                                          ui(new Ui::PrivacyDialog),
                                                          walletModel(0),
                                                          currentBalance(-1)
{
    nDisplayUnit = 0; 
    ui->setupUi(this);

    
    ui->zULOpayAmount->setValidator( new QDoubleValidator(0.0, 21000000.0, 20, this) );
    ui->labelMintAmountValue->setValidator( new QIntValidator(0, 999999, this) );

    
    ui->labelCoinControlQuantity->setText (tr("Coins automatically selected"));
    ui->labelCoinControlAmount->setText (tr("Coins automatically selected"));
    ui->labelzULOSyncStatus->setText("(" + tr("out of sync") + ")");

    
    ui->TEMintStatus->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    ui->TEMintStatus->setLineWidth (2);
    ui->TEMintStatus->setMidLineWidth (2);
    ui->TEMintStatus->setPlainText(tr("Mint Status: Okay"));

    
    connect(ui->pushButtonCoinControl, SIGNAL(clicked()), this, SLOT(coinControlButtonClicked()));

    
    QAction* clipboardQuantityAction = new QAction(tr("Copy quantity"), this);
    QAction* clipboardAmountAction = new QAction(tr("Copy amount"), this);
    connect(clipboardQuantityAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardQuantity()));
    connect(clipboardAmountAction, SIGNAL(triggered()), this, SLOT(coinControlClipboardAmount()));
    ui->labelCoinControlQuantity->addAction(clipboardQuantityAction);
    ui->labelCoinControlAmount->addAction(clipboardAmountAction);

    
    ui->labelzDenom1Text->setText("Denom. with value <b>1</b>:");
    ui->labelzDenom2Text->setText("Denom. with value <b>5</b>:");
    ui->labelzDenom3Text->setText("Denom. with value <b>10</b>:");
    ui->labelzDenom4Text->setText("Denom. with value <b>50</b>:");
    ui->labelzDenom5Text->setText("Denom. with value <b>100</b>:");
    ui->labelzDenom6Text->setText("Denom. with value <b>500</b>:");
    ui->labelzDenom7Text->setText("Denom. with value <b>1000</b>:");
    ui->labelzDenom8Text->setText("Denom. with value <b>5000</b>:");

    
    QSettings settings;
    if (!settings.contains("nSecurityLevel")){
        nSecurityLevel = 42;
        settings.setValue("nSecurityLevel", nSecurityLevel);
    }
    else{
        nSecurityLevel = settings.value("nSecurityLevel").toInt();
    }
    
    if (!settings.contains("fMinimizeChange")){
        fMinimizeChange = false;
        settings.setValue("fMinimizeChange", fMinimizeChange);
    }
    else{
        fMinimizeChange = settings.value("fMinimizeChange").toBool();
    }
    ui->checkBoxMinimizeChange->setChecked(fMinimizeChange);

    
    showOutOfSyncWarning(true);

    
    ui->WarningLabel->hide();    
    ui->dummyHideWidget->hide(); 

    
    if(GetAdjustedTime() > GetSporkValue(SPORK_16_ZEROCOIN_MAINTENANCE_MODE)) {
        ui->pushButtonMintzULO->setEnabled(false);
        ui->pushButtonMintzULO->setToolTip(tr("zULO is currently disabled due to maintenance."));

        ui->pushButtonSpendzULO->setEnabled(false);
        ui->pushButtonSpendzULO->setToolTip(tr("zULO is currently disabled due to maintenance."));
    }
}

PrivacyDialog::~PrivacyDialog()
{
    delete ui;
}

void PrivacyDialog::setModel(WalletModel* walletModel)
{
    this->walletModel = walletModel;

    if (walletModel && walletModel->getOptionsModel()) {
        
        setBalance(walletModel->getBalance(), walletModel->getUnconfirmedBalance(), walletModel->getImmatureBalance(),
                   walletModel->getZerocoinBalance(), walletModel->getUnconfirmedZerocoinBalance(), walletModel->getImmatureZerocoinBalance(),
                   walletModel->getWatchBalance(), walletModel->getWatchUnconfirmedBalance(), walletModel->getWatchImmatureBalance());
        
        connect(walletModel, SIGNAL(balanceChanged(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)), this, 
                               SLOT(setBalance(CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount, CAmount)));
        ui->securityLevel->setValue(nSecurityLevel);
    }
}

void PrivacyDialog::on_pasteButton_clicked()
{
    
    ui->payTo->setText(QApplication::clipboard()->text());
}

void PrivacyDialog::on_addressBookButton_clicked()
{
    if (!walletModel)
        return;
    AddressBookPage dlg(AddressBookPage::ForSelection, AddressBookPage::SendingTab, this);
    dlg.setModel(walletModel->getAddressTableModel());
    if (dlg.exec()) {
        ui->payTo->setText(dlg.getReturnValue());
        ui->zULOpayAmount->setFocus();
    }
}

void PrivacyDialog::on_pushButtonMintzULO_clicked()
{
    if (!walletModel || !walletModel->getOptionsModel())
        return;

    if(GetAdjustedTime() > GetSporkValue(SPORK_16_ZEROCOIN_MAINTENANCE_MODE)) {
        QMessageBox::information(this, tr("Mint Zerocoin"),
                                 tr("zULO is currently undergoing maintenance."), QMessageBox::Ok,
                                 QMessageBox::Ok);
        return;
    }

    
    ui->TEMintStatus->setPlainText(tr("Mint Status: Okay"));

    
    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();
    if (encStatus == walletModel->Locked) {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock(true));
        if (!ctx.isValid()) {
            
            ui->TEMintStatus->setPlainText(tr("Error: Your wallet is locked. Please enter the wallet passphrase first."));
            return;
        }
    }

    QString sAmount = ui->labelMintAmountValue->text();
    CAmount nAmount = sAmount.toInt() * COIN;

    
    if(nAmount <= 0){
        ui->TEMintStatus->setPlainText(tr("Message: Enter an amount > 0."));
        return;
    }

    ui->TEMintStatus->setPlainText(tr("Minting ") + ui->labelMintAmountValue->text() + " zULO...");
    ui->TEMintStatus->repaint ();
    
    int64_t nTime = GetTimeMillis();
    
    CWalletTx wtx;
    vector<CZerocoinMint> vMints;
    string strError = pwalletMain->MintZerocoin(nAmount, wtx, vMints, CoinControlDialog::coinControl);
    
    
    if (strError != ""){
        ui->TEMintStatus->setPlainText(QString::fromStdString(strError));
        return;
    }

    double fDuration = (double)(GetTimeMillis() - nTime)/1000.0;

    
    QString strStatsHeader = tr("Successfully minted ") + ui->labelMintAmountValue->text() + tr(" zULO in ") + 
                             QString::number(fDuration) + tr(" sec. Used denominations:\n");
    
    
    ui->labelMintAmountValue->setText ("0");
            
    QString strStats = "";
    ui->TEMintStatus->setPlainText(strStatsHeader);

    for (CZerocoinMint mint : vMints) {
        boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        strStats = strStats + QString::number(mint.GetDenomination()) + " ";
        ui->TEMintStatus->setPlainText(strStatsHeader + strStats);
        ui->TEMintStatus->repaint ();
        
    }

    
    setBalance(walletModel->getBalance(), walletModel->getUnconfirmedBalance(), walletModel->getImmatureBalance(), 
               walletModel->getZerocoinBalance(), walletModel->getUnconfirmedZerocoinBalance(), walletModel->getImmatureZerocoinBalance(),
               walletModel->getWatchBalance(), walletModel->getWatchUnconfirmedBalance(), walletModel->getWatchImmatureBalance());
    coinControlUpdateLabels();

    return;
}

void PrivacyDialog::on_pushButtonMintReset_clicked()
{
    if (!walletModel || !walletModel->getOptionsModel())
        return;

    ui->TEMintStatus->setPlainText(tr("Starting ResetMintZerocoin: rescanning complete blockchain, this will need up to 30 minutes depending on your hardware. \nPlease be patient..."));
    ui->TEMintStatus->repaint ();

    int64_t nTime = GetTimeMillis();
    string strResetMintResult = pwalletMain->ResetMintZerocoin(false); 
    double fDuration = (double)(GetTimeMillis() - nTime)/1000.0;
    ui->TEMintStatus->setPlainText(QString::fromStdString(strResetMintResult) + tr("Duration: ") + QString::number(fDuration) + tr(" sec.\n"));
    ui->TEMintStatus->repaint ();

    return;
}

void PrivacyDialog::on_pushButtonSpentReset_clicked()
{
    if (!walletModel || !walletModel->getOptionsModel())
        return;

    ui->TEMintStatus->setPlainText(tr("Starting ResetSpentZerocoin: "));
    ui->TEMintStatus->repaint ();
    int64_t nTime = GetTimeMillis();
    string strResetSpentResult = pwalletMain->ResetSpentZerocoin();
    double fDuration = (double)(GetTimeMillis() - nTime)/1000.0;
    ui->TEMintStatus->setPlainText(QString::fromStdString(strResetSpentResult) + tr("Duration: ") + QString::number(fDuration) + tr(" sec.\n"));
    ui->TEMintStatus->repaint ();

    return;
}

void PrivacyDialog::on_pushButtonSpendzULO_clicked()
{

    if (!walletModel || !walletModel->getOptionsModel() || !pwalletMain)
        return;

    if(GetAdjustedTime() > GetSporkValue(SPORK_16_ZEROCOIN_MAINTENANCE_MODE)) {
        QMessageBox::information(this, tr("Mint Zerocoin"),
                                 tr("zULO is currently undergoing maintenance."), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    
    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();
    if (encStatus == walletModel->Locked || encStatus == walletModel->UnlockedForAnonymizationOnly) {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock(true));
        if (!ctx.isValid()) {
            
            return;
        }
        
        sendzULO();
        return;
    }
    
    sendzULO();
}

void PrivacyDialog::on_pushButtonZUloControl_clicked()
{
    ZUloControlDialog* zUloControl = new ZUloControlDialog(this);
    zUloControl->setModel(walletModel);
    zUloControl->exec();
}

void PrivacyDialog::setZUloControlLabels(int64_t nAmount, int nQuantity)
{
    ui->labelzUloSelected_int->setText(QString::number(nAmount));
    ui->labelQuantitySelected_int->setText(QString::number(nQuantity));
}

static inline int64_t roundint64(double d)
{
    return (int64_t)(d > 0 ? d + 0.5 : d - 0.5);
}

void PrivacyDialog::sendzULO()
{
    QSettings settings;

    
    CBitcoinAddress address(ui->payTo->text().toStdString());
    if(ui->payTo->text().isEmpty()){
        QMessageBox::information(this, tr("Spend Zerocoin"), tr("No 'Pay To' address provided, creating local payment"), QMessageBox::Ok, QMessageBox::Ok);
    }
    else{
        if (!address.IsValid()) {
            QMessageBox::warning(this, tr("Spend Zerocoin"), tr("Invalid Tesra Address"), QMessageBox::Ok, QMessageBox::Ok);
            ui->payTo->setFocus();
            return;
        }
    }

    
    double dAmount = ui->zULOpayAmount->text().toDouble();
    CAmount nAmount = roundint64(dAmount* COIN);

    
    if (!MoneyRange(nAmount) || nAmount <= 0.0) {
        QMessageBox::warning(this, tr("Spend Zerocoin"), tr("Invalid Send Amount"), QMessageBox::Ok, QMessageBox::Ok);
        ui->zULOpayAmount->setFocus();
        return;
    }

    
    bool fMintChange = ui->checkBoxMintChange->isChecked();

    
    fMinimizeChange = ui->checkBoxMinimizeChange->isChecked();
    settings.setValue("fMinimizeChange", fMinimizeChange);

    
    bool fWholeNumber = floor(dAmount) == dAmount;
    double dzFee = 0.0;

    if(!fWholeNumber)
        dzFee = 1.0 - (dAmount - floor(dAmount));

    if(!fWholeNumber && fMintChange){
        QString strFeeWarning = "You've entered an amount with fractional digits and want the change to be converted to Zerocoin.<br /><br /><b>";
        strFeeWarning += QString::number(dzFee, 'f', 8) + " ULO </b>will be added to the standard transaction fees!<br />";
        QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm additional Fees"),
            strFeeWarning,
            QMessageBox::Yes | QMessageBox::Cancel,
            QMessageBox::Cancel);

        if (retval != QMessageBox::Yes) {
            
            ui->zULOpayAmount->setFocus();
            return;
        }
    }

    
    nSecurityLevel = ui->securityLevel->value();
    settings.setValue("nSecurityLevel", nSecurityLevel);

    

    
    QString strAddressLabel = "";
    if(!ui->payTo->text().isEmpty() && !ui->addAsLabel->text().isEmpty()){
        strAddressLabel = "<br />(" + ui->addAsLabel->text() + ") ";        
    }

    
    QString strQuestionString = tr("Are you sure you want to send?<br /><br />");
    QString strAmount = "<b>" + QString::number(dAmount, 'f', 8) + " zULO</b>";
    QString strAddress = tr(" to address ") + QString::fromStdString(address.ToString()) + strAddressLabel + " <br />";

    if(ui->payTo->text().isEmpty()){
        
        strAddress = tr(" to a newly generated (unused and therefore anonymous) local address <br />");
    }

    QString strSecurityLevel = tr("with Security Level ") + ui->securityLevel->text() + " ?";
    strQuestionString += strAmount + strAddress + strSecurityLevel;

    
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm send coins"),
        strQuestionString,
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if (retval != QMessageBox::Yes) {
        
        return;
    }
    
    int64_t nTime = GetTimeMillis();
    ui->TEMintStatus->setPlainText(tr("Spending Zerocoin.\nComputationally expensive, might need several minutes depending on the selected Security Level and your hardware. \nPlease be patient..."));
    ui->TEMintStatus->repaint();

    
    vector<CZerocoinMint> vMintsSelected;
    if (!ZUloControlDialog::listSelectedMints.empty()) {
        vMintsSelected = ZUloControlDialog::GetSelectedMints();
    }

    
    CWalletTx wtxNew;
    CZerocoinSpendReceipt receipt;
    bool fSuccess = false;
    if(ui->payTo->text().isEmpty()){
        
        fSuccess = pwalletMain->SpendZerocoin(nAmount, nSecurityLevel, wtxNew, receipt, vMintsSelected, fMintChange, fMinimizeChange);    
    }
    else {
        
        fSuccess = pwalletMain->SpendZerocoin(nAmount, nSecurityLevel, wtxNew, receipt, vMintsSelected, fMintChange, fMinimizeChange, &address);
    }

    
    if (!fSuccess) {
        int nNeededSpends = receipt.GetNeededSpends(); 
        const int nMaxSpends = Params().Zerocoin_MaxSpendsPerTransaction(); 
        if (nNeededSpends > nMaxSpends) {
            QString strStatusMessage = tr("Too much inputs (") + QString::number(nNeededSpends, 10) + tr(") needed. \nMaximum allowed: ") + QString::number(nMaxSpends, 10);
            strStatusMessage += tr("\nEither mint higher denominations (so fewer inputs are needed) or reduce the amount to spend.");
            QMessageBox::warning(this, tr("Spend Zerocoin"), strStatusMessage.toStdString().c_str(), QMessageBox::Ok, QMessageBox::Ok);
            ui->TEMintStatus->setPlainText(tr("Spend Zerocoin failed with status = ") +QString::number(receipt.GetStatus(), 10) + "\n" + "Message: " + QString::fromStdString(strStatusMessage.toStdString()));
        }
        else {
            QMessageBox::warning(this, tr("Spend Zerocoin"), receipt.GetStatusMessage().c_str(), QMessageBox::Ok, QMessageBox::Ok);
            ui->TEMintStatus->setPlainText(tr("Spend Zerocoin failed with status = ") +QString::number(receipt.GetStatus(), 10) + "\n" + "Message: " + QString::fromStdString(receipt.GetStatusMessage()));
        }
        ui->zULOpayAmount->setFocus();
        ui->TEMintStatus->repaint();
        return;
    }

    
    ZUloControlDialog::listSelectedMints.clear();

    
    QString strStats = "";
    CAmount nValueIn = 0;
    int nCount = 0;
    for (CZerocoinSpend spend : receipt.GetSpends()) {
        strStats += tr("zUlo Spend #: ") + QString::number(nCount) + ", ";
        strStats += tr("denomination: ") + QString::number(spend.GetDenomination()) + ", ";
        strStats += tr("serial: ") + spend.GetSerial().ToString().c_str() + "\n";
        strStats += tr("Spend is 1 of : ") + QString::number(spend.GetMintCount()) + " mints in the accumulator\n";
        nValueIn += libzerocoin::ZerocoinDenominationToAmount(spend.GetDenomination());
    }

    CAmount nValueOut = 0;
    for (const CTxOut& txout: wtxNew.vout) {
        strStats += tr("value out: ") + FormatMoney(txout.nValue).c_str() + " Ulo, ";
        nValueOut += txout.nValue;

        strStats += tr("address: ");
        CTxDestination dest;
        if(txout.scriptPubKey.IsZerocoinMint())
            strStats += tr("zUlo Mint");
        else if(ExtractDestination(txout.scriptPubKey, dest))
            strStats += tr(CBitcoinAddress(dest).ToString().c_str());
        strStats += "\n";
    }
    double fDuration = (double)(GetTimeMillis() - nTime)/1000.0;
    strStats += tr("Duration: ") + QString::number(fDuration) + tr(" sec.\n");
    strStats += tr("Sending successful, return code: ") + QString::number(receipt.GetStatus()) + "\n";

    QString strReturn;
    strReturn += tr("txid: ") + wtxNew.GetHash().ToString().c_str() + "\n";
    strReturn += tr("fee: ") + QString::fromStdString(FormatMoney(nValueIn-nValueOut)) + "\n";
    strReturn += strStats;

    
    ui->zULOpayAmount->setText ("0");

    ui->TEMintStatus->setPlainText(strReturn);
    ui->TEMintStatus->repaint();
}

void PrivacyDialog::on_payTo_textChanged(const QString& address)
{
    updateLabel(address);
}


void PrivacyDialog::coinControlClipboardQuantity()
{
    GUIUtil::setClipboard(ui->labelCoinControlQuantity->text());
}


void PrivacyDialog::coinControlClipboardAmount()
{
    GUIUtil::setClipboard(ui->labelCoinControlAmount->text().left(ui->labelCoinControlAmount->text().indexOf(" ")));
}


void PrivacyDialog::coinControlButtonClicked()
{
    CoinControlDialog dlg;
    dlg.setModel(walletModel);
    dlg.exec();
    coinControlUpdateLabels();
}


void PrivacyDialog::coinControlUpdateLabels()
{
    if (!walletModel || !walletModel->getOptionsModel() || !walletModel->getOptionsModel()->getCoinControlFeatures())
        return;

     
    CoinControlDialog::payAmounts.clear();

    if (CoinControlDialog::coinControl->HasSelected()) {
        
        CoinControlDialog::updateLabels(walletModel, this);
    } else {
        ui->labelCoinControlQuantity->setText (tr("Coins automatically selected"));
        ui->labelCoinControlAmount->setText (tr("Coins automatically selected"));
    }
}

bool PrivacyDialog::updateLabel(const QString& address)
{
    if (!walletModel)
        return false;

    
    QString associatedLabel = walletModel->getAddressTableModel()->labelForAddress(address);
    if (!associatedLabel.isEmpty()) {
        ui->addAsLabel->setText(associatedLabel);
        return true;
    }

    return false;
}

void PrivacyDialog::setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance, 
                               const CAmount& zerocoinBalance, const CAmount& unconfirmedZerocoinBalance, const CAmount& immatureZerocoinBalance,
                               const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance)
{

    currentBalance = balance;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    currentZerocoinBalance = zerocoinBalance;
    currentUnconfirmedZerocoinBalance = unconfirmedZerocoinBalance;
    currentImmatureZerocoinBalance = immatureZerocoinBalance;
    currentWatchOnlyBalance = watchOnlyBalance;
    currentWatchUnconfBalance = watchUnconfBalance;
    currentWatchImmatureBalance = watchImmatureBalance;

    CWalletDB walletdb(pwalletMain->strWalletFile);
    list<CZerocoinMint> listMints = walletdb.ListMintedCoins(true, false, true);
 
    std::map<libzerocoin::CoinDenomination, CAmount> mapDenomBalances;
    std::map<libzerocoin::CoinDenomination, int> mapUnconfirmed;
    std::map<libzerocoin::CoinDenomination, int> mapImmature;
    for (const auto& denom : libzerocoin::zerocoinDenomList){
        mapDenomBalances.insert(make_pair(denom, 0));
        mapUnconfirmed.insert(make_pair(denom, 0));
        mapImmature.insert(make_pair(denom, 0));
    }

    int nBestHeight = chainActive.Height();
    for (auto& mint : listMints){
        
        mapDenomBalances.at(mint.GetDenomination())++;

        if (!mint.GetHeight() || chainActive.Height() - mint.GetHeight() <= Params().Zerocoin_MintRequiredConfirmations()) {
            
            mapUnconfirmed.at(mint.GetDenomination())++;
        }
        else {
            
            CBlockIndex *pindex = chainActive[mint.GetHeight() + 1];
            int nHeight2CheckpointsDeep = nBestHeight - (nBestHeight % 10) - 20;
            int nMintsAdded = 0;
            while (pindex->nHeight < nHeight2CheckpointsDeep) { 
                nMintsAdded += count(pindex->vMintDenominationsInBlock.begin(), pindex->vMintDenominationsInBlock.end(), mint.GetDenomination());
                if (nMintsAdded >= Params().Zerocoin_RequiredAccumulation())
                    break;
                pindex = chainActive[pindex->nHeight + 1];
            }
            if (nMintsAdded < Params().Zerocoin_RequiredAccumulation()){
                
                mapImmature.at(mint.GetDenomination())++;
            }
        }
    }

    int64_t nCoins = 0;
    int64_t nSumPerCoin = 0;
    int64_t nUnconfirmed = 0;
    int64_t nImmature = 0;
    QString strDenomStats, strUnconfirmed = "";

    for (const auto& denom : libzerocoin::zerocoinDenomList) {
        nCoins = libzerocoin::ZerocoinDenominationToInt(denom);
        nSumPerCoin = nCoins * mapDenomBalances.at(denom);
        nUnconfirmed = mapUnconfirmed.at(denom);
        nImmature = mapImmature.at(denom);

        strUnconfirmed = "";
        if (nUnconfirmed) {
            strUnconfirmed += QString::number(nUnconfirmed) + QString(" unconf. ");
        }
        if(nImmature) {
            strUnconfirmed += QString::number(nImmature) + QString(" immature ");
        }
        if(nImmature || nUnconfirmed) {
            strUnconfirmed = QString("( ") + strUnconfirmed + QString(") ");
        }

        strDenomStats = strUnconfirmed + QString::number(mapDenomBalances.at(denom)) + " x " +
                        QString::number(nCoins) + " = <b>" + 
                        QString::number(nSumPerCoin) + " zULO </b>";

        switch (nCoins) {
            case libzerocoin::CoinDenomination::ZQ_ONE: 
                ui->labelzDenom1Amount->setText(strDenomStats);
                break;
            case libzerocoin::CoinDenomination::ZQ_FIVE:
                ui->labelzDenom2Amount->setText(strDenomStats);
                break;
            case libzerocoin::CoinDenomination::ZQ_TEN:
                ui->labelzDenom3Amount->setText(strDenomStats);
                break;
            case libzerocoin::CoinDenomination::ZQ_FIFTY:
                ui->labelzDenom4Amount->setText(strDenomStats);
                break;
            case libzerocoin::CoinDenomination::ZQ_ONE_HUNDRED:
                ui->labelzDenom5Amount->setText(strDenomStats);
                break;
            case libzerocoin::CoinDenomination::ZQ_FIVE_HUNDRED:
                ui->labelzDenom6Amount->setText(strDenomStats);
                break;
            case libzerocoin::CoinDenomination::ZQ_ONE_THOUSAND:
                ui->labelzDenom7Amount->setText(strDenomStats);
                break;
            case libzerocoin::CoinDenomination::ZQ_FIVE_THOUSAND:
                ui->labelzDenom8Amount->setText(strDenomStats);
                break;
            default:
                
                break;
        }
    }
    CAmount matureZerocoinBalance = zerocoinBalance - immatureZerocoinBalance;
    CAmount nLockedBalance = 0;
    if (walletModel) {
        nLockedBalance = walletModel->getLockedBalance();
    }

    ui->labelzAvailableAmount->setText(QString::number(zerocoinBalance/COIN) + QString(" zULO "));
    ui->labelzAvailableAmount_2->setText(QString::number(matureZerocoinBalance/COIN) + QString(" zULO "));
    ui->labelzULOAmountValue->setText(BitcoinUnits::floorHtmlWithUnit(nDisplayUnit, balance - immatureBalance - nLockedBalance, false, BitcoinUnits::separatorAlways));
}

void PrivacyDialog::updateDisplayUnit()
{
    if (walletModel && walletModel->getOptionsModel()) {
        nDisplayUnit = walletModel->getOptionsModel()->getDisplayUnit();
        if (currentBalance != -1)
            setBalance(currentBalance, currentUnconfirmedBalance, currentImmatureBalance,
                       currentZerocoinBalance, currentUnconfirmedZerocoinBalance, currentImmatureZerocoinBalance,
                       currentWatchOnlyBalance, currentWatchUnconfBalance, currentWatchImmatureBalance);
    }
}

void PrivacyDialog::showOutOfSyncWarning(bool fShow)
{
    ui->labelzULOSyncStatus->setVisible(fShow);
}

void PrivacyDialog::keyPressEvent(QKeyEvent* event)
{
    if (event->key() != Qt::Key_Escape) 
    {
        this->QDialog::keyPressEvent(event);
    } else {
        event->ignore();
    }
}
