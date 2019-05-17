



#ifndef BITCOIN_QT_TRANSACTIONRECORD_H
#define BITCOIN_QT_TRANSACTIONRECORD_H

#include "amount.h"
#include "uint256.h"

#include <QList>
#include <QString>

class CWallet;
class CWalletTx;

/** UI model for transaction status. The transaction status is the part of a transaction that will change over time.
 */
class TransactionStatus
{
public:
    TransactionStatus() : countsForBalance(false), sortKey(""),
                          matures_in(0), status(Offline), depth(0), open_for(0), cur_num_blocks(-1)
    {
    }

    enum Status {
        Confirmed, 
        
        OpenUntilDate,  
        OpenUntilBlock, 
        Offline,        
        Unconfirmed,    
        Confirming,     
        Conflicted,     
        
        Immature,       
        MaturesWarning, 
        NotAccepted     
    };

    
    bool countsForBalance;
    
    std::string sortKey;

    /** @name Generated (mined) transactions
       @{*/
    int matures_in;
    

    /** @name Reported status
       @{*/
    Status status;
    qint64 depth;
    qint64 open_for; /**< Timestamp if status==OpenUntilDate, otherwise number
                      of additional blocks that need to be mined before
                      finalization */
    

    
    int cur_num_blocks;

    
    int cur_num_ix_locks;
};

/** UI model for a transaction. A core transaction can be represented by multiple UI transactions if it has
    multiple outputs.
 */
class TransactionRecord
{
public:
    enum Type {
        Other,
        Generated,
        StakeMint,
        SendToAddress,
        SendToOther,
        RecvWithAddress,
        MNReward,
        RecvFromOther,
        SendToSelf,
        ZerocoinMint,
        ZerocoinSpend,
        RecvFromZerocoinSpend,
        ZerocoinSpend_Change_zUlo,
        ZerocoinSpend_FromMe,
        RecvWithObfuscation,
        ObfuscationDenominate,
        ObfuscationCollateralPayment,
        ObfuscationMakeCollaterals,
        ObfuscationCreateDenominations,
        Obfuscated
    };

    
    static const int RecommendedNumConfirmations = 6;

    TransactionRecord() : hash(), time(0), type(Other), address(""), debit(0), credit(0), idx(0)
    {
    }

    TransactionRecord(uint256 hash, qint64 time) : hash(hash), time(time), type(Other), address(""), debit(0),
                                                   credit(0), idx(0)
    {
    }

    TransactionRecord(uint256 hash, qint64 time, Type type, const std::string& address, const CAmount& debit, const CAmount& credit) : hash(hash), time(time), type(type), address(address), debit(debit), credit(credit),
                                                                                                                                       idx(0)
    {
    }

    /** Decompose CWallet transaction to model transaction records.
     */
    static bool showTransaction(const CWalletTx& wtx);
    static QList<TransactionRecord> decomposeTransaction(const CWallet* wallet, const CWalletTx& wtx);

    /** @name Immutable transaction attributes
      @{*/
    uint256 hash;
    qint64 time;
    Type type;
    std::string address;
    CAmount debit;
    CAmount credit;
    

    
    int idx;

    
    TransactionStatus status;

    
    bool involvesWatchAddress;

    
    QString getTxID() const;

    
    int getOutputIndex() const;

    /** Update status from core wallet tx.
     */
    void updateStatus(const CWalletTx& wtx);

    /** Return whether a status update is needed.
     */
    bool statusUpdateNeeded();
};

#endif 
