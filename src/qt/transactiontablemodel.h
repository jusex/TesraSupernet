



#ifndef BITCOIN_QT_TRANSACTIONTABLEMODEL_H
#define BITCOIN_QT_TRANSACTIONTABLEMODEL_H

#include "bitcoinunits.h"

#include <QAbstractTableModel>
#include <QStringList>

class TransactionRecord;
class TransactionTablePriv;
class WalletModel;

class CWallet;

/** UI model for the transaction table of a wallet.
 */
class TransactionTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TransactionTableModel(CWallet* wallet, WalletModel* parent = 0);
    ~TransactionTableModel();

    enum ColumnIndex {
        Status = 0,
        Watchonly = 1,
        Date = 2,
        Type = 3,
        ToAddress = 4,
        Amount = 5
    };

    /** Roles to get specific information from a transaction row.
        These are independent of column.
    */
    enum RoleIndex {
        
        TypeRole = Qt::UserRole,
        
        DateRole,
        
        WatchonlyRole,
        
        WatchonlyDecorationRole,
        
        LongDescriptionRole,
        
        AddressRole,
        
        LabelRole,
        
        AmountRole,
        
        TxIDRole,
        
        TxHashRole,
        
        ConfirmedRole,
        
        FormattedAmountRole,
        
        StatusRole
    };

    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    bool processingQueuedTransactions() { return fProcessingQueuedTransactions; }

private:
    CWallet* wallet;
    WalletModel* walletModel;
    QStringList columns;
    TransactionTablePriv* priv;
    bool fProcessingQueuedTransactions;

    void subscribeToCoreSignals();
    void unsubscribeFromCoreSignals();

    QString lookupAddress(const std::string& address, bool tooltip) const;
    QVariant addressColor(const TransactionRecord* wtx) const;
    QString formatTxStatus(const TransactionRecord* wtx) const;
    QString formatTxDate(const TransactionRecord* wtx) const;
    QString formatTxType(const TransactionRecord* wtx) const;
    QString formatTxToAddress(const TransactionRecord* wtx, bool tooltip) const;
    QString formatTxAmount(const TransactionRecord* wtx, bool showUnconfirmed = true, BitcoinUnits::SeparatorStyle separators = BitcoinUnits::separatorStandard) const;
    QString formatTooltip(const TransactionRecord* rec) const;
    QVariant txStatusDecoration(const TransactionRecord* wtx) const;
    QVariant txWatchonlyDecoration(const TransactionRecord* wtx) const;
    QVariant txAddressDecoration(const TransactionRecord* wtx) const;

public slots:
    
    void updateTransaction(const QString& hash, int status, bool showTransaction);
    void updateConfirmations();
    void updateDisplayUnit();
    
    void updateAmountColumnTitle();
    
    void setProcessingQueuedTransactions(bool value) { fProcessingQueuedTransactions = value; }

    friend class TransactionTablePriv;
};

#endif 
