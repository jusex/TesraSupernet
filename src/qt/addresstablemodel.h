



#ifndef BITCOIN_QT_ADDRESSTABLEMODEL_H
#define BITCOIN_QT_ADDRESSTABLEMODEL_H

#include <QAbstractTableModel>
#include <QStringList>

class AddressTablePriv;
class WalletModel;

class CWallet;

/**
   Qt model of the address book in the core. This allows views to access and modify the address book.
 */
class AddressTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit AddressTableModel(CWallet* wallet, WalletModel* parent = 0);
    ~AddressTableModel();

    enum ColumnIndex {
        Label = 0,  
        Address = 1 
    };

    enum RoleIndex {
        TypeRole = Qt::UserRole 
    };

    
    enum EditStatus {
        OK,                    
        NO_CHANGES,            
        INVALID_ADDRESS,       
        DUPLICATE_ADDRESS,     
        WALLET_UNLOCK_FAILURE, 
        KEY_GENERATION_FAILURE 
    };

    static const QString Send;    
    static const QString Receive; 
    static const QString Zerocoin; 

    /** @name Methods overridden from QAbstractTableModel
        @{*/
    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex& parent) const;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    Qt::ItemFlags flags(const QModelIndex& index) const;
    

    /* Add an address to the model.
       Returns the added address on success, and an empty string otherwise.
     */
    QString addRow(const QString& type, const QString& label, const QString& address);

    /* Look up label for address in address book, if not found return empty string.
     */
    QString labelForAddress(const QString& address) const;

    /* Look up row index of an address in the model.
       Return -1 if not found.
     */
    int lookupAddress(const QString& address) const;

    EditStatus getEditStatus() const { return editStatus; }

private:
    WalletModel* walletModel;
    CWallet* wallet;
    AddressTablePriv* priv;
    QStringList columns;
    EditStatus editStatus;

    
    void emitDataChanged(int index);

public slots:
    /* Update address list from core.
     */
    void updateEntry(const QString& address, const QString& label, bool isMine, const QString& purpose, int status);
    void updateEntry(const QString &pubCoin, const QString &isUsed, int status);
    friend class AddressTablePriv;
};

#endif 
