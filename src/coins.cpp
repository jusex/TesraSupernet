




#include "coins.h"

#include "random.h"

#include <assert.h>

/**
 * calculate number of bytes for the bitmask, and its number of non-zero bytes
 * each bit in the bitmask represents the availability of one output, but the
 * availabilities of the first two outputs are encoded separately
 */
void CCoins::CalcMaskSize(unsigned int& nBytes, unsigned int& nNonzeroBytes) const
{
    unsigned int nLastUsedByte = 0;
    for (unsigned int b = 0; 2 + b * 8 < vout.size(); b++) {
        bool fZero = true;
        for (unsigned int i = 0; i < 8 && 2 + b * 8 + i < vout.size(); i++) {
            if (!vout[2 + b * 8 + i].IsNull()) {
                fZero = false;
                continue;
            }
        }
        if (!fZero) {
            nLastUsedByte = b + 1;
            nNonzeroBytes++;
        }
    }
    nBytes += nLastUsedByte;
}

bool CCoins::Spend(const COutPoint& out, CTxInUndo& undo)
{
    if (out.n >= vout.size())
        return false;
    if (vout[out.n].IsNull())
        return false;
    undo = CTxInUndo(vout[out.n]);
    vout[out.n].SetNull();
    Cleanup();
    if (vout.size() == 0) {
        undo.nHeight = nHeight;
        undo.fCoinBase = fCoinBase;
        undo.fCoinStake = fCoinStake;
        undo.nVersion = this->nVersion;
    }
    return true;
}

bool CCoins::Spend(int nPos)
{
    CTxInUndo undo;
    COutPoint out(0, nPos);
    return Spend(out, undo);
}


bool CCoinsView::GetCoins(const uint256& txid, CCoins& coins) const { return false; }
bool CCoinsView::HaveCoins(const uint256& txid) const { return false; }
uint256 CCoinsView::GetBestBlock() const { return uint256(0); }
bool CCoinsView::BatchWrite(CCoinsMap& mapCoins, const uint256& hashBlock) { return false; }
bool CCoinsView::GetStats(CCoinsStats& stats) const { return false; }


CCoinsViewBacked::CCoinsViewBacked(CCoinsView* viewIn) : base(viewIn) {}
bool CCoinsViewBacked::GetCoins(const uint256& txid, CCoins& coins) const { return base->GetCoins(txid, coins); }
bool CCoinsViewBacked::HaveCoins(const uint256& txid) const { return base->HaveCoins(txid); }
uint256 CCoinsViewBacked::GetBestBlock() const { return base->GetBestBlock(); }
void CCoinsViewBacked::SetBackend(CCoinsView& viewIn) { base = &viewIn; }
bool CCoinsViewBacked::BatchWrite(CCoinsMap& mapCoins, const uint256& hashBlock) { return base->BatchWrite(mapCoins, hashBlock); }
bool CCoinsViewBacked::GetStats(CCoinsStats& stats) const { return base->GetStats(stats); }

CCoinsKeyHasher::CCoinsKeyHasher() : salt(GetRandHash()) {}

CCoinsViewCache::CCoinsViewCache(CCoinsView* baseIn) : CCoinsViewBacked(baseIn), hasModifier(false), hashBlock(0) {}

CCoinsViewCache::~CCoinsViewCache()
{
    assert(!hasModifier);
}

CCoinsMap::const_iterator CCoinsViewCache::FetchCoins(const uint256& txid) const
{
    CCoinsMap::iterator it = cacheCoins.find(txid);
    if (it != cacheCoins.end())
        return it;
    CCoins tmp;
    if (!base->GetCoins(txid, tmp))
        return cacheCoins.end();
    CCoinsMap::iterator ret = cacheCoins.insert(std::make_pair(txid, CCoinsCacheEntry())).first;
    tmp.swap(ret->second.coins);
    if (ret->second.coins.IsPruned()) {
        
        
        ret->second.flags = CCoinsCacheEntry::FRESH;
    }
    return ret;
}

bool CCoinsViewCache::GetCoins(const uint256& txid, CCoins& coins) const
{
    CCoinsMap::const_iterator it = FetchCoins(txid);
    if (it != cacheCoins.end()) {
        coins = it->second.coins;
        return true;
    }
    return false;
}

CCoinsModifier CCoinsViewCache::ModifyCoins(const uint256& txid)
{
    assert(!hasModifier);
    std::pair<CCoinsMap::iterator, bool> ret = cacheCoins.insert(std::make_pair(txid, CCoinsCacheEntry()));
    if (ret.second) {
        if (!base->GetCoins(txid, ret.first->second.coins)) {
            
            ret.first->second.coins.Clear();
            ret.first->second.flags = CCoinsCacheEntry::FRESH;
        } else if (ret.first->second.coins.IsPruned()) {
            
            ret.first->second.flags = CCoinsCacheEntry::FRESH;
        }
    }
    
    ret.first->second.flags |= CCoinsCacheEntry::DIRTY;
    return CCoinsModifier(*this, ret.first);
}

const CCoins* CCoinsViewCache::AccessCoins(const uint256& txid) const
{
    CCoinsMap::const_iterator it = FetchCoins(txid);
    if (it == cacheCoins.end()) {
        return NULL;
    } else {
        return &it->second.coins;
    }
}

bool CCoinsViewCache::HaveCoins(const uint256& txid) const
{
    CCoinsMap::const_iterator it = FetchCoins(txid);
    
    
    
    
    return (it != cacheCoins.end() && !it->second.coins.vout.empty());
}

uint256 CCoinsViewCache::GetBestBlock() const
{
    if (hashBlock == uint256(0))
        hashBlock = base->GetBestBlock();
    return hashBlock;
}

void CCoinsViewCache::SetBestBlock(const uint256& hashBlockIn)
{
    hashBlock = hashBlockIn;
}

bool CCoinsViewCache::BatchWrite(CCoinsMap& mapCoins, const uint256& hashBlockIn)
{
    assert(!hasModifier);
    for (CCoinsMap::iterator it = mapCoins.begin(); it != mapCoins.end();) {
        if (it->second.flags & CCoinsCacheEntry::DIRTY) { 
            CCoinsMap::iterator itUs = cacheCoins.find(it->first);
            if (itUs == cacheCoins.end()) {
                if (!it->second.coins.IsPruned()) {
                    
                    
                    
                    
                    assert(it->second.flags & CCoinsCacheEntry::FRESH);
                    CCoinsCacheEntry& entry = cacheCoins[it->first];
                    entry.coins.swap(it->second.coins);
                    entry.flags = CCoinsCacheEntry::DIRTY | CCoinsCacheEntry::FRESH;
                }
            } else {
                if ((itUs->second.flags & CCoinsCacheEntry::FRESH) && it->second.coins.IsPruned()) {
                    
                    
                    
                    cacheCoins.erase(itUs);
                } else {
                    
                    itUs->second.coins.swap(it->second.coins);
                    itUs->second.flags |= CCoinsCacheEntry::DIRTY;
                }
            }
        }
        CCoinsMap::iterator itOld = it++;
        mapCoins.erase(itOld);
    }
    hashBlock = hashBlockIn;
    return true;
}

bool CCoinsViewCache::Flush()
{
    bool fOk = base->BatchWrite(cacheCoins, hashBlock);
    cacheCoins.clear();
    return fOk;
}

unsigned int CCoinsViewCache::GetCacheSize() const
{
    return cacheCoins.size();
}

const CTxOut& CCoinsViewCache::GetOutputFor(const CTxIn& input) const
{
    const CCoins* coins = AccessCoins(input.prevout.hash);
    assert(coins && coins->IsAvailable(input.prevout.n));
    return coins->vout[input.prevout.n];
}

CAmount CCoinsViewCache::GetValueIn(const CTransaction& tx) const
{
    if (tx.IsCoinBase())
        return 0;

    
    if (tx.IsZerocoinSpend())
        return tx.GetZerocoinSpent();

    CAmount nResult = 0;
    for (unsigned int i = 0; i < tx.vin.size(); i++)
        nResult += GetOutputFor(tx.vin[i]).nValue;

    return nResult;
}

bool CCoinsViewCache::HaveInputs(const CTransaction& tx) const
{
    if (!tx.IsCoinBase() && !tx.IsZerocoinSpend()) {
        for (unsigned int i = 0; i < tx.vin.size(); i++) {
            const COutPoint& prevout = tx.vin[i].prevout;
            const CCoins* coins = AccessCoins(prevout.hash);
            if (!coins || !coins->IsAvailable(prevout.n)) {
                return false;
            }
        }
    }
    return true;
}

double CCoinsViewCache::GetPriority(const CTransaction& tx, int nHeight) const
{
    if (tx.IsCoinBase() || tx.IsCoinStake())
        return 0.0;
    double dResult = 0.0;
    for (const CTxIn& txin:  tx.vin) {
        const CCoins* coins = AccessCoins(txin.prevout.hash);
        assert(coins);
        if (!coins->IsAvailable(txin.prevout.n)) continue;
        if (coins->nHeight < nHeight) {
            dResult += coins->vout[txin.prevout.n].nValue * (nHeight - coins->nHeight);
        }
    }
    return tx.ComputePriority(dResult);
}

CCoinsModifier::CCoinsModifier(CCoinsViewCache& cache_, CCoinsMap::iterator it_) : cache(cache_), it(it_)
{
    assert(!cache.hasModifier);
    cache.hasModifier = true;
}

CCoinsModifier::~CCoinsModifier()
{
    assert(cache.hasModifier);
    cache.hasModifier = false;
    it->second.coins.Cleanup();
    if ((it->second.flags & CCoinsCacheEntry::FRESH) && it->second.coins.IsPruned()) {
        cache.cacheCoins.erase(it);
    }
}
