





#include "sendcoinsentry.h"
#include "ui_sendcoinsentry.h"

#include "addressbookpage.h"
#include "addresstablemodel.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "walletmodel.h"

#include <QApplication>
#include <QClipboard>

SendCoinsEntry::SendCoinsEntry(QWidget* parent) : QStackedWidget(parent),
                                                  ui(new Ui::SendCoinsEntry),
                                                  model(0)
{
    ui->setupUi(this);

    setCurrentWidget(ui->SendCoins);

#ifdef Q_OS_MAC
    ui->payToLayout->setSpacing(4);
#endif
#if QT_VERSION >= 0x040700
    ui->addAsLabel->setPlaceholderText(tr("Enter a label for this address to add it to your address book"));
#endif

    
    GUIUtil::setupAddressWidget(ui->payTo, this);
    
    ui->payTo_is->setFont(GUIUtil::bitcoinAddressFont());

    
    connect(ui->payAmount, SIGNAL(valueChanged()), this, SIGNAL(payAmountChanged()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteClicked()));
    connect(ui->deleteButton_is, SIGNAL(clicked()), this, SLOT(deleteClicked()));
    connect(ui->deleteButton_s, SIGNAL(clicked()), this, SLOT(deleteClicked()));
}

SendCoinsEntry::~SendCoinsEntry()
{
    delete ui;
}

void SendCoinsEntry::on_pasteButton_clicked()
{
    
    ui->payTo->setText(QApplication::clipboard()->text());
}

void SendCoinsEntry::on_addressBookButton_clicked()
{
    if (!model)
        return;
    AddressBookPage dlg(AddressBookPage::ForSelection, AddressBookPage::SendingTab, this);
    dlg.setModel(model->getAddressTableModel());
    if (dlg.exec()) {
        ui->payTo->setText(dlg.getReturnValue());
        ui->payAmount->setFocus();
    }
}

void SendCoinsEntry::on_payTo_textChanged(const QString& address)
{
    updateLabel(address);
}

void SendCoinsEntry::setModel(WalletModel* model)
{
    this->model = model;

    if (model && model->getOptionsModel())
        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

    clear();
}

void SendCoinsEntry::clear()
{
    
    ui->payTo->clear();
    ui->addAsLabel->clear();
    ui->payAmount->clear();
    ui->messageTextLabel->clear();
    ui->messageTextLabel->hide();
    ui->messageLabel->hide();
    
    ui->payTo_is->clear();
    ui->memoTextLabel_is->clear();
    ui->payAmount_is->clear();
    
    ui->payTo_s->clear();
    ui->memoTextLabel_s->clear();
    ui->payAmount_s->clear();

    
    updateDisplayUnit();
}

void SendCoinsEntry::deleteClicked()
{
    emit removeEntry(this);
}

bool SendCoinsEntry::validate()
{
    if (!model)
        return false;

    
    bool retval = true;

    
    if (recipient.paymentRequest.IsInitialized())
        return retval;

    if (!model->validateAddress(ui->payTo->text())) {
        ui->payTo->setValid(false);
        retval = false;
    }

    if (!ui->payAmount->validate()) {
        retval = false;
    }

    
    if (ui->payAmount->value(0) <= 0) {
        ui->payAmount->setValid(false);
        retval = false;
    }

    
    if (retval && GUIUtil::isDust(ui->payTo->text(), ui->payAmount->value())) {
        ui->payAmount->setValid(false);
        retval = false;
    }

    return retval;
}

SendCoinsRecipient SendCoinsEntry::getValue()
{
    
    if (recipient.paymentRequest.IsInitialized())
        return recipient;

    
    recipient.address = ui->payTo->text();
    recipient.label = ui->addAsLabel->text();
    recipient.amount = ui->payAmount->value();
    recipient.message = ui->messageTextLabel->text();

    return recipient;
}

QWidget* SendCoinsEntry::setupTabChain(QWidget* prev)
{
    QWidget::setTabOrder(prev, ui->payTo);
    QWidget::setTabOrder(ui->payTo, ui->addAsLabel);
    QWidget* w = ui->payAmount->setupTabChain(ui->addAsLabel);
    QWidget::setTabOrder(w, ui->addressBookButton);
    QWidget::setTabOrder(ui->addressBookButton, ui->pasteButton);
    QWidget::setTabOrder(ui->pasteButton, ui->deleteButton);
    return ui->deleteButton;
}

void SendCoinsEntry::setValue(const SendCoinsRecipient& value)
{
    recipient = value;

    if (recipient.paymentRequest.IsInitialized()) 
    {
        if (recipient.authenticatedMerchant.isEmpty()) 
        {
            ui->payTo_is->setText(recipient.address);
            ui->memoTextLabel_is->setText(recipient.message);
            ui->payAmount_is->setValue(recipient.amount);
            ui->payAmount_is->setReadOnly(true);
            setCurrentWidget(ui->SendCoins_InsecurePaymentRequest);
        } else 
        {
            ui->payTo_s->setText(recipient.authenticatedMerchant);
            ui->memoTextLabel_s->setText(recipient.message);
            ui->payAmount_s->setValue(recipient.amount);
            ui->payAmount_s->setReadOnly(true);
            setCurrentWidget(ui->SendCoins_SecurePaymentRequest);
        }
    } else 
    {
        
        ui->messageTextLabel->setText(recipient.message);
        ui->messageTextLabel->setVisible(!recipient.message.isEmpty());
        ui->messageLabel->setVisible(!recipient.message.isEmpty());

        ui->addAsLabel->clear();
        ui->payTo->setText(recipient.address); 
        if (!recipient.label.isEmpty())        
            ui->addAsLabel->setText(recipient.label);
        ui->payAmount->setValue(recipient.amount);
    }
}

void SendCoinsEntry::setAddress(const QString& address)
{
    ui->payTo->setText(address);
    ui->payAmount->setFocus();
}

bool SendCoinsEntry::isClear()
{
    return ui->payTo->text().isEmpty() && ui->payTo_is->text().isEmpty() && ui->payTo_s->text().isEmpty();
}

void SendCoinsEntry::setFocus()
{
    ui->payTo->setFocus();
}

void SendCoinsEntry::updateDisplayUnit()
{
    if (model && model->getOptionsModel()) {
        
        ui->payAmount->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
        ui->payAmount_is->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
        ui->payAmount_s->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
    }
}

bool SendCoinsEntry::updateLabel(const QString& address)
{
    if (!model)
        return false;

    
    QString associatedLabel = model->getAddressTableModel()->labelForAddress(address);
    if (!associatedLabel.isEmpty()) {
        ui->addAsLabel->setText(associatedLabel);
        return true;
    }

    return false;
}
