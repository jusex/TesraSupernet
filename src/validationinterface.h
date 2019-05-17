




#ifndef BITCOIN_VALIDATIONINTERFACE_H
#define BITCOIN_VALIDATIONINTERFACE_H

#include <boost/signals2/signal.hpp>
#include <boost/shared_ptr.hpp>

class CBlock;
struct CBlockLocator;
class CBlockIndex;
class CReserveScript;
class CTransaction;
class CValidationInterface;
class CValidationState;
class uint256;




void RegisterValidationInterface(CValidationInterface* pwalletIn);

void UnregisterValidationInterface(CValidationInterface* pwalletIn);

void UnregisterAllValidationInterfaces();

void SyncWithWallets(const CTransaction& tx, const CBlock* pblock);

class CValidationInterface {
protected:
    virtual void UpdatedBlockTip(const CBlockIndex *pindex) {}
    virtual void SyncTransaction(const CTransaction &tx, const CBlock *pblock) {}
    virtual void NotifyTransactionLock(const CTransaction &tx) {}
    virtual void SetBestChain(const CBlockLocator &locator) {}
    virtual bool UpdatedTransaction(const uint256 &hash) { return false;}
    virtual void Inventory(const uint256 &hash) {}

    virtual void ResendWalletTransactions() {}
    virtual void BlockChecked(const CBlock&, const CValidationState&) {}
    virtual void GetScriptForMining(boost::shared_ptr<CReserveScript>&) {};
    virtual void ResetRequestCount(const uint256 &hash) {};
    friend void ::RegisterValidationInterface(CValidationInterface*);
    friend void ::UnregisterValidationInterface(CValidationInterface*);
    friend void ::UnregisterAllValidationInterfaces();
};

struct CMainSignals {
    
    boost::signals2::signal<void (const CBlockIndex *)> UpdatedBlockTip;
    
    boost::signals2::signal<void (const CTransaction &, const CBlock *)> SyncTransaction;
    
    boost::signals2::signal<void (const CTransaction &)> NotifyTransactionLock;
    
    boost::signals2::signal<bool (const uint256 &)> UpdatedTransaction;
    
    boost::signals2::signal<void (const CBlockLocator &)> SetBestChain;
    
    boost::signals2::signal<void (const uint256 &)> Inventory;
    
    boost::signals2::signal<void (int64_t nBestBlockTime)> Broadcast;
    
    boost::signals2::signal<void (const CBlock&, const CValidationState&)> BlockChecked;
    
    boost::signals2::signal<void (boost::shared_ptr<CReserveScript>&)> ScriptForMining;
    
    boost::signals2::signal<void (const uint256 &)> BlockFound;
};

CMainSignals& GetMainSignals();

#endif 
