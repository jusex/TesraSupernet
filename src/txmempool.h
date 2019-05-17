





#ifndef BITCOIN_TXMEMPOOL_H
#define BITCOIN_TXMEMPOOL_H

#include <list>

#include "amount.h"
#include "coins.h"
#include "primitives/transaction.h"
#include "sync.h"

class CAutoFile;

inline double AllowFreeThreshold()
{
    return COIN * 1440 / 250;
}

inline bool AllowFree(double dPriority)
{
    
    
    return dPriority > AllowFreeThreshold();
}



static const unsigned int MEMPOOL_HEIGHT = 0x7FFFFFFF;

/**
 * CTxMemPool stores these:
 */
class CTxMemPoolEntry
{
private:
    CTransaction tx;
    CAmount nFee;         
    size_t nTxSize;       
    size_t nModSize;      
    int64_t nTime;        
    double dPriority;     
    unsigned int nHeight; 

public:
    CTxMemPoolEntry(const CTransaction& _tx, const CAmount& _nFee, int64_t _nTime, double _dPriority, unsigned int _nHeight);
    CTxMemPoolEntry();
    CTxMemPoolEntry(const CTxMemPoolEntry& other);

    const CTransaction& GetTx() const { return this->tx; }
    double GetPriority(unsigned int currentHeight) const;
    CAmount GetFee() const { return nFee; }
    size_t GetTxSize() const { return nTxSize; }
    int64_t GetTime() const { return nTime; }
    unsigned int GetHeight() const { return nHeight; }
};

class CMinerPolicyEstimator;


class CInPoint
{
public:
    const CTransaction* ptx;
    uint32_t n;

    CInPoint() { SetNull(); }
    CInPoint(const CTransaction* ptxIn, uint32_t nIn)
    {
        ptx = ptxIn;
        n = nIn;
    }
    void SetNull()
    {
        ptx = NULL;
        n = (uint32_t)-1;
    }
    bool IsNull() const { return (ptx == NULL && n == (uint32_t)-1); }
};

/**
 * CTxMemPool stores valid-according-to-the-current-best-chain
 * transactions that may be included in the next block.
 *
 * Transactions are added when they are seen on the network
 * (or created by the local node), but not all transactions seen
 * are added to the pool: if a new transaction double-spends
 * an input of a transaction in the pool, it is dropped,
 * as are non-standard transactions.
 */
class CTxMemPool
{
private:
    bool fSanityCheck; 
    unsigned int nTransactionsUpdated;
    CMinerPolicyEstimator* minerPolicyEstimator;

    CFeeRate minRelayFee; 
    uint64_t totalTxSize; 

public:
    mutable CCriticalSection cs;
    std::map<uint256, CTxMemPoolEntry> mapTx;
    std::map<COutPoint, CInPoint> mapNextTx;
    std::map<uint256, std::pair<double, CAmount> > mapDeltas;

    CTxMemPool(const CFeeRate& _minRelayFee);
    ~CTxMemPool();

    /**
     * If sanity-checking is turned on, check makes sure the pool is
     * consistent (does not contain two transactions that spend the same inputs,
     * all inputs are in the mapNextTx array). If sanity-checking is turned off,
     * check does nothing.
     */
    void check(const CCoinsViewCache* pcoins) const;
    void setSanityCheck(bool _fSanityCheck) { fSanityCheck = _fSanityCheck; }

    bool addUnchecked(const uint256& hash, const CTxMemPoolEntry& entry);
    void remove(const CTransaction& tx, std::list<CTransaction>& removed, bool fRecursive = false);
    void removeCoinbaseSpends(const CCoinsViewCache* pcoins, unsigned int nMemPoolHeight);
    void removeConflicts(const CTransaction& tx, std::list<CTransaction>& removed);
    void removeForBlock(const std::vector<CTransaction>& vtx, unsigned int nBlockHeight, std::list<CTransaction>& conflicts);
    void clear();
    void queryHashes(std::vector<uint256>& vtxid);
    void pruneSpent(const uint256& hash, CCoins& coins);
    unsigned int GetTransactionsUpdated() const;
    void AddTransactionsUpdated(unsigned int n);

    
    void PrioritiseTransaction(const uint256 hash, const std::string strHash, double dPriorityDelta, const CAmount& nFeeDelta);
    void ApplyDeltas(const uint256 hash, double& dPriorityDelta, CAmount& nFeeDelta);
    void ClearPrioritisation(const uint256 hash);

    unsigned long size()
    {
        LOCK(cs);
        return mapTx.size();
    }
    uint64_t GetTotalTxSize()
    {
        LOCK(cs);
        return totalTxSize;
    }

    bool exists(uint256 hash)
    {
        LOCK(cs);
        return (mapTx.count(hash) != 0);
    }

    bool lookup(uint256 hash, CTransaction& result) const;

    
    CFeeRate estimateFee(int nBlocks) const;

    
    double estimatePriority(int nBlocks) const;

    
    bool WriteFeeEstimates(CAutoFile& fileout) const;
    bool ReadFeeEstimates(CAutoFile& filein);
};

/** 
 * CCoinsView that brings transactions from a memorypool into view.
 * It does not check for spendings by memory pool transactions.
 */
class CCoinsViewMemPool : public CCoinsViewBacked
{
protected:
    CTxMemPool& mempool;

public:
    CCoinsViewMemPool(CCoinsView* baseIn, CTxMemPool& mempoolIn);
    bool GetCoins(const uint256& txid, CCoins& coins) const;
    bool HaveCoins(const uint256& txid) const;
};

#endif 
