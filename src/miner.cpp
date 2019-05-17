






#include "miner.h"

#include "amount.h"
#include "hash.h"
#include "main.h"
#include "masternode-sync.h"
#include "net.h"
#include "pow.h"
#include "primitives/block.h"
#include "script/sign.h"
#include "timedata.h"
#include "util.h"
#include "utilmoneystr.h"
#ifdef ENABLE_WALLET
#include "wallet.h"
#endif
#include "masternode-payments.h"
#include "accumulators.h"
#include "spork.h"

#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>

using namespace std;

















    CMutableTransaction originalRewardTx;

    ByteCodeExecResult bceResult;
    uint64_t minGasPrice = 1;
    uint64_t hardBlockGasLimit;
    uint64_t softBlockGasLimit;
    uint64_t txGasLimit;





class COrphan
{
public:
    const CTransaction* ptx;
    set<uint256> setDependsOn;
    CFeeRate feeRate;
    double dPriority;

    COrphan(const CTransaction* ptxIn) : ptx(ptxIn), feeRate(0), dPriority(0)
    {
    }
};

uint64_t nLastBlockTx = 0;
uint64_t nLastBlockSize = 0;
int64_t nLastCoinStakeSearchInterval = 0;


typedef boost::tuple<double, CFeeRate, const CTransaction*> TxPriority;
class TxPriorityCompare
{
    bool byFee;

public:
    TxPriorityCompare(bool _byFee) : byFee(_byFee) {}

    bool operator()(const TxPriority& a, const TxPriority& b)
    {
        if (byFee) {
            if (a.get<1>() == b.get<1>())
                return a.get<0>() < b.get<0>();
            return a.get<1>() < b.get<1>();
        } else {
            if (a.get<0>() == b.get<0>())
                return a.get<1>() < b.get<1>();
            return a.get<0>() < b.get<0>();
        }
    }
};

void UpdateTime(CBlockHeader* pblock, const CBlockIndex* pindexPrev)
{
    pblock->nTime = std::max(pindexPrev->GetMedianTimePast() + 1, GetAdjustedTime());

    
    if (Params().AllowMinDifficultyBlocks()) {
        pblock->nBits = GetNextWorkRequired(pindexPrev, pblock);
#ifdef  POW_IN_POS_PHASE
        if (pindexPrev->nHeight >= Params().POW_Start_BLOCK_In_POS() - 1) {
            pblock->nBits2 = GetNextPowWorkRequired(pindexPrev, pblock);
        }
#endif
    }
}

CBlockTemplate* CreateNewPowBlock(CBlockIndex* pindexPrev, CWallet* pwallet)
{
    CPubKey pubkey;
    CReserveKey reservekey(pwallet);
    
    if (!reservekey.GetReservedKey(pubkey))
        return NULL;

    CScript scriptPubKey = CScript() << ToByteVector(pubkey) << OP_CHECKSIG;

    
    unique_ptr<CBlockTemplate> pblocktemplate(new CBlockTemplate());
    if (!pblocktemplate.get())
        return NULL;

    if (!ReadBlockFromDisk(pblocktemplate->block, pindexPrev))
        return NULL;
    
    CBlock* pblock = &pblocktemplate->block; 

    if (!pblock->IsProofOfStake())
        return NULL;

    CAmount nTxFees = 0;

    
    CMutableTransaction txNew;
    txNew.vin.resize(1);
    txNew.vin[0].prevout.SetNull();
    txNew.vout.resize(1);
    txNew.vout[0].scriptPubKey = scriptPubKey;
    
    pblock->vtx[0] = txNew;

    
    pblock->payee.clear();

    int nBlockSigOps = 100;

    pblocktemplate->vTxFees.push_back(-1);

    {
        LOCK2(cs_main, mempool.cs);

        CCoinsViewCache view(pcoinsTip);

        int index = 0;
        for (const CTransaction tx : pblock->vtx) {
            unsigned int nMaxBlockSigOps = MAX_BLOCK_SIGOPS_CURRENT;
            unsigned int nTxSigOps = GetLegacySigOpCount(tx);
            if (nBlockSigOps + nTxSigOps >= nMaxBlockSigOps)
                continue;

            nTxSigOps += GetP2SHSigOpCount(tx, view);
            if (nBlockSigOps + nTxSigOps >= nMaxBlockSigOps)
                continue;

            pblocktemplate->vTxFees[index] = nTxFees;
        }
    }

    return pblocktemplate.release();
}


void RebuildRefundTransaction(CBlock *pblock,CAmount &nFees)
{
    CMutableTransaction contrTx(originalRewardTx);

    contrTx.vout[2].nValue -= bceResult.refundSender;

    
    int i = contrTx.vout.size();
    contrTx.vout.resize(contrTx.vout.size() + bceResult.refundOutputs.size());
    for (CTxOut &vout : bceResult.refundOutputs)
    {
        contrTx.vout[i] = vout;
        i++;
    }
    pblock->vtx[1] = std::move(contrTx);

}


bool AttemptToAddContractToBlock(const CTransaction &iter, uint64_t minGasPrice,CBlockTemplate *pblockTemplate,uint64_t &nBlockSize,int &nBlockSigOps,uint64_t &nBlockTx,CCoinsViewCache &view,CAmount &nFees)
{

    



    if (GetBoolArg("-disablecontractstaking", false))
    {
        return false;
    }

    uint256 oldHashStateRoot;
    uint256 oldHashUTXORoot;

    GetState(oldHashStateRoot, oldHashUTXORoot);

    
    uint64_t nBlockWeight = nBlockSize;
    int nBlockSigOpsCost = nBlockSigOps;

    CBlock *pblock = &pblockTemplate->block;

    TesraTxConverter convert(iter, NULL, &pblock->vtx);

    ExtractTesraTX resultConverter;
    if(!convert.extractionTesraTransactions(resultConverter)){
        
        
        return false;
    }
    std::vector<TesraTransaction> tesraTransactions = resultConverter.first;
    dev::u256 txGas = 0;
    for(TesraTransaction tesraTransaction : tesraTransactions){
        txGas += tesraTransaction.gas();
        if(txGas > txGasLimit) {
            
            return false;
        }

        if(bceResult.usedGas + tesraTransaction.gas() > softBlockGasLimit){
            
            return false;
        }
        if(tesraTransaction.gasPrice() < minGasPrice){
            
            return false;
        }
    }
    
    ByteCodeExec exec(*pblock, tesraTransactions, hardBlockGasLimit);
    if(!exec.performByteCode()){
        
        UpdateState(oldHashStateRoot,oldHashUTXORoot);
        return false;
    }



    ByteCodeExecResult testExecResult;
    if(!exec.processingResults(testExecResult)){
        UpdateState(oldHashStateRoot,oldHashUTXORoot);
        return false;
    }


    if(bceResult.usedGas + testExecResult.usedGas > softBlockGasLimit){
        
        UpdateState(oldHashStateRoot,oldHashUTXORoot);

        return false;
    }


    
    nBlockSize += ::GetSerializeSize(iter, SER_NETWORK, PROTOCOL_VERSION);
    nBlockSigOpsCost += GetLegacySigOpCount(iter);
    


    for (CTransaction &t : testExecResult.valueTransfers) {
        nBlockWeight += ::GetSerializeSize(t, SER_NETWORK, PROTOCOL_VERSION);;
        nBlockSigOpsCost += GetLegacySigOpCount(t);
    }


    int proofTx = pblock->IsProofOfStake() ? 1 : 0;

    

    
    nBlockSigOpsCost -= GetLegacySigOpCount(pblock->vtx[proofTx]);



    
    CMutableTransaction contrTx(pblock->vtx[proofTx]);
    
    int i = contrTx.vout.size();
    contrTx.vout.resize(contrTx.vout.size()+testExecResult.refundOutputs.size());
    for(CTxOut& vout : testExecResult.refundOutputs){
        contrTx.vout[i]=vout;
        i++;
    }
    nBlockSigOpsCost += GetLegacySigOpCount(contrTx);
    


    
    if (nBlockSigOpsCost > (int)MAX_BLOCK_SIGOPS_CURRENT ||
            nBlockWeight > GetArg("-blockmaxsize", DEFAULT_BLOCK_MAX_SIZE)) {
        
        UpdateState(oldHashStateRoot,oldHashUTXORoot);

        return false;
    }

    

    
    bceResult.usedGas += testExecResult.usedGas;
    bceResult.refundSender += testExecResult.refundSender;
    bceResult.refundOutputs.insert(bceResult.refundOutputs.end(), testExecResult.refundOutputs.begin(), testExecResult.refundOutputs.end());
    bceResult.valueTransfers = std::move(testExecResult.valueTransfers);


    pblock->vtx.emplace_back(iter);


    


    pblockTemplate->vTxFees.push_back(view.GetValueIn(iter) - iter.GetValueOut());

    pblockTemplate->vTxSigOps.push_back(GetLegacySigOpCount(iter));

    nBlockSize += ::GetSerializeSize(iter, SER_NETWORK, PROTOCOL_VERSION);
    ++nBlockTx;
    nBlockSigOps += GetLegacySigOpCount(iter);

    nFees += view.GetValueIn(iter) - iter.GetValueOut();



    for (CTransaction &t : bceResult.valueTransfers) {
        pblock->vtx.emplace_back(t);
        nBlockSize += ::GetSerializeSize(t, SER_NETWORK, PROTOCOL_VERSION);
        nBlockSigOps += GetLegacySigOpCount(t);
        ++nBlockTx;
    }
    
    nBlockSigOps -= GetLegacySigOpCount(pblock->vtx[proofTx]);

    RebuildRefundTransaction(pblock,nFees);

    nBlockSigOps += GetLegacySigOpCount(pblock->vtx[proofTx]);

    bceResult.valueTransfers.clear();

    return true;
}

CBlockTemplate* CreateNewBlock(const CScript& scriptPubKeyIn, CWallet* pwallet, bool fProofOfStake)
{
    CReserveKey reservekey(pwallet);

    
    
    originalRewardTx.nLockTime = 0;
    originalRewardTx.nVersion = 0;
    originalRewardTx.vin.clear();
    originalRewardTx.vout.clear();

    bceResult.refundSender = 0;
    bceResult.usedGas = 0;
    bceResult.refundOutputs.clear();
    bceResult.valueTransfers.clear();

    minGasPrice = 1;
    hardBlockGasLimit = 0;
    softBlockGasLimit = 0;
    txGasLimit = 0;
    

    unique_ptr<CBlockTemplate> pblocktemplate(new CBlockTemplate());
    if (!pblocktemplate.get())
        return NULL;
    CBlock* pblock = &pblocktemplate->block; 

    
    
    if (Params().MineBlocksOnDemand())
        pblock->nVersion = GetArg("-blockversion", pblock->nVersion);

    
    bool fZerocoinActive = (GetAdjustedTime() >= Params().Zerocoin_StartTime()) && (chainActive.Height() + 1 >= Params().Zerocoin_StartHeight());
    bool fContractActive = (chainActive.Height() + 1 >= Params().Contract_StartHeight());
    if (chainActive.Height() + 1 <= Params().LAST_POW_BLOCK()) {
        pblock->nVersion = POW_VERSION;
    } else if (!fZerocoinActive)
        pblock->nVersion = POS_VERSION;
    else if (!fContractActive)
        pblock->nVersion = ZEROCOIN_VERSION;
    else
        pblock->nVersion = SMART_CONTRACT_VERSION;

    
    CMutableTransaction txNew;
    CMutableTransaction txCoinStake;
   

    txNew.vin.resize(1);
    txNew.vin[0].prevout.SetNull();
    txNew.vout.resize(1);
    txNew.vout[0].scriptPubKey = scriptPubKeyIn;
    pblock->vtx.push_back(txNew);
    pblocktemplate->vTxFees.push_back(-1);   
    pblocktemplate->vTxSigOps.push_back(-1); 
    

    
    static int64_t nLastCoinStakeSearchTime = GetAdjustedTime(); 

    if (fProofOfStake) {
        boost::this_thread::interruption_point();
        pblock->nTime = GetAdjustedTime();
        CBlockIndex* pindexPrev = chainActive.Tip();
        pblock->nBits = GetNextWorkRequired(pindexPrev, pblock);
#ifdef  POW_IN_POS_PHASE
        pblock->nBits2 = GetNextPowWorkRequired(pindexPrev, pblock);
#endif
        int64_t nSearchTime = pblock->nTime; 
        bool fStakeFound = false;
        if (nSearchTime >= nLastCoinStakeSearchTime) {
            unsigned int nTxNewTime = 0;
            if (pwallet->CreateCoinStake(*pwallet, pblock, nSearchTime - nLastCoinStakeSearchTime, txCoinStake, nTxNewTime)) {
                pblock->nTime = nTxNewTime;
                pblock->vtx[0].vout[0].SetEmpty();
                pblock->vtx.push_back(CTransaction(txCoinStake));
                originalRewardTx = txCoinStake;
                fStakeFound = true;
            }
            nLastCoinStakeSearchInterval = nSearchTime - nLastCoinStakeSearchTime;
            nLastCoinStakeSearchTime = nSearchTime;
        }

        if (!fStakeFound)
            return NULL;
    }

    
      minGasPrice = GetMinGasPrice(chainActive.Height()+1);
      if (GetBoolArg("-staker-min-tx-gas-price", false))
      {
          CAmount stakerMinGasPrice;
          string strMinxGasPrice = "";
          strMinxGasPrice = GetArg("-staker-min-tx-gas-price", strMinxGasPrice);
          if (ParseMoney(strMinxGasPrice, stakerMinGasPrice))
          {
              minGasPrice = std::max(minGasPrice, (uint64_t)stakerMinGasPrice);
          }
      }
      hardBlockGasLimit = GetBlockGasLimit(chainActive.Height()+1);
      softBlockGasLimit = GetArg("-staker-soft-block-gas-limit", hardBlockGasLimit);
      softBlockGasLimit = std::min(softBlockGasLimit, hardBlockGasLimit);
      txGasLimit = GetArg("-staker-max-tx-gas-limit", softBlockGasLimit);

      

      uint256 oldHashStateRoot, oldHashUTXORoot;
      GetState(oldHashStateRoot, oldHashUTXORoot);


      










    
    unsigned int nBlockMaxSize = GetArg("-blockmaxsize", DEFAULT_BLOCK_MAX_SIZE);
    
    unsigned int nBlockMaxSizeNetwork = MAX_BLOCK_SIZE_CURRENT;
    nBlockMaxSize = std::max((unsigned int)1000, std::min((nBlockMaxSizeNetwork - 1000), nBlockMaxSize));

    
    
    unsigned int nBlockPrioritySize = GetArg("-blockprioritysize", DEFAULT_BLOCK_PRIORITY_SIZE);
    nBlockPrioritySize = std::min(nBlockMaxSize, nBlockPrioritySize);

    
    
    unsigned int nBlockMinSize = GetArg("-blockminsize", DEFAULT_BLOCK_MIN_SIZE);
    nBlockMinSize = std::min(nBlockMaxSize, nBlockMinSize);

    
    CAmount nFees = 0;

    {
        LOCK2(cs_main, mempool.cs);

        CBlockIndex* pindexPrev = chainActive.Tip();
        const int nHeight = pindexPrev->nHeight + 1;
        CCoinsViewCache view(pcoinsTip);

        
        list<COrphan> vOrphan; 
        map<uint256, vector<COrphan*> > mapDependers;
        bool fPrintPriority = GetBoolArg("-printpriority", false);

        
        vector<TxPriority> vecPriority;
        vecPriority.reserve(mempool.mapTx.size());
        for (map<uint256, CTxMemPoolEntry>::iterator mi = mempool.mapTx.begin();
             mi != mempool.mapTx.end(); ++mi) {
            const CTransaction& tx = mi->second.GetTx();
            if (tx.IsCoinBase() || tx.IsCoinStake() || !IsFinalTx(tx, nHeight)){
                continue;
            }

            if (GetAdjustedTime() > GetSporkValue(SPORK_16_ZEROCOIN_MAINTENANCE_MODE) && tx.ContainsZerocoins())
                continue;

            COrphan* porphan = NULL;
            double dPriority = 0;
            CAmount nTotalIn = 0;
            bool fMissingInputs = false;
            for (const CTxIn& txin : tx.vin) {
                
                if (tx.IsZerocoinSpend()) {
                    nTotalIn = tx.GetZerocoinSpent();
                    break;
                }

                
                if (!view.HaveCoins(txin.prevout.hash)) {
                    
                    
                    
                    if (!mempool.mapTx.count(txin.prevout.hash)) {
                        LogPrintf("ERROR: mempool transaction missing input\n");
                        if (fDebug) assert("mempool transaction missing input" == 0);
                        fMissingInputs = true;
                        if (porphan)
                            vOrphan.pop_back();
                        break;
                    }

                    
                    if (!porphan) {
                        
                        vOrphan.push_back(COrphan(&tx));
                        porphan = &vOrphan.back();
                    }
                    mapDependers[txin.prevout.hash].push_back(porphan);
                    porphan->setDependsOn.insert(txin.prevout.hash);
                    nTotalIn += mempool.mapTx[txin.prevout.hash].GetTx().vout[txin.prevout.n].nValue;
                    continue;
                }

                
                if (mapInvalidOutPoints.count(txin.prevout)) {
                    LogPrintf("%s : found invalid input %s in tx %s", __func__, txin.prevout.ToString(), tx.GetHash().ToString());
                    fMissingInputs = true;
                    break;
                }

                const CCoins* coins = view.AccessCoins(txin.prevout.hash);
                assert(coins);

                CAmount nValueIn = coins->vout[txin.prevout.n].nValue;
                nTotalIn += nValueIn;

                int nConf = nHeight - coins->nHeight;

                dPriority += (double)nValueIn * nConf;
            }
            if (fMissingInputs) continue;

            
            unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
            dPriority = tx.ComputePriority(dPriority, nTxSize);

            uint256 hash = tx.GetHash();
            mempool.ApplyDeltas(hash, dPriority, nTotalIn);

            CFeeRate feeRate(nTotalIn - tx.GetValueOut(), nTxSize);

            if (porphan) {
                porphan->dPriority = dPriority;
                porphan->feeRate = feeRate;
            } else
                vecPriority.push_back(TxPriority(dPriority, feeRate, &mi->second.GetTx()));
        }

        
        uint64_t nBlockSize = 1000;
        uint64_t nBlockTx = 0;
        int nBlockSigOps = 100;
        bool fSortedByFee = (nBlockPrioritySize <= 0);

        TxPriorityCompare comparer(fSortedByFee);
        std::make_heap(vecPriority.begin(), vecPriority.end(), comparer);

        vector<CBigNum> vBlockSerials;
        vector<CBigNum> vTxSerials;

        
        while (!vecPriority.empty()) {



            
            double dPriority = vecPriority.front().get<0>();
            CFeeRate feeRate = vecPriority.front().get<1>();
            const CTransaction& tx = *(vecPriority.front().get<2>());

            std::pop_heap(vecPriority.begin(), vecPriority.end(), comparer);
            vecPriority.pop_back();

            
            unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
            if (nBlockSize + nTxSize >= nBlockMaxSize)
                continue;

            
            unsigned int nMaxBlockSigOps = MAX_BLOCK_SIGOPS_CURRENT;
            unsigned int nTxSigOps = GetLegacySigOpCount(tx);
            if (nBlockSigOps + nTxSigOps >= nMaxBlockSigOps)
                continue;

            
            const uint256& hash = tx.GetHash();
            double dPriorityDelta = 0;
            CAmount nFeeDelta = 0;
            mempool.ApplyDeltas(hash, dPriorityDelta, nFeeDelta);
            if (!tx.IsZerocoinSpend() && fSortedByFee && (dPriorityDelta <= 0) && (nFeeDelta <= 0) && (feeRate < ::minRelayTxFee) && (nBlockSize + nTxSize >= nBlockMinSize))
                continue;

            
            
            if (!fSortedByFee &&
                ((nBlockSize + nTxSize >= nBlockPrioritySize) || !AllowFree(dPriority))) {
                fSortedByFee = true;
                comparer = TxPriorityCompare(fSortedByFee);
                std::make_heap(vecPriority.begin(), vecPriority.end(), comparer);
            }

            if (!view.HaveInputs(tx))
                continue;

            
            if (tx.IsZerocoinSpend()) {
                int nHeightTx = 0;
                if (IsTransactionInChain(tx.GetHash(), nHeightTx))
                    continue;

                bool fDoubleSerial = false;
                for (const CTxIn txIn : tx.vin) {
                    if (txIn.scriptSig.IsZerocoinSpend()) {
                        libzerocoin::CoinSpend spend = TxInToZerocoinSpend(txIn);
                        if (!spend.HasValidSerial(Params().Zerocoin_Params()))
                            fDoubleSerial = true;
                        if (count(vBlockSerials.begin(), vBlockSerials.end(), spend.getCoinSerialNumber()))
                            fDoubleSerial = true;
                        if (count(vTxSerials.begin(), vTxSerials.end(), spend.getCoinSerialNumber()))
                            fDoubleSerial = true;
                        if (fDoubleSerial)
                            break;
                        vTxSerials.emplace_back(spend.getCoinSerialNumber());
                    }
                }
                
                if (fDoubleSerial)
                    continue;
            }

            CAmount nTxFees = view.GetValueIn(tx) - tx.GetValueOut();

            nTxSigOps += GetP2SHSigOpCount(tx, view);
            if (nBlockSigOps + nTxSigOps >= nMaxBlockSigOps)
                continue;

            
            
            
            CValidationState state;
            if (!CheckInputs(tx, state, view, true, MANDATORY_SCRIPT_VERIFY_FLAGS, true))
                continue;


            
            
            if (tx.HasCreateOrCall()) {
                if(!AttemptToAddContractToBlock(tx, minGasPrice,pblocktemplate.get(),nBlockSize,nBlockSigOps,nBlockTx,view, nFees))
                {
                    std::pop_heap(vecPriority.begin(), vecPriority.end(), comparer);
                    vecPriority.pop_back();
                    continue;
                }

            } else {
                pblock->vtx.push_back(tx);
                pblocktemplate->vTxFees.push_back(nTxFees);
                pblocktemplate->vTxSigOps.push_back(nTxSigOps);
                nBlockSize += nTxSize;
                ++nBlockTx;
                nBlockSigOps += nTxSigOps;
                nFees += nTxFees;


                if (fPrintPriority) {
                    LogPrintf("priority %.1f fee %s txid %s\n",
                        dPriority, feeRate.ToString(), tx.GetHash().ToString());
                }

            }
            for (const CBigNum bnSerial : vTxSerials)
                vBlockSerials.emplace_back(bnSerial);

            

            CTxUndo txundo;
            UpdateCoins(tx, state, view, txundo, nHeight);



            
            if (mapDependers.count(hash)) {
                BOOST_FOREACH (COrphan* porphan, mapDependers[hash]) {
                    if (!porphan->setDependsOn.empty()) {
                        porphan->setDependsOn.erase(hash);
                        if (porphan->setDependsOn.empty()) {
                            vecPriority.push_back(TxPriority(porphan->dPriority, porphan->feeRate, porphan->ptx));
                            std::push_heap(vecPriority.begin(), vecPriority.end(), comparer);
                        }
                    }
                }
            }
        }

        if (!fProofOfStake) {
            
            FillBlockPayee(txNew, GetBlockValue(pindexPrev->nHeight), nFees, fProofOfStake);

            
            if (txNew.vout.size() > 1) {
                pblock->payee = txNew.vout[1].scriptPubKey;
            }
        } else {


            
            
            
            if(chainActive.Tip()->nHeight >= Params().Contract_StartHeight()) {

                uint256 hashStateRoot, hashUTXORoot;
                GetState(hashStateRoot, hashUTXORoot);

                LogPrintf("CreateNewBlock: Populate state and utxo before RebuildRefundTransaction:\nstateroot:%s\nutxoRoot: %s\n", hashStateRoot.GetHex().c_str(), hashUTXORoot.GetHex().c_str());
                CScript contract = CScript() << ParseHex(hashStateRoot.GetHex().c_str()) << ParseHex(hashUTXORoot.GetHex().c_str()) << OP_VM_STATE;

                UpdateState(oldHashStateRoot, oldHashUTXORoot);

                RebuildRefundTransaction(pblock,nFees);
                txCoinStake = pblock->vtx[1];

                txCoinStake.vout[1] = CTxOut(0, contract);

            }


            
            if (pblock->nVersion < SMART_CONTRACT_VERSION)
                txCoinStake.vout[1].nValue += nFees;
            else
                txCoinStake.vout[2].nValue += nFees;



            
            for (size_t nIn = 0; nIn < txCoinStake.vin.size(); nIn++) {
                const CWalletTx* pcoin = pwallet->GetWalletTx(txCoinStake.vin[nIn].prevout.hash);

                if (!SignSignature(*pwallet, *pcoin, txCoinStake, nIn)) {
                    LogPrintStr("CreateCoinStake : failed to re-sign coinstake");
                    return NULL;
                }
            }

            pblock->vtx[1] = CTransaction(txCoinStake);
        }

        nLastBlockTx = nBlockTx;
        nLastBlockSize = nBlockSize;
        LogPrintf("CreateNewBlock(): total size %u\n", nBlockSize);







        

        
        if (!fProofOfStake) {
            pblock->vtx[0] = txNew;
            pblocktemplate->vTxFees[0] = -nFees;
        }
        pblock->vtx[0].vin[0].scriptSig = CScript() << nHeight << OP_0;

        
        pblock->hashPrevBlock = pindexPrev->GetBlockHash();
        if (!fProofOfStake)
            UpdateTime(pblock, pindexPrev);
        pblock->nBits = GetNextWorkRequired(pindexPrev, pblock);
        if (!fProofOfStake)
            pblock->nNonce = 0;
        uint256 nCheckpoint = 0;
        if(fZerocoinActive && !CalculateAccumulatorCheckpoint(nHeight, nCheckpoint)){
            LogPrintf("%s: failed to get accumulator checkpoint\n", __func__);
        }
        pblock->nAccumulatorCheckpoint = nCheckpoint;
        pblocktemplate->vTxSigOps[0] = GetLegacySigOpCount(pblock->vtx[0]);

        CValidationState state;
        if (!TestBlockValidity(state, *pblock, pindexPrev, false, false)) {
            LogPrintf("CreateNewBlock() : TestBlockValidity failed\n");
            mempool.clear();
            return NULL;
        }
    }

    return pblocktemplate.release();
}

void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce)
{
    
    static uint256 hashPrevBlock;
    if (hashPrevBlock != pblock->hashPrevBlock) {
        nExtraNonce = 0;
        hashPrevBlock = pblock->hashPrevBlock;
    }
    ++nExtraNonce;
    unsigned int nHeight = pindexPrev->nHeight + 1; 
    CMutableTransaction txCoinbase(pblock->vtx[0]);
    txCoinbase.vin[0].scriptSig = (CScript() << nHeight << CScriptNum(nExtraNonce)) + COINBASE_FLAGS;
    assert(txCoinbase.vin[0].scriptSig.size() <= 100);

    pblock->vtx[0] = txCoinbase;
    pblock->hashMerkleRoot = pblock->BuildMerkleTree();
}

#ifdef ENABLE_WALLET




double dHashesPerSec = 0.0;
int64_t nHPSTimerStart = 0;

CBlockTemplate* CreateNewBlockWithKey(CReserveKey& reservekey, CWallet* pwallet, bool fProofOfStake)
{
    CPubKey pubkey;
    if (!reservekey.GetReservedKey(pubkey))
        return NULL;

    CScript scriptPubKey = CScript() << ToByteVector(pubkey) << OP_CHECKSIG;
    return CreateNewBlock(scriptPubKey, pwallet, fProofOfStake);
}

bool ProcessBlockFound(CBlock* pblock, CWallet& wallet, CReserveKey& reservekey)
{
    LogPrintf("%s\n", pblock->ToString());
    LogPrintf("generated %s\n", FormatMoney(pblock->vtx[0].vout[0].nValue));

    
    {
        LOCK(cs_main);
        if (pblock->hashPrevBlock != chainActive.Tip()->GetBlockHash())
            return error("TESRAMiner : generated block is stale");
    }

    
    reservekey.KeepKey();

    
    {
        LOCK(wallet.cs_wallet);
        wallet.mapRequestCount[pblock->GetHash()] = 0;
    }

    
    CValidationState state;
    if (!ProcessNewBlock(state, NULL, pblock))
        return error("TESRAMiner : ProcessNewBlock, block not accepted");

    for (CNode* node : vNodes) {
        node->PushInventory(CInv(MSG_BLOCK, pblock->GetHash()));
    }

    return true;
}

bool fGenerateBitcoins = false;



void BitcoinMiner(CWallet* pwallet, bool fProofOfStake)
{
    LogPrintf("TESRA %s Miner started\n", fProofOfStake ? "POS" : "POW");
    SetThreadPriority(THREAD_PRIORITY_LOWEST);
    RenameThread("tesra-miner");

    
    CReserveKey reservekey(pwallet);
    unsigned int nExtraNonce = 0;

    
    static bool fMintableCoins = false;
    static int nMintableLastCheck = 0;


    while (fGenerateBitcoins || fProofOfStake) {
        if (chainActive.Height() < Params().LAST_POW_BLOCK() || fProofOfStake) {
            if (fProofOfStake) {
                if ((GetTime() - nMintableLastCheck > 5 * 60)) 
                {
                    nMintableLastCheck = GetTime();
                    fMintableCoins = pwallet->MintableCoins();
                }

                if (chainActive.Tip()->nHeight < Params().LAST_POW_BLOCK()) {
                    MilliSleep(5000);
                    continue;
                }

                while (vNodes.empty() || pwallet->IsLocked() || !fMintableCoins || (pwallet->GetBalance() > 0 && nReserveBalance >= pwallet->GetBalance()) || !masternodeSync.IsSynced()) {
                    nLastCoinStakeSearchInterval = 0;
                    
                    if (!fMintableCoins) {
                        if (GetTime() - nMintableLastCheck > 1 * 60) 
                        {
                            nMintableLastCheck = GetTime();
                            fMintableCoins = pwallet->MintableCoins();
                        }
                    }
                    MilliSleep(5000);
                    if (!fGenerateBitcoins && !fProofOfStake)
                        continue;
                }

                if (mapHashedBlocks.count(chainActive.Tip()->nHeight)) 
                {
                    if (GetTime() - mapHashedBlocks[chainActive.Tip()->nHeight] < max(pwallet->nHashInterval, (unsigned int)1)) 
                    {
                        MilliSleep(5000);
                        continue;
                    }
                }
            }

            
            
            
            unsigned int nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();
            CBlockIndex* pindexPrev = chainActive.Tip();
            if (!pindexPrev)
                continue;

            unique_ptr<CBlockTemplate> pblocktemplate(CreateNewBlockWithKey(reservekey, pwallet, fProofOfStake));
            if (!pblocktemplate.get())
                continue;

            CBlock* pblock = &pblocktemplate->block;
            IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);

            
            if (fProofOfStake) {
                LogPrintf("CPU POS Miner : proof-of-stake block found %s \n", pblock->GetHash().ToString().c_str());

                if (!pblock->SignBlock(*pwallet)) {
                    LogPrintf("BitcoinMiner(): Signing new block failed \n");
                    continue;
                }

                LogPrintf("CPUMiner : proof-of-stake block was signed %s \n", pblock->GetHash().ToString().c_str());
                SetThreadPriority(THREAD_PRIORITY_NORMAL);
                ProcessBlockFound(pblock, *pwallet, reservekey);
                SetThreadPriority(THREAD_PRIORITY_LOWEST);

                continue;
            }

            LogPrintf("Running TESRAMiner with %u transactions in block (%u bytes)\n", pblock->vtx.size(),
                ::GetSerializeSize(*pblock, SER_NETWORK, PROTOCOL_VERSION));

            
            
            
            int64_t nStart = GetTime();
            uint256 hashTarget = uint256().SetCompact(pblock->nBits);
            while (true) {
                unsigned int nHashesDone = 0;

                uint256 hash;
                while (true) {
                    hash = pblock->GetHash();
                    if (hash <= hashTarget) {
                        
                        SetThreadPriority(THREAD_PRIORITY_NORMAL);
                        LogPrintf("BitcoinMiner:\n");
                        LogPrintf("proof-of-work found  \n  hash: %s  \ntarget: %s\n", hash.GetHex(), hashTarget.GetHex());
                        ProcessBlockFound(pblock, *pwallet, reservekey);
                        SetThreadPriority(THREAD_PRIORITY_LOWEST);

                        
                        
                        if (Params().MineBlocksOnDemand())
                            throw boost::thread_interrupted();

                        break;
                    }
                    pblock->nNonce += 1;
                    nHashesDone += 1;
                    if ((pblock->nNonce & 0xFF) == 0)
                        break;
                }

                
                static int64_t nHashCounter;
                if (nHPSTimerStart == 0) {
                    nHPSTimerStart = GetTimeMillis();
                    nHashCounter = 0;
                } else
                    nHashCounter += nHashesDone;
                if (GetTimeMillis() - nHPSTimerStart > 4000) {
                    static CCriticalSection cs;
                    {
                        LOCK(cs);
                        if (GetTimeMillis() - nHPSTimerStart > 4000) {
                            dHashesPerSec = 1000.0 * nHashCounter / (GetTimeMillis() - nHPSTimerStart);
                            nHPSTimerStart = GetTimeMillis();
                            nHashCounter = 0;
                            static int64_t nLogTime;
                            if (GetTime() - nLogTime > 30 * 60) {
                                nLogTime = GetTime();
                                LogPrintf("hashmeter %6.0f khash/s\n", dHashesPerSec / 1000.0);
                            }
                        }
                    }
                }

                
                boost::this_thread::interruption_point();
                
                if (vNodes.empty() && Params().MiningRequiresPeers())
                    break;
                if (pblock->nNonce >= 0xffff0000)
                    break;
                if (mempool.GetTransactionsUpdated() != nTransactionsUpdatedLast && GetTime() - nStart > 60)
                    break;
                if (pindexPrev != chainActive.Tip())
                    break;

                
                UpdateTime(pblock, pindexPrev);
                if (Params().AllowMinDifficultyBlocks()) {
                    
                    hashTarget.SetCompact(pblock->nBits);
                }
            }
        } else {
            CBlock block;

            CPubKey pubkey;

            static int height = 0;

            CBlockIndex *pindexCurrent = chainActive.Tip();

            
            if (pindexCurrent->nHeight < Params().POW_Start_BLOCK_In_POS() - 1) {
                MilliSleep(5000);
                continue;
            }

#ifdef      POW_IN_POS_PHASE
            if (height != pindexCurrent->nHeight) {
                if(ReadBlockFromDisk(block, pindexCurrent) && block.IsProofOfStake() && reservekey.GetReservedKey(pubkey)) {
                    height = pindexCurrent->nHeight;
                    
                    CMutableTransaction coinBaseTx;

                    CScript scriptPubKey = CScript() << ToByteVector(pubkey) << OP_CHECKSIG;

                    coinBaseTx.vin.resize(1);
                    coinBaseTx.vin[0].prevout.SetNull();
                    coinBaseTx.vout.resize(1);
                    coinBaseTx.vout[0].scriptPubKey = scriptPubKey;
                    coinBaseTx.vout[0].nValue = GetTmpBlockValue(height);

                    block.vtx[0] = coinBaseTx;
                    block.hashMerkleRoot = block.BuildMerkleTree();

                    block.nNonce = 0;
                    block.nBits = block.nBits2;

                    int64_t nStart = GetTime();
                    unsigned int nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();

                    uint256 blockhash = *pindexCurrent->phashBlock;
                    uint256 hashTarget = uint256().SetCompact(block.nBits);
                    while (true) {
                        unsigned int nHashesDone = 0;

                        uint256 hash;
                        while (pindexCurrent == chainActive.Tip()) {
                            hash = HashCryptoNight(BEGIN(block.nVersion), END(block.nAccumulatorCheckpoint));
                            if (hash < hashTarget) {
                                CTmpBlockParams tmpBlockParams;

                                tmpBlockParams.ori_hash = blockhash;
                                tmpBlockParams.nNonce = block.nNonce;
                                tmpBlockParams.coinBaseTx = coinBaseTx;

                                CBlockHeader blockHeader = block.GetBlockHeader();

                                ProcessNewTmpBlockParam(tmpBlockParams, blockHeader);

                                
                                reservekey.KeepKey();
                            }
                            block.nNonce += 1;
                            nHashesDone += 1;
                            if ((block.nNonce & 0xFF) == 0)
                                break;
                        }

                        
                        static int64_t nHashCounter;
                        if (nHPSTimerStart == 0) {
                            nHPSTimerStart = GetTimeMillis();
                            nHashCounter = 0;
                        } else
                            nHashCounter += nHashesDone;
                        if (GetTimeMillis() - nHPSTimerStart > 4000) {
                            static CCriticalSection cs;
                            {
                                LOCK(cs);
                                if (GetTimeMillis() - nHPSTimerStart > 4000) {
                                    dHashesPerSec = 1000.0 * nHashCounter / (GetTimeMillis() - nHPSTimerStart);
                                    nHPSTimerStart = GetTimeMillis();
                                    nHashCounter = 0;
                                    static int64_t nLogTime;
                                    if (GetTime() - nLogTime > 30 * 60) {
                                        nLogTime = GetTime();
                                        LogPrintf("hashmeter %6.0f khash/s\n", dHashesPerSec / 1000.0);
                                    }
                                }
                            }
                        }
                        
                        
                        boost::this_thread::interruption_point();
                        
                        if (vNodes.empty() && Params().MiningRequiresPeers())
                            break;
                        if (block.nNonce >= 0xffff0000)
                            break;
                        if (mempool.GetTransactionsUpdated() != nTransactionsUpdatedLast && GetTime() - nStart > 60)
                            break;
                        if (pindexCurrent != chainActive.Tip())
                            break;
                    }
                } else {
                    MilliSleep(1000);
                }
            } else {
#endif
                MilliSleep(5000);
#ifdef      POW_IN_POS_PHASE
            }
#endif
        }
    }
}

void static ThreadBitcoinMiner(void* parg)
{
    boost::this_thread::interruption_point();
    CWallet* pwallet = (CWallet*)parg;
    try {
        BitcoinMiner(pwallet, false);
        boost::this_thread::interruption_point();
    } catch (std::exception& e) {
        LogPrintf("ThreadBitcoinMiner() exception");
    } catch (...) {
        LogPrintf("ThreadBitcoinMiner() exception");
    }

    LogPrintf("ThreadBitcoinMiner exiting\n");
}

void GenerateBitcoins(bool fGenerate, CWallet* pwallet, int nThreads)
{
    static boost::thread_group* minerThreads = NULL;
    fGenerateBitcoins = fGenerate;

    if (nThreads < 0) {
        
        if (Params().DefaultMinerThreads())
            nThreads = Params().DefaultMinerThreads();
        else
            nThreads = boost::thread::hardware_concurrency();
    }

    if (minerThreads != NULL) {
        minerThreads->interrupt_all();
        delete minerThreads;
        minerThreads = NULL;
    }

    if (nThreads == 0 || !fGenerate)
        return;

    minerThreads = new boost::thread_group();
    for (int i = 0; i < nThreads; i++)
        minerThreads->create_thread(boost::bind(&ThreadBitcoinMiner, pwallet));
}

#endif 
