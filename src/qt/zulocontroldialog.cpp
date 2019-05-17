



#include "zulocontroldialog.h"
#include "ui_zulocontroldialog.h"

#include "main.h"
#include "walletmodel.h"

using namespace std;

std::list<std::string> ZUloControlDialog::listSelectedMints;
std::list<CZerocoinMint> ZUloControlDialog::listMints;

ZUloControlDialog::ZUloControlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ZUloControlDialog),
    model(0)
{
    ui->setupUi(this);
    listMints.clear();
    privacyDialog = (PrivacyDialog*)parent;

    
    connect(ui->treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(updateSelection(QTreeWidgetItem*, int)));

    
    connect(ui->pushButtonAll, SIGNAL(clicked()), this, SLOT(ButtonAllClicked()));
}

ZUloControlDialog::~ZUloControlDialog()
{
    delete ui;
}

void ZUloControlDialog::setModel(WalletModel *model)
{
    this->model = model;
    updateList();
}


void ZUloControlDialog::updateList()
{
    
    ui->treeWidget->blockSignals(true);
    ui->treeWidget->clear();

    
    QFlags<Qt::ItemFlag> flgTristate = Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsTristate;
    map<libzerocoin::CoinDenomination, int> mapDenomPosition;
    for (auto denom : libzerocoin::zerocoinDenomList) {
        QTreeWidgetItem* itemDenom(new QTreeWidgetItem);
        ui->treeWidget->addTopLevelItem(itemDenom);

        
        mapDenomPosition[denom] = ui->treeWidget->indexOfTopLevelItem(itemDenom);

        itemDenom->setFlags(flgTristate);
        itemDenom->setText(COLUMN_DENOMINATION, QString::number(denom));
    }

    
    std::list<CZerocoinMint> list;
    model->listZerocoinMints(list, true, false, true);
    this->listMints = list;

    
    int nBestHeight = chainActive.Height();
    for(const CZerocoinMint mint : listMints) {
        
        libzerocoin::CoinDenomination denom = mint.GetDenomination();
        QTreeWidgetItem *itemMint = new QTreeWidgetItem(ui->treeWidget->topLevelItem(mapDenomPosition.at(denom)));

        
        std::string strPubCoin = mint.GetValue().GetHex();
        itemMint->setCheckState(COLUMN_CHECKBOX, Qt::Unchecked);
        if(count(listSelectedMints.begin(), listSelectedMints.end(), strPubCoin))
            itemMint->setCheckState(COLUMN_CHECKBOX, Qt::Checked);

        itemMint->setText(COLUMN_DENOMINATION, QString::number(mint.GetDenomination()));
        itemMint->setText(COLUMN_PUBCOIN, QString::fromStdString(strPubCoin));

        int nConfirmations = (mint.GetHeight() ? nBestHeight - mint.GetHeight() : 0);
        if (nConfirmations < 0) {
            
            nConfirmations = 0;
        }

        itemMint->setText(COLUMN_CONFIRMATIONS, QString::number(nConfirmations));

        
        int nMintsAdded = 0;
        if(mint.GetHeight() != 0 && mint.GetHeight() < nBestHeight - 2) {
            CBlockIndex *pindex = chainActive[mint.GetHeight() + 1];

            int nHeight2CheckpointsDeep = nBestHeight - (nBestHeight % 10) - 20;
            while (pindex->nHeight < nHeight2CheckpointsDeep) { 
                nMintsAdded += count(pindex->vMintDenominationsInBlock.begin(), pindex->vMintDenominationsInBlock.end(), mint.GetDenomination());
                if(nMintsAdded >= Params().Zerocoin_RequiredAccumulation())
                    break;
                pindex = chainActive[pindex->nHeight + 1];
            }
        }

        
        bool fSpendable = nMintsAdded >= Params().Zerocoin_RequiredAccumulation() && nConfirmations >= Params().Zerocoin_MintRequiredConfirmations();
        if(!fSpendable) {
            itemMint->setDisabled(true);
            itemMint->setCheckState(COLUMN_CHECKBOX, Qt::Unchecked);

            
            auto it = std::find(listSelectedMints.begin(), listSelectedMints.end(), mint.GetValue().GetHex());
            if (it != listSelectedMints.end()) {
                listSelectedMints.erase(it);
            }

            string strReason = "";
            if(nConfirmations < Params().Zerocoin_MintRequiredConfirmations())
                strReason = strprintf("Needs %d more confirmations", Params().Zerocoin_MintRequiredConfirmations() - nConfirmations);
            else
                strReason = strprintf("Needs %d more mints added to network", Params().Zerocoin_RequiredAccumulation() - nMintsAdded);

            itemMint->setText(COLUMN_ISSPENDABLE, QString::fromStdString(strReason));
        } else {
            itemMint->setText(COLUMN_ISSPENDABLE, QString("Yes"));
        }
    }

    ui->treeWidget->blockSignals(false);
    updateLabels();
}


void ZUloControlDialog::updateSelection(QTreeWidgetItem* item, int column)
{
    
    if (item->parent() && column == COLUMN_CHECKBOX && !item->isDisabled()){

        
        std::string strPubcoin = item->text(COLUMN_PUBCOIN).toStdString();
        auto iter = std::find(listSelectedMints.begin(), listSelectedMints.end(), strPubcoin);
        bool fSelected = iter != listSelectedMints.end();

        
        if (item->checkState(COLUMN_CHECKBOX) == Qt::Checked) {
            if (fSelected) return;
            listSelectedMints.emplace_back(strPubcoin);
        } else {
            if (!fSelected) return;
            listSelectedMints.erase(iter);
        }
        updateLabels();
    }
}


void ZUloControlDialog::updateLabels()
{
    int64_t nAmount = 0;
    for (const CZerocoinMint mint : listMints) {
        if (count(listSelectedMints.begin(), listSelectedMints.end(), mint.GetValue().GetHex())) {
            nAmount += mint.GetDenomination();
        }
    }

    
    ui->labelZUlo_int->setText(QString::number(nAmount));
    ui->labelQuantity_int->setText(QString::number(listSelectedMints.size()));

    
    privacyDialog->setZUloControlLabels(nAmount, listSelectedMints.size());
}

std::vector<CZerocoinMint> ZUloControlDialog::GetSelectedMints()
{
    std::vector<CZerocoinMint> listReturn;
    for (const CZerocoinMint mint : listMints) {
        if (count(listSelectedMints.begin(), listSelectedMints.end(), mint.GetValue().GetHex())) {
            listReturn.emplace_back(mint);
        }
    }

    return listReturn;
}


void ZUloControlDialog::ButtonAllClicked()
{
    ui->treeWidget->blockSignals(true);
    Qt::CheckState state = Qt::Checked;
    for(int i = 0; i < ui->treeWidget->topLevelItemCount(); i++) {
        if(ui->treeWidget->topLevelItem(i)->checkState(COLUMN_CHECKBOX) != Qt::Unchecked) {
            state = Qt::Unchecked;
            break;
        }
    }

    
    ui->treeWidget->clear();

    if(state == Qt::Checked) {
        for(const CZerocoinMint mint : listMints)
            listSelectedMints.emplace_back(mint.GetValue().GetHex());
    } else {
        listSelectedMints.clear();
    }

    updateList();
}
