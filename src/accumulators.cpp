



#include "accumulators.h"
#include "accumulatormap.h"
#include "chainparams.h"
#include "main.h"
#include "txdb.h"
#include "init.h"
#include "spork.h"

using namespace libzerocoin;

std::map<uint32_t, CBigNum> mapAccumulatorValues;
std::list<uint256> listAccCheckpointsNoDB;

uint32_t ParseChecksum(uint256 nChecksum, CoinDenomination denomination)
{
    
    int pos = distance(zerocoinDenomList.begin(), find(zerocoinDenomList.begin(), zerocoinDenomList.end(), denomination));
    nChecksum = nChecksum >> (32*((zerocoinDenomList.size() - 1) - pos));
    return nChecksum.Get32();
}

uint32_t GetChecksum(const CBigNum &bnValue)
{
    CDataStream ss(SER_GETHASH, 0);
    ss << bnValue;
    uint256 hash = Hash(ss.begin(), ss.end());

    return hash.Get32();
}

bool GetAccumulatorValueFromChecksum(uint32_t nChecksum, bool fMemoryOnly, CBigNum& bnAccValue)
{
    if (mapAccumulatorValues.count(nChecksum)) {
        bnAccValue = mapAccumulatorValues.at(nChecksum);
        return true;
    }

    if (fMemoryOnly)
        return false;

    if (!zerocoinDB->ReadAccumulatorValue(nChecksum, bnAccValue)) {
        bnAccValue = 0;
    }

    return true;
}

bool GetAccumulatorValueFromDB(uint256 nCheckpoint, CoinDenomination denom, CBigNum& bnAccValue)
{
    uint32_t nChecksum = ParseChecksum(nCheckpoint, denom);
    return GetAccumulatorValueFromChecksum(nChecksum, false, bnAccValue);
}

void AddAccumulatorChecksum(const uint32_t nChecksum, const CBigNum &bnValue, bool fMemoryOnly)
{
    if(!fMemoryOnly)
        zerocoinDB->WriteAccumulatorValue(nChecksum, bnValue);
    mapAccumulatorValues.insert(make_pair(nChecksum, bnValue));
}

void DatabaseChecksums(AccumulatorMap& mapAccumulators)
{
    uint256 nCheckpoint = 0;
    for (auto& denom : zerocoinDenomList) {
        CBigNum bnValue = mapAccumulators.GetValue(denom);
        uint32_t nCheckSum = GetChecksum(bnValue);
        AddAccumulatorChecksum(nCheckSum, bnValue, false);
        nCheckpoint = nCheckpoint << 32 | nCheckSum;
    }
}

bool EraseChecksum(uint32_t nChecksum)
{
    
    mapAccumulatorValues.erase(nChecksum);
    return zerocoinDB->EraseAccumulatorValue(nChecksum);
}

bool EraseAccumulatorValues(const uint256& nCheckpointErase, const uint256& nCheckpointPrevious)
{
    for (auto& denomination : zerocoinDenomList) {
        uint32_t nChecksumErase = ParseChecksum(nCheckpointErase, denomination);
        uint32_t nChecksumPrevious = ParseChecksum(nCheckpointPrevious, denomination);

        
        if(nChecksumErase == nChecksumPrevious)
            continue;

        if (!EraseChecksum(nChecksumErase))
            return false;
    }

    return true;
}

bool LoadAccumulatorValuesFromDB(const uint256 nCheckpoint)
{
    for (auto& denomination : zerocoinDenomList) {
        uint32_t nChecksum = ParseChecksum(nCheckpoint, denomination);

        
        CBigNum bnValue;
        if (!zerocoinDB->ReadAccumulatorValue(nChecksum, bnValue)) {
            LogPrint("zero","%s : Missing databased value for checksum %d\n", __func__, nChecksum);
            if (!count(listAccCheckpointsNoDB.begin(), listAccCheckpointsNoDB.end(), nCheckpoint))
                listAccCheckpointsNoDB.push_back(nCheckpoint);
            return false;
        }
        mapAccumulatorValues.insert(make_pair(nChecksum, bnValue));
    }
    return true;
}


bool EraseCheckpoints(int nStartHeight, int nEndHeight)
{
    if (chainActive.Height() < nStartHeight)
        return false;

    nEndHeight = min(chainActive.Height(), nEndHeight);

    CBlockIndex* pindex = chainActive[nStartHeight];
    uint256 nCheckpointPrev = pindex->pprev->nAccumulatorCheckpoint;

    
    list<uint32_t> listCheckpointsPrev;
    for (auto denom : zerocoinDenomList)
        listCheckpointsPrev.emplace_back(ParseChecksum(nCheckpointPrev, denom));

    while (true) {
        uint256 nCheckpointDelete = pindex->nAccumulatorCheckpoint;

        for (auto denom : zerocoinDenomList) {
            uint32_t nChecksumDelete = ParseChecksum(nCheckpointDelete, denom);
            if (count(listCheckpointsPrev.begin(), listCheckpointsPrev.end(), nCheckpointDelete))
                continue;
            EraseChecksum(nChecksumDelete);
        }
        LogPrintf("%s : erasing checksums for block %d\n", __func__, pindex->nHeight);

        if (pindex->nHeight + 1 <= nEndHeight)
            pindex = chainActive.Next(pindex);
        else
            break;
    }

    return true;
}


bool CalculateAccumulatorCheckpoint(int nHeight, uint256& nCheckpoint)
{
    if (nHeight < Params().Zerocoin_StartHeight()) {
        nCheckpoint = 0;
        return true;
    }

    
    if (nHeight % 10 != 0) {
        nCheckpoint = chainActive[nHeight - 1]->nAccumulatorCheckpoint;
        return true;
    }

    
    AccumulatorMap mapAccumulators;
    if (!mapAccumulators.Load(chainActive[nHeight - 1]->nAccumulatorCheckpoint)) {
        if (chainActive[nHeight - 1]->nAccumulatorCheckpoint == 0) {
            
            mapAccumulators.Reset();
        } else {
            LogPrintf("%s: failed to reset to previous checkpoint\n", __func__);
            return false;
        }
    }

    
    bool fFilterInvalid = false;

    
    int nTotalMintsFound = 0;
    CBlockIndex *pindex = chainActive[nHeight - 20];

    while (pindex->nHeight < nHeight - 10) {
        
        if (ShutdownRequested()) {
            return false;
        }

        
        if (pindex->nHeight < Params().Zerocoin_StartHeight()) {
            pindex = chainActive[pindex->nHeight + 1];
            continue;
        }

        
        CBlock block;
        if(!ReadBlockFromDisk(block, pindex)) {
            LogPrint("zero","%s: failed to read block from disk\n", __func__);
            return false;
        }

        std::list<PublicCoin> listPubcoins;
        if (!BlockToPubcoinList(block, listPubcoins, fFilterInvalid)) {
            LogPrint("zero","%s: failed to get zerocoin mintlist from block %n\n", __func__, pindex->nHeight);
            return false;
        }

        nTotalMintsFound += listPubcoins.size();
        LogPrint("zero", "%s found %d mints\n", __func__, listPubcoins.size());

        
        for (const PublicCoin pubcoin : listPubcoins) {
            if(!mapAccumulators.Accumulate(pubcoin, true)) {
                LogPrintf("%s: failed to add pubcoin to accumulator at height %n\n", __func__, pindex->nHeight);
                return false;
            }
        }
        pindex = chainActive.Next(pindex);
    }

    
    if (nTotalMintsFound == 0) {
        nCheckpoint = chainActive[nHeight - 1]->nAccumulatorCheckpoint;
    }
    else
        nCheckpoint = mapAccumulators.GetCheckpoint();

    
    DatabaseChecksums(mapAccumulators);

    LogPrint("zero", "%s checkpoint=%s\n", __func__, nCheckpoint.GetHex());
    return true;
}

bool InvalidCheckpointRange(int nHeight)
{
    return false;
}

bool GenerateAccumulatorWitness(const PublicCoin &coin, Accumulator& accumulator, AccumulatorWitness& witness, int nSecurityLevel, int& nMintsAdded, string& strError)
{
    uint256 txid;
    if (!zerocoinDB->ReadCoinMint(coin.getValue(), txid)) {
        LogPrint("zero","%s failed to read mint from db\n", __func__);
        return false;
    }

    CTransaction txMinted;
    uint256 hashBlock;
    if (!GetTransaction(txid, txMinted, hashBlock)) {
        LogPrint("zero","%s failed to read tx\n", __func__);
        return false;
    }

    int nHeightMintAdded= mapBlockIndex[hashBlock]->nHeight;
    uint256 nCheckpointBeforeMint = 0;
    CBlockIndex* pindex = chainActive[nHeightMintAdded];
    int nChanges = 0;

    
    
    while (pindex->nHeight < chainActive.Tip()->nHeight - 1) {
        if (pindex->nHeight == nHeightMintAdded) {
            pindex = chainActive[pindex->nHeight + 1];
            continue;
        }

        
        if (pindex->nHeight % 10 == 0) {
            nChanges++;

            if (nChanges == 1) {
                nCheckpointBeforeMint = pindex->nAccumulatorCheckpoint;
                break;
            }
        }
        pindex = chainActive.Next(pindex);
    }

    
    int nAccStartHeight = nHeightMintAdded - (nHeightMintAdded % 10);

    
    CBigNum bnAccValue = 0;
    if (GetAccumulatorValueFromDB(nCheckpointBeforeMint, coin.getDenomination(), bnAccValue)) {
        if (bnAccValue > 0) {
            accumulator.setValue(bnAccValue);
            witness.resetValue(accumulator, coin);
        }
    }

    
    
    
    if (nSecurityLevel < 100) {
        
        nSecurityLevel += CBigNum::randBignum(10).getint();

        
        if (nSecurityLevel >= 100)
            nSecurityLevel = 99;
    }

    
    pindex = chainActive[nAccStartHeight];
    int nChainHeight = chainActive.Height();
    int nHeightStop = nChainHeight % 10;
    nHeightStop = nChainHeight - nHeightStop - 20; 
    int nCheckpointsAdded = 0;
    nMintsAdded = 0;
    while (pindex->nHeight < nHeightStop + 1) {
        if (pindex->nHeight != nAccStartHeight && pindex->pprev->nAccumulatorCheckpoint != pindex->nAccumulatorCheckpoint)
            ++nCheckpointsAdded;

        
        
        if (!InvalidCheckpointRange(pindex->nHeight) && (pindex->nHeight >= nHeightStop || (nSecurityLevel != 100 && nCheckpointsAdded >= nSecurityLevel))) {
            uint32_t nChecksum = ParseChecksum(chainActive[pindex->nHeight + 10]->nAccumulatorCheckpoint, coin.getDenomination());
            CBigNum bnAccValue = 0;
            if (!zerocoinDB->ReadAccumulatorValue(nChecksum, bnAccValue)) {
                LogPrintf("%s : failed to find checksum in database for accumulator\n", __func__);
                return false;
            }
            accumulator.setValue(bnAccValue);
            break;
        }

        
        if (pindex->MintedDenomination(coin.getDenomination())) {
            
            CBlock block;
            if(!ReadBlockFromDisk(block, pindex)) {
                LogPrintf("%s: failed to read block from disk while adding pubcoins to witness\n", __func__);
                return false;
            }

            list<PublicCoin> listPubcoins;
            if(!BlockToPubcoinList(block, listPubcoins, true)) {
                LogPrintf("%s: failed to get zerocoin mintlist from block %n\n", __func__, pindex->nHeight);
                return false;
            }

            
            for (const PublicCoin pubcoin : listPubcoins) {
                if (pubcoin.getDenomination() != coin.getDenomination())
                    continue;

                if (pindex->nHeight == nHeightMintAdded && pubcoin.getValue() == coin.getValue())
                    continue;

                witness.addRawValue(pubcoin.getValue());
                ++nMintsAdded;
            }
        }

        pindex = chainActive[pindex->nHeight + 1];
    }

    if (nMintsAdded < Params().Zerocoin_RequiredAccumulation()) {
        strError = _(strprintf("Less than %d mints added, unable to create spend", Params().Zerocoin_RequiredAccumulation()).c_str());
        LogPrintf("%s : %s\n", __func__, strError);
        return false;
    }

    
    int nZerocoinStartHeight = GetZerocoinStartHeight();
    pindex = chainActive[nZerocoinStartHeight];
    while (pindex->nHeight < nAccStartHeight) {
        nMintsAdded += count(pindex->vMintDenominationsInBlock.begin(), pindex->vMintDenominationsInBlock.end(), coin.getDenomination());
        pindex = chainActive[pindex->nHeight + 1];
    }

    LogPrint("zero","%s : %d mints added to witness\n", __func__, nMintsAdded);
    return true;
}