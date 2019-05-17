





#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>

#include "db.h"
#include "kernel.h"
#include "script/interpreter.h"
#include "timedata.h"
#include "util.h"

using namespace std;

bool fTestNet = false; 



unsigned int nModifierInterval;
int nStakeTargetSpacing = 60;
unsigned int getIntervalVersion(bool fTestNet)
{
    if (fTestNet)
        return MODIFIER_INTERVAL_TESTNET;
    else
        return MODIFIER_INTERVAL;
}


static std::map<int, unsigned int> mapStakeModifierCheckpoints =
    boost::assign::map_list_of(0, 0xfd11f4e7u);


int64_t GetWeight(int64_t nIntervalBeginning, int64_t nIntervalEnd)
{
    return nIntervalEnd - nIntervalBeginning - nStakeMinAge;
}


static bool GetLastStakeModifier(const CBlockIndex* pindex, uint64_t& nStakeModifier, int64_t& nModifierTime)
{
    if (!pindex)
        return error("GetLastStakeModifier: null pindex");
    while (pindex && pindex->pprev && !pindex->GeneratedStakeModifier())
        pindex = pindex->pprev;
    if (!pindex->GeneratedStakeModifier())
        return error("GetLastStakeModifier: no generation at genesis block");
    nStakeModifier = pindex->nStakeModifier;
    nModifierTime = pindex->GetBlockTime();
    return true;
}


static int64_t GetStakeModifierSelectionIntervalSection(int nSection)
{
    assert(nSection >= 0 && nSection < 64);
    int64_t a = getIntervalVersion(fTestNet) * 63 / (63 + ((63 - nSection) * (MODIFIER_INTERVAL_RATIO - 1)));
    return a;
}


static int64_t GetStakeModifierSelectionInterval()
{
    int64_t nSelectionInterval = 0;
    for (int nSection = 0; nSection < 64; nSection++) {
        nSelectionInterval += GetStakeModifierSelectionIntervalSection(nSection);
    }
    return nSelectionInterval;
}




static bool SelectBlockFromCandidates(
    vector<pair<int64_t, uint256> >& vSortedByTimestamp,
    map<uint256, const CBlockIndex*>& mapSelectedBlocks,
    int64_t nSelectionIntervalStop,
    uint64_t nStakeModifierPrev,
    const CBlockIndex** pindexSelected)
{
    bool fModifierV2 = false;
    bool fFirstRun = true;
    bool fSelected = false;
    uint256 hashBest = 0;
    *pindexSelected = (const CBlockIndex*)0;
    BOOST_FOREACH (const PAIRTYPE(int64_t, uint256) & item, vSortedByTimestamp) {
        if (!mapBlockIndex.count(item.second))
            return error("SelectBlockFromCandidates: failed to find block index for candidate block %s", item.second.ToString().c_str());

        const CBlockIndex* pindex = mapBlockIndex[item.second];
        if (fSelected && pindex->GetBlockTime() > nSelectionIntervalStop)
            break;

        
        if (fFirstRun){
            fModifierV2 = pindex->nHeight >= Params().ModifierUpgradeBlock();
            fFirstRun = false;
        }

        if (mapSelectedBlocks.count(pindex->GetBlockHash()) > 0)
            continue;

        
        uint256 hashProof;
        if(fModifierV2)
            hashProof = pindex->GetBlockHash();
        else
            hashProof = pindex->IsProofOfStake() ? 0 : pindex->GetBlockHash();

        CDataStream ss(SER_GETHASH, 0);
        ss << hashProof << nStakeModifierPrev;
        uint256 hashSelection = Hash(ss.begin(), ss.end());

        
        
        
        if (pindex->IsProofOfStake())
            hashSelection >>= 32;

        if (fSelected && hashSelection < hashBest) {
            hashBest = hashSelection;
            *pindexSelected = (const CBlockIndex*)pindex;
        } else if (!fSelected) {
            fSelected = true;
            hashBest = hashSelection;
            *pindexSelected = (const CBlockIndex*)pindex;
        }
    }
    if (GetBoolArg("-printstakemodifier", false))
        LogPrintf("SelectBlockFromCandidates: selection hash=%s\n", hashBest.ToString().c_str());
    return fSelected;
}














bool ComputeNextStakeModifier(const CBlockIndex* pindexPrev, uint64_t& nStakeModifier, bool& fGeneratedStakeModifier)
{
    nStakeModifier = 0;
    fGeneratedStakeModifier = false;
    if (!pindexPrev) {
        fGeneratedStakeModifier = true;
        return true; 
    }
    if (pindexPrev->nHeight == 0) {
        
        fGeneratedStakeModifier = true;
        nStakeModifier = uint64_t("stakemodifier");
        return true;
    }

    
    
    int64_t nModifierTime = 0;
    if (!GetLastStakeModifier(pindexPrev, nStakeModifier, nModifierTime))
        return error("ComputeNextStakeModifier: unable to get last modifier");

    if (GetBoolArg("-printstakemodifier", false))
        LogPrintf("ComputeNextStakeModifier: prev modifier= %s time=%s\n", boost::lexical_cast<std::string>(nStakeModifier).c_str(), DateTimeStrFormat("%Y-%m-%d %H:%M:%S", nModifierTime).c_str());

    if (nModifierTime / getIntervalVersion(fTestNet) >= pindexPrev->GetBlockTime() / getIntervalVersion(fTestNet))
        return true;

    
    vector<pair<int64_t, uint256> > vSortedByTimestamp;
    vSortedByTimestamp.reserve(64 * getIntervalVersion(fTestNet) / nStakeTargetSpacing);
    int64_t nSelectionInterval = GetStakeModifierSelectionInterval();
    int64_t nSelectionIntervalStart = (pindexPrev->GetBlockTime() / getIntervalVersion(fTestNet)) * getIntervalVersion(fTestNet) - nSelectionInterval;
    const CBlockIndex* pindex = pindexPrev;

    while (pindex && pindex->GetBlockTime() >= nSelectionIntervalStart) {
        vSortedByTimestamp.push_back(make_pair(pindex->GetBlockTime(), pindex->GetBlockHash()));
        pindex = pindex->pprev;
    }

    int nHeightFirstCandidate = pindex ? (pindex->nHeight + 1) : 0;
    reverse(vSortedByTimestamp.begin(), vSortedByTimestamp.end());
    sort(vSortedByTimestamp.begin(), vSortedByTimestamp.end());

    
    uint64_t nStakeModifierNew = 0;
    int64_t nSelectionIntervalStop = nSelectionIntervalStart;
    map<uint256, const CBlockIndex*> mapSelectedBlocks;
    for (int nRound = 0; nRound < min(64, (int)vSortedByTimestamp.size()); nRound++) {
        
        nSelectionIntervalStop += GetStakeModifierSelectionIntervalSection(nRound);

        
        if (!SelectBlockFromCandidates(vSortedByTimestamp, mapSelectedBlocks, nSelectionIntervalStop, nStakeModifier, &pindex))
            return error("ComputeNextStakeModifier: unable to select block at round %d", nRound);

        
        nStakeModifierNew |= (((uint64_t)pindex->GetStakeEntropyBit()) << nRound);

        
        mapSelectedBlocks.insert(make_pair(pindex->GetBlockHash(), pindex));
        if (fDebug || GetBoolArg("-printstakemodifier", false))
            LogPrintf("ComputeNextStakeModifier: selected round %d stop=%s height=%d bit=%d\n",
                nRound, DateTimeStrFormat("%Y-%m-%d %H:%M:%S", nSelectionIntervalStop).c_str(), pindex->nHeight, pindex->GetStakeEntropyBit());
    }

    
    if (fDebug || GetBoolArg("-printstakemodifier", false)) {
        string strSelectionMap = "";
        
        strSelectionMap.insert(0, pindexPrev->nHeight - nHeightFirstCandidate + 1, '-');
        pindex = pindexPrev;
        while (pindex && pindex->nHeight >= nHeightFirstCandidate) {
            
            if (pindex->IsProofOfStake())
                strSelectionMap.replace(pindex->nHeight - nHeightFirstCandidate, 1, "=");
            pindex = pindex->pprev;
        }
        BOOST_FOREACH (const PAIRTYPE(uint256, const CBlockIndex*) & item, mapSelectedBlocks) {
            
            
            strSelectionMap.replace(item.second->nHeight - nHeightFirstCandidate, 1, item.second->IsProofOfStake() ? "S" : "W");
        }
        LogPrintf("ComputeNextStakeModifier: selection height [%d, %d] map %s\n", nHeightFirstCandidate, pindexPrev->nHeight, strSelectionMap.c_str());
    }
    if (fDebug || GetBoolArg("-printstakemodifier", false)) {
        LogPrintf("ComputeNextStakeModifier: new modifier=%s time=%s\n", boost::lexical_cast<std::string>(nStakeModifierNew).c_str(), DateTimeStrFormat("%Y-%m-%d %H:%M:%S", pindexPrev->GetBlockTime()).c_str());
    }

    nStakeModifier = nStakeModifierNew;
    fGeneratedStakeModifier = true;
    return true;
}



bool GetKernelStakeModifier(uint256 hashBlockFrom, uint64_t& nStakeModifier, int& nStakeModifierHeight, int64_t& nStakeModifierTime, bool fPrintProofOfStake)
{
    nStakeModifier = 0;
    if (!mapBlockIndex.count(hashBlockFrom))
        return error("GetKernelStakeModifier() : block not indexed");
    const CBlockIndex* pindexFrom = mapBlockIndex[hashBlockFrom];
    nStakeModifierHeight = pindexFrom->nHeight;
    nStakeModifierTime = pindexFrom->GetBlockTime();
    int64_t nStakeModifierSelectionInterval = GetStakeModifierSelectionInterval();
    const CBlockIndex* pindex = pindexFrom;
    CBlockIndex* pindexNext = chainActive[pindexFrom->nHeight + 1];

    
    while (nStakeModifierTime < pindexFrom->GetBlockTime() + nStakeModifierSelectionInterval) {
        if (!pindexNext) {
            
            return error("Null pindexNext\n");
        }

        pindex = pindexNext;
        pindexNext = chainActive[pindexNext->nHeight + 1];
        if (pindex->GeneratedStakeModifier()) {
            nStakeModifierHeight = pindex->nHeight;
            nStakeModifierTime = pindex->GetBlockTime();
        }
    }
    nStakeModifier = pindex->nStakeModifier;
    return true;
}

uint256 stakeHash(unsigned int nTimeTx, CDataStream ss, unsigned int prevoutIndex, uint256 prevoutHash, unsigned int nTimeBlockFrom)
{
    
    ss << nTimeBlockFrom << prevoutIndex << prevoutHash << nTimeTx;
    return Hash(ss.begin(), ss.end());
}


bool stakeTargetHit(uint256 hashProofOfStake, int64_t nValueIn, uint256 bnTargetPerCoinDay)
{
    
    uint256 bnCoinDayWeight = uint256(nValueIn) / 100;

    
    return hashProofOfStake < (bnCoinDayWeight * bnTargetPerCoinDay);
}


bool CheckStakeKernelHash(unsigned int nBits, const CBlock blockFrom, const CTransaction txPrev, const COutPoint prevout, unsigned int& nTimeTx, unsigned int nHashDrift, bool fCheck, uint256& hashProofOfStake, bool fPrintProofOfStake)
{
    
    int64_t nValueIn = txPrev.vout[prevout.n].nValue;
    unsigned int nTimeBlockFrom = blockFrom.GetBlockTime();

    if (nTimeTx < nTimeBlockFrom) 
        return error("CheckStakeKernelHash() : nTime violation");

    if (nTimeBlockFrom + nStakeMinAge > nTimeTx) 
        return error("CheckStakeKernelHash() : min age violation - nTimeBlockFrom=%d nStakeMinAge=%d nTimeTx=%d", nTimeBlockFrom, nStakeMinAge, nTimeTx);

    
    uint256 bnTargetPerCoinDay;
    bnTargetPerCoinDay.SetCompact(nBits);

    
    uint64_t nStakeModifier = 0;
    int nStakeModifierHeight = 0;
    int64_t nStakeModifierTime = 0;
    if (!GetKernelStakeModifier(blockFrom.GetHash(), nStakeModifier, nStakeModifierHeight, nStakeModifierTime, fPrintProofOfStake)) {
        LogPrintf("CheckStakeKernelHash(): failed to get kernel stake modifier \n");
        return false;
    }

    
    CDataStream ss(SER_GETHASH, 0);
    ss << nStakeModifier;

    
    if (fCheck) {
        hashProofOfStake = stakeHash(nTimeTx, ss, prevout.n, prevout.hash, nTimeBlockFrom);
        return stakeTargetHit(hashProofOfStake, nValueIn, bnTargetPerCoinDay);
    }

    bool fSuccess = false;
    unsigned int nTryTime = 0;
    unsigned int i;
    int nHeightStart = chainActive.Height();
    for (i = 0; i < (nHashDrift); i++) 
    {
        
        if (chainActive.Height() != nHeightStart)
            break;

        
        nTryTime = nTimeTx + nHashDrift - i;
        hashProofOfStake = stakeHash(nTryTime, ss, prevout.n, prevout.hash, nTimeBlockFrom);

        
        if (!stakeTargetHit(hashProofOfStake, nValueIn, bnTargetPerCoinDay))
            continue;

        fSuccess = true; 
        nTimeTx = nTryTime;

        if (fDebug || fPrintProofOfStake) {
            LogPrintf("CheckStakeKernelHash() : using modifier %s at height=%d timestamp=%s for block from height=%d timestamp=%s\n",
                boost::lexical_cast<std::string>(nStakeModifier).c_str(), nStakeModifierHeight,
                DateTimeStrFormat("%Y-%m-%d %H:%M:%S", nStakeModifierTime).c_str(),
                mapBlockIndex[blockFrom.GetHash()]->nHeight,
                DateTimeStrFormat("%Y-%m-%d %H:%M:%S", blockFrom.GetBlockTime()).c_str());
            LogPrintf("CheckStakeKernelHash() : pass protocol=%s modifier=%s nTimeBlockFrom=%u prevoutHash=%s nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProof=%s\n",
                "0.3",
                boost::lexical_cast<std::string>(nStakeModifier).c_str(),
                nTimeBlockFrom, prevout.hash.ToString().c_str(), nTimeBlockFrom, prevout.n, nTryTime,
                hashProofOfStake.ToString().c_str());
        }
        break;
    }

    mapHashedBlocks.clear();
    mapHashedBlocks[chainActive.Tip()->nHeight] = GetTime(); 
    return fSuccess;
}


bool CheckProofOfStake(const CBlock block, uint256& hashProofOfStake)
{
    const CTransaction tx = block.vtx[1];
    if (!tx.IsCoinStake())
        return error("CheckProofOfStake() : called on non-coinstake %s", tx.GetHash().ToString().c_str());

    
    const CTxIn& txin = tx.vin[0];

    
    uint256 hashBlock;
    CTransaction txPrev;
    if (!GetTransaction(txin.prevout.hash, txPrev, hashBlock, true))
        return error("CheckProofOfStake() : INFO: read txPrev failed");

    
    if (!VerifyScript(txin.scriptSig, txPrev.vout[txin.prevout.n].scriptPubKey, STANDARD_SCRIPT_VERIFY_FLAGS, TransactionSignatureChecker(&tx, 0)))
        return error("CheckProofOfStake() : VerifySignature failed on coinstake %s", tx.GetHash().ToString().c_str());

    CBlockIndex* pindex = NULL;
    BlockMap::iterator it = mapBlockIndex.find(hashBlock);
    if (it != mapBlockIndex.end())
        pindex = it->second;
    else
        return error("CheckProofOfStake() : read block failed");

    
    CBlock blockprev;
    if (!ReadBlockFromDisk(blockprev, pindex->GetBlockPos()))
        return error("CheckProofOfStake(): INFO: failed to find block");

    unsigned int nInterval = 0;
    unsigned int nTime = block.nTime;
    if (!CheckStakeKernelHash(block.nBits, blockprev, txPrev, txin.prevout, nTime, nInterval, true, hashProofOfStake, fDebug))
        return error("CheckProofOfStake() : INFO: check kernel failed on coinstake %s, hashProof=%s \n", tx.GetHash().ToString().c_str(), hashProofOfStake.ToString().c_str()); 

    return true;
}


bool CheckCoinStakeTimestamp(int64_t nTimeBlock, int64_t nTimeTx)
{
    
    return (nTimeBlock == nTimeTx);
}


unsigned int GetStakeModifierChecksum(const CBlockIndex* pindex)
{
    assert(pindex->pprev || pindex->GetBlockHash() == Params().HashGenesisBlock());
    
    CDataStream ss(SER_GETHASH, 0);
    if (pindex->pprev)
        ss << pindex->pprev->nStakeModifierChecksum;
    ss << pindex->nFlags << pindex->hashProofOfStake << pindex->nStakeModifier;
    uint256 hashChecksum = Hash(ss.begin(), ss.end());
    hashChecksum >>= (256 - 32);
    return hashChecksum.Get64();
}


bool CheckStakeModifierCheckpoints(int nHeight, unsigned int nStakeModifierChecksum)
{
    if (fTestNet) return true; 
    if (mapStakeModifierCheckpoints.count(nHeight)) {
        return nStakeModifierChecksum == mapStakeModifierCheckpoints[nHeight];
    }
    return true;
}
