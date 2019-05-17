



#ifndef BITCOIN_QT_WALLETMODEL_H
#define BITCOIN_QT_WALLETMODEL_H

#include "paymentrequestplus.h"
#include "walletmodeltransaction.h"

#include "allocators.h" 
#include "swifttx.h"
#include "wallet.h"

#include <map>
#include <vector>

#include <QObject>

class AddressTableModel;
class OptionsModel;
class RecentRequestsTableModel;
class TransactionTableModel;
class WalletModelTransaction;

class CCoinControl;
class CKeyID;
class COutPoint;
class COutput;
class CPubKey;
class CWallet;
class uint256;

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

class SendCoinsRecipient
{
public:
    explicit SendCoinsRecipient() : amount(0), nVersion(SendCoinsRecipient::CURRENT_VERSION) {}
    explicit SendCoinsRecipient(const QString& addr, const QString& label, const CAmount& amount, const QString& message) : address(addr), label(label), amount(amount), message(message), nVersion(SendCoinsRecipient::CURRENT_VERSION) {}

    
    
    
    
    
    QString address;
    QString label;
    AvailableCoinsType inputType;
    bool useSwiftTX;
    CAmount amount;
    
    QString message;

    
    PaymentRequestPlus paymentRequest;
    
    QString authenticatedMerchant;

    static const int CURRENT_VERSION = 1;
    int nVersion;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        std::string sAddress = address.toStdString();
        std::string sLabel = label.toStdString();
        std::string sMessage = message.toStdString();
        std::string sPaymentRequest;
        if (!ser_action.ForRead() && paymentRequest.IsInitialized())
            paymentRequest.SerializeToString(&sPaymentRequest);
        std::string sAuthenticatedMerchant = authenticatedMerchant.toStdString();

        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(sAddress);
        READWRITE(sLabel);
        READWRITE(amount);
        READWRITE(sMessage);
        READWRITE(sPaymentRequest);
        READWRITE(sAuthenticatedMerchant);

        if (ser_action.ForRead()) {
            address = QString::fromStdString(sAddress);
            label = QString::fromStdString(sLabel);
            message = QString::fromStdString(sMessage);
            if (!sPaymentRequest.empty())
                paymentRequest.parse(QByteArray::fromRawData(sPaymentRequest.data(), sPaymentRequest.size()));
            authenticatedMerchant = QString::fromStdString(sAuthenticatedMerchant);
        }
    }
};


class WalletModel : public QObject
{
    Q_OBJECT

public:
    explicit WalletModel(CWallet* wallet, OptionsModel* optionsModel, QObject* parent = 0);
    ~WalletModel();

    enum StatusCode 
    {
        OK,
        InvalidAmount,
        InvalidAddress,
        AmountExceedsBalance,
        AmountWithFeeExceedsBalance,
        DuplicateAddress,
        TransactionCreationFailed, 
        TransactionCommitFailed,
        AnonymizeOnlyUnlocked,
        InsaneFee
    };

    enum EncryptionStatus {
        Unencrypted,                 
        Locked,                      
        Unlocked,                    
        UnlockedForAnonymizationOnly 
    };

    OptionsModel* getOptionsModel();
    AddressTableModel* getAddressTableModel();

    TransactionTableModel* getTransactionTableModel();
    RecentRequestsTableModel* getRecentRequestsTableModel();

    CAmount getBalance(const CCoinControl* coinControl = NULL) const;
    CAmount getUnconfirmedBalance() const;
    CAmount getImmatureBalance() const;
    CAmount getLockedBalance() const;
    CAmount getZerocoinBalance() const;
    CAmount getUnconfirmedZerocoinBalance() const;
    CAmount getImmatureZerocoinBalance() const;
    bool haveWatchOnly() const;
    CAmount getWatchBalance() const;
    CAmount getWatchUnconfirmedBalance() const;
    CAmount getWatchImmatureBalance() const;
    EncryptionStatus getEncryptionStatus() const;
    CKey generateNewKey() const; 
    bool setAddressBook(const CTxDestination& address, const string& strName, const string& strPurpose);
    void encryptKey(const CKey key, const std::string& pwd, const std::string& slt, std::vector<unsigned char>& crypted);
    void decryptKey(const std::vector<unsigned char>& crypted, const std::string& slt, const std::string& pwd, CKey& key);
    void emitBalanceChanged(); 

    
    bool validateAddress(const QString& address);

    
    struct SendCoinsReturn {
        SendCoinsReturn(StatusCode status = OK) : status(status) {}
        StatusCode status;
    };

    
    SendCoinsReturn prepareTransaction(WalletModelTransaction& transaction, const CCoinControl* coinControl = NULL);

    
    SendCoinsReturn sendCoins(WalletModelTransaction& transaction);

    
    bool setWalletEncrypted(bool encrypted, const SecureString& passphrase);
    
    bool setWalletLocked(bool locked, const SecureString& passPhrase = SecureString(), bool anonymizeOnly = false);
    bool changePassphrase(const SecureString& oldPass, const SecureString& newPass);
    
    bool isAnonymizeOnlyUnlocked();
    
    bool backupWallet(const QString& filename);

    
    class UnlockContext
    {
    public:
        UnlockContext(bool valid, bool relock);
        ~UnlockContext();

        bool isValid() const { return valid; }

        
        UnlockContext(const UnlockContext& obj) { CopyFrom(obj); }
        UnlockContext& operator=(const UnlockContext& rhs)
        {
            CopyFrom(rhs);
            return *this;
        }

    private:
        bool valid;
        mutable bool relock; 

        void CopyFrom(const UnlockContext& rhs);
    };

    UnlockContext requestUnlock(bool relock = false);

    bool getPubKey(const CKeyID& address, CPubKey& vchPubKeyOut) const;
    bool isMine(CBitcoinAddress address);
    void getOutputs(const std::vector<COutPoint>& vOutpoints, std::vector<COutput>& vOutputs);
    bool isSpent(const COutPoint& outpoint) const;
    void listCoins(std::map<QString, std::vector<COutput> >& mapCoins) const;

    bool isLockedCoin(uint256 hash, unsigned int n) const;
    void lockCoin(COutPoint& output);
    void unlockCoin(COutPoint& output);
    void listLockedCoins(std::vector<COutPoint>& vOutpts);

    void listZerocoinMints(std::list<CZerocoinMint>& listMints, bool fUnusedOnly = false, bool fMaturedOnly = false, bool fUpdateStatus = false);

    void loadReceiveRequests(std::vector<std::string>& vReceiveRequests);
    bool saveReceiveRequest(const std::string& sAddress, const int64_t nId, const std::string& sRequest);

private:
    CWallet* wallet;
    bool fHaveWatchOnly;
    bool fHaveMultiSig;
    bool fForceCheckBalanceChanged;

    
    
    OptionsModel* optionsModel;

    AddressTableModel* addressTableModel;
    TransactionTableModel* transactionTableModel;
    RecentRequestsTableModel* recentRequestsTableModel;

    
    CAmount cachedBalance;
    CAmount cachedUnconfirmedBalance;
    CAmount cachedImmatureBalance;
    CAmount cachedZerocoinBalance;
    CAmount cachedUnconfirmedZerocoinBalance;
    CAmount cachedImmatureZerocoinBalance;
    CAmount cachedWatchOnlyBalance;
    CAmount cachedWatchUnconfBalance;
    CAmount cachedWatchImmatureBalance;
    EncryptionStatus cachedEncryptionStatus;
    int cachedNumBlocks;
    int cachedTxLocks;
    int cachedZeromintPercentage;

    QTimer* pollTimer;

    void subscribeToCoreSignals();
    void unsubscribeFromCoreSignals();
    void checkBalanceChanged();

signals:
    
    void balanceChanged(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance, 
                        const CAmount& zerocoinBalance, const CAmount& unconfirmedZerocoinBalance, const CAmount& immatureZerocoinBalance, 
                        const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance);

    
    void encryptionStatusChanged(int status);

    
    
    
    void requireUnlock();

    
    void message(const QString& title, const QString& message, unsigned int style);

    
    void coinsSent(CWallet* wallet, SendCoinsRecipient recipient, QByteArray transaction);

    
    void showProgress(const QString& title, int nProgress);

    
    void notifyWatchonlyChanged(bool fHaveWatchonly);

    
    void notifyMultiSigChanged(bool fHaveMultiSig);
public slots:
    
    void updateStatus();
    
    void updateTransaction();
    
    void updateAddressBook(const QString& address, const QString& label, bool isMine, const QString& purpose, int status);
    
    void updateAddressBook(const QString &pubCoin, const QString &isUsed, int status);
    
    void updateWatchOnlyFlag(bool fHaveWatchonly);
    
    void updateMultiSigFlag(bool fHaveMultiSig);
    
    void pollBalanceChanged();
};

#endif 
