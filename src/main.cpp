






#include "main.h"

#include "accumulators.h"
#include "addrman.h"
#include "alert.h"
#include "chainparams.h"
#include "checkpoints.h"
#include "checkqueue.h"
#include "init.h"
#include "lz4io.h"
#include "kernel.h"
#include "masternode-budget.h"
#include "masternode-payments.h"
#include "masternodeman.h"
#include "merkleblock.h"
#include "net.h"
#include "obfuscation.h"
#include "pow.h"
#include "spork.h"
#include "sporkdb.h"
#include "swifttx.h"
#include "txdb.h"
#include "txmempool.h"
#include "tmpblocksmempool.h"
#include "ui_interface.h"
#include "util.h"
#include "utilmoneystr.h"

#include "primitives/zerocoin.h"
#include "libzerocoin/Denominations.h"

#include <sstream>

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

using namespace boost;
using namespace std;
using namespace libzerocoin;

#if defined(NDEBUG)
#error "TESRA cannot be compiled without assertions."
#endif


#define SCRIPT_OFFSET 6

#define BIGNUM_SIZE   4
/**
 * Global state
 */

CCriticalSection cs_main;

BlockMap mapBlockIndex;
map<uint256, uint256> mapProofOfStake;
set<pair<COutPoint, unsigned int> > setStakeSeen;
map<unsigned int, unsigned int> mapHashedBlocks;
CChain chainActive;
CBlockIndex* pindexBestHeader = NULL;
int64_t nTimeBestReceived = 0;
CWaitableCriticalSection csBestBlock;
CConditionVariable cvBlockChange;
int nScriptCheckThreads = 0;
bool fImporting = false;
bool fReindex = false;
bool fTxIndex = true;
bool fAddrIndex = false;
bool fLogEvents = true;
bool fIsBareMultisigStd = true;
bool fCheckBlockIndex = false;
bool fVerifyingBlocks = false;
unsigned int nCoinCacheSize = 5000;
bool fAlerts = DEFAULT_ALERTS;

unsigned int nStakeMinAge = 60 * 60;
int64_t nReserveBalance = 0;

/** Fees smaller than this (in duffs) are considered zero fee (for relaying and mining)
 * We are ~100 times smaller then bitcoin now (2015-06-23), set minRelayTxFee only 10 times higher
 * so it's still 10 times lower comparing to bitcoin.
 */
CFeeRate minRelayTxFee = CFeeRate(10000);

CTxMemPool mempool(::minRelayTxFee);

#ifdef  POW_IN_POS_PHASE

TmpBlocksMempool  tmpblockmempool;

#endif

struct COrphanTx {
    CTransaction tx;
    NodeId fromPeer;
};
map<uint256, COrphanTx> mapOrphanTransactions;
map<uint256, set<uint256> > mapOrphanTransactionsByPrev;
map<uint256, int64_t> mapRejectedBlocks;


void EraseOrphansFor(NodeId peer);

static void CheckBlockIndex();


CScript COINBASE_FLAGS;

const string strMessageMagic = "DarkNet Signed Message:\n";


namespace
{
struct CBlockIndexWorkComparator {
    bool operator()(CBlockIndex* pa, CBlockIndex* pb) const
    {
        
        if (pa->nChainWork > pb->nChainWork) return false;
        if (pa->nChainWork < pb->nChainWork) return true;

        
        if (pa->nSequenceId < pb->nSequenceId) return false;
        if (pa->nSequenceId > pb->nSequenceId) return true;

        
        
        if (pa < pb) return false;
        if (pa > pb) return true;

        
        return false;
    }
};

CBlockIndex* pindexBestInvalid;

/**
     * The set of all CBlockIndex entries with BLOCK_VALID_TRANSACTIONS (for itself and all ancestors) and
     * as good as our current tip or better. Entries may be failed, though.
     */
set<CBlockIndex*, CBlockIndexWorkComparator> setBlockIndexCandidates;

int nSyncStarted = 0;

multimap<CBlockIndex*, CBlockIndex*> mapBlocksUnlinked;

CCriticalSection cs_LastBlockFile;
std::vector<CBlockFileInfo> vinfoBlockFile;
int nLastBlockFile = 0;

/**
     * Every received block is assigned a unique and increasing identifier, so we
     * know which one to give priority in case of a fork.
     */
CCriticalSection cs_nBlockSequenceId;

uint32_t nBlockSequenceId = 1;

/**
     * Sources of received blocks, to be able to send them reject messages or ban
     * them, if processing happens afterwards. Protected by cs_main.
     */
map<uint256, NodeId> mapBlockSource;


struct QueuedBlock {
    uint256 hash;
    CBlockIndex* pindex;        
    int64_t nTime;              
    int nValidatedQueuedBefore; 
    bool fValidatedHeaders;     
};
map<uint256, pair<NodeId, list<QueuedBlock>::iterator> > mapBlocksInFlight;


int nQueuedValidatedHeaders = 0;


int nPreferredDownload = 0;


set<CBlockIndex*> setDirtyBlockIndex;


set<int> setDirtyFileInfo;
} 








namespace
{
struct CMainSignals {
    
    boost::signals2::signal<void(const CTransaction&, const CBlock*)> SyncTransaction;
    
    
    
    boost::signals2::signal<void(const uint256&)> UpdatedTransaction;
    
    boost::signals2::signal<void(const CBlockLocator&)> SetBestChain;
    
    boost::signals2::signal<void(const uint256&)> Inventory;
    
    boost::signals2::signal<void()> Broadcast;
    
    boost::signals2::signal<void(const CBlock&, const CValidationState&)> BlockChecked;
} g_signals;

} 

void RegisterValidationInterface(CValidationInterface* pwalletIn)
{
    g_signals.SyncTransaction.connect(boost::bind(&CValidationInterface::SyncTransaction, pwalletIn, _1, _2));
    
    g_signals.UpdatedTransaction.connect(boost::bind(&CValidationInterface::UpdatedTransaction, pwalletIn, _1));
    g_signals.SetBestChain.connect(boost::bind(&CValidationInterface::SetBestChain, pwalletIn, _1));
    g_signals.Inventory.connect(boost::bind(&CValidationInterface::Inventory, pwalletIn, _1));
    g_signals.Broadcast.connect(boost::bind(&CValidationInterface::ResendWalletTransactions, pwalletIn));
    g_signals.BlockChecked.connect(boost::bind(&CValidationInterface::BlockChecked, pwalletIn, _1, _2));
}

void UnregisterValidationInterface(CValidationInterface* pwalletIn)
{
    g_signals.BlockChecked.disconnect(boost::bind(&CValidationInterface::BlockChecked, pwalletIn, _1, _2));
    g_signals.Broadcast.disconnect(boost::bind(&CValidationInterface::ResendWalletTransactions, pwalletIn));
    g_signals.Inventory.disconnect(boost::bind(&CValidationInterface::Inventory, pwalletIn, _1));
    g_signals.SetBestChain.disconnect(boost::bind(&CValidationInterface::SetBestChain, pwalletIn, _1));
    g_signals.UpdatedTransaction.disconnect(boost::bind(&CValidationInterface::UpdatedTransaction, pwalletIn, _1));
    
    g_signals.SyncTransaction.disconnect(boost::bind(&CValidationInterface::SyncTransaction, pwalletIn, _1, _2));
}

void UnregisterAllValidationInterfaces()
{
    g_signals.BlockChecked.disconnect_all_slots();
    g_signals.Broadcast.disconnect_all_slots();
    g_signals.Inventory.disconnect_all_slots();
    g_signals.SetBestChain.disconnect_all_slots();
    g_signals.UpdatedTransaction.disconnect_all_slots();
    
    g_signals.SyncTransaction.disconnect_all_slots();
}

void SyncWithWallets(const CTransaction& tx, const CBlock* pblock)
{
    g_signals.SyncTransaction(tx, pblock);
}






namespace
{
struct CBlockReject {
    unsigned char chRejectCode;
    string strRejectReason;
    uint256 hashBlock;
};

/**
 * Maintain validation-specific state about nodes, protected by cs_main, instead
 * by CNode's own locks. This simplifies asynchronous operation, where
 * processing of incoming data is done after the ProcessMessage call returns,
 * and we're no longer holding the node's locks.
 */
struct CNodeState {
    
    CService address;
    
    bool fCurrentlyConnected;
    
    int nMisbehavior;
    
    bool fShouldBan;
    
    std::string name;
    
    std::vector<CBlockReject> rejects;
    
    CBlockIndex* pindexBestKnownBlock;
    
    uint256 hashLastUnknownBlock;
    
    CBlockIndex* pindexLastCommonBlock;
    
    bool fSyncStarted;
    
    int64_t nStallingSince;
    list<QueuedBlock> vBlocksInFlight;
    int nBlocksInFlight;
    
    bool fPreferredDownload;

    CNodeState()
    {
        fCurrentlyConnected = false;
        nMisbehavior = 0;
        fShouldBan = false;
        pindexBestKnownBlock = NULL;
        hashLastUnknownBlock = uint256(0);
        pindexLastCommonBlock = NULL;
        fSyncStarted = false;
        nStallingSince = 0;
        nBlocksInFlight = 0;
        fPreferredDownload = false;
    }
};


map<NodeId, CNodeState> mapNodeState;


CNodeState* State(NodeId pnode)
{
    map<NodeId, CNodeState>::iterator it = mapNodeState.find(pnode);
    if (it == mapNodeState.end())
        return NULL;
    return &it->second;
}

int GetHeight()
{
    while (true) {
        TRY_LOCK(cs_main, lockMain);
        if (!lockMain) {
            MilliSleep(50);
            continue;
        }
        return chainActive.Height();
    }
}

void UpdatePreferredDownload(CNode* node, CNodeState* state)
{
    nPreferredDownload -= state->fPreferredDownload;

    
    state->fPreferredDownload = (!node->fInbound || node->fWhitelisted) && !node->fOneShot && !node->fClient;

    nPreferredDownload += state->fPreferredDownload;
}

void InitializeNode(NodeId nodeid, const CNode* pnode)
{
    LOCK(cs_main);
    CNodeState& state = mapNodeState.insert(std::make_pair(nodeid, CNodeState())).first->second;
    state.name = pnode->addrName;
    state.address = pnode->addr;
}

void FinalizeNode(NodeId nodeid)
{
    LOCK(cs_main);
    CNodeState* state = State(nodeid);

    if (state->fSyncStarted)
        nSyncStarted--;

    if (state->nMisbehavior == 0 && state->fCurrentlyConnected) {
        AddressCurrentlyConnected(state->address);
    }

    BOOST_FOREACH (const QueuedBlock& entry, state->vBlocksInFlight)
            mapBlocksInFlight.erase(entry.hash);
    EraseOrphansFor(nodeid);
    nPreferredDownload -= state->fPreferredDownload;

    mapNodeState.erase(nodeid);
}


void MarkBlockAsReceived(const uint256& hash)
{
    map<uint256, pair<NodeId, list<QueuedBlock>::iterator> >::iterator itInFlight = mapBlocksInFlight.find(hash);
    if (itInFlight != mapBlocksInFlight.end()) {
        CNodeState* state = State(itInFlight->second.first);
        nQueuedValidatedHeaders -= itInFlight->second.second->fValidatedHeaders;
        state->vBlocksInFlight.erase(itInFlight->second.second);
        state->nBlocksInFlight--;
        state->nStallingSince = 0;
        mapBlocksInFlight.erase(itInFlight);
    }
}


void MarkBlockAsInFlight(NodeId nodeid, const uint256& hash, CBlockIndex* pindex = NULL)
{
    CNodeState* state = State(nodeid);
    assert(state != NULL);

    
    MarkBlockAsReceived(hash);

    QueuedBlock newentry = {hash, pindex, GetTimeMicros(), nQueuedValidatedHeaders, pindex != NULL};
    nQueuedValidatedHeaders += newentry.fValidatedHeaders;
    list<QueuedBlock>::iterator it = state->vBlocksInFlight.insert(state->vBlocksInFlight.end(), newentry);
    state->nBlocksInFlight++;
    mapBlocksInFlight[hash] = std::make_pair(nodeid, it);
}


void ProcessBlockAvailability(NodeId nodeid)
{
    CNodeState* state = State(nodeid);
    assert(state != NULL);

    if (state->hashLastUnknownBlock != 0) {
        BlockMap::iterator itOld = mapBlockIndex.find(state->hashLastUnknownBlock);
        if (itOld != mapBlockIndex.end() && itOld->second->nChainWork > 0) {
            if (state->pindexBestKnownBlock == NULL || itOld->second->nChainWork >= state->pindexBestKnownBlock->nChainWork)
                state->pindexBestKnownBlock = itOld->second;
            state->hashLastUnknownBlock = uint256(0);
        }
    }
}


void UpdateBlockAvailability(NodeId nodeid, const uint256& hash)
{
    CNodeState* state = State(nodeid);
    assert(state != NULL);

    ProcessBlockAvailability(nodeid);

    BlockMap::iterator it = mapBlockIndex.find(hash);
    if (it != mapBlockIndex.end() && it->second->nChainWork > 0) {
        
        if (state->pindexBestKnownBlock == NULL || it->second->nChainWork >= state->pindexBestKnownBlock->nChainWork)
            state->pindexBestKnownBlock = it->second;
    } else {
        
        state->hashLastUnknownBlock = hash;
    }
}

/** Find the last common ancestor two blocks have.
 *  Both pa and pb must be non-NULL. */
CBlockIndex* LastCommonAncestor(CBlockIndex* pa, CBlockIndex* pb)
{
    if (pa->nHeight > pb->nHeight) {
        pa = pa->GetAncestor(pb->nHeight);
    } else if (pb->nHeight > pa->nHeight) {
        pb = pb->GetAncestor(pa->nHeight);
    }

    while (pa != pb && pa && pb) {
        pa = pa->pprev;
        pb = pb->pprev;
    }

    
    assert(pa == pb);
    return pa;
}

/** Update pindexLastCommonBlock and add not-in-flight missing successors to vBlocks, until it has
 *  at most count entries. */
void FindNextBlocksToDownload(NodeId nodeid, unsigned int count, std::vector<CBlockIndex*>& vBlocks, NodeId& nodeStaller)
{
    if (count == 0)
        return;

    vBlocks.reserve(vBlocks.size() + count);
    CNodeState* state = State(nodeid);
    assert(state != NULL);

    
    ProcessBlockAvailability(nodeid);

    if (state->pindexBestKnownBlock == NULL || state->pindexBestKnownBlock->nChainWork < chainActive.Tip()->nChainWork) {
        
        return;
    }

    if (state->pindexLastCommonBlock == NULL) {
        
        
        state->pindexLastCommonBlock = chainActive[std::min(state->pindexBestKnownBlock->nHeight, chainActive.Height())];
    }

    
    
    state->pindexLastCommonBlock = LastCommonAncestor(state->pindexLastCommonBlock, state->pindexBestKnownBlock);
    if (state->pindexLastCommonBlock == state->pindexBestKnownBlock)
        return;

    std::vector<CBlockIndex*> vToFetch;
    CBlockIndex* pindexWalk = state->pindexLastCommonBlock;
    
    
    
    int nWindowEnd = state->pindexLastCommonBlock->nHeight + BLOCK_DOWNLOAD_WINDOW;
    int nMaxHeight = std::min<int>(state->pindexBestKnownBlock->nHeight, nWindowEnd + 1);
    NodeId waitingfor = -1;
    while (pindexWalk->nHeight < nMaxHeight) {
        
        
        
        int nToFetch = std::min(nMaxHeight - pindexWalk->nHeight, std::max<int>(count - vBlocks.size(), 128));
        vToFetch.resize(nToFetch);
        pindexWalk = state->pindexBestKnownBlock->GetAncestor(pindexWalk->nHeight + nToFetch);
        vToFetch[nToFetch - 1] = pindexWalk;
        for (unsigned int i = nToFetch - 1; i > 0; i--) {
            vToFetch[i - 1] = vToFetch[i]->pprev;
        }

        
        
        
        BOOST_FOREACH (CBlockIndex* pindex, vToFetch) {
            if (!pindex->IsValid(BLOCK_VALID_TREE)) {
                
                return;
            }
            if (pindex->nStatus & BLOCK_HAVE_DATA) {
                if (pindex->nChainTx)
                    state->pindexLastCommonBlock = pindex;
            } else if (mapBlocksInFlight.count(pindex->GetBlockHash()) == 0) {
                
                if (pindex->nHeight > nWindowEnd) {
                    
                    if (vBlocks.size() == 0 && waitingfor != nodeid) {
                        
                        nodeStaller = waitingfor;
                    }
                    return;
                }
                vBlocks.push_back(pindex);
                if (vBlocks.size() == count) {
                    return;
                }
            } else if (waitingfor == -1) {
                
                waitingfor = mapBlocksInFlight[pindex->GetBlockHash()].first;
            }
        }
    }
}

} 

bool GetNodeStateStats(NodeId nodeid, CNodeStateStats& stats)
{
    LOCK(cs_main);
    CNodeState* state = State(nodeid);
    if (state == NULL)
        return false;
    stats.nMisbehavior = state->nMisbehavior;
    stats.nSyncHeight = state->pindexBestKnownBlock ? state->pindexBestKnownBlock->nHeight : -1;
    stats.nCommonHeight = state->pindexLastCommonBlock ? state->pindexLastCommonBlock->nHeight : -1;
    BOOST_FOREACH (const QueuedBlock& queue, state->vBlocksInFlight) {
        if (queue.pindex)
            stats.vHeightInFlight.push_back(queue.pindex->nHeight);
    }
    return true;
}

void RegisterNodeSignals(CNodeSignals& nodeSignals)
{
    nodeSignals.GetHeight.connect(&GetHeight);
    nodeSignals.ProcessMessages.connect(&ProcessMessages);
    nodeSignals.SendMessages.connect(&SendMessages);
    nodeSignals.InitializeNode.connect(&InitializeNode);
    nodeSignals.FinalizeNode.connect(&FinalizeNode);
}

void UnregisterNodeSignals(CNodeSignals& nodeSignals)
{
    nodeSignals.GetHeight.disconnect(&GetHeight);
    nodeSignals.ProcessMessages.disconnect(&ProcessMessages);
    nodeSignals.SendMessages.disconnect(&SendMessages);
    nodeSignals.InitializeNode.disconnect(&InitializeNode);
    nodeSignals.FinalizeNode.disconnect(&FinalizeNode);
}

CBlockIndex* FindForkInGlobalIndex(const CChain& chain, const CBlockLocator& locator)
{
    
    BOOST_FOREACH (const uint256& hash, locator.vHave) {
        BlockMap::iterator mi = mapBlockIndex.find(hash);
        if (mi != mapBlockIndex.end()) {
            CBlockIndex* pindex = (*mi).second;
            if (chain.Contains(pindex))
                return pindex;
        }
    }
    return chain.Genesis();
}

CCoinsViewCache* pcoinsTip = NULL;
CBlockTreeDB* pblocktree = NULL;
CZerocoinDB* zerocoinDB = NULL;
CSporkDB* pSporkDB = NULL;
CAddressDB *paddressmap = NULL;






bool AddOrphanTx(const CTransaction& tx, NodeId peer)
{
    uint256 hash = tx.GetHash();
    if (mapOrphanTransactions.count(hash))
        return false;

    
    
    
    
    
    
    
    unsigned int sz = tx.GetSerializeSize(SER_NETWORK, CTransaction::CURRENT_VERSION);
    if (sz > 5000) {
        LogPrint("mempool", "ignoring large orphan tx (size: %u, hash: %s)\n", sz, hash.ToString());
        return false;
    }

    mapOrphanTransactions[hash].tx = tx;
    mapOrphanTransactions[hash].fromPeer = peer;
    BOOST_FOREACH (const CTxIn& txin, tx.vin)
            mapOrphanTransactionsByPrev[txin.prevout.hash].insert(hash);

    LogPrint("mempool", "stored orphan tx %s (mapsz %u prevsz %u)\n", hash.ToString(),
             mapOrphanTransactions.size(), mapOrphanTransactionsByPrev.size());
    return true;
}

void static EraseOrphanTx(uint256 hash)
{
    map<uint256, COrphanTx>::iterator it = mapOrphanTransactions.find(hash);
    if (it == mapOrphanTransactions.end())
        return;
    BOOST_FOREACH (const CTxIn& txin, it->second.tx.vin) {
        map<uint256, set<uint256> >::iterator itPrev = mapOrphanTransactionsByPrev.find(txin.prevout.hash);
        if (itPrev == mapOrphanTransactionsByPrev.end())
            continue;
        itPrev->second.erase(hash);
        if (itPrev->second.empty())
            mapOrphanTransactionsByPrev.erase(itPrev);
    }
    mapOrphanTransactions.erase(it);
}

void EraseOrphansFor(NodeId peer)
{
    int nErased = 0;
    map<uint256, COrphanTx>::iterator iter = mapOrphanTransactions.begin();
    while (iter != mapOrphanTransactions.end()) {
        map<uint256, COrphanTx>::iterator maybeErase = iter++; 
        if (maybeErase->second.fromPeer == peer) {
            EraseOrphanTx(maybeErase->second.tx.GetHash());
            ++nErased;
        }
    }
    if (nErased > 0) LogPrint("mempool", "Erased %d orphan tx from peer %d\n", nErased, peer);
}


unsigned int LimitOrphanTxSize(unsigned int nMaxOrphans)
{
    unsigned int nEvicted = 0;
    while (mapOrphanTransactions.size() > nMaxOrphans) {
        
        uint256 randomhash = GetRandHash();
        map<uint256, COrphanTx>::iterator it = mapOrphanTransactions.lower_bound(randomhash);
        if (it == mapOrphanTransactions.end())
            it = mapOrphanTransactions.begin();
        EraseOrphanTx(it->first);
        ++nEvicted;
    }
    return nEvicted;
}

bool IsStandardTx(const CTransaction& tx, string& reason)
{
    AssertLockHeld(cs_main);
    if (tx.nVersion > CTransaction::CURRENT_VERSION || tx.nVersion < 1) {
        reason = "version";
        return false;
    }

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    if (!IsFinalTx(tx, chainActive.Height() + 1)) {
        reason = "non-final";
        return false;
    }

    
    
    
    
    unsigned int sz = tx.GetSerializeSize(SER_NETWORK, CTransaction::CURRENT_VERSION);
    unsigned int nMaxSize = tx.ContainsZerocoins() ? MAX_ZEROCOIN_TX_SIZE : MAX_STANDARD_TX_SIZE;
    if (sz >= nMaxSize) {
        reason = "tx-size";
        return false;
    }

    for (const CTxIn& txin : tx.vin) {
        if (txin.scriptSig.IsZerocoinSpend())
            continue;
        
        
        
        
        
        
        
        if (txin.scriptSig.size() > 1650) {
            reason = "scriptsig-size";
            return false;
        }
        if (!txin.scriptSig.IsPushOnly()) {
            reason = "scriptsig-not-pushonly";
            return false;
        }
    }

    unsigned int nDataOut = 0;
    txnouttype whichType;
    BOOST_FOREACH (const CTxOut& txout, tx.vout) {
        if (!::IsStandard(txout.scriptPubKey, whichType)) {
            reason = "scriptpubkey";
            return false;
        }

        if (whichType == TX_NULL_DATA)
            nDataOut++;
        else if ((whichType == TX_MULTISIG) && (!fIsBareMultisigStd)) {
            reason = "bare-multisig";
            return false;
        } else if (txout.IsDust(::minRelayTxFee)) {
            reason = "dust";
            return false;
        }
    }

    
    if (nDataOut > 1) {
        reason = "multi-op-return";
        return false;
    }

    return true;
}

bool IsFinalTx(const CTransaction& tx, int nBlockHeight, int64_t nBlockTime)
{
    AssertLockHeld(cs_main);
    
    if (tx.nLockTime == 0)
        return true;
    if (nBlockHeight == 0)
        nBlockHeight = chainActive.Height();
    if (nBlockTime == 0)
        nBlockTime = GetAdjustedTime();
    if ((int64_t)tx.nLockTime < ((int64_t)tx.nLockTime < LOCKTIME_THRESHOLD ? (int64_t)nBlockHeight : nBlockTime))
        return true;
    BOOST_FOREACH (const CTxIn& txin, tx.vin)
            if (!txin.IsFinal())
            return false;
    return true;
}

/**
 * Check transaction inputs to mitigate two
 * potential denial-of-service attacks:
 *
 * 1. scriptSigs with extra data stuffed into them,
 *    not consumed by scriptPubKey (or P2SH script)
 * 2. P2SH scripts with a crazy number of expensive
 *    CHECKSIG/CHECKMULTISIG operations
 */
bool AreInputsStandard(const CTransaction& tx, const CCoinsViewCache& mapInputs)
{
    if (tx.IsCoinBase() || tx.IsZerocoinSpend())
        return true; 
    

    for (unsigned int i = 0; i < tx.vin.size(); i++) {
        const CTxOut& prev = mapInputs.GetOutputFor(tx.vin[i]);

        vector<vector<unsigned char> > vSolutions;
        txnouttype whichType;
        
        const CScript& prevScript = prev.scriptPubKey;
        if (!Solver(prevScript, whichType, vSolutions))
            return false;
        int nArgsExpected = ScriptSigArgsExpected(whichType, vSolutions);
        if (nArgsExpected < 0)
            return false;

        
        
        
        
        
        
        vector<vector<unsigned char> > stack;
        if (!EvalScript(stack, tx.vin[i].scriptSig, false, BaseSignatureChecker()))
            return false;

        if (whichType == TX_SCRIPTHASH) {
            if (stack.empty())
                return false;
            CScript subscript(stack.back().begin(), stack.back().end());
            vector<vector<unsigned char> > vSolutions2;
            txnouttype whichType2;
            if (Solver(subscript, whichType2, vSolutions2)) {
                int tmpExpected = ScriptSigArgsExpected(whichType2, vSolutions2);
                if (tmpExpected < 0)
                    return false;
                nArgsExpected += tmpExpected;
            } else {
                
                unsigned int sigops = subscript.GetSigOpCount(true);
                
                return (sigops <= MAX_P2SH_SIGOPS);
            }
        }

        if (stack.size() != (unsigned int)nArgsExpected)
            return false;
    }

    return true;
}

unsigned int GetLegacySigOpCount(const CTransaction& tx)
{
    unsigned int nSigOps = 0;
    BOOST_FOREACH (const CTxIn& txin, tx.vin) {
        nSigOps += txin.scriptSig.GetSigOpCount(false);
    }
    BOOST_FOREACH (const CTxOut& txout, tx.vout) {
        nSigOps += txout.scriptPubKey.GetSigOpCount(false);
    }
    return nSigOps;
}

unsigned int GetP2SHSigOpCount(const CTransaction& tx, const CCoinsViewCache& inputs)
{
    if (tx.IsCoinBase() || tx.IsZerocoinSpend())
        return 0;

    unsigned int nSigOps = 0;
    for (unsigned int i = 0; i < tx.vin.size(); i++) {
        const CTxOut& prevout = inputs.GetOutputFor(tx.vin[i]);
        if (prevout.scriptPubKey.IsPayToScriptHash())
            nSigOps += prevout.scriptPubKey.GetSigOpCount(tx.vin[i].scriptSig);
    }
    return nSigOps;
}

int GetInputAge(CTxIn& vin)
{
    CCoinsView viewDummy;
    CCoinsViewCache view(&viewDummy);
    {
        LOCK(mempool.cs);
        CCoinsViewMemPool viewMempool(pcoinsTip, mempool);
        view.SetBackend(viewMempool); 

        const CCoins* coins = view.AccessCoins(vin.prevout.hash);

        if (coins) {
            if (coins->nHeight < 0) return 0;
            return (chainActive.Tip()->nHeight + 1) - coins->nHeight;
        } else
            return -1;
    }
}

int GetInputAgeIX(uint256 nTXHash, CTxIn& vin)
{
    int sigs = 0;
    int nResult = GetInputAge(vin);
    if (nResult < 0) nResult = 0;

    if (nResult < 6) {
        std::map<uint256, CTransactionLock>::iterator i = mapTxLocks.find(nTXHash);
        if (i != mapTxLocks.end()) {
            sigs = (*i).second.CountSignatures();
        }
        if (sigs >= SWIFTTX_SIGNATURES_REQUIRED) {
            return nSwiftTXDepth + nResult;
        }
    }

    return -1;
}

int GetIXConfirmations(uint256 nTXHash)
{
    int sigs = 0;

    std::map<uint256, CTransactionLock>::iterator i = mapTxLocks.find(nTXHash);
    if (i != mapTxLocks.end()) {
        sigs = (*i).second.CountSignatures();
    }
    if (sigs >= SWIFTTX_SIGNATURES_REQUIRED) {
        return nSwiftTXDepth;
    }

    return 0;
}








bool GetCoinAge(const CTransaction& tx, const unsigned int nTxTime, uint64_t& nCoinAge)
{
    uint256 bnCentSecond = 0; 
    nCoinAge = 0;

    CBlockIndex* pindex = NULL;
    BOOST_FOREACH (const CTxIn& txin, tx.vin) {
        
        CTransaction txPrev;
        uint256 hashBlockPrev;
        if (!GetTransaction(txin.prevout.hash, txPrev, hashBlockPrev, true)) {
            LogPrintf("GetCoinAge: failed to find vin transaction \n");
            continue; 
        }

        BlockMap::iterator it = mapBlockIndex.find(hashBlockPrev);
        if (it != mapBlockIndex.end())
            pindex = it->second;
        else {
            LogPrintf("GetCoinAge() failed to find block index \n");
            continue;
        }

        
        CBlockHeader prevblock = pindex->GetBlockHeader();

        if (prevblock.nTime + nStakeMinAge > nTxTime)
            continue; 

        if (nTxTime < prevblock.nTime) {
            LogPrintf("GetCoinAge: Timestamp Violation: txtime less than txPrev.nTime");
            return false; 
        }

        int64_t nValueIn = txPrev.vout[txin.prevout.n].nValue;
        bnCentSecond += uint256(nValueIn) * (nTxTime - prevblock.nTime);
    }

    uint256 bnCoinDay = bnCentSecond / COIN / (24 * 60 * 60);
    LogPrintf("coin age bnCoinDay=%s\n", bnCoinDay.ToString().c_str());
    nCoinAge = bnCoinDay.GetCompact();
    return true;
}

bool MoneyRange(CAmount nValueOut)
{
    return nValueOut >= 0 && nValueOut <= Params().MaxMoneyOut();
}

int GetZerocoinStartHeight()
{
    return Params().Zerocoin_StartHeight();
}

void FindMints(vector<CZerocoinMint> vMintsToFind, vector<CZerocoinMint>& vMintsToUpdate, vector<CZerocoinMint>& vMissingMints, bool fExtendedSearch)
{
    
    
    for (CZerocoinMint mint : vMintsToFind) {
        uint256 txHash;
        if (!zerocoinDB->ReadCoinMint(mint.GetValue(), txHash)) {
            vMissingMints.push_back(mint);
            continue;
        }

        
        CTransaction tx;
        uint256 hashBlock;
        if (!GetTransaction(txHash, tx, hashBlock, true)) {
            LogPrintf("%s : cannot find tx %s\n", __func__, txHash.GetHex());
            vMissingMints.push_back(mint);
            continue;
        }

        if (!mapBlockIndex.count(hashBlock)) {
            LogPrintf("%s : cannot find block %s\n", __func__, hashBlock.GetHex());
            vMissingMints.push_back(mint);
            continue;
        }

        
        uint256 hashTxSpend = 0;
        zerocoinDB->ReadCoinSpend(mint.GetSerialNumber(), hashTxSpend);
        bool fSpent = hashTxSpend != 0;

        
        CTransaction txSpend;
        uint256 hashBlockSpend;
        if (fSpent && !GetTransaction(hashTxSpend, txSpend, hashBlockSpend, true)) {
            LogPrintf("%s : cannot find spend tx %s\n", __func__, hashTxSpend.GetHex());
            zerocoinDB->EraseCoinSpend(mint.GetSerialNumber());
            mint.SetUsed(false);
            vMintsToUpdate.push_back(mint);
            continue;
        }

        
        int nHeightTx = 0;
        if (fSpent && !IsSerialInBlockchain(mint.GetSerialNumber(), nHeightTx)) {
            LogPrintf("%s : cannot find block %s. Erasing coinspend from zerocoinDB.\n", __func__, hashBlockSpend.GetHex());
            zerocoinDB->EraseCoinSpend(mint.GetSerialNumber());
            mint.SetUsed(false);
            vMintsToUpdate.push_back(mint);
            continue;
        }

        
        if (mint.GetTxHash() == txHash && mint.GetHeight() == mapBlockIndex[hashBlock]->nHeight && mint.IsUsed() == fSpent)
            continue;

        
        mint.SetTxHash(txHash);
        mint.SetHeight(mapBlockIndex[hashBlock]->nHeight);
        mint.SetUsed(fSpent);

        vMintsToUpdate.push_back(mint);
    }

    if (fExtendedSearch)
    {
        
        int nZerocoinStartHeight = GetZerocoinStartHeight();

        for (int i = nZerocoinStartHeight; i < chainActive.Height(); i++) {

            if(i % 1000 == 0)
                LogPrintf("%s : scanned %d blocks\n", __func__, i - nZerocoinStartHeight);

            if(chainActive[i]->vMintDenominationsInBlock.empty())
                continue;

            CBlock block;
            if(!ReadBlockFromDisk(block, chainActive[i]))
                continue;

            list<CZerocoinMint> vMints;
            if(!BlockToZerocoinMintList(block, vMints, true))
                continue;

            
            for (CZerocoinMint mintBlockChain : vMints) {
                for (CZerocoinMint mintMissing : vMissingMints) {
                    if (mintMissing.GetValue() == mintBlockChain.GetValue()) {
                        LogPrintf("%s FOUND %s in block %d\n", __func__, mintMissing.GetValue().GetHex(), i);
                        mintMissing.SetHeight(i);
                        mintMissing.SetTxHash(mintBlockChain.GetTxHash());
                        vMintsToUpdate.push_back(mintMissing);
                    }
                }
            }
        }
    }

    
    for (CZerocoinMint mintMissing : vMissingMints) {
        for (CZerocoinMint mintFound : vMintsToUpdate) {
            if (mintMissing.GetValue() == mintFound.GetValue())
                std::remove(vMissingMints.begin(), vMissingMints.end(), mintMissing);
        }
    }

}

bool GetZerocoinMint(const CBigNum& bnPubcoin, uint256& txHash)
{
    txHash = 0;
    return zerocoinDB->ReadCoinMint(bnPubcoin, txHash);
}

bool IsSerialKnown(const CBigNum& bnSerial)
{
    uint256 txHash = 0;
    return zerocoinDB->ReadCoinSpend(bnSerial, txHash);
}

bool IsSerialInBlockchain(const CBigNum& bnSerial, int& nHeightTx)
{
    uint256 txHash = 0;
    
    if (!zerocoinDB->ReadCoinSpend(bnSerial, txHash))
        return false;

    CTransaction tx;
    uint256 hashBlock;
    if (!GetTransaction(txHash, tx, hashBlock, true))
        return false;

    bool inChain = mapBlockIndex.count(hashBlock) && chainActive.Contains(mapBlockIndex[hashBlock]);
    if (inChain)
        nHeightTx = mapBlockIndex.at(hashBlock)->nHeight;

    return inChain;
}

bool RemoveSerialFromDB(const CBigNum& bnSerial)
{
    return zerocoinDB->EraseCoinSpend(bnSerial);
}


bool RecordMintToDB(PublicCoin publicZerocoin, const uint256& txHash)
{
    
    
    
    CZerocoinMint pubCoinTx;
    uint256 hashFromDB;
    if (zerocoinDB->ReadCoinMint(publicZerocoin.getValue(), hashFromDB)) {
        if(hashFromDB == txHash)
            return true;

        LogPrintf("RecordMintToDB: failed, we already have this public coin recorded\n");
        return false;
    }

    if (!zerocoinDB->WriteCoinMint(publicZerocoin, txHash)) {
        LogPrintf("RecordMintToDB: failed to record public coin to DB\n");
        return false;
    }

    return true;
}

bool TxOutToPublicCoin(const CTxOut txout, PublicCoin& pubCoin, CValidationState& state)
{
    CBigNum publicZerocoin;
    vector<unsigned char> vchZeroMint;
    vchZeroMint.insert(vchZeroMint.end(), txout.scriptPubKey.begin() + SCRIPT_OFFSET,
                       txout.scriptPubKey.begin() + txout.scriptPubKey.size());
    publicZerocoin.setvch(vchZeroMint);

    CoinDenomination denomination = AmountToZerocoinDenomination(txout.nValue);
    LogPrint("zero", "%s ZCPRINT denomination %d pubcoin %s\n", __func__, denomination, publicZerocoin.GetHex());
    if (denomination == ZQ_ERROR)
        return state.DoS(100, error("TxOutToPublicCoin : txout.nValue is not correct"));

    PublicCoin checkPubCoin(Params().Zerocoin_Params(), publicZerocoin, denomination);
    pubCoin = checkPubCoin;

    return true;
}

bool BlockToPubcoinList(const CBlock& block, list<PublicCoin>& listPubcoins, bool fFilterInvalid)
{
    for (const CTransaction tx : block.vtx) {
        if(!tx.IsZerocoinMint())
            continue;

        
        if (fFilterInvalid) {
            bool fValid = true;
            for (const CTxIn in : tx.vin) {
                if (!ValidOutPoint(in.prevout, INT_MAX)) {
                    fValid = false;
                    break;
                }
            }
            if (!fValid)
                continue;
        }

        uint256 txHash = tx.GetHash();
        for (unsigned int i = 0; i < tx.vout.size(); i++) {
            
            if (fFilterInvalid && !ValidOutPoint(COutPoint(txHash, i), INT_MAX))
                break;

            const CTxOut txOut = tx.vout[i];
            if(!txOut.scriptPubKey.IsZerocoinMint())
                continue;

            CValidationState state;
            PublicCoin pubCoin(Params().Zerocoin_Params());
            if(!TxOutToPublicCoin(txOut, pubCoin, state))
                return false;

            listPubcoins.emplace_back(pubCoin);
        }
    }

    return true;
}


bool BlockToZerocoinMintList(const CBlock& block, std::list<CZerocoinMint>& vMints, bool fFilterInvalid)
{
    for (const CTransaction tx : block.vtx) {
        if(!tx.IsZerocoinMint())
            continue;

        
        if (fFilterInvalid) {
            bool fValid = true;
            for (const CTxIn in : tx.vin) {
                if (!ValidOutPoint(in.prevout, INT_MAX)) {
                    fValid = false;
                    break;
                }
            }
            if (!fValid)
                continue;
        }

        uint256 txHash = tx.GetHash();
        for (unsigned int i = 0; i < tx.vout.size(); i++) {
            
            if (fFilterInvalid && !ValidOutPoint(COutPoint(txHash, i), INT_MAX))
                break;

            const CTxOut txOut = tx.vout[i];
            if(!txOut.scriptPubKey.IsZerocoinMint())
                continue;

            CValidationState state;
            PublicCoin pubCoin(Params().Zerocoin_Params());
            if(!TxOutToPublicCoin(txOut, pubCoin, state))
                return false;

            CZerocoinMint mint = CZerocoinMint(pubCoin.getDenomination(), pubCoin.getValue(), 0, 0, false);
            mint.SetTxHash(tx.GetHash());
            vMints.push_back(mint);
        }
    }

    return true;
}

bool BlockToMintValueVector(const CBlock& block, const CoinDenomination denom, vector<CBigNum>& vValues)
{
    for (const CTransaction tx : block.vtx) {
        if(!tx.IsZerocoinMint())
            continue;

        for (const CTxOut txOut : tx.vout) {
            if(!txOut.scriptPubKey.IsZerocoinMint())
                continue;

            CValidationState state;
            PublicCoin coin(Params().Zerocoin_Params());
            if(!TxOutToPublicCoin(txOut, coin, state))
                return false;

            if (coin.getDenomination() != denom)
                continue;

            vValues.push_back(coin.getValue());
        }
    }

    return true;
}


std::list<libzerocoin::CoinDenomination> ZerocoinSpendListFromBlock(const CBlock& block, bool fFilterInvalid)
{
    std::list<libzerocoin::CoinDenomination> vSpends;
    for (const CTransaction tx : block.vtx) {
        if (!tx.IsZerocoinSpend())
            continue;

        for (const CTxIn txin : tx.vin) {
            if (!txin.scriptSig.IsZerocoinSpend())
                continue;

            if (fFilterInvalid) {
                CoinSpend spend = TxInToZerocoinSpend(txin);
                if (mapInvalidSerials.count(spend.getCoinSerialNumber()))
                    continue;
            }

            libzerocoin::CoinDenomination c = libzerocoin::IntToZerocoinDenomination(txin.nSequence);
            vSpends.push_back(c);
        }
    }
    return vSpends;
}

bool CheckZerocoinMint(const uint256& txHash, const CTxOut& txout, CValidationState& state, bool fCheckOnly)
{
    PublicCoin pubCoin(Params().Zerocoin_Params());
    if(!TxOutToPublicCoin(txout, pubCoin, state))
        return state.DoS(100, error("CheckZerocoinMint(): TxOutToPublicCoin() failed"));

    if (!pubCoin.validate())
        return state.DoS(100, error("CheckZerocoinMint() : PubCoin does not validate"));

    if(!fCheckOnly && !RecordMintToDB(pubCoin, txHash))
        return state.DoS(100, error("CheckZerocoinMint(): RecordMintToDB() failed"));

    return true;
}

CoinSpend TxInToZerocoinSpend(const CTxIn& txin)
{
    
    std::vector<char, zero_after_free_allocator<char> > dataTxIn;
    dataTxIn.insert(dataTxIn.end(), txin.scriptSig.begin() + BIGNUM_SIZE, txin.scriptSig.end());

    CDataStream serializedCoinSpend(dataTxIn, SER_NETWORK, PROTOCOL_VERSION);
    return CoinSpend(Params().Zerocoin_Params(), serializedCoinSpend);
}

bool IsZerocoinSpendUnknown(CoinSpend coinSpend, uint256 hashTx, CValidationState& state)
{
    uint256 hashTxFromDB;
    if(zerocoinDB->ReadCoinSpend(coinSpend.getCoinSerialNumber(), hashTxFromDB))
        return hashTx == hashTxFromDB;

    if(!zerocoinDB->WriteCoinSpend(coinSpend.getCoinSerialNumber(), hashTx))
        return state.DoS(100, error("CheckZerocoinSpend(): Failed to write zerocoin mint to database"));

    return true;
}

bool CheckZerocoinSpend(const CTransaction tx, bool fVerifySignature, CValidationState& state)
{
    
    if (tx.vout.size() > 2) {
        int outs = 0;
        for (const CTxOut out : tx.vout) {
            if (out.IsZerocoinMint())
                continue;
            outs++;
        }
        if (outs > 2)
            return state.DoS(100, error("CheckZerocoinSpend(): over two non-mint outputs in a zerocoinspend transaction"));
    }

    
    CMutableTransaction txTemp;
    for (const CTxOut out : tx.vout) {
        txTemp.vout.push_back(out);
    }
    uint256 hashTxOut = txTemp.GetHash();

    bool fValidated = false;
    set<CBigNum> serials;
    list<CoinSpend> vSpends;
    CAmount nTotalRedeemed = 0;
    for (const CTxIn& txin : tx.vin) {

        
        if (!txin.scriptSig.IsZerocoinSpend())
            continue;

        CoinSpend newSpend = TxInToZerocoinSpend(txin);
        vSpends.push_back(newSpend);

        
        if (newSpend.getDenomination() == ZQ_ERROR)
            return state.DoS(100, error("Zerocoinspend does not have the correct denomination"));

        
        if (newSpend.getDenomination() != txin.nSequence)
            return state.DoS(100, error("Zerocoinspend nSequence denomination does not match CoinSpend"));

        
        if (newSpend.getTxOutHash() != hashTxOut)
            return state.DoS(100, error("Zerocoinspend does not use the same txout that was used in the SoK"));

        
        if (fVerifySignature) {
            
            CBigNum bnAccumulatorValue = 0;
            if(!zerocoinDB->ReadAccumulatorValue(newSpend.getAccumulatorChecksum(), bnAccumulatorValue))
                return state.DoS(100, error("Zerocoinspend could not find accumulator associated with checksum"));

            Accumulator accumulator(Params().Zerocoin_Params(), newSpend.getDenomination(), bnAccumulatorValue);

            
            if(!newSpend.Verify(accumulator))
                return state.DoS(100, error("CheckZerocoinSpend(): zerocoin spend did not verify"));
        }

        if (serials.count(newSpend.getCoinSerialNumber()))
            return state.DoS(100, error("Zerocoinspend serial is used twice in the same tx"));
        serials.insert(newSpend.getCoinSerialNumber());

        
        nTotalRedeemed += ZerocoinDenominationToAmount(newSpend.getDenomination());
        fValidated = true;
    }

    if (nTotalRedeemed < tx.GetValueOut()) {
        LogPrintf("redeemed = %s , spend = %s \n", FormatMoney(nTotalRedeemed), FormatMoney(tx.GetValueOut()));
        return state.DoS(100, error("Transaction spend more than was redeemed in zerocoins"));
    }

    
    if (pwalletMain) {
        CWalletDB walletdb(pwalletMain->strWalletFile);
        list <CBigNum> listMySerials = walletdb.ListMintedCoinsSerial();
        for (const auto& newSpend : vSpends) {
            list<CBigNum>::iterator it = find(listMySerials.begin(), listMySerials.end(), newSpend.getCoinSerialNumber());
            if (it != listMySerials.end()) {
                LogPrintf("%s: %s detected spent zerocoin mint in transaction %s \n", __func__, it->GetHex(), tx.GetHash().GetHex());
                pwalletMain->NotifyZerocoinChanged(pwalletMain, it->GetHex(), "Used", CT_UPDATED);
            }
        }
    }

    return fValidated;
}

bool CheckTransaction(const CTransaction& tx, bool fZerocoinActive, bool fRejectBadUTXO, CValidationState& state)
{
    
    if (tx.vin.empty())
        return state.DoS(10, error("CheckTransaction() : vin empty"),
                         REJECT_INVALID, "bad-txns-vin-empty");
    if (tx.vout.empty())
        return state.DoS(10, error("CheckTransaction() : vout empty"),
                         REJECT_INVALID, "bad-txns-vout-empty");

    
    unsigned int nMaxSize = MAX_ZEROCOIN_TX_SIZE;

    if (::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION) > nMaxSize)
        return state.DoS(100, error("CheckTransaction() : size limits failed"),
                         REJECT_INVALID, "bad-txns-oversize");

    CBlockIndex * pBlockIndex = chainActive.Tip();
    bool enablecontract = (pBlockIndex && pBlockIndex->nHeight >= Params().Contract_StartHeight());

    
    CAmount nValueOut = 0;
    int nZCSpendCount = 0;

    if (enablecontract && tx.IsCoinBase2())
    {
        if (!tx.vout.at(1).scriptPubKey.HasOpVmHashState())
            return state.DoS(100, error("CheckTransaction() : miss contract state"),
                             REJECT_INVALID, "bad-txns-vout-hashstate");
    }

    BOOST_FOREACH (const CTxOut& txout, tx.vout) {
        if (txout.IsEmpty() && !tx.IsCoinBase() && !tx.IsCoinStake())
            return state.DoS(100, error("CheckTransaction(): txout empty for user transaction"));

        if (txout.nValue < 0)
            return state.DoS(100, error("CheckTransaction() : txout.nValue negative"),
                             REJECT_INVALID, "bad-txns-vout-negative");
        if (txout.nValue > Params().MaxMoneyOut())
            return state.DoS(100, error("CheckTransaction() : txout.nValue too high"),
                             REJECT_INVALID, "bad-txns-vout-toolarge");
        nValueOut += txout.nValue;
        if (!MoneyRange(nValueOut))
            return state.DoS(100, error("CheckTransaction() : txout total out of range"),
                             REJECT_INVALID, "bad-txns-txouttotal-toolarge");

        
        

        if (txout.scriptPubKey.HasOpCall() || txout.scriptPubKey.HasOpCreate())
        {
            if (!enablecontract)
            {
                return state.DoS(100, error("CheckTransaction() : smart contract not enabled!"),
                                 REJECT_INVALID, "not arrive to the contract height,refuse");
            }
            std::vector<std::vector<unsigned char>> vSolutions;
            txnouttype whichType;
            if (!Solver(txout.scriptPubKey, whichType, vSolutions, true))
            {
                return state.DoS(100, error("CheckTransaction() : smart contract script not standard"),
                                 REJECT_INVALID, "bad-txns-contract-nonstandard");
            }
        }
        

        if (fZerocoinActive && txout.IsZerocoinMint()) {
            if(!CheckZerocoinMint(tx.GetHash(), txout, state, false))
                return state.DoS(100, error("CheckTransaction() : invalid zerocoin mint"));
        }
        if (fZerocoinActive && txout.scriptPubKey.IsZerocoinSpend())
            nZCSpendCount++;
    }

    if (fZerocoinActive) {
        if (nZCSpendCount > Params().Zerocoin_MaxSpendsPerTransaction())
            return state.DoS(100, error("CheckTransaction() : there are more zerocoin spends than are allowed in one transaction"));

        if (tx.IsZerocoinSpend()) {
            
            for (const CTxIn in : tx.vin) {
                if (!in.scriptSig.IsZerocoinSpend())
                    return state.DoS(100,
                                     error("CheckTransaction() : zerocoinspend contains inputs that are not zerocoins"));
            }

            
            bool fVerifySignature = !IsInitialBlockDownload() && (GetTime() - chainActive.Tip()->GetBlockTime() < (60*60*24));
            if (!CheckZerocoinSpend(tx, fVerifySignature, state))
                return state.DoS(100, error("CheckTransaction() : invalid zerocoin spend"));
        }
    }

    
    set<COutPoint> vInOutPoints;
    set<CBigNum> vZerocoinSpendSerials;
    for (const CTxIn& txin : tx.vin) {
        if (vInOutPoints.count(txin.prevout))
            return state.DoS(100, error("CheckTransaction() : duplicate inputs"),
                             REJECT_INVALID, "bad-txns-inputs-duplicate");

        
        if (!txin.scriptSig.IsZerocoinSpend())
            vInOutPoints.insert(txin.prevout);
    }

    if (tx.IsCoinBase()) {
        if (tx.vin[0].scriptSig.size() < 2 || tx.vin[0].scriptSig.size() > 150)
            return state.DoS(100, error("CheckTransaction() : coinbase script size=%d", tx.vin[0].scriptSig.size()),
                    REJECT_INVALID, "bad-cb-length");
    } else if (fZerocoinActive && tx.IsZerocoinSpend()) {
        if(tx.vin.size() < 1 || static_cast<int>(tx.vin.size()) > Params().Zerocoin_MaxSpendsPerTransaction())
            return state.DoS(10, error("CheckTransaction() : Zerocoin Spend has more than allowed txin's"), REJECT_INVALID, "bad-zerocoinspend");
    } else {
        BOOST_FOREACH (const CTxIn& txin, tx.vin){

            if (txin.prevout.IsNull() && (fZerocoinActive && !txin.scriptSig.IsZerocoinSpend()))
                return state.DoS(10, error("CheckTransaction() : prevout is null"),
                                 REJECT_INVALID, "bad-txns-prevout-null");

            
            
            if (txin.scriptSig.HasOpSpend())
            {
                if (!enablecontract)
                {
                    return state.DoS(100, error("CheckTransaction() : smart contract not enabled!"),
                                     REJECT_INVALID, "not arrive to the contract height, refuse");
                }
            }
        }
    }

    return true;
}

bool CheckFinalTx(const CTransaction& tx, int flags)
{
    AssertLockHeld(cs_main);

    
    
    
    
    
    
    flags = std::max(flags, 0);

    
    
    
    
    
    
    const int nBlockHeight = chainActive.Height() + 1;

    
    
    
    
    
    const int64_t nBlockTime = (flags & LOCKTIME_MEDIAN_TIME_PAST) ? chainActive.Tip()->GetMedianTimePast() : GetAdjustedTime();

    return IsFinalTx(tx, nBlockHeight, nBlockTime);
}

CAmount GetMinRelayFee(const CTransaction& tx, unsigned int nBytes, bool fAllowFree)
{
    {
        LOCK(mempool.cs);
        uint256 hash = tx.GetHash();
        double dPriorityDelta = 0;
        CAmount nFeeDelta = 0;
        mempool.ApplyDeltas(hash, dPriorityDelta, nFeeDelta);
        if (dPriorityDelta > 0 || nFeeDelta > 0)
            return 0;
    }

    CAmount nMinFee = ::minRelayTxFee.GetFee(nBytes);

    if (fAllowFree) {
        
        
        
        
        if (nBytes < (DEFAULT_BLOCK_PRIORITY_SIZE - 1000))
            nMinFee = 0;
    }

    if (!MoneyRange(nMinFee))
        nMinFee = Params().MaxMoneyOut();
    return nMinFee;
}


bool AcceptToMemoryPool(CTxMemPool& pool, CValidationState& state, const CTransaction& tx, bool fLimitFree, bool* pfMissingInputs, bool fRejectInsaneFee, bool ignoreFees)
{
    AssertLockHeld(cs_main);
    if (pfMissingInputs)
        *pfMissingInputs = false;

    
    if (GetAdjustedTime() > GetSporkValue(SPORK_16_ZEROCOIN_MAINTENANCE_MODE) && tx.ContainsZerocoins())
        return state.DoS(10, error("AcceptToMemoryPool : Zerocoin transactions are temporarily disabled for maintenance"), REJECT_INVALID, "bad-tx");

    if (!CheckTransaction(tx, chainActive.Height() >= Params().Zerocoin_StartHeight(), true, state))
        return state.DoS(100, error("AcceptToMemoryPool: : CheckTransaction failed"), REJECT_INVALID, "bad-tx");

    
    if (tx.IsCoinBase())
        return state.DoS(100, error("AcceptToMemoryPool: : coinbase as individual tx"),
                         REJECT_INVALID, "coinbase");

    
    if (tx.IsCoinStake())
        return state.DoS(100, error("AcceptToMemoryPool: coinstake as individual tx"),
                         REJECT_INVALID, "coinstake");

    
    string reason;
    if (Params().RequireStandard() && !IsStandardTx(tx, reason))
        return state.DoS(0,
                         error("AcceptToMemoryPool : nonstandard transaction: %s", reason),
                         REJECT_NONSTANDARD, reason);
    
    uint256 hash = tx.GetHash();
    if (pool.exists(hash)) {
        LogPrintf("%s tx already in mempool\n", __func__);
        return false;
    }

    

    BOOST_FOREACH (const CTxIn& in, tx.vin) {
        if (mapLockedInputs.count(in.prevout)) {
            if (mapLockedInputs[in.prevout] != tx.GetHash()) {
                return state.DoS(0,
                                 error("AcceptToMemoryPool : conflicts with existing transaction lock: %s", reason),
                                 REJECT_INVALID, "tx-lock-conflict");
            }
        }
    }

    
    if (!tx.IsZerocoinSpend()) {
        LOCK(pool.cs); 
        for (unsigned int i = 0; i < tx.vin.size(); i++) {
            COutPoint outpoint = tx.vin[i].prevout;
            if (pool.mapNextTx.count(outpoint)) {
                
                return false;
            }
        }
    }


    {
        CCoinsView dummy;
        CCoinsViewCache view(&dummy);

        CAmount nValueIn = 0;
        if(tx.IsZerocoinSpend()){
            nValueIn = tx.GetZerocoinSpent();

            
            int nHeightTx = 0;
            if (IsTransactionInChain(tx.GetHash(), nHeightTx))
                return state.Invalid(error("AcceptToMemoryPool : zUlo spend tx %s already in block %d", tx.GetHash().GetHex(), nHeightTx),
                                     REJECT_DUPLICATE, "bad-txns-inputs-spent");

            
            for (const CTxIn& txIn : tx.vin) {
                if (!txIn.scriptSig.IsZerocoinSpend())
                    continue;
                CoinSpend spend = TxInToZerocoinSpend(txIn);
                int nHeightTx = 0;
                if (IsSerialInBlockchain(spend.getCoinSerialNumber(), nHeightTx))
                    return state.Invalid(error("%s : zUlo spend with serial %s is already in block %d\n",
                                               __func__, spend.getCoinSerialNumber().GetHex(), nHeightTx));

                
                if (!spend.HasValidSerial(Params().Zerocoin_Params()))
                    return state.Invalid(error("%s : zUlo spend with serial %s from tx %s is not in valid range\n",
                                               __func__, spend.getCoinSerialNumber().GetHex(), tx.GetHash().GetHex()));
            }
        } else {
            LOCK(pool.cs);
            CCoinsViewMemPool viewMemPool(pcoinsTip, pool);
            view.SetBackend(viewMemPool);

            
            if (view.HaveCoins(hash))
                return false;

            
            
            
            for (const CTxIn txin : tx.vin) {
                if (!view.HaveCoins(txin.prevout.hash)) {
                    if (pfMissingInputs)
                        *pfMissingInputs = true;
                    return false;
                }

                
                if (!ValidOutPoint(txin.prevout, chainActive.Height())) {
                    return state.Invalid(error("%s : tried to spend invalid input %s in tx %s", __func__, txin.prevout.ToString(),
                                               tx.GetHash().GetHex()), REJECT_INVALID, "bad-txns-invalid-inputs");
                }
            }

            
            if (!view.HaveInputs(tx))
                return state.Invalid(error("AcceptToMemoryPool : inputs already spent"),
                                     REJECT_DUPLICATE, "bad-txns-inputs-spent");

            
            view.GetBestBlock();

            nValueIn = view.GetValueIn(tx);

            
            view.SetBackend(dummy);
        }

        
        if (Params().RequireStandard() && !AreInputsStandard(tx, view))
            return error("AcceptToMemoryPool: : nonstandard transaction input");

        
        
        
        
        
        if (!tx.IsZerocoinSpend()) {
            unsigned int nSigOps = GetLegacySigOpCount(tx);
            unsigned int nMaxSigOps = MAX_TX_SIGOPS_CURRENT;
            nSigOps += GetP2SHSigOpCount(tx, view);
            if(nSigOps > nMaxSigOps)
                return state.DoS(0,
                                 error("AcceptToMemoryPool : too many sigops %s, %d > %d",
                                       hash.ToString(), nSigOps, nMaxSigOps),
                                 REJECT_NONSTANDARD, "bad-txns-too-many-sigops");
        }

        CAmount nValueOut = tx.GetValueOut();
        CAmount nFees = nValueIn - nValueOut;
        LogPrintf("nFees nValueIn: %d ,\tnValueOut: %d\n", nValueIn, nValueOut);


        
        
        CAmount nMinGasPrice = 0;
        if (tx.HasCreateOrCall())
        {
            if (!tx.CheckSenderScript(view))
            {
                return state.DoS(1, error("AcceptToMemoryPool : CheckSenderScript."), REJECT_INVALID, "bad-txns-invalid-sender-script");
            }

            int level = 0;
            string errinfo;

            if (!CheckContractTx(tx, nFees, nMinGasPrice, level, errinfo))
            {
                if(REJECT_HIGHFEE == level){
                    return state.DoS(level, error(errinfo.c_str()), REJECT_HIGHFEE);
                }
                return state.DoS(level, error(errinfo.c_str()), REJECT_INVALID);
            }

        }
        

        double dPriority = 0;
        if (!tx.IsZerocoinSpend())
            view.GetPriority(tx, chainActive.Height());

        CTxMemPoolEntry entry(tx, nFees, GetTime(), dPriority, chainActive.Height());
        unsigned int nSize = entry.GetTxSize();

        
        
        if (mapObfuscationBroadcastTxes.count(hash)) {
            mempool.PrioritiseTransaction(hash, hash.ToString(), 1000, 0.1 * COIN);
        } else if (!ignoreFees) {
            CAmount txMinFee = GetMinRelayFee(tx, nSize, true);
            if (fLimitFree && nFees < txMinFee && !tx.IsZerocoinSpend())
                return state.DoS(0, error("AcceptToMemoryPool : not enough fees %s, %d < %d",
                                          hash.ToString(), nFees, txMinFee),
                                 REJECT_INSUFFICIENTFEE, "insufficient fee");

            
            if (tx.IsZerocoinMint()) {
                if(nFees < Params().Zerocoin_MintFee() * tx.GetZerocoinMintCount())
                    return state.DoS(0, false, REJECT_INSUFFICIENTFEE, "insufficient fee for zerocoinmint");
            } else if (!tx.IsZerocoinSpend() && GetBoolArg("-relaypriority", true) && nFees < ::minRelayTxFee.GetFee(nSize) && !AllowFree(view.GetPriority(tx, chainActive.Height() + 1))) {
                return state.DoS(0, false, REJECT_INSUFFICIENTFEE, "insufficient priority");
            }

            
            
            
            if (fLimitFree && nFees < ::minRelayTxFee.GetFee(nSize) && !tx.IsZerocoinSpend()) {
                static CCriticalSection csFreeLimiter;
                static double dFreeCount;
                static int64_t nLastTime;
                int64_t nNow = GetTime();

                LOCK(csFreeLimiter);

                
                dFreeCount *= pow(1.0 - 1.0 / 600.0, (double)(nNow - nLastTime));
                nLastTime = nNow;
                
                
                if (dFreeCount >= GetArg("-limitfreerelay", 30) * 10 * 1000)
                    return state.DoS(0, error("AcceptToMemoryPool : free transaction rejected by rate limiter"),
                                     REJECT_INSUFFICIENTFEE, "rate limited free transaction");
                LogPrint("mempool", "Rate limit dFreeCount: %g => %g\n", dFreeCount, dFreeCount + nSize);
                dFreeCount += nSize;
            }
        }


        LogPrintf("fRejectInsaneFee: %d \n", fRejectInsaneFee);
        LogPrintf("nFees: %d \n", nFees);
        LogPrintf("nSize: %d \n", nSize);
        LogPrintf("minRelayTxFee.GetFee(nSize):* 10000 :  %d \n", ::minRelayTxFee.GetFee(nSize) * 10000);


        if (fRejectInsaneFee && nFees > ::minRelayTxFee.GetFee(nSize) * 10000)
            return error("AcceptToMemoryPool: : insane fees %s, %d > %d",
                         hash.ToString(),
                         nFees, ::minRelayTxFee.GetFee(nSize) * 10000);

        
        
        if (!CheckInputs(tx, state, view, true, STANDARD_SCRIPT_VERIFY_FLAGS, true)) {
            return error("AcceptToMemoryPool: : ConnectInputs failed %s", hash.ToString());
        }

        
        
        
        
        
        
        
        
        
        if (!CheckInputs(tx, state, view, true, MANDATORY_SCRIPT_VERIFY_FLAGS, true)) {
            return error("AcceptToMemoryPool: : BUG! PLEASE REPORT THIS! ConnectInputs failed against MANDATORY but not STANDARD flags %s", hash.ToString());
        }

        
        pool.addUnchecked(hash, entry);
    }

    SyncWithWallets(tx, NULL);

    return true;
}

bool AcceptableInputs(CTxMemPool& pool, CValidationState& state, const CTransaction& tx, bool fLimitFree, bool* pfMissingInputs, bool fRejectInsaneFee, bool isDSTX)
{
    AssertLockHeld(cs_main);
    if (pfMissingInputs)
        *pfMissingInputs = false;

    if (!CheckTransaction(tx, chainActive.Height() >= Params().Zerocoin_StartHeight(), true, state))
        return error("AcceptableInputs: : CheckTransaction failed");

    
    if (tx.IsCoinBase())
        return state.DoS(100, error("AcceptableInputs: : coinbase as individual tx"),
                         REJECT_INVALID, "coinbase");

    
    string reason;
    
    
    
    
    

    
    uint256 hash = tx.GetHash();
    if (pool.exists(hash))
        return false;

    

    BOOST_FOREACH (const CTxIn& in, tx.vin) {
        if (mapLockedInputs.count(in.prevout)) {
            if (mapLockedInputs[in.prevout] != tx.GetHash()) {
                return state.DoS(0,
                                 error("AcceptableInputs : conflicts with existing transaction lock: %s", reason),
                                 REJECT_INVALID, "tx-lock-conflict");
            }
        }
    }

    
    if (!tx.IsZerocoinSpend()) {
        LOCK(pool.cs); 
        for (unsigned int i = 0; i < tx.vin.size(); i++) {
            COutPoint outpoint = tx.vin[i].prevout;
            if (pool.mapNextTx.count(outpoint)) {
                
                return false;
            }
        }
    }


    {
        CCoinsView dummy;
        CCoinsViewCache view(&dummy);

        CAmount nValueIn = 0;
        {
            LOCK(pool.cs);
            CCoinsViewMemPool viewMemPool(pcoinsTip, pool);
            view.SetBackend(viewMemPool);

            
            if (view.HaveCoins(hash))
                return false;

            
            
            
            for (const CTxIn txin : tx.vin) {
                if (!view.HaveCoins(txin.prevout.hash)) {
                    if (pfMissingInputs)
                        *pfMissingInputs = true;
                    return false;
                }

                
                if (!ValidOutPoint(txin.prevout, chainActive.Height())) {
                    return state.Invalid(error("%s : tried to spend invalid input %s in tx %s", __func__, txin.prevout.ToString(),
                                               tx.GetHash().GetHex()), REJECT_INVALID, "bad-txns-invalid-inputs");
                }
            }

            
            if (!view.HaveInputs(tx))
                return state.Invalid(error("AcceptableInputs : inputs already spent"),
                                     REJECT_DUPLICATE, "bad-txns-inputs-spent");

            
            view.GetBestBlock();

            nValueIn = view.GetValueIn(tx);

            
            view.SetBackend(dummy);
        }

        
        
        
        

        
        
        
        
        
        unsigned int nSigOps = GetLegacySigOpCount(tx);
        unsigned int nMaxSigOps = MAX_TX_SIGOPS_CURRENT;
        nSigOps += GetP2SHSigOpCount(tx, view);
        if (nSigOps > nMaxSigOps)
            return state.DoS(0,
                             error("AcceptableInputs : too many sigops %s, %d > %d",
                                   hash.ToString(), nSigOps, nMaxSigOps),
                             REJECT_NONSTANDARD, "bad-txns-too-many-sigops");

        CAmount nValueOut = tx.GetValueOut();
        CAmount nFees = nValueIn - nValueOut;
        double dPriority = view.GetPriority(tx, chainActive.Height());

        CTxMemPoolEntry entry(tx, nFees, GetTime(), dPriority, chainActive.Height());
        unsigned int nSize = entry.GetTxSize();

        
        
        if (isDSTX) {
            mempool.PrioritiseTransaction(hash, hash.ToString(), 1000, 0.1 * COIN);
        } else { 
            CAmount txMinFee = GetMinRelayFee(tx, nSize, true);
            if (fLimitFree && nFees < txMinFee && !tx.IsZerocoinSpend())
                return state.DoS(0, error("AcceptableInputs : not enough fees %s, %d < %d",
                                          hash.ToString(), nFees, txMinFee),
                                 REJECT_INSUFFICIENTFEE, "insufficient fee");

            
            if (GetBoolArg("-relaypriority", true) && nFees < ::minRelayTxFee.GetFee(nSize) && !AllowFree(view.GetPriority(tx, chainActive.Height() + 1))) {
                return state.DoS(0, false, REJECT_INSUFFICIENTFEE, "insufficient priority");
            }

            
            
            
            if (fLimitFree && nFees < ::minRelayTxFee.GetFee(nSize) && !tx.IsZerocoinSpend()) {
                static CCriticalSection csFreeLimiter;
                static double dFreeCount;
                static int64_t nLastTime;
                int64_t nNow = GetTime();

                LOCK(csFreeLimiter);

                
                dFreeCount *= pow(1.0 - 1.0 / 600.0, (double)(nNow - nLastTime));
                nLastTime = nNow;
                
                
                if (dFreeCount >= GetArg("-limitfreerelay", 30) * 10 * 1000)
                    return state.DoS(0, error("AcceptableInputs : free transaction rejected by rate limiter"),
                                     REJECT_INSUFFICIENTFEE, "rate limited free transaction");
                LogPrint("mempool", "Rate limit dFreeCount: %g => %g\n", dFreeCount, dFreeCount + nSize);
                dFreeCount += nSize;
            }
        }

        if (fRejectInsaneFee && nFees > ::minRelayTxFee.GetFee(nSize) * 10000)
            return error("AcceptableInputs: : insane fees %s, %d > %d",
                         hash.ToString(),
                         nFees, ::minRelayTxFee.GetFee(nSize) * 10000);

        
        
        if (!CheckInputs(tx, state, view, false, STANDARD_SCRIPT_VERIFY_FLAGS, true)) {
            return error("AcceptableInputs: : ConnectInputs failed %s", hash.ToString());
        }

        
        
        
        
        
        
        
        
        
        
        
        
        
        

        
        
    }

    

    return true;
}


bool GetTransaction(const uint256& hash, CTransaction& txOut, uint256& hashBlock, bool fAllowSlow)
{
    CBlockIndex* pindexSlow = NULL;
    {
        LOCK(cs_main);
        {
            if (mempool.lookup(hash, txOut)) {
                return true;
            }
        }

        if (fTxIndex) {
            CDiskTxPos postx;
            if (pblocktree->ReadTxIndex(hash, postx)) {
                CAutoFile file(OpenBlockFile(postx, true), SER_DISK, CLIENT_VERSION);
                if (file.IsNull())
                    return error("%s: OpenBlockFile failed", __func__);
                CBlockHeader header;
                try {
                    file >> header;
                    fseek(file.Get(), postx.nTxOffset, SEEK_CUR);
                    file >> txOut;
                } catch (std::exception& e) {
                    return error("%s : Deserialize or I/O error - %s", __func__, e.what());
                }
                hashBlock = header.GetHash();
                if (txOut.GetHash() != hash)
                    return error("%s : txid mismatch", __func__);
                return true;
            }

            
            return false;
        }

        if (fAllowSlow) { 
            int nHeight = -1;
            {
                CCoinsViewCache& view = *pcoinsTip;
                const CCoins* coins = view.AccessCoins(hash);
                if (coins)
                    nHeight = coins->nHeight;
            }
            if (nHeight > 0)
                pindexSlow = chainActive[nHeight];
        }
    }

    if (pindexSlow) {
        CBlock block;
        if (ReadBlockFromDisk(block, pindexSlow)) {
            BOOST_FOREACH (const CTransaction& tx, block.vtx) {
                if (tx.GetHash() == hash) {
                    txOut = tx;
                    hashBlock = pindexSlow->GetBlockHash();
                    return true;
                }
            }
        }
    }

    return false;
}







bool WriteBlockToDisk(CBlock& block, CDiskBlockPos& pos)
{
    
    CAutoFile fileout(OpenBlockFile(pos), SER_DISK, CLIENT_VERSION);
    if (fileout.IsNull())
        return error("WriteBlockToDisk : OpenBlockFile failed");

    
    unsigned int nSize = fileout.GetSerializeSize(block);
    fileout << FLATDATA(Params().MessageStart()) << nSize;

    
    long fileOutPos = ftell(fileout.Get());
    if (fileOutPos < 0)
        return error("WriteBlockToDisk : ftell failed");
    pos.nPos = (unsigned int)fileOutPos;
    fileout << block;

    return true;
}

bool ReadBlockFromDisk(CBlock& block, const CDiskBlockPos& pos)
{
    block.SetNull();

    
    CAutoFile filein(OpenBlockFile(pos, true), SER_DISK, CLIENT_VERSION);
    if (filein.IsNull())
        return error("ReadBlockFromDisk : OpenBlockFile failed");

    
    try {
        filein >> block;
    } catch (std::exception& e) {
        return error("%s : Deserialize or I/O error - %s", __func__, e.what());
    }

    
    if (block.IsProofOfWork()) {
        if (!CheckProofOfWork(block.GetHash(), block.nBits))
            return error("ReadBlockFromDisk : Errors in block header");
    }

    return true;
}

bool ReadBlockFromDisk(CBlock& block, const CBlockIndex* pindex)
{
    if (!ReadBlockFromDisk(block, pindex->GetBlockPos()))
        return false;
    if (block.GetHash() != pindex->GetBlockHash()) {
        LogPrintf("%s : block=%s index=%s\n", __func__, block.GetHash().ToString().c_str(), pindex->GetBlockHash().ToString().c_str());
        return error("ReadBlockFromDisk(CBlock&, CBlockIndex*) : GetHash() doesn't match index");
    }
    return true;
}


double ConvertBitsToDouble(unsigned int nBits)
{
    int nShift = (nBits >> 24) & 0xff;

    double dDiff =
            (double)0x0000ffff / (double)(nBits & 0x00ffffff);

    while (nShift < 29) {
        dDiff *= 256.0;
        nShift++;
    }
    while (nShift > 29) {
        dDiff /= 256.0;
        nShift--;
    }

    return dDiff;
}

int64_t GetTmpBlockValue(int nHeight)
{
    int64_t nSubsidy = 0;

    nSubsidy = GetBlockValue(nHeight);
    if (nHeight <= Params().LAST_POW_BLOCK()) {
        nSubsidy = 0 * COIN;
    } else if (nHeight <= Params().POW_Start_BLOCK_In_POS() - 2 && nHeight > Params().LAST_POW_BLOCK()) {
        nSubsidy = 0 * COIN;
    } else if (nHeight <= 4665599 && nHeight >= Params().POW_Start_BLOCK_In_POS() - 1) {
        nSubsidy *= 0.2;
    } else if (nHeight <= 8812799 && nHeight >= 4665600) {
        nSubsidy *= 0.3;
    } else if (nHeight <= 15033599 && nHeight >= 8812800) {
        nSubsidy *= 0.4;
    } else if (nHeight >= 15033600) {
        nSubsidy *= 0.5;
    } else {
        nSubsidy = 0;
    }

    return nSubsidy;
}

int64_t GetBlockValue(int nHeight)
{
    int64_t nSubsidy = 0;

    if (nHeight == 0) {
        nSubsidy = 10000000000 * COIN;
    } else if (nHeight <= Params().LAST_POW_BLOCK() && nHeight > 0) {
        nSubsidy = 10 * 1 * COIN;
    } else if (nHeight <= 518399 && nHeight > Params().LAST_POW_BLOCK()) {
        nSubsidy = 10 * 1 * COIN;
    } else if (nHeight <= 4665599 && nHeight >= 518400) {
        nSubsidy = 325 * 0.95 * COIN;
    } else if (nHeight <= 6739199 && nHeight >= 4665600) {
        nSubsidy = 625 * 0.95 * COIN;
    } else if (nHeight <= 8812799 && nHeight >= 6739200) {
        nSubsidy = 600 * 0.95 * COIN;
    } else if (nHeight <= 10886399 && nHeight >= 8812800) {
        nSubsidy = 575 * 0.95 * COIN;
    } else if (nHeight <= 12959999 && nHeight >= 10886400) {
        nSubsidy = 550 * 0.95 * COIN;
    } else if (6479999 <= 15033599 && nHeight >= 12960000) {
        nSubsidy = 525 * 0.95 * COIN;
    } else if (nHeight <= 17107199 && nHeight >= 15033600) {
        nSubsidy = 500 * 0.95 * COIN;
    } else if (nHeight <= 19180799 && nHeight >= 17107200) {
        nSubsidy = 475 * 0.95 * COIN;
    } else if (nHeight <= 21254399 && nHeight >= 19180800) {
        nSubsidy = 450 * 0.95 * COIN;
    } else if (nHeight <= 23327999 && nHeight >= 21254400) {
        nSubsidy = 425 * 0.95 * COIN;
    } else if (nHeight >= 23328000) {
        nSubsidy = 250 * 0.95 * COIN;
    } else {
        nSubsidy = 0 * COIN;
    }
    return nSubsidy;
}

int64_t GetMasternodePayment(int nHeight, int64_t blockValue, int nMasternodeCount)
{
    int64_t ret = 0;

    if (Params().NetworkID() == CBaseChainParams::TESTNET) {
        if (nHeight < 200)
            return 0;
    }

    if (nHeight <= Params().LAST_POW_BLOCK() && nHeight > 0) {
        ret = blockValue / 10;
    } else if (nHeight > Params().LAST_POW_BLOCK()) {
        int64_t nMoneySupply = chainActive.Tip()->nMoneySupply;

        
        if (nMasternodeCount < 1){
            if (IsSporkActive(SPORK_8_MASTERNODE_PAYMENT_ENFORCEMENT))
                nMasternodeCount = mnodeman.stable_size();
            else
                nMasternodeCount = mnodeman.size();
        }

        int64_t mNodeCoins = nMasternodeCount * MASTERNODE_COIN_AMOUNT * COIN;

        
        LogPrintf("Adjusting seesaw at height %d with %d masternodes (without drift: %d) at %ld\n", nHeight, nMasternodeCount, nMasternodeCount - Params().MasternodeCountDrift(), GetTime());

        if (fDebug)
            LogPrintf("GetMasternodePayment(): moneysupply=%s, nodecoins=%s \n", FormatMoney(nMoneySupply).c_str(),
                      FormatMoney(mNodeCoins).c_str());

        if (mNodeCoins == 0) {
            ret = 0;
        } else if (nHeight < 2692800) {
            if (mNodeCoins <= (nMoneySupply * .05) && mNodeCoins > 0) {
                ret = blockValue * .85;
            } else if (mNodeCoins <= (nMoneySupply * .1) && mNodeCoins > (nMoneySupply * .05)) {
                ret = blockValue * .8;
            } else if (mNodeCoins <= (nMoneySupply * .15) && mNodeCoins > (nMoneySupply * .1)) {
                ret = blockValue * .75;
            } else if (mNodeCoins <= (nMoneySupply * .2) && mNodeCoins > (nMoneySupply * .15)) {
                ret = blockValue * .7;
            } else if (mNodeCoins <= (nMoneySupply * .25) && mNodeCoins > (nMoneySupply * .2)) {
                ret = blockValue * .65;
            } else if (mNodeCoins <= (nMoneySupply * .3) && mNodeCoins > (nMoneySupply * .25)) {
                ret = blockValue * .6;
            } else if (mNodeCoins <= (nMoneySupply * .35) && mNodeCoins > (nMoneySupply * .3)) {
                ret = blockValue * .55;
            } else if (mNodeCoins <= (nMoneySupply * .4) && mNodeCoins > (nMoneySupply * .35)) {
                ret = blockValue * .5;
            } else if (mNodeCoins <= (nMoneySupply * .45) && mNodeCoins > (nMoneySupply * .4)) {
                ret = blockValue * .45;
            } else if (mNodeCoins <= (nMoneySupply * .5) && mNodeCoins > (nMoneySupply * .45)) {
                ret = blockValue * .4;
            } else if (mNodeCoins <= (nMoneySupply * .55) && mNodeCoins > (nMoneySupply * .5)) {
                ret = blockValue * .35;
            } else if (mNodeCoins <= (nMoneySupply * .6) && mNodeCoins > (nMoneySupply * .55)) {
                ret = blockValue * .3;
            } else if (mNodeCoins <= (nMoneySupply * .65) && mNodeCoins > (nMoneySupply * .6)) {
                ret = blockValue * .25;
            } else if (mNodeCoins <= (nMoneySupply * .7) && mNodeCoins > (nMoneySupply * .65)) {
                ret = blockValue * .2;
            } else if (mNodeCoins <= (nMoneySupply * .75) && mNodeCoins > (nMoneySupply * .7)) {
                ret = blockValue * .15;
            } else {
                ret = blockValue * .1;
            }
        } else if (nHeight > 2692800) {
            if (mNodeCoins <= (nMoneySupply * .01) && mNodeCoins > 0) {
                ret = blockValue * .90;
            } else if (mNodeCoins <= (nMoneySupply * .02) && mNodeCoins > (nMoneySupply * .01)) {
                ret = blockValue * .88;
            } else if (mNodeCoins <= (nMoneySupply * .03) && mNodeCoins > (nMoneySupply * .02)) {
                ret = blockValue * .87;
            } else if (mNodeCoins <= (nMoneySupply * .04) && mNodeCoins > (nMoneySupply * .03)) {
                ret = blockValue * .86;
            } else if (mNodeCoins <= (nMoneySupply * .05) && mNodeCoins > (nMoneySupply * .04)) {
                ret = blockValue * .85;
            } else if (mNodeCoins <= (nMoneySupply * .06) && mNodeCoins > (nMoneySupply * .05)) {
                ret = blockValue * .84;
            } else if (mNodeCoins <= (nMoneySupply * .07) && mNodeCoins > (nMoneySupply * .06)) {
                ret = blockValue * .83;
            } else if (mNodeCoins <= (nMoneySupply * .08) && mNodeCoins > (nMoneySupply * .07)) {
                ret = blockValue * .82;
            } else if (mNodeCoins <= (nMoneySupply * .09) && mNodeCoins > (nMoneySupply * .08)) {
                ret = blockValue * .81;
            } else if (mNodeCoins <= (nMoneySupply * .10) && mNodeCoins > (nMoneySupply * .09)) {
                ret = blockValue * .80;
            } else if (mNodeCoins <= (nMoneySupply * .11) && mNodeCoins > (nMoneySupply * .10)) {
                ret = blockValue * .79;
            } else if (mNodeCoins <= (nMoneySupply * .12) && mNodeCoins > (nMoneySupply * .11)) {
                ret = blockValue * .78;
            } else if (mNodeCoins <= (nMoneySupply * .13) && mNodeCoins > (nMoneySupply * .12)) {
                ret = blockValue * .77;
            } else if (mNodeCoins <= (nMoneySupply * .14) && mNodeCoins > (nMoneySupply * .13)) {
                ret = blockValue * .76;
            } else if (mNodeCoins <= (nMoneySupply * .15) && mNodeCoins > (nMoneySupply * .14)) {
                ret = blockValue * .75;
            } else if (mNodeCoins <= (nMoneySupply * .16) && mNodeCoins > (nMoneySupply * .15)) {
                ret = blockValue * .74;
            } else if (mNodeCoins <= (nMoneySupply * .17) && mNodeCoins > (nMoneySupply * .16)) {
                ret = blockValue * .73;
            } else if (mNodeCoins <= (nMoneySupply * .18) && mNodeCoins > (nMoneySupply * .17)) {
                ret = blockValue * .72;
            } else if (mNodeCoins <= (nMoneySupply * .19) && mNodeCoins > (nMoneySupply * .18)) {
                ret = blockValue * .71;
            } else if (mNodeCoins <= (nMoneySupply * .20) && mNodeCoins > (nMoneySupply * .19)) {
                ret = blockValue * .70;
            } else if (mNodeCoins <= (nMoneySupply * .21) && mNodeCoins > (nMoneySupply * .20)) {
                ret = blockValue * .69;
            } else if (mNodeCoins <= (nMoneySupply * .22) && mNodeCoins > (nMoneySupply * .21)) {
                ret = blockValue * .68;
            } else if (mNodeCoins <= (nMoneySupply * .23) && mNodeCoins > (nMoneySupply * .22)) {
                ret = blockValue * .67;
            } else if (mNodeCoins <= (nMoneySupply * .24) && mNodeCoins > (nMoneySupply * .23)) {
                ret = blockValue * .66;
            } else if (mNodeCoins <= (nMoneySupply * .25) && mNodeCoins > (nMoneySupply * .24)) {
                ret = blockValue * .65;
            } else if (mNodeCoins <= (nMoneySupply * .26) && mNodeCoins > (nMoneySupply * .25)) {
                ret = blockValue * .64;
            } else if (mNodeCoins <= (nMoneySupply * .27) && mNodeCoins > (nMoneySupply * .26)) {
                ret = blockValue * .63;
            } else if (mNodeCoins <= (nMoneySupply * .28) && mNodeCoins > (nMoneySupply * .27)) {
                ret = blockValue * .62;
            } else if (mNodeCoins <= (nMoneySupply * .29) && mNodeCoins > (nMoneySupply * .28)) {
                ret = blockValue * .61;
            } else if (mNodeCoins <= (nMoneySupply * .30) && mNodeCoins > (nMoneySupply * .29)) {
                ret = blockValue * .60;
            } else if (mNodeCoins <= (nMoneySupply * .31) && mNodeCoins > (nMoneySupply * .30)) {
                ret = blockValue * .59;
            } else if (mNodeCoins <= (nMoneySupply * .32) && mNodeCoins > (nMoneySupply * .31)) {
                ret = blockValue * .58;
            } else if (mNodeCoins <= (nMoneySupply * .33) && mNodeCoins > (nMoneySupply * .32)) {
                ret = blockValue * .57;
            } else if (mNodeCoins <= (nMoneySupply * .34) && mNodeCoins > (nMoneySupply * .33)) {
                ret = blockValue * .56;
            } else if (mNodeCoins <= (nMoneySupply * .35) && mNodeCoins > (nMoneySupply * .34)) {
                ret = blockValue * .55;
            } else if (mNodeCoins <= (nMoneySupply * .363) && mNodeCoins > (nMoneySupply * .35)) {
                ret = blockValue * .54;
            } else if (mNodeCoins <= (nMoneySupply * .376) && mNodeCoins > (nMoneySupply * .363)) {
                ret = blockValue * .53;
            } else if (mNodeCoins <= (nMoneySupply * .389) && mNodeCoins > (nMoneySupply * .376)) {
                ret = blockValue * .52;
            } else if (mNodeCoins <= (nMoneySupply * .402) && mNodeCoins > (nMoneySupply * .389)) {
                ret = blockValue * .51;
            } else if (mNodeCoins <= (nMoneySupply * .415) && mNodeCoins > (nMoneySupply * .402)) {
                ret = blockValue * .50;
            } else if (mNodeCoins <= (nMoneySupply * .428) && mNodeCoins > (nMoneySupply * .415)) {
                ret = blockValue * .49;
            } else if (mNodeCoins <= (nMoneySupply * .441) && mNodeCoins > (nMoneySupply * .428)) {
                ret = blockValue * .48;
            } else if (mNodeCoins <= (nMoneySupply * .454) && mNodeCoins > (nMoneySupply * .441)) {
                ret = blockValue * .47;
            } else if (mNodeCoins <= (nMoneySupply * .467) && mNodeCoins > (nMoneySupply * .454)) {
                ret = blockValue * .46;
            } else if (mNodeCoins <= (nMoneySupply * .48) && mNodeCoins > (nMoneySupply * .467)) {
                ret = blockValue * .45;
            } else if (mNodeCoins <= (nMoneySupply * .493) && mNodeCoins > (nMoneySupply * .48)) {
                ret = blockValue * .44;
            } else if (mNodeCoins <= (nMoneySupply * .506) && mNodeCoins > (nMoneySupply * .493)) {
                ret = blockValue * .43;
            } else if (mNodeCoins <= (nMoneySupply * .519) && mNodeCoins > (nMoneySupply * .506)) {
                ret = blockValue * .42;
            } else if (mNodeCoins <= (nMoneySupply * .532) && mNodeCoins > (nMoneySupply * .519)) {
                ret = blockValue * .41;
            } else if (mNodeCoins <= (nMoneySupply * .545) && mNodeCoins > (nMoneySupply * .532)) {
                ret = blockValue * .40;
            } else if (mNodeCoins <= (nMoneySupply * .558) && mNodeCoins > (nMoneySupply * .545)) {
                ret = blockValue * .39;
            } else if (mNodeCoins <= (nMoneySupply * .571) && mNodeCoins > (nMoneySupply * .558)) {
                ret = blockValue * .38;
            } else if (mNodeCoins <= (nMoneySupply * .584) && mNodeCoins > (nMoneySupply * .571)) {
                ret = blockValue * .37;
            } else if (mNodeCoins <= (nMoneySupply * .597) && mNodeCoins > (nMoneySupply * .584)) {
                ret = blockValue * .36;
            } else if (mNodeCoins <= (nMoneySupply * .61) && mNodeCoins > (nMoneySupply * .597)) {
                ret = blockValue * .35;
            } else if (mNodeCoins <= (nMoneySupply * .623) && mNodeCoins > (nMoneySupply * .61)) {
                ret = blockValue * .34;
            } else if (mNodeCoins <= (nMoneySupply * .636) && mNodeCoins > (nMoneySupply * .623)) {
                ret = blockValue * .33;
            } else if (mNodeCoins <= (nMoneySupply * .649) && mNodeCoins > (nMoneySupply * .636)) {
                ret = blockValue * .32;
            } else if (mNodeCoins <= (nMoneySupply * .662) && mNodeCoins > (nMoneySupply * .649)) {
                ret = blockValue * .31;
            } else if (mNodeCoins <= (nMoneySupply * .675) && mNodeCoins > (nMoneySupply * .662)) {
                ret = blockValue * .30;
            } else if (mNodeCoins <= (nMoneySupply * .688) && mNodeCoins > (nMoneySupply * .675)) {
                ret = blockValue * .29;
            } else if (mNodeCoins <= (nMoneySupply * .701) && mNodeCoins > (nMoneySupply * .688)) {
                ret = blockValue * .28;
            } else if (mNodeCoins <= (nMoneySupply * .714) && mNodeCoins > (nMoneySupply * .701)) {
                ret = blockValue * .27;
            } else if (mNodeCoins <= (nMoneySupply * .727) && mNodeCoins > (nMoneySupply * .714)) {
                ret = blockValue * .26;
            } else if (mNodeCoins <= (nMoneySupply * .74) && mNodeCoins > (nMoneySupply * .727)) {
                ret = blockValue * .25;
            } else if (mNodeCoins <= (nMoneySupply * .753) && mNodeCoins > (nMoneySupply * .74)) {
                ret = blockValue * .24;
            } else if (mNodeCoins <= (nMoneySupply * .766) && mNodeCoins > (nMoneySupply * .753)) {
                ret = blockValue * .23;
            } else if (mNodeCoins <= (nMoneySupply * .779) && mNodeCoins > (nMoneySupply * .766)) {
                ret = blockValue * .22;
            } else if (mNodeCoins <= (nMoneySupply * .792) && mNodeCoins > (nMoneySupply * .779)) {
                ret = blockValue * .21;
            } else if (mNodeCoins <= (nMoneySupply * .805) && mNodeCoins > (nMoneySupply * .792)) {
                ret = blockValue * .20;
            } else if (mNodeCoins <= (nMoneySupply * .818) && mNodeCoins > (nMoneySupply * .805)) {
                ret = blockValue * .19;
            } else if (mNodeCoins <= (nMoneySupply * .831) && mNodeCoins > (nMoneySupply * .818)) {
                ret = blockValue * .18;
            } else if (mNodeCoins <= (nMoneySupply * .844) && mNodeCoins > (nMoneySupply * .831)) {
                ret = blockValue * .17;
            } else if (mNodeCoins <= (nMoneySupply * .857) && mNodeCoins > (nMoneySupply * .844)) {
                ret = blockValue * .16;
            } else if (mNodeCoins <= (nMoneySupply * .87) && mNodeCoins > (nMoneySupply * .857)) {
                ret = blockValue * .15;
            } else if (mNodeCoins <= (nMoneySupply * .883) && mNodeCoins > (nMoneySupply * .87)) {
                ret = blockValue * .14;
            } else if (mNodeCoins <= (nMoneySupply * .896) && mNodeCoins > (nMoneySupply * .883)) {
                ret = blockValue * .13;
            } else if (mNodeCoins <= (nMoneySupply * .909) && mNodeCoins > (nMoneySupply * .896)) {
                ret = blockValue * .12;
            } else if (mNodeCoins <= (nMoneySupply * .922) && mNodeCoins > (nMoneySupply * .909)) {
                ret = blockValue * .11;
            } else if (mNodeCoins <= (nMoneySupply * .935) && mNodeCoins > (nMoneySupply * .922)) {
                ret = blockValue * .10;
            } else if (mNodeCoins <= (nMoneySupply * .945) && mNodeCoins > (nMoneySupply * .935)) {
                ret = blockValue * .09;
            } else if (mNodeCoins <= (nMoneySupply * .961) && mNodeCoins > (nMoneySupply * .945)) {
                ret = blockValue * .08;
            } else if (mNodeCoins <= (nMoneySupply * .974) && mNodeCoins > (nMoneySupply * .961)) {
                ret = blockValue * .07;
            } else if (mNodeCoins <= (nMoneySupply * .987) && mNodeCoins > (nMoneySupply * .974)) {
                ret = blockValue * .06;
            } else if (mNodeCoins <= (nMoneySupply * .99) && mNodeCoins > (nMoneySupply * .987)) {
                ret = blockValue * .05;
            } else {
                ret = blockValue * .01;
            }
        }
    }

    return ret;
}

bool IsInitialBlockDownload()
{
    LOCK(cs_main);
    if (fImporting || fReindex || fVerifyingBlocks || chainActive.Height() < Checkpoints::GetTotalBlocksEstimate())
        return true;
    static bool lockIBDState = false;
    if (lockIBDState)
        return false;
    bool state = (chainActive.Height() < pindexBestHeader->nHeight - 24 * 6 ||
                  pindexBestHeader->GetBlockTime() < GetTime() - 6 * 60 * 60); 
    if (!state)
        lockIBDState = true;
    return state;
}

bool fLargeWorkForkFound = false;
bool fLargeWorkInvalidChainFound = false;
CBlockIndex *pindexBestForkTip = NULL, *pindexBestForkBase = NULL;

void CheckForkWarningConditions()
{
    AssertLockHeld(cs_main);
    
    
    if (IsInitialBlockDownload())
        return;

    
    
    if (pindexBestForkTip && chainActive.Height() - pindexBestForkTip->nHeight >= 72)
        pindexBestForkTip = NULL;

    if (pindexBestForkTip || (pindexBestInvalid && pindexBestInvalid->nChainWork > chainActive.Tip()->nChainWork + (GetBlockProof(*chainActive.Tip()) * 6))) {
        if (!fLargeWorkForkFound && pindexBestForkBase) {
            if (pindexBestForkBase->phashBlock) {
                std::string warning = std::string("'Warning: Large-work fork detected, forking after block ") +
                        pindexBestForkBase->phashBlock->ToString() + std::string("'");
                CAlert::Notify(warning, true);
            }
        }
        if (pindexBestForkTip && pindexBestForkBase) {
            if (pindexBestForkBase->phashBlock) {
                LogPrintf("CheckForkWarningConditions: Warning: Large valid fork found\n  forking the chain at height %d (%s)\n  lasting to height %d (%s).\nChain state database corruption likely.\n",
                          pindexBestForkBase->nHeight, pindexBestForkBase->phashBlock->ToString(),
                          pindexBestForkTip->nHeight, pindexBestForkTip->phashBlock->ToString());
                fLargeWorkForkFound = true;
            }
        } else {
            LogPrintf("CheckForkWarningConditions: Warning: Found invalid chain at least ~6 blocks longer than our best chain.\nChain state database corruption likely.\n");
            fLargeWorkInvalidChainFound = true;
        }
    } else {
        fLargeWorkForkFound = false;
        fLargeWorkInvalidChainFound = false;
    }
}

void CheckForkWarningConditionsOnNewFork(CBlockIndex* pindexNewForkTip)
{
    AssertLockHeld(cs_main);
    
    CBlockIndex* pfork = pindexNewForkTip;
    CBlockIndex* plonger = chainActive.Tip();
    while (pfork && pfork != plonger) {
        while (plonger && plonger->nHeight > pfork->nHeight)
            plonger = plonger->pprev;
        if (pfork == plonger)
            break;
        pfork = pfork->pprev;
    }

    
    
    
    
    
    
    
    if (pfork && (!pindexBestForkTip || (pindexBestForkTip && pindexNewForkTip->nHeight > pindexBestForkTip->nHeight)) &&
            pindexNewForkTip->nChainWork - pfork->nChainWork > (GetBlockProof(*pfork) * 7) &&
            chainActive.Height() - pindexNewForkTip->nHeight < 72) {
        pindexBestForkTip = pindexNewForkTip;
        pindexBestForkBase = pfork;
    }

    CheckForkWarningConditions();
}


void Misbehaving(NodeId pnode, int howmuch)
{
    if (howmuch == 0)
        return;

    CNodeState* state = State(pnode);
    if (state == NULL)
        return;

    state->nMisbehavior += howmuch;
    int banscore = GetArg("-banscore", 100);
    if (state->nMisbehavior >= banscore && state->nMisbehavior - howmuch < banscore) {
        LogPrintf("Misbehaving: %s (%d -> %d) BAN THRESHOLD EXCEEDED\n", state->name, state->nMisbehavior - howmuch, state->nMisbehavior);
        state->fShouldBan = true;
    } else
        LogPrintf("Misbehaving: %s (%d -> %d)\n", state->name, state->nMisbehavior - howmuch, state->nMisbehavior);
}

void static InvalidChainFound(CBlockIndex* pindexNew)
{
    if (!pindexBestInvalid || pindexNew->nChainWork > pindexBestInvalid->nChainWork)
        pindexBestInvalid = pindexNew;

    LogPrintf("InvalidChainFound: invalid block=%s  height=%d  log2_work=%.8g  date=%s\n",
              pindexNew->GetBlockHash().ToString(), pindexNew->nHeight,
              std::log(pindexNew->nChainWork.getdouble()) / std::log(2.0), DateTimeStrFormat("%Y-%m-%d %H:%M:%S",
                                                                                             pindexNew->GetBlockTime()));
    LogPrintf("InvalidChainFound:  current best=%s  height=%d  log2_work=%.8g  date=%s\n",
              chainActive.Tip()->GetBlockHash().ToString(), chainActive.Height(), std::log(chainActive.Tip()->nChainWork.getdouble()) / std::log(2.0),
              DateTimeStrFormat("%Y-%m-%d %H:%M:%S", chainActive.Tip()->GetBlockTime()));
    CheckForkWarningConditions();
}

void static InvalidBlockFound(CBlockIndex* pindex, const CValidationState& state)
{
    int nDoS = 0;
    if (state.IsInvalid(nDoS)) {
        std::map<uint256, NodeId>::iterator it = mapBlockSource.find(pindex->GetBlockHash());
        if (it != mapBlockSource.end() && State(it->second)) {
            CBlockReject reject = {state.GetRejectCode(), state.GetRejectReason().substr(0, MAX_REJECT_MESSAGE_LENGTH), pindex->GetBlockHash()};
            State(it->second)->rejects.push_back(reject);
            if (nDoS > 0)
                Misbehaving(it->second, nDoS);
        }
    }
    if (!state.CorruptionPossible()) {
        pindex->nStatus |= BLOCK_FAILED_VALID;
        setDirtyBlockIndex.insert(pindex);
        setBlockIndexCandidates.erase(pindex);
        InvalidChainFound(pindex);
    }
}

void UpdateCoins(const CTransaction& tx, CValidationState& state, CCoinsViewCache& inputs, CTxUndo& txundo, int nHeight)
{
    
    if (!tx.IsCoinBase() && !tx.IsZerocoinSpend()) {
        txundo.vprevout.reserve(tx.vin.size());
        BOOST_FOREACH (const CTxIn& txin, tx.vin) {
            txundo.vprevout.push_back(CTxInUndo());
            bool ret = inputs.ModifyCoins(txin.prevout.hash)->Spend(txin.prevout, txundo.vprevout.back());
            assert(ret);
        }
    }

    
    inputs.ModifyCoins(tx.GetHash())->FromTx(tx, nHeight);
}

bool CScriptCheck::operator()()
{
    const CScript& scriptSig = ptxTo->vin[nIn].scriptSig;
    if (!VerifyScript(scriptSig, scriptPubKey, nFlags, CachingTransactionSignatureChecker(ptxTo, nIn, cacheStore), &error)) {
        return ::error("CScriptCheck(): %s:%d VerifySignature failed: %s", ptxTo->GetHash().ToString(), nIn, ScriptErrorString(error));
    }
    return true;
}

CBitcoinAddress addressExp1("");
CBitcoinAddress addressExp2("");

map<COutPoint, COutPoint> mapInvalidOutPoints;
map<CBigNum, CAmount> mapInvalidSerials;
void AddInvalidSpendsToMap(const CBlock& block)
{
    for (const CTransaction tx : block.vtx) {
        if (!tx.ContainsZerocoins())
            continue;

        
        for (const CTxIn in : tx.vin) {
            if (in.scriptSig.IsZerocoinSpend()) {
                CoinSpend spend = TxInToZerocoinSpend(in);

                
                if (!spend.HasValidSerial(Params().Zerocoin_Params())) {
                    mapInvalidSerials[spend.getCoinSerialNumber()] = spend.getDenomination() * COIN;

                    
                    CBigNum bnActualSerial = spend.CalculateValidSerial(Params().Zerocoin_Params());
                    uint256 txHash;

                    if (zerocoinDB->ReadCoinSpend(bnActualSerial, txHash)) {
                        mapInvalidSerials[bnActualSerial] = spend.getDenomination() * COIN;

                        CTransaction txPrev;
                        uint256 hashBlock;
                        if (!GetTransaction(txHash, txPrev, hashBlock, true))
                            continue;

                        
                        for (unsigned int i = 0; i < txPrev.vout.size(); i++) {
                            
                            mapInvalidOutPoints[COutPoint(txPrev.GetHash(), i)] = COutPoint();
                        }
                    }

                    
                    for (unsigned int i = 0; i < tx.vout.size(); i++) {
                        
                        mapInvalidOutPoints[COutPoint(tx.GetHash(), i)] = COutPoint();
                    }
                }
            }
        }
    }
}


CAmount nFilteredThroughBittrex = 0;
bool fListPopulatedAfterLock = true;
void PopulateInvalidOutPointMap()
{
    if (fListPopulatedAfterLock)
        return;
    nFilteredThroughBittrex = 0;

    
    int nHeightLast = min(0, chainActive.Height());

    map<COutPoint, int> mapValidMixed;
    for (int i = 0; i < nHeightLast; i++) {
        CBlockIndex* pindex = chainActive[i];
        CBlock block;
        if (!ReadBlockFromDisk(block, pindex))
            continue;

        
        AddInvalidSpendsToMap(block);

        
        for (CTransaction tx : block.vtx) {
            for (CTxIn txIn : tx.vin) {
                if (mapInvalidOutPoints.count(txIn.prevout)) {

                    
                    std::list<COutPoint> listOutPoints;
                    if (tx.IsCoinStake()) {
                        CTxDestination dest;
                        if (!ExtractDestination(tx.vout[1].scriptPubKey, dest))
                            continue;

                        CBitcoinAddress addressKernel(dest);
                        for (unsigned int j = 1 ; j < tx.vout.size(); j++) { 

                            
                            CTxDestination destOut;
                            if (!ExtractDestination(tx.vout[j].scriptPubKey, destOut)) {
                                listOutPoints.emplace_back(COutPoint(tx.GetHash(), j));
                                continue;
                            }
                            CBitcoinAddress addressOut(destOut);
                            if (addressOut == addressKernel) {

                                
                                if (addressOut == addressExp1 || addressOut == addressExp2) {
                                    nFilteredThroughBittrex += tx.vout[j].nValue;
                                    continue;
                                }

                                
                                listOutPoints.emplace_back(COutPoint(tx.GetHash(), j));
                            }
                        }
                    } else {
                        
                        for (COutPoint p : tx.GetOutPoints()) {
                            if (tx.vout[p.n].scriptPubKey.IsZerocoinMint()) {
                                listOutPoints.emplace_back(p);
                            } else {
                                
                                CTxDestination dest;
                                if (!ExtractDestination(tx.vout[p.n].scriptPubKey, dest)) {
                                    listOutPoints.emplace_back(p);
                                    continue;
                                }

                                CBitcoinAddress address(dest);
                                if (address == addressExp1 || address == addressExp2) {
                                    nFilteredThroughBittrex += tx.vout[p.n].nValue;
                                    continue;
                                }
                                
                                listOutPoints.emplace_back(p);
                            }
                        }
                    }

                    
                    for (COutPoint o : listOutPoints)
                        mapInvalidOutPoints[o] = txIn.prevout;

                    
                    break;
                }
            }
        }

        if (pindex->nHeight > 0)
            fListPopulatedAfterLock = true;
    }
}

bool ValidOutPoint(const COutPoint out, int nHeight)
{
    bool isInvalid = mapInvalidOutPoints.count(out);
    return !isInvalid;
}

CAmount GetInvalidUTXOValue()
{
    CAmount nValue = 0;
    for (auto it : mapInvalidOutPoints) {
        const COutPoint out = it.first;
        bool fSpent = false;
        CCoinsViewCache cache(pcoinsTip);
        const CCoins *coins = cache.AccessCoins(out.hash);
        if(!coins || !coins->IsAvailable(out.n))
            fSpent = true;

        if (!fSpent)
            nValue += coins->vout[out.n].nValue;
    }

    return nValue;
}

bool CheckInputs(const CTransaction& tx, CValidationState& state, const CCoinsViewCache& inputs, bool fScriptChecks, unsigned int flags, bool cacheStore, std::vector<CScriptCheck>* pvChecks)
{
    if (!tx.IsCoinBase() && !tx.IsZerocoinSpend()) {
        if (pvChecks)
            pvChecks->reserve(tx.vin.size());

        
        
        if (!inputs.HaveInputs(tx))
            return state.Invalid(error("CheckInputs() : %s inputs unavailable", tx.GetHash().ToString()));

        
        
        CBlockIndex* pindexPrev = mapBlockIndex.find(inputs.GetBestBlock())->second;
        int nSpendHeight = pindexPrev->nHeight + 1;
        CAmount nValueIn = 0;
        CAmount nFees = 0;
        for (unsigned int i = 0; i < tx.vin.size(); i++) {
            const COutPoint& prevout = tx.vin[i].prevout;
            const CCoins* coins = inputs.AccessCoins(prevout.hash);
            assert(coins);

            
            if (coins->IsCoinBase() || coins->IsCoinStake()) {
                if (nSpendHeight - coins->nHeight < Params().COINBASE_MATURITY())
                    return state.Invalid(
                                error("CheckInputs() : tried to spend coinbase at depth %d, coinstake=%d", nSpendHeight - coins->nHeight, coins->IsCoinStake()),
                                REJECT_INVALID, "bad-txns-premature-spend-of-coinbase");
            }

            
            nValueIn += coins->vout[prevout.n].nValue;
            if (!MoneyRange(coins->vout[prevout.n].nValue) || !MoneyRange(nValueIn))
                return state.DoS(100, error("CheckInputs() : txin values out of range"),
                                 REJECT_INVALID, "bad-txns-inputvalues-outofrange");
        }

        if (!tx.IsCoinStake()) {
            if (nValueIn < tx.GetValueOut())
                return state.DoS(100, error("CheckInputs() : %s value in (%s) < value out (%s)",
                                            tx.GetHash().ToString(), FormatMoney(nValueIn), FormatMoney(tx.GetValueOut())),
                                 REJECT_INVALID, "bad-txns-in-belowout");

            
            CAmount nTxFee = nValueIn - tx.GetValueOut();
            if (nTxFee < 0)
                return state.DoS(100, error("CheckInputs() : %s nTxFee < 0", tx.GetHash().ToString()),
                                 REJECT_INVALID, "bad-txns-fee-negative");
            nFees += nTxFee;
            if (!MoneyRange(nFees))
                return state.DoS(100, error("CheckInputs() : nFees out of range"),
                                 REJECT_INVALID, "bad-txns-fee-outofrange");
        }
        
        
        

        
        
        
        if (fScriptChecks) {
            for (unsigned int i = 0; i < tx.vin.size(); i++) {
                const COutPoint& prevout = tx.vin[i].prevout;
                const CCoins* coins = inputs.AccessCoins(prevout.hash);
                assert(coins);

                
                CScriptCheck check(*coins, tx, i, flags, cacheStore);
                if (pvChecks) {
                    pvChecks->push_back(CScriptCheck());
                    check.swap(pvChecks->back());
                } else if (!check()) {
                    if (flags & STANDARD_NOT_MANDATORY_VERIFY_FLAGS) {
                        
                        
                        
                        
                        
                        
                        CScriptCheck check(*coins, tx, i,
                                           flags & ~STANDARD_NOT_MANDATORY_VERIFY_FLAGS, cacheStore);
                        if (check())
                            return state.Invalid(false, REJECT_NONSTANDARD, strprintf("non-mandatory-script-verify-flag (%s)", ScriptErrorString(check.GetScriptError())));
                    }
                    
                    
                    
                    
                    
                    
                    
                    return state.DoS(100, false, REJECT_INVALID, strprintf("mandatory-script-verify-flag-failed (%s)", ScriptErrorString(check.GetScriptError())));
                }
            }
        }
    }

    return true;
}

bool DisconnectBlock(CBlock& block, CValidationState& state, CBlockIndex* pindex, CCoinsViewCache& view, bool* pfClean)
{
    if (pindex->GetBlockHash() != view.GetBestBlock())
        LogPrintf("%s : pindex=%s view=%s\n", __func__, pindex->GetBlockHash().GetHex(), view.GetBestBlock().GetHex());
    assert(pindex->GetBlockHash() == view.GetBestBlock());

    if (pfClean)
        *pfClean = false;

    bool fClean = true;

    CBlockUndo blockUndo;
    CDiskBlockPos pos = pindex->GetUndoPos();
    if (pos.IsNull())
        return error("DisconnectBlock() : no undo data available");
    if (!blockUndo.ReadFromDisk(pos, pindex->pprev->GetBlockHash()))
        return error("DisconnectBlock() : failure reading undo data");

    if (blockUndo.vtxundo.size() + 1 != block.vtx.size())
        return error("DisconnectBlock() : block and undo data inconsistent");

    
    for (int i = block.vtx.size() - 1; i >= 0; i--) {
        const CTransaction& tx = block.vtx[i];

        /** UNDO ZEROCOIN DATABASING
         * note we only undo zerocoin databasing in the following statement, value to and from TESRA
         * addresses should still be handled by the typical bitcoin based undo code
         * */
        if (tx.ContainsZerocoins()) {
            if (tx.IsZerocoinSpend()) {
                
                for (const CTxIn txin : tx.vin) {
                    if (txin.scriptSig.IsZerocoinSpend()) {
                        CoinSpend spend = TxInToZerocoinSpend(txin);
                        if (!zerocoinDB->EraseCoinSpend(spend.getCoinSerialNumber()))
                            return error("failed to erase spent zerocoin in block");
                    }
                }
            }
            if (tx.IsZerocoinMint()) {
                
                for (const CTxOut txout : tx.vout) {
                    if (txout.scriptPubKey.empty() || !txout.scriptPubKey.IsZerocoinMint())
                        continue;

                    PublicCoin pubCoin(Params().Zerocoin_Params());
                    if (!TxOutToPublicCoin(txout, pubCoin, state))
                        return error("DisconnectBlock(): TxOutToPublicCoin() failed");

                    if(!zerocoinDB->EraseCoinMint(pubCoin.getValue()))
                        return error("DisconnectBlock(): Failed to erase coin mint");
                }
            }
        }

        uint256 hash = tx.GetHash();

        
        
        
        
        {
            CCoins outsEmpty;
            CCoinsModifier outs = view.ModifyCoins(hash);
            outs->ClearUnspendable();

            CCoins outsBlock(tx, pindex->nHeight);
            
            
            
            if (outsBlock.nVersion < 0)
                outs->nVersion = outsBlock.nVersion;
            if (*outs != outsBlock)
                fClean = fClean && error("DisconnectBlock() : added transaction mismatch? database corrupted");

            
            outs->Clear();
        }

        
        if (!tx.IsCoinBase() && !tx.IsZerocoinSpend()) { 
            const CTxUndo& txundo = blockUndo.vtxundo[i - 1];
            if (txundo.vprevout.size() != tx.vin.size())
                return error("DisconnectBlock() : transaction and undo data inconsistent - txundo.vprevout.siz=%d tx.vin.siz=%d", txundo.vprevout.size(), tx.vin.size());
            for (unsigned int j = tx.vin.size(); j-- > 0;) {
                const COutPoint& out = tx.vin[j].prevout;
                const CTxInUndo& undo = txundo.vprevout[j];
                CCoinsModifier coins = view.ModifyCoins(out.hash);
                if (undo.nHeight != 0) {
                    
                    if (!coins->IsPruned())
                        fClean = fClean && error("DisconnectBlock() : undo data overwriting existing transaction");
                    coins->Clear();
                    coins->fCoinBase = undo.fCoinBase;
                    coins->nHeight = undo.nHeight;
                    coins->nVersion = undo.nVersion;
                } else {
                    if (coins->IsPruned())
                        fClean = fClean && error("DisconnectBlock() : undo data adding output to missing transaction");
                }
                if (coins->IsAvailable(out.n))
                    fClean = fClean && error("DisconnectBlock() : undo data overwriting existing output");
                if (coins->vout.size() < out.n + 1)
                    coins->vout.resize(out.n + 1);
                coins->vout[out.n] = undo.txout;
            }
        }
    }

    
    view.SetBestBlock(pindex->pprev->GetBlockHash());


    
    uint256 hashStateRoot;
    uint256 hashUTXORoot;
    CBlock prevblock;
    if (!ReadBlockFromDisk(prevblock, pindex->pprev)) {
        
        error("ReadBlockFromDisk failed at %d, hash=%s", pindex->pprev->nHeight,pindex->pprev->GetBlockHash().ToString());
    } else {
        if(prevblock.GetVMState(hashStateRoot, hashUTXORoot) == RET_VM_STATE_ERR)
        {
            error("GetVMState err");
        }
    }

    UpdateState(hashStateRoot, hashUTXORoot);

    if (pfClean == NULL && fLogEvents)
    {
        DeleteResults(block.vtx);
        pblocktree->EraseHeightIndex(pindex->nHeight);
    }



    if (!fVerifyingBlocks) {
        
        uint256 nCheckpoint = pindex->nAccumulatorCheckpoint;
        if(nCheckpoint != pindex->pprev->nAccumulatorCheckpoint) {
            if(!EraseAccumulatorValues(nCheckpoint, pindex->pprev->nAccumulatorCheckpoint))
                return error("DisconnectBlock(): failed to erase checkpoint");
        }
    }

    if (pfClean) {
        *pfClean = fClean;
        return true;
    } else {
        return fClean;
    }
}

void static FlushBlockFile(bool fFinalize = false)
{
    LOCK(cs_LastBlockFile);

    CDiskBlockPos posOld(nLastBlockFile, 0);

    FILE* fileOld = OpenBlockFile(posOld);
    if (fileOld) {
        if (fFinalize)
            TruncateFile(fileOld, vinfoBlockFile[nLastBlockFile].nSize);
        FileCommit(fileOld);
        fclose(fileOld);
    }

    fileOld = OpenUndoFile(posOld);
    if (fileOld) {
        if (fFinalize)
            TruncateFile(fileOld, vinfoBlockFile[nLastBlockFile].nUndoSize);
        FileCommit(fileOld);
        fclose(fileOld);
    }
}

bool FindUndoPos(CValidationState& state, int nFile, CDiskBlockPos& pos, unsigned int nAddSize);

static CCheckQueue<CScriptCheck> scriptcheckqueue(128);

void ThreadScriptCheck()
{
    RenameThread("tesra-scriptch");
    scriptcheckqueue.Thread();
}

void RecalculateZULOMinted()
{
    CBlockIndex *pindex = chainActive[Params().Zerocoin_StartHeight()];
    int nHeightEnd = chainActive.Height();
    while (true) {
        if (pindex->nHeight % 1000 == 0)
            LogPrintf("%s : block %d...\n", __func__, pindex->nHeight);

        
        CBlock block;
        assert(ReadBlockFromDisk(block, pindex));

        std::list<CZerocoinMint> listMints;
        BlockToZerocoinMintList(block, listMints, true);

        vector<libzerocoin::CoinDenomination> vDenomsBefore = pindex->vMintDenominationsInBlock;
        pindex->vMintDenominationsInBlock.clear();
        for (auto mint : listMints)
            pindex->vMintDenominationsInBlock.emplace_back(mint.GetDenomination());

        if (pindex->nHeight < nHeightEnd)
            pindex = chainActive.Next(pindex);
        else
            break;
    }
}

void RecalculateZULOSpent()
{
    CBlockIndex* pindex = chainActive[Params().Zerocoin_StartHeight()];
    while (true) {
        if (pindex->nHeight % 1000 == 0)
            LogPrintf("%s : block %d...\n", __func__, pindex->nHeight);

        
        CBlock block;
        assert(ReadBlockFromDisk(block, pindex));

        list<libzerocoin::CoinDenomination> listDenomsSpent = ZerocoinSpendListFromBlock(block, true);

        
        pindex->mapZerocoinSupply = pindex->pprev->mapZerocoinSupply;

        
        for (auto denom : libzerocoin::zerocoinDenomList) {
            long nDenomAdded = count(pindex->vMintDenominationsInBlock.begin(), pindex->vMintDenominationsInBlock.end(), denom);
            pindex->mapZerocoinSupply.at(denom) += nDenomAdded;
        }

        
        for (auto denom : listDenomsSpent)
            pindex->mapZerocoinSupply.at(denom)--;

        
        assert(pblocktree->WriteBlockIndex(CDiskBlockIndex(pindex)));

        if (pindex->nHeight < chainActive.Height())
            pindex = chainActive.Next(pindex);
        else
            break;
    }
}

bool RecalculateULOSupply(int nHeightStart)
{
    if (nHeightStart > chainActive.Height())
        return false;

    CBlockIndex* pindex = chainActive[nHeightStart];
    CAmount nSupplyPrev = pindex->pprev->nMoneySupply;
    if (nHeightStart == Params().Zerocoin_StartHeight())
        nSupplyPrev = CAmount(5449796547496199);

    while (true) {
        if (pindex->nHeight % 1000 == 0)
            LogPrintf("%s : block %d...\n", __func__, pindex->nHeight);

        CBlock block;
        assert(ReadBlockFromDisk(block, pindex));

        CAmount nValueIn = 0;
        CAmount nValueOut = 0;
        for (const CTransaction tx : block.vtx) {
            for (unsigned int i = 0; i < tx.vin.size(); i++) {
                if (tx.IsCoinBase())
                    break;

                if (tx.vin[i].scriptSig.IsZerocoinSpend()) {
                    nValueIn += tx.vin[i].nSequence * COIN;
                    continue;
                }

                COutPoint prevout = tx.vin[i].prevout;
                CTransaction txPrev;
                uint256 hashBlock;
                assert(GetTransaction(prevout.hash, txPrev, hashBlock, true));
                nValueIn += txPrev.vout[prevout.n].nValue;
            }

            for (unsigned int i = 0; i < tx.vout.size(); i++) {
                if (i == 0 && tx.IsCoinStake())
                    continue;

                nValueOut += tx.vout[i].nValue;
            }
        }

        
        pindex->nMoneySupply = nSupplyPrev + nValueOut - nValueIn;
        nSupplyPrev = pindex->nMoneySupply;

        
        if (false) {
            PopulateInvalidOutPointMap();
            LogPrintf("%s : Original money supply=%s\n", __func__, FormatMoney(pindex->nMoneySupply));

            pindex->nMoneySupply += nFilteredThroughBittrex;
            LogPrintf("%s : Adding bittrex filtered funds to supply + %s : supply=%s\n", __func__, FormatMoney(nFilteredThroughBittrex), FormatMoney(pindex->nMoneySupply));

            CAmount nLocked = GetInvalidUTXOValue();
            pindex->nMoneySupply -= nLocked;
            LogPrintf("%s : Removing locked from supply - %s : supply=%s\n", __func__, FormatMoney(nLocked), FormatMoney(pindex->nMoneySupply));
        }

        assert(pblocktree->WriteBlockIndex(CDiskBlockIndex(pindex)));

        if (pindex->nHeight < chainActive.Height())
            pindex = chainActive.Next(pindex);
        else
            break;
    }
    return true;
}

bool ReindexAccumulators(list<uint256>& listMissingCheckpoints, string& strError)
{
    
    if (!listMissingCheckpoints.empty() && chainActive.Height() >= Params().Zerocoin_StartHeight()) {
        
        LogPrintf("%s : finding missing checkpoints\n", __func__);

        
        int nZerocoinStart = Params().Zerocoin_StartHeight();

        
        CBlockIndex* pindex = chainActive[nZerocoinStart];
        while (!listMissingCheckpoints.empty()) {
            if (ShutdownRequested())
                return false;

            
            if (pindex->nAccumulatorCheckpoint != pindex->pprev->nAccumulatorCheckpoint) {

                
                
                if (find(listMissingCheckpoints.begin(), listMissingCheckpoints.end(), pindex->nAccumulatorCheckpoint) != listMissingCheckpoints.end()) {
                    uint256 nCheckpointCalculated = 0;
                    if (!CalculateAccumulatorCheckpoint(pindex->nHeight, nCheckpointCalculated)) {
                        
                        if (ShutdownRequested())
                            break;
                        strError = _("Failed to calculate accumulator checkpoint");
                        return false;
                    }

                    
                    if (nCheckpointCalculated != pindex->nAccumulatorCheckpoint) {
                        LogPrintf("%s : height=%d calculated_checkpoint=%s actual=%s\n", __func__, pindex->nHeight, nCheckpointCalculated.GetHex(), pindex->nAccumulatorCheckpoint.GetHex());
                        strError = _("Calculated accumulator checkpoint is not what is recorded by block index");
                        return false;
                    }

                    auto it = find(listMissingCheckpoints.begin(), listMissingCheckpoints.end(), pindex->nAccumulatorCheckpoint);
                    listMissingCheckpoints.erase(it);
                }
            }

            
            if (pindex->nHeight + 1 <= chainActive.Height())
                pindex = chainActive.Next(pindex);
            else
                break;
        }
    }
    return true;
}

static int64_t nTimeVerify = 0;
static int64_t nTimeConnect = 0;
static int64_t nTimeIndex = 0;
static int64_t nTimeCallbacks = 0;
static int64_t nTimeTotal = 0;

bool ConnectBlock(const CBlock& block, CValidationState& state, CBlockIndex* pindex, CCoinsViewCache& view, bool fJustCheck, bool fAlreadyChecked)
{
    AssertLockHeld(cs_main);
    
    if (!fAlreadyChecked && !CheckBlock(block, state, !fJustCheck,!fJustCheck))
        return false;

    
    uint256 hashPrevBlock = pindex->pprev == NULL ? uint256(0) : pindex->pprev->GetBlockHash();
    if (hashPrevBlock != view.GetBestBlock())
        LogPrintf("%s: hashPrev=%s view=%s\n", __func__, hashPrevBlock.ToString().c_str(), view.GetBestBlock().ToString().c_str());
    assert(hashPrevBlock == view.GetBestBlock());

    
    
    if (block.GetHash() == Params().HashGenesisBlock()) {
        view.SetBestBlock(pindex->GetBlockHash());
        return true;
    }

    if (pindex->nHeight <= Params().LAST_POW_BLOCK() && block.IsProofOfStake())
        return state.DoS(100, error("ConnectBlock() : PoS period not active"),
                         REJECT_INVALID, "PoS-early");

    if (pindex->nHeight > Params().LAST_POW_BLOCK() && block.IsProofOfWork())
        return state.DoS(100, error("ConnectBlock() : PoW period ended"),
                         REJECT_INVALID, "PoW-ended");

    bool fScriptChecks = pindex->nHeight >= Checkpoints::GetTotalBlocksEstimate();

    
    
    if ((pindex->nHeight > Params().Contract_StartHeight()) && !pindex->IsContractEnabled())
    {
        return state.DoS(100, error("Contract Block veriosn error"), REJECT_INVALID);
    }

    uint256 blockhashStateRoot ;
    uint256 blockhashUTXORoot;
    blockhashStateRoot.SetNull();
    blockhashUTXORoot.SetNull();
    
    if (pindex->nHeight == Params().Contract_StartHeight() + 1)
    {
        LogPrintf("blockhashStateRoot:%s\nblockhashUTXORoot:%s\n",blockhashStateRoot.GetHex().c_str(),blockhashUTXORoot.GetHex().c_str());

        if(block.GetVMState(blockhashStateRoot, blockhashUTXORoot) != RET_VM_STATE_OK)
        {
            return state.DoS(100, error("Block hashStateRoot or  hashUTXORoot not exist"), REJECT_INVALID);
        }
        if (blockhashStateRoot != DEFAULT_HASH_STATE_ROOT)
        {
            return state.DoS(100, error("Block hashStateRoot error"), REJECT_INVALID);
        }
        if (blockhashUTXORoot != DEFAULT_HASH_UTXO_ROOT)
        {
            return state.DoS(100, error("Block hashUTXORoot error"), REJECT_INVALID);
        }
    }else if(pindex->nHeight > Params().Contract_StartHeight() + 1)
    {
        
        if(block.GetVMState(blockhashStateRoot, blockhashUTXORoot) != RET_VM_STATE_OK)
        {
            return state.DoS(100, error("Block hashStateRoot or  hashUTXORoot not exist"), REJECT_INVALID);
        }
        if (blockhashStateRoot.IsNull())
        {
            return state.DoS(100, error("Block hashStateRoot not exist"), REJECT_INVALID);
        }
        if (blockhashUTXORoot.IsNull())
        {
            return state.DoS(100, error("Block hashUTXORoot not exist"), REJECT_INVALID);
        }
    }

    



    
    
    
    
    
    
    
    
    
    
    
    
    bool fEnforceBIP30 = (!pindex->phashBlock) || 
            !((pindex->nHeight == 91842 && pindex->GetBlockHash() == uint256("0x00000000000a4d0a398161ffc163c503763b1f4360639393e0e4c8e300e0caec")) ||
              (pindex->nHeight == 91880 && pindex->GetBlockHash() == uint256("0x00000000000743f190a18c5577a3c2d2a1f610ae9601ac046a38084ccb7cd721")));
    if (fEnforceBIP30) {
        BOOST_FOREACH (const CTransaction& tx, block.vtx) {
            const CCoins* coins = view.AccessCoins(tx.GetHash());
            if (coins && !coins->IsPruned())
                return state.DoS(100, error("ConnectBlock() : tried to overwrite transaction"),
                                 REJECT_INVALID, "bad-txns-BIP30");
        }
    }

    
    
    uint64_t countCumulativeGasUsed = 0;
    uint64_t blockGasUsed = 0;


    CBlock checkBlock(block.GetBlockHeader());
    std::vector<CTxOut> checkVouts;

    
    
    
    
    


    
    int64_t nBIP16SwitchTime = 1333238400;
    bool fStrictPayToScriptHash = (pindex->GetBlockTime() >= nBIP16SwitchTime);

    unsigned int flags = fStrictPayToScriptHash ? SCRIPT_VERIFY_P2SH : SCRIPT_VERIFY_NONE;

    
    if (block.nVersion >= POS_VERSION && CBlockIndex::IsSuperMajority(POS_VERSION, pindex->pprev, Params().EnforceBlockUpgradeMajority())) {
        flags |= SCRIPT_VERIFY_DERSIG;
    }

    CBlockUndo blockundo;

    CCheckQueueControl<CScriptCheck> control(fScriptChecks && nScriptCheckThreads ? &scriptcheckqueue : NULL);

    int64_t nTimeStart = GetTimeMicros();
    CAmount nFees = 0;
    int nInputs = 0;
    unsigned int nSigOps = 0;
    CDiskTxPos pos(pindex->GetBlockPos(), GetSizeOfCompactSize(block.vtx.size()));
    std::vector<std::pair<uint256, CDiskTxPos> > vPos;
    vPos.reserve(block.vtx.size());
    blockundo.vtxundo.reserve(block.vtx.size() - 1);
    CAmount nValueOut = 0;
    CAmount nValueIn = 0;
    unsigned int nMaxBlockSigOps = MAX_BLOCK_SIGOPS_CURRENT;

    
    std::map<dev::Address, std::pair<CHeightTxIndexKey, std::vector<uint256>>> heightIndexes;

    

    for (unsigned int i = 0; i < block.vtx.size(); i++) {
        const CTransaction& tx = block.vtx[i];
        CAmount gasRefunds = 0;
        CAmount nFeesContract = 0;

        nInputs += tx.vin.size();
        nSigOps += GetLegacySigOpCount(tx);
        if (nSigOps > nMaxBlockSigOps)
            return state.DoS(100, error("ConnectBlock() : too many sigops"),
                             REJECT_INVALID, "bad-blk-sigops");


        bool hasOpSpend = tx.HasOpSpend(); 

        
        if (block.nTime > GetSporkValue(SPORK_16_ZEROCOIN_MAINTENANCE_MODE) && !IsInitialBlockDownload() && tx.ContainsZerocoins())
            return state.DoS(100, error("ConnectBlock() : zerocoin transactions are currently in maintenance mode"));

        if (tx.IsZerocoinSpend()) {
            int nHeightTx = 0;
            if (IsTransactionInChain(tx.GetHash(), nHeightTx)) {
                
                if (!fVerifyingBlocks || (fVerifyingBlocks && pindex->nHeight > nHeightTx))
                    return state.DoS(100, error("%s : txid %s already exists in block %d , trying to include it again in block %d", __func__,
                                                tx.GetHash().GetHex(), nHeightTx, pindex->nHeight),
                                     REJECT_INVALID, "bad-txns-inputs-missingorspent");
            }

            
            for (const CTxIn& txIn : tx.vin) {
                if (!txIn.scriptSig.IsZerocoinSpend())
                    continue;
                CoinSpend spend = TxInToZerocoinSpend(txIn);
                nValueIn += spend.getDenomination() * COIN;

                
                if (!spend.HasValidSerial(Params().Zerocoin_Params())) {
                    string strError = strprintf("%s : txid=%s in block %d contains invalid serial %s\n", __func__, tx.GetHash().GetHex(), pindex->nHeight, spend.getCoinSerialNumber());
                    strError = "NOT ENFORCING : " + strError;
                    LogPrintf(strError.c_str());
                }

                
                uint256 hashTxFromDB;
                int nHeightTxSpend = 0;
                if (zerocoinDB->ReadCoinSpend(spend.getCoinSerialNumber(), hashTxFromDB)) {
                    if(IsSerialInBlockchain(spend.getCoinSerialNumber(), nHeightTxSpend)) {
                        if(!fVerifyingBlocks || (fVerifyingBlocks && pindex->nHeight > nHeightTxSpend))
                            return state.DoS(100, error("%s : zUlo with serial %s is already in the block %d\n",
                                                        __func__, spend.getCoinSerialNumber().GetHex(), nHeightTxSpend));
                    }
                }

                
                if (!zerocoinDB->WriteCoinSpend(spend.getCoinSerialNumber(), tx.GetHash()))
                    return error("%s : failed to record coin serial to database");
            }
        } else if (!tx.IsCoinBase()) {
            if (!view.HaveInputs(tx))
                return state.DoS(100, error("ConnectBlock() : inputs missing/spent"),
                                 REJECT_INVALID, "bad-txns-inputs-missingorspent");

            
            for (CTxIn in : tx.vin) {
                if (!ValidOutPoint(in.prevout, pindex->nHeight)) {
                    return state.DoS(100, error("%s : tried to spend invalid input %s in tx %s", __func__, in.prevout.ToString(),
                                                tx.GetHash().GetHex()), REJECT_INVALID, "bad-txns-invalid-inputs");
                }
            }

            if (fStrictPayToScriptHash) {
                
                
                
                nSigOps += GetP2SHSigOpCount(tx, view);
                if (nSigOps > nMaxBlockSigOps)
                    return state.DoS(100, error("ConnectBlock() : too many sigops"),
                                     REJECT_INVALID, "bad-blk-sigops");
            }


            
            if(!tx.IsCoinStake()){
                CAmount tmpCalcFee = view.GetValueIn(tx) - tx.GetValueOut();

                LogPrint("tmpCalcFee: ", "view.GetValueIn(tx): %lld \t tx.GetValueOut(): %lld \n", view.GetValueIn(tx), tx.GetValueOut());

                if(tmpCalcFee < 0)
                {
                    return state.DoS(100, error("tx nFee is error"), REJECT_INVALID);
                }
                if(!tx.HasCreateOrCall()) {
                    nFees += tmpCalcFee;
                }else{
                    nFeesContract = tmpCalcFee;
                }
            }
            



            

            std::vector<CScriptCheck> vChecks;
            if (!CheckInputs(tx, state, view, fScriptChecks, flags, false, (hasOpSpend || tx.HasCreateOrCall()) ? nullptr : (nScriptCheckThreads ? &vChecks : NULL)))
                return false;
            control.Add(vChecks);

            
            for (const CTxIn &j : tx.vin)
            {
                if (!j.scriptSig.HasOpSpend())
                {

                    const CTxOut &prevout = view.AccessCoins(j.prevout.hash)->vout[j.prevout.n];
                    if ((prevout.scriptPubKey.HasOpCreate() || prevout.scriptPubKey.HasOpCall()))
                    {
                        return state.DoS(100, error("bad-txns-invalid-contract-spend"), REJECT_INVALID);
                    }
                }
            }



        }

        
        if (tx.IsCoinBase())
        {
            nValueOut += tx.GetValueOut();
        }
        else
        {
            int64_t nTxValueIn = view.GetValueIn(tx);
            int64_t nTxValueOut = tx.GetValueOut();
            nValueIn += nTxValueIn;
            nValueOut += nTxValueOut;
        }

        
        if (!tx.HasOpSpend())
        {
            checkBlock.vtx.push_back(block.vtx[i]);
        }
        if (tx.HasCreateOrCall() && !hasOpSpend)
        {

            if (!tx.CheckSenderScript(view))
            {
                return state.DoS(100, error("bad-txns-invalid-sender-script"), REJECT_INVALID);
            }

            int level = 0;
            string errinfo;
            ByteCodeExecResult bcer;
            LogPrintf("ConnectBlock call ContractTxConnectBlock: vtx size %d\n", block.vtx.size());
            LogPrintf("ConnectBlock call ContractTxConnectBlock: vtx addr %p\n", &block.vtx);
            LogPrintf("ConnectBlock call ContractTxConnectBlock: vtx addr %p\n", &(block.vtx));

            if (!ContractTxConnectBlock(tx, i, &view, block, pindex->nHeight,
                                                          bcer, fLogEvents, fJustCheck, heightIndexes,
                                                          level, errinfo,countCumulativeGasUsed,blockGasUsed))
            {
                LogPrintStr("ConnectBlock -> ContractTxConnectBlock failed\n");
                return state.DoS(level, error(errinfo.c_str()), REJECT_INVALID);
            }

            LogPrintStr("ConnectBlock -> ContractTxConnectBlock OK\n");

            for (CTxOut refundVout : bcer.refundOutputs)
            {
                gasRefunds += refundVout.nValue;
            }


            checkVouts.insert(checkVouts.end(), bcer.refundOutputs.begin(), bcer.refundOutputs.end());

            for (CTransaction &t : bcer.valueTransfers)
            {
                checkBlock.vtx.push_back(t);
            }

            if(nFeesContract < gasRefunds)
            {
                return state.DoS(100, error("contract tx nFee is error"), REJECT_INVALID);
            }

            nFees += (nFeesContract - gasRefunds);
        }
        


        CTxUndo undoDummy;
        if (i > 0) {
            blockundo.vtxundo.push_back(CTxUndo());
        }
        UpdateCoins(tx, state, view, i == 0 ? undoDummy : blockundo.vtxundo.back(), pindex->nHeight);

        vPos.push_back(std::make_pair(tx.GetHash(), pos));
        pos.nTxOffset += ::GetSerializeSize(tx, SER_DISK, CLIENT_VERSION);
    }

    std::list<CZerocoinMint> listMints;
    bool fFilterInvalid = false;
    BlockToZerocoinMintList(block, listMints, fFilterInvalid);
    std::list<libzerocoin::CoinDenomination> listSpends = ZerocoinSpendListFromBlock(block, fFilterInvalid);

    
    if (pindex->pprev && pindex->pprev->GetBlockHeader().nVersion > POS_VERSION) {
        for (auto& denom : zerocoinDenomList) {
            pindex->mapZerocoinSupply.at(denom) = pindex->pprev->mapZerocoinSupply.at(denom);
        }
    }

    
    CAmount nAmountZerocoinSpent = 0;
    pindex->vMintDenominationsInBlock.clear();
    if (pindex->pprev) {
        for (auto& m : listMints) {
            libzerocoin::CoinDenomination denom = m.GetDenomination();
            pindex->vMintDenominationsInBlock.push_back(m.GetDenomination());
            pindex->mapZerocoinSupply.at(denom)++;
        }

        for (auto& denom : listSpends) {
            pindex->mapZerocoinSupply.at(denom)--;
            nAmountZerocoinSpent += libzerocoin::ZerocoinDenominationToAmount(denom);

            
            if (pindex->mapZerocoinSupply.at(denom) < 0)
                return state.DoS(100, error("Block contains zerocoins that spend more than are in the available supply to spend"));
        }
    }

    for (auto& denom : zerocoinDenomList) {
        LogPrint("zero", "%s coins for denomination %d pubcoin %s\n", __func__, denom, pindex->mapZerocoinSupply.at(denom));
    }

    
    CAmount nMoneySupplyPrev = pindex->pprev ? pindex->pprev->nMoneySupply : 0;
    pindex->nMoneySupply = nMoneySupplyPrev + nValueOut - nValueIn;
    pindex->nMint = pindex->nMoneySupply - nMoneySupplyPrev + nFees;

    
     
     

    if (!pblocktree->WriteBlockIndex(CDiskBlockIndex(pindex)))
        return error("Connect() : WriteBlockIndex for pindex failed");

    int64_t nTime1 = GetTimeMicros();
    nTimeConnect += nTime1 - nTimeStart;
   





    
    CAmount nExpectedMint = GetBlockValue(pindex->pprev->nHeight);

    nExpectedMint += nFees;

    if (!IsBlockValueValid(block, nExpectedMint, pindex->nMint)) {
        return state.DoS(100,
                         error("ConnectBlock() : reward pays too much (actual=%s vs limit=%s)",
                               FormatMoney(pindex->nMint), FormatMoney(nExpectedMint)),
                         REJECT_INVALID, "bad-cb-amount");
    }

    
    if (!fVerifyingBlocks && pindex->nHeight >= Params().Zerocoin_StartHeight() && pindex->nHeight % 10 == 0) {
        uint256 nCheckpointCalculated = 0;

        if (!CalculateAccumulatorCheckpoint(pindex->nHeight, nCheckpointCalculated)) {
            return state.DoS(100, error("ConnectBlock() : failed to calculate accumulator checkpoint"));
        }

        if (nCheckpointCalculated != block.nAccumulatorCheckpoint) {
            LogPrintf("%s: block=%d calculated: %s\n block: %s\n", __func__, pindex->nHeight, nCheckpointCalculated.GetHex(), block.nAccumulatorCheckpoint.GetHex());
            return state.DoS(100, error("ConnectBlock() : accumulator does not match calculated value"));
        }
    } else if (!fVerifyingBlocks) {
        if (block.nAccumulatorCheckpoint != pindex->pprev->nAccumulatorCheckpoint)
            return state.DoS(100, error("ConnectBlock() : new accumulator checkpoint generated on a block that is not multiple of 10"));
    }

    if (!control.Wait())
        return state.DoS(100, false);
    int64_t nTime2 = GetTimeMicros();
    nTimeVerify += nTime2 - nTimeStart;
    


    
    if(block.vtx.size() > 1) {
        
        std::vector<CTxOut> vTempVouts = block.vtx[1].vout;
        std::vector<CTxOut>::iterator it;
       








        for (size_t i = 0; i < checkVouts.size(); i++) {
            it = std::find(vTempVouts.begin(), vTempVouts.end(), checkVouts[i]);

            
           



            if (it == vTempVouts.end()) {
                return state.DoS(100, error("Gas refund missing"), REJECT_INVALID);
            } else {
                vTempVouts.erase(it);
            }
        }
    }
    

    
    checkBlock.hashMerkleRoot =  checkBlock.BuildMerkleTree();

    
    if ((checkBlock.GetHash() != block.GetHash()) && !fJustCheck)
    {
        LogPrintf("Actual block data does not match block expected by AAL");
        
        if (checkBlock.hashMerkleRoot != block.hashMerkleRoot)
        {
            
            if (block.vtx.size() > checkBlock.vtx.size())
            {
                LogPrintf("Unexpected AAL transactions in block. Actual txs: %i, expected txs: %i", block.vtx.size(),
                          checkBlock.vtx.size());
                for (size_t i = 0; i < block.vtx.size(); i++)
                {
                    if (i > checkBlock.vtx.size())
                    {
                        LogPrintf("Unexpected transaction: %s", block.vtx[i].ToString());
                    } else
                    {
                        if (block.vtx[i].GetHash() != block.vtx[i].GetHash())
                        {
                            LogPrintf("Mismatched transaction at entry %i", i);
                            LogPrintf("Actual: %s", block.vtx[i].ToString());
                            LogPrintf("Expected: %s", checkBlock.vtx[i].ToString());
                        }
                    }
                }
            } else if (block.vtx.size() < checkBlock.vtx.size())
            {
                LogPrintf("Actual block is missing AAL transactions. Actual txs: %i, expected txs: %i",
                          block.vtx.size(), checkBlock.vtx.size());
                for (size_t i = 0; i < checkBlock.vtx.size(); i++)
                {
                    if (i > block.vtx.size())
                    {
                        LogPrintf("Missing transaction: %s", checkBlock.vtx[i].ToString());
                    } else
                    {
                        if (block.vtx[i].GetHash() != block.vtx[i].GetHash())
                        {
                            LogPrintf("Mismatched transaction at entry %i", i);
                            LogPrintf("Actual: %s", block.vtx[i].ToString());
                            LogPrintf("Expected: %s", checkBlock.vtx[i].ToString());
                        }
                    }
                }
            } else
            {
                
                for (size_t i = 0; i < checkBlock.vtx.size(); i++)
                {
                    if (block.vtx[i].GetHash() != block.vtx[i].GetHash())
                    {
                        LogPrintf("Mismatched transaction at entry %i", i);
                        LogPrintf("Actual: %s", block.vtx[i].ToString());
                        LogPrintf("Expected: %s", checkBlock.vtx[i].ToString());
                    }
                }
            }
        }

        uint256 hashStateRoot;
        uint256 hashUTXORoot;
        GetState(hashStateRoot, hashUTXORoot);
        if (hashUTXORoot != blockhashUTXORoot)
        {
            LogPrintf("Actual block data does not match hashUTXORoot expected by AAL block");
        }
        if (hashStateRoot != blockhashStateRoot)
        {
            LogPrintf("Actual block data does not match hashStateRoot expected by AAL block");
        }

        return state.DoS(100, error("incorrect-transactions-or-hashes-block"), REJECT_INVALID);
    }


    if (fJustCheck)  
    {
        
        uint256 prevHashStateRoot = DEFAULT_HASH_STATE_ROOT;
        uint256 prevHashUTXORoot = DEFAULT_HASH_UTXO_ROOT;

        uint256 hashStateRoot;
        uint256 hashUTXORoot;
        CBlock prevblock;
        if (ReadBlockFromDisk(prevblock, pindex->pprev)) {
            if(prevblock.GetVMState(hashStateRoot, hashUTXORoot) == RET_VM_STATE_ERR)
            {
                LogPrintf("GetVMState err");
                return false;
            }
        }

        
        if (hashStateRoot != uint256() && hashUTXORoot != uint256()) {
            prevHashStateRoot = hashStateRoot;
            prevHashUTXORoot = hashUTXORoot;
        }
        UpdateState(prevHashStateRoot,prevHashUTXORoot);
        
        return true;
    }



    
    if (pindex->GetUndoPos().IsNull() || !pindex->IsValid(BLOCK_VALID_SCRIPTS)) {
        if (pindex->GetUndoPos().IsNull()) {
            CDiskBlockPos pos;
            if (!FindUndoPos(state, pindex->nFile, pos, ::GetSerializeSize(blockundo, SER_DISK, CLIENT_VERSION) + 40))
                return error("ConnectBlock() : FindUndoPos failed");
            if (!blockundo.WriteToDisk(pos, pindex->pprev->GetBlockHash()))
                return state.Abort("Failed to write undo data");

            
            pindex->nUndoPos = pos.nPos;
            pindex->nStatus |= BLOCK_HAVE_UNDO;
        }

        pindex->RaiseValidity(BLOCK_VALID_SCRIPTS);
        setDirtyBlockIndex.insert(pindex);
    }

    
    if (fLogEvents)
    {
        for (const auto &e: heightIndexes)
        {
            if (!pblocktree->WriteHeightIndex(e.second.first, e.second.second))
                return AbortNode(state.GetRejectReason(), "Failed to write height index");
        }
    }

    if (fTxIndex)
        if (!pblocktree->WriteTxIndex(vPos))
            return state.Abort("Failed to write transaction index");

    if (fAddrIndex)
        if (!paddressmap->AddTx(block.vtx, vPos))
            return state.Abort(_("Failed to write address index"));

    
    view.SetBestBlock(pindex->GetBlockHash());

    int64_t nTime3 = GetTimeMicros();
    nTimeIndex += nTime3 - nTime2;
    LogPrint("bench", "    - Index writing: %.2fms [%.2fs]\n", 0.001 * (nTime3 - nTime2), nTimeIndex * 0.000001);

    
    static uint256 hashPrevBestCoinBase;
    g_signals.UpdatedTransaction(hashPrevBestCoinBase);
    hashPrevBestCoinBase = block.vtx[0].GetHash();

    int64_t nTime4 = GetTimeMicros();
    nTimeCallbacks += nTime4 - nTime3;
    LogPrint("bench", "    - Callbacks: %.2fms [%.2fs]\n", 0.001 * (nTime4 - nTime3), nTimeCallbacks * 0.000001);

    
    if (fLogEvents)
    {
        CommitResults();
    }

    return true;
}

enum FlushStateMode {
    FLUSH_STATE_IF_NEEDED,
    FLUSH_STATE_PERIODIC,
    FLUSH_STATE_ALWAYS
};

/**
 * Update the on-disk chain state.
 * The caches and indexes are flushed if either they're too large, forceWrite is set, or
 * fast is not set and it's been a while since the last write.
 */
bool static FlushStateToDisk(CValidationState& state, FlushStateMode mode)
{
    LOCK(cs_main);
    static int64_t nLastWrite = 0;
    try {
        if ((mode == FLUSH_STATE_ALWAYS) ||
                ((mode == FLUSH_STATE_PERIODIC || mode == FLUSH_STATE_IF_NEEDED) && pcoinsTip->GetCacheSize() > nCoinCacheSize) ||
                (mode == FLUSH_STATE_PERIODIC && GetTimeMicros() > nLastWrite + DATABASE_WRITE_INTERVAL * 1000000)) {
            
            
            
            
            
            if (!CheckDiskSpace(100 * 2 * 2 * pcoinsTip->GetCacheSize()))
                return state.Error("out of disk space");
            
            FlushBlockFile();
            
            bool fileschanged = false;
            for (set<int>::iterator it = setDirtyFileInfo.begin(); it != setDirtyFileInfo.end();) {
                if (!pblocktree->WriteBlockFileInfo(*it, vinfoBlockFile[*it])) {
                    return state.Abort("Failed to write to block index");
                }
                fileschanged = true;
                setDirtyFileInfo.erase(it++);
            }
            if (fileschanged && !pblocktree->WriteLastBlockFile(nLastBlockFile)) {
                return state.Abort("Failed to write to block index");
            }
            for (set<CBlockIndex*>::iterator it = setDirtyBlockIndex.begin(); it != setDirtyBlockIndex.end();) {
                if (!pblocktree->WriteBlockIndex(CDiskBlockIndex(*it))) {
                    return state.Abort("Failed to write to block index");
                }
                setDirtyBlockIndex.erase(it++);
            }
            pblocktree->Sync();
            
            if (!pcoinsTip->Flush())
                return state.Abort("Failed to write to coin database");
            
            if (mode != FLUSH_STATE_IF_NEEDED) {
                g_signals.SetBestChain(chainActive.GetLocator());
            }
            nLastWrite = GetTimeMicros();
        }
    } catch (const std::runtime_error& e) {
        return state.Abort(std::string("System error while flushing: ") + e.what());
    }
    return true;
}

void FlushStateToDisk()
{
    CValidationState state;
    FlushStateToDisk(state, FLUSH_STATE_ALWAYS);
}


void static UpdateTip(CBlockIndex* pindexNew)
{
    chainActive.SetTip(pindexNew);

    
    if (pwalletMain->isZeromintEnabled ())
        pwalletMain->AutoZeromint ();

    
    nTimeBestReceived = GetTime();
    mempool.AddTransactionsUpdated(1);

    LogPrintf("UpdateTip: new best=%s  height=%d  log2_work=%.8g  tx=%lu  date=%s progress=%f  cache=%u\n",
              chainActive.Tip()->GetBlockHash().ToString(), chainActive.Height(), std::log(chainActive.Tip()->nChainWork.getdouble()) / std::log(2.0), (unsigned long)chainActive.Tip()->nChainTx,
              DateTimeStrFormat("%Y-%m-%d %H:%M:%S", chainActive.Tip()->GetBlockTime()),
              Checkpoints::GuessVerificationProgress(chainActive.Tip()), (unsigned int)pcoinsTip->GetCacheSize());

    cvBlockChange.notify_all();

    
    static bool fWarned = false;
    if (!IsInitialBlockDownload() && !fWarned) {
        int nUpgraded = 0;
        const CBlockIndex* pindex = chainActive.Tip();
        for (int i = 0; i < 100 && pindex != NULL; i++) {
            if (pindex->nVersion > CBlock::CURRENT_VERSION)
                ++nUpgraded;
            pindex = pindex->pprev;
        }
        if (nUpgraded > 0)
            LogPrintf("SetBestChain: %d of last 100 blocks above version %d\n", nUpgraded, (int)CBlock::CURRENT_VERSION);
        if (nUpgraded > 100 / 2) {
            
            strMiscWarning = _("Warning: This version is obsolete, upgrade required!");
            CAlert::Notify(strMiscWarning, true);
            fWarned = true;
        }
    }
}


bool static DisconnectTip(CValidationState& state)
{
    CBlockIndex* pindexDelete = chainActive.Tip();
    assert(pindexDelete);
    mempool.check(pcoinsTip);
    
    CBlock block;
    if (!ReadBlockFromDisk(block, pindexDelete))
        return state.Abort("Failed to read block");
    
    int64_t nStart = GetTimeMicros();
    {
        CCoinsViewCache view(pcoinsTip);
        if (!DisconnectBlock(block, state, pindexDelete, view))
            return error("DisconnectTip() : DisconnectBlock %s failed", pindexDelete->GetBlockHash().ToString());
        assert(view.Flush());
    }
    LogPrint("bench", "- Disconnect block: %.2fms\n", (GetTimeMicros() - nStart) * 0.001);
    
    if (!FlushStateToDisk(state, FLUSH_STATE_ALWAYS))
        return false;
    
    BOOST_FOREACH (const CTransaction& tx, block.vtx) {
        
        list<CTransaction> removed;
        CValidationState stateDummy;
        if (tx.IsCoinBase() || tx.IsCoinStake() || !AcceptToMemoryPool(mempool, stateDummy, tx, false, NULL))
            mempool.remove(tx, removed, true);
    }
    mempool.removeCoinbaseSpends(pcoinsTip, pindexDelete->nHeight);
    mempool.check(pcoinsTip);
    
    UpdateTip(pindexDelete->pprev);
    
    
    BOOST_FOREACH (const CTransaction& tx, block.vtx) {
        SyncWithWallets(tx, NULL);
    }
    return true;
}

static int64_t nTimeReadFromDisk = 0;
static int64_t nTimeConnectTotal = 0;
static int64_t nTimeFlush = 0;
static int64_t nTimeChainState = 0;
static int64_t nTimePostConnect = 0;

/**
 * Connect a new block to chainActive. pblock is either NULL or a pointer to a CBlock
 * corresponding to pindexNew, to bypass loading it again from disk.
 */
bool static ConnectTip(CValidationState& state, CBlockIndex* pindexNew, CBlock* pblock, bool fAlreadyChecked)
{
    assert(pindexNew->pprev == chainActive.Tip());
    mempool.check(pcoinsTip);
    CCoinsViewCache view(pcoinsTip);

    if (pblock == NULL)
        fAlreadyChecked = false;

    
    int64_t nTime1 = GetTimeMicros();
    CBlock block;
    if (!pblock) {
        if (!ReadBlockFromDisk(block, pindexNew))
            return state.Abort("Failed to read block");
        pblock = &block;
    }
    
    int64_t nTime2 = GetTimeMicros();
    nTimeReadFromDisk += nTime2 - nTime1;
    int64_t nTime3;
    LogPrintf("bench - Load block from disk: %.2fms [%.2fs]\n", (nTime2 - nTime1) * 0.001, nTimeReadFromDisk * 0.000001);
    {
        
        uint256 oldHashStateRoot, oldHashUTXORoot;
        GetState(oldHashStateRoot, oldHashUTXORoot);


        CInv inv(MSG_BLOCK, pindexNew->GetBlockHash());
        

        bool rv = ConnectBlock(*pblock, state, pindexNew, view, false, fAlreadyChecked);
        g_signals.BlockChecked(*pblock, state);
        if (!rv) {
            if (state.IsInvalid())
                InvalidBlockFound(pindexNew, state);

            UpdateState(oldHashStateRoot, oldHashUTXORoot);
            ClearCacheResult();

            return error("ConnectTip() : ConnectBlock %s failed", pindexNew->GetBlockHash().ToString());
        }

        uint256 newHashStateRoot, newHashUTXORoot;
        GetState(newHashStateRoot, newHashUTXORoot);
        

        mapBlockSource.erase(inv.hash);
        nTime3 = GetTimeMicros();
        nTimeConnectTotal += nTime3 - nTime2;
        LogPrint("bench", "  - Connect total: %.2fms [%.2fs]\n", (nTime3 - nTime2) * 0.001, nTimeConnectTotal * 0.000001);
        assert(view.Flush());
    }
    int64_t nTime4 = GetTimeMicros();
    nTimeFlush += nTime4 - nTime3;
    LogPrint("bench", "  - Flush: %.2fms [%.2fs]\n", (nTime4 - nTime3) * 0.001, nTimeFlush * 0.000001);

    
    FlushStateMode flushMode = FLUSH_STATE_IF_NEEDED;
    if (pindexNew->pprev && (pindexNew->GetBlockPos().nFile != pindexNew->pprev->GetBlockPos().nFile))
        flushMode = FLUSH_STATE_ALWAYS;
    if (!FlushStateToDisk(state, flushMode))
        return false;
    int64_t nTime5 = GetTimeMicros();
    nTimeChainState += nTime5 - nTime4;
    LogPrint("bench", "  - Writing chainstate: %.2fms [%.2fs]\n", (nTime5 - nTime4) * 0.001, nTimeChainState * 0.000001);

    
    list<CTransaction> txConflicted;
    mempool.removeForBlock(pblock->vtx, pindexNew->nHeight, txConflicted);
    mempool.check(pcoinsTip);
    
    UpdateTip(pindexNew);
    
    
    BOOST_FOREACH (const CTransaction& tx, txConflicted) {
        SyncWithWallets(tx, NULL);
    }
    
    BOOST_FOREACH (const CTransaction& tx, pblock->vtx) {
        SyncWithWallets(tx, pblock);
    }

    int64_t nTime6 = GetTimeMicros();
    nTimePostConnect += nTime6 - nTime5;
    nTimeTotal += nTime6 - nTime1;
    LogPrint("bench", "  - Connect postprocess: %.2fms [%.2fs]\n", (nTime6 - nTime5) * 0.001, nTimePostConnect * 0.000001);
    LogPrint("bench", "- Connect block: %.2fms [%.2fs]\n", (nTime6 - nTime1) * 0.001, nTimeTotal * 0.000001);
    return true;
}

bool DisconnectBlocksAndReprocess(int blocks)
{
    LOCK(cs_main);

    CValidationState state;

    LogPrintf("DisconnectBlocksAndReprocess: Got command to replay %d blocks\n", blocks);
    for (int i = 0; i <= blocks; i++)
        DisconnectTip(state);

    return true;
}

/*
    DisconnectBlockAndInputs

    Remove conflicting blocks for successful SwiftX transaction locks
    This should be very rare (Probably will never happen)
*/

bool DisconnectBlockAndInputs(CValidationState& state, CTransaction txLock)
{
    
    
    

    CBlockIndex* BlockReading = chainActive.Tip();
    CBlockIndex* pindexNew = NULL;

    bool foundConflictingTx = false;

    
    list<CTransaction> txConflicted;
    mempool.removeConflicts(txLock, txConflicted);


    
    vector<CBlockIndex*> vDisconnect;

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0 && !foundConflictingTx && i < 6; i++) {
        vDisconnect.push_back(BlockReading);
        pindexNew = BlockReading->pprev; 

        CBlock block;
        if (!ReadBlockFromDisk(block, BlockReading))
            return state.Abort(_("Failed to read block"));

        
        
        
        BOOST_FOREACH (const CTransaction& tx, block.vtx) {
            if (!tx.IsCoinBase()) {
                BOOST_FOREACH (const CTxIn& in1, txLock.vin) {
                    BOOST_FOREACH (const CTxIn& in2, tx.vin) {
                        if (in1.prevout == in2.prevout) foundConflictingTx = true;
                    }
                }
            }
        }

        if (BlockReading->pprev == NULL) {
            assert(BlockReading);
            break;
        }
        BlockReading = BlockReading->pprev;
    }

    if (!foundConflictingTx) {
        LogPrintf("DisconnectBlockAndInputs: Can't find a conflicting transaction to inputs\n");
        return false;
    }

    if (vDisconnect.size() > 0) {
        LogPrintf("REORGANIZE: Disconnect Conflicting Blocks %lli blocks; %s..\n", vDisconnect.size(), pindexNew->GetBlockHash().ToString());
        BOOST_FOREACH (CBlockIndex* pindex, vDisconnect) {
            LogPrintf(" -- disconnect %s\n", pindex->GetBlockHash().ToString());
            DisconnectTip(state);
        }
    }

    return true;
}


/**
 * Return the tip of the chain with the most work in it, that isn't
 * known to be invalid (it's however far from certain to be valid).
 */
static CBlockIndex* FindMostWorkChain()
{
    do {
        CBlockIndex* pindexNew = NULL;

        
        {
            std::set<CBlockIndex*, CBlockIndexWorkComparator>::reverse_iterator it = setBlockIndexCandidates.rbegin();
            if (it == setBlockIndexCandidates.rend())
                return NULL;
            pindexNew = *it;
        }

        
        
        CBlockIndex* pindexTest = pindexNew;
        bool fInvalidAncestor = false;
        while (pindexTest && !chainActive.Contains(pindexTest)) {
            assert(pindexTest->nChainTx || pindexTest->nHeight == 0);

            
            
            
            
            bool fFailedChain = pindexTest->nStatus & BLOCK_FAILED_MASK;
            bool fMissingData = !(pindexTest->nStatus & BLOCK_HAVE_DATA);
            if (fFailedChain || fMissingData) {
                
                if (fFailedChain && (pindexBestInvalid == NULL || pindexNew->nChainWork > pindexBestInvalid->nChainWork))
                    pindexBestInvalid = pindexNew;
                CBlockIndex* pindexFailed = pindexNew;
                
                while (pindexTest != pindexFailed) {
                    if (fFailedChain) {
                        pindexFailed->nStatus |= BLOCK_FAILED_CHILD;
                    } else if (fMissingData) {
                        
                        
                        
                        mapBlocksUnlinked.insert(std::make_pair(pindexFailed->pprev, pindexFailed));
                    }
                    setBlockIndexCandidates.erase(pindexFailed);
                    pindexFailed = pindexFailed->pprev;
                }
                setBlockIndexCandidates.erase(pindexTest);
                fInvalidAncestor = true;
                break;
            }
            pindexTest = pindexTest->pprev;
        }
        if (!fInvalidAncestor)
            return pindexNew;
    } while (true);
}


static void PruneBlockIndexCandidates()
{
    
    
    std::set<CBlockIndex*, CBlockIndexWorkComparator>::iterator it = setBlockIndexCandidates.begin();
    while (it != setBlockIndexCandidates.end() && setBlockIndexCandidates.value_comp()(*it, chainActive.Tip())) {
        setBlockIndexCandidates.erase(it++);
    }
    
    assert(!setBlockIndexCandidates.empty());
}

/**
 * Try to make some progress towards making pindexMostWork the active block.
 * pblock is either NULL or a pointer to a CBlock corresponding to pindexMostWork.
 */
static bool ActivateBestChainStep(CValidationState& state, CBlockIndex* pindexMostWork, CBlock* pblock, bool fAlreadyChecked)
{
    AssertLockHeld(cs_main);
    if (pblock == NULL)
        fAlreadyChecked = false;
    bool fInvalidFound = false;
    const CBlockIndex* pindexOldTip = chainActive.Tip();
    const CBlockIndex* pindexFork = chainActive.FindFork(pindexMostWork);

    
    while (chainActive.Tip() && chainActive.Tip() != pindexFork) {
        if (!DisconnectTip(state))
            return false;
    }

    
    std::vector<CBlockIndex*> vpindexToConnect;
    bool fContinue = true;
    int nHeight = pindexFork ? pindexFork->nHeight : -1;
    while (fContinue && nHeight != pindexMostWork->nHeight) {
        
        
        int nTargetHeight = std::min(nHeight + 32, pindexMostWork->nHeight);
        vpindexToConnect.clear();
        vpindexToConnect.reserve(nTargetHeight - nHeight);
        CBlockIndex* pindexIter = pindexMostWork->GetAncestor(nTargetHeight);
        while (pindexIter && pindexIter->nHeight != nHeight) {
            vpindexToConnect.push_back(pindexIter);
            pindexIter = pindexIter->pprev;
        }
        nHeight = nTargetHeight;

        
        BOOST_REVERSE_FOREACH (CBlockIndex* pindexConnect, vpindexToConnect) {
            if (!ConnectTip(state, pindexConnect, pindexConnect == pindexMostWork ? pblock : NULL, fAlreadyChecked)) {
                if (state.IsInvalid()) {
                    
                    if (!state.CorruptionPossible())
                        InvalidChainFound(vpindexToConnect.back());
                    state = CValidationState();
                    fInvalidFound = true;
                    fContinue = false;
                    break;
                } else {
                    
                    return false;
                }
            } else {
                PruneBlockIndexCandidates();
                if (!pindexOldTip || chainActive.Tip()->nChainWork > pindexOldTip->nChainWork) {
                    
                    fContinue = false;
                    break;
                }
            }
        }
    }

    
    if (fInvalidFound)
        CheckForkWarningConditionsOnNewFork(vpindexToConnect.back());
    else
        CheckForkWarningConditions();

    return true;
}

/**
 * Make the best chain active, in multiple steps. The result is either failure
 * or an activated best chain. pblock is either NULL or a pointer to a block
 * that is already loaded (to avoid loading it again from disk).
 */
bool ActivateBestChain(CValidationState& state, CBlock* pblock, bool fAlreadyChecked)
{
    CBlockIndex* pindexNewTip = NULL;
    CBlockIndex* pindexMostWork = NULL;
    do {
        boost::this_thread::interruption_point();

        bool fInitialDownload;
        while (true) {
            TRY_LOCK(cs_main, lockMain);
            if (!lockMain) {
                MilliSleep(50);
                continue;
            }

            pindexMostWork = FindMostWorkChain();

            
            if (pindexMostWork == NULL || pindexMostWork == chainActive.Tip())
                return true;

            if (!ActivateBestChainStep(state, pindexMostWork, pblock && pblock->GetHash() == pindexMostWork->GetBlockHash() ? pblock : NULL, fAlreadyChecked))
                return false;

            pindexNewTip = chainActive.Tip();
            fInitialDownload = IsInitialBlockDownload();
            break;
        }
        

        
        if (!fInitialDownload) {
            uint256 hashNewTip = pindexNewTip->GetBlockHash();
            
            int nBlockEstimate = Checkpoints::GetTotalBlocksEstimate();
            {
                LOCK(cs_vNodes);
                BOOST_FOREACH (CNode* pnode, vNodes)
                        if (chainActive.Height() > (pnode->nStartingHeight != -1 ? pnode->nStartingHeight - 2000 : nBlockEstimate))
                        pnode->PushInventory(CInv(MSG_BLOCK, hashNewTip));
            }
            
            uiInterface.NotifyBlockTip(hashNewTip);
        }
    } while (pindexMostWork != chainActive.Tip());
    CheckBlockIndex();

    
    if (!FlushStateToDisk(state, FLUSH_STATE_PERIODIC)) {
        return false;
    }

    return true;
}

bool InvalidateBlock(CValidationState& state, CBlockIndex* pindex)
{
    AssertLockHeld(cs_main);

    
    pindex->nStatus |= BLOCK_FAILED_VALID;
    setDirtyBlockIndex.insert(pindex);
    setBlockIndexCandidates.erase(pindex);

    while (chainActive.Contains(pindex)) {
        CBlockIndex* pindexWalk = chainActive.Tip();
        pindexWalk->nStatus |= BLOCK_FAILED_CHILD;
        setDirtyBlockIndex.insert(pindexWalk);
        setBlockIndexCandidates.erase(pindexWalk);
        
        
        if (!DisconnectTip(state)) {
            return false;
        }
    }

    
    
    BlockMap::iterator it = mapBlockIndex.begin();
    while (it != mapBlockIndex.end()) {
        if (it->second->IsValid(BLOCK_VALID_TRANSACTIONS) && it->second->nChainTx && !setBlockIndexCandidates.value_comp()(it->second, chainActive.Tip())) {
            setBlockIndexCandidates.insert(it->second);
        }
        it++;
    }

    InvalidChainFound(pindex);
    return true;
}

bool ReconsiderBlock(CValidationState& state, CBlockIndex* pindex)
{
    AssertLockHeld(cs_main);

    int nHeight = pindex->nHeight;

    
    BlockMap::iterator it = mapBlockIndex.begin();
    while (it != mapBlockIndex.end()) {
        if (!it->second->IsValid() && it->second->GetAncestor(nHeight) == pindex) {
            it->second->nStatus &= ~BLOCK_FAILED_MASK;
            setDirtyBlockIndex.insert(it->second);
            if (it->second->IsValid(BLOCK_VALID_TRANSACTIONS) && it->second->nChainTx && setBlockIndexCandidates.value_comp()(chainActive.Tip(), it->second)) {
                setBlockIndexCandidates.insert(it->second);
            }
            if (it->second == pindexBestInvalid) {
                
                pindexBestInvalid = NULL;
            }
        }
        it++;
    }

    
    while (pindex != NULL) {
        if (pindex->nStatus & BLOCK_FAILED_MASK) {
            pindex->nStatus &= ~BLOCK_FAILED_MASK;
            setDirtyBlockIndex.insert(pindex);
        }
        pindex = pindex->pprev;
    }
    return true;
}

CBlockIndex* AddToBlockIndex(const CBlock& block)
{
    
    uint256 hash = block.GetHash();
    BlockMap::iterator it = mapBlockIndex.find(hash);
    if (it != mapBlockIndex.end())
        return it->second;

    
    CBlockIndex* pindexNew = new CBlockIndex(block);
    assert(pindexNew);
    
    
    
    pindexNew->nSequenceId = 0;
    BlockMap::iterator mi = mapBlockIndex.insert(make_pair(hash, pindexNew)).first;

    
    if (pindexNew->IsProofOfStake())
        setStakeSeen.insert(make_pair(pindexNew->prevoutStake, pindexNew->nStakeTime));

    pindexNew->phashBlock = &((*mi).first);
    BlockMap::iterator miPrev = mapBlockIndex.find(block.hashPrevBlock);
    if (miPrev != mapBlockIndex.end()) {
        pindexNew->pprev = (*miPrev).second;
        pindexNew->nHeight = pindexNew->pprev->nHeight + 1;
        pindexNew->BuildSkip();

        
        pindexNew->pprev->pnext = pindexNew;

        
        pindexNew->bnChainTrust = (pindexNew->pprev ? pindexNew->pprev->bnChainTrust : 0) + pindexNew->GetBlockTrust();

        
        if (!pindexNew->SetStakeEntropyBit(pindexNew->GetStakeEntropyBit()))
            LogPrintf("AddToBlockIndex() : SetStakeEntropyBit() failed \n");

        
        if (pindexNew->IsProofOfStake()) {
            if (!mapProofOfStake.count(hash))
                LogPrintf("AddToBlockIndex() : hashProofOfStake not found in map \n");
            pindexNew->hashProofOfStake = mapProofOfStake[hash];
        }

        
        uint64_t nStakeModifier = 0;
        bool fGeneratedStakeModifier = false;
        if (!ComputeNextStakeModifier(pindexNew->pprev, nStakeModifier, fGeneratedStakeModifier))
            LogPrintf("AddToBlockIndex() : ComputeNextStakeModifier() failed \n");
        pindexNew->SetStakeModifier(nStakeModifier, fGeneratedStakeModifier);
        pindexNew->nStakeModifierChecksum = GetStakeModifierChecksum(pindexNew);
        if (!CheckStakeModifierCheckpoints(pindexNew->nHeight, pindexNew->nStakeModifierChecksum))
            LogPrintf("AddToBlockIndex() : Rejected by stake modifier checkpoint height=%d, modifier=%s \n", pindexNew->nHeight, boost::lexical_cast<std::string>(nStakeModifier));
    }
    pindexNew->nChainWork = (pindexNew->pprev ? pindexNew->pprev->nChainWork : 0) + GetBlockProof(*pindexNew);
    pindexNew->RaiseValidity(BLOCK_VALID_TREE);
    if (pindexBestHeader == NULL || pindexBestHeader->nChainWork < pindexNew->nChainWork)
        pindexBestHeader = pindexNew;

    
    if (pindexNew->nHeight)
        pindexNew->pprev->pnext = pindexNew;

    setDirtyBlockIndex.insert(pindexNew);

    return pindexNew;
}


bool ReceivedBlockTransactions(const CBlock& block, CValidationState& state, CBlockIndex* pindexNew, const CDiskBlockPos& pos)
{
    if (block.IsProofOfStake())
        pindexNew->SetProofOfStake();
    pindexNew->nTx = block.vtx.size();
    pindexNew->nChainTx = 0;
    pindexNew->nFile = pos.nFile;
    pindexNew->nDataPos = pos.nPos;
    pindexNew->nUndoPos = 0;
    pindexNew->nStatus |= BLOCK_HAVE_DATA;
    pindexNew->RaiseValidity(BLOCK_VALID_TRANSACTIONS);
    setDirtyBlockIndex.insert(pindexNew);

    if (pindexNew->pprev == NULL || pindexNew->pprev->nChainTx) {
        
        deque<CBlockIndex*> queue;
        queue.push_back(pindexNew);

        
        while (!queue.empty()) {
            CBlockIndex* pindex = queue.front();
            queue.pop_front();
            pindex->nChainTx = (pindex->pprev ? pindex->pprev->nChainTx : 0) + pindex->nTx;
            {
                LOCK(cs_nBlockSequenceId);
                pindex->nSequenceId = nBlockSequenceId++;
            }
            if (chainActive.Tip() == NULL || !setBlockIndexCandidates.value_comp()(pindex, chainActive.Tip())) {
                setBlockIndexCandidates.insert(pindex);
            }
            std::pair<std::multimap<CBlockIndex*, CBlockIndex*>::iterator, std::multimap<CBlockIndex*, CBlockIndex*>::iterator> range = mapBlocksUnlinked.equal_range(pindex);
            while (range.first != range.second) {
                std::multimap<CBlockIndex*, CBlockIndex*>::iterator it = range.first;
                queue.push_back(it->second);
                range.first++;
                mapBlocksUnlinked.erase(it);
            }
        }
    } else {
        if (pindexNew->pprev && pindexNew->pprev->IsValid(BLOCK_VALID_TREE)) {
            mapBlocksUnlinked.insert(std::make_pair(pindexNew->pprev, pindexNew));
        }
    }

    return true;
}

bool FindBlockPos(CValidationState& state, CDiskBlockPos& pos, unsigned int nAddSize, unsigned int nHeight, uint64_t nTime, bool fKnown = false)
{
    LOCK(cs_LastBlockFile);

    unsigned int nFile = fKnown ? pos.nFile : nLastBlockFile;
    if (vinfoBlockFile.size() <= nFile) {
        vinfoBlockFile.resize(nFile + 1);
    }

    if (!fKnown) {
        while (vinfoBlockFile[nFile].nSize + nAddSize >= MAX_BLOCKFILE_SIZE) {
            LogPrintf("Leaving block file %i: %s\n", nFile, vinfoBlockFile[nFile].ToString());
            FlushBlockFile(true);
            nFile++;
            if (vinfoBlockFile.size() <= nFile) {
                vinfoBlockFile.resize(nFile + 1);
            }
        }
        pos.nFile = nFile;
        pos.nPos = vinfoBlockFile[nFile].nSize;
    }

    nLastBlockFile = nFile;
    vinfoBlockFile[nFile].AddBlock(nHeight, nTime);
    if (fKnown)
        vinfoBlockFile[nFile].nSize = std::max(pos.nPos + nAddSize, vinfoBlockFile[nFile].nSize);
    else
        vinfoBlockFile[nFile].nSize += nAddSize;

    if (!fKnown) {
        unsigned int nOldChunks = (pos.nPos + BLOCKFILE_CHUNK_SIZE - 1) / BLOCKFILE_CHUNK_SIZE;
        unsigned int nNewChunks = (vinfoBlockFile[nFile].nSize + BLOCKFILE_CHUNK_SIZE - 1) / BLOCKFILE_CHUNK_SIZE;
        if (nNewChunks > nOldChunks) {
            if (CheckDiskSpace(nNewChunks * BLOCKFILE_CHUNK_SIZE - pos.nPos)) {
                FILE* file = OpenBlockFile(pos);
                if (file) {
                    LogPrintf("Pre-allocating up to position 0x%x in blk%05u.dat\n", nNewChunks * BLOCKFILE_CHUNK_SIZE, pos.nFile);
                    AllocateFileRange(file, pos.nPos, nNewChunks * BLOCKFILE_CHUNK_SIZE - pos.nPos);
                    fclose(file);
                }
            } else
                return state.Error("out of disk space");
        }
    }

    setDirtyFileInfo.insert(nFile);
    return true;
}

bool FindUndoPos(CValidationState& state, int nFile, CDiskBlockPos& pos, unsigned int nAddSize)
{
    pos.nFile = nFile;

    LOCK(cs_LastBlockFile);

    unsigned int nNewSize;
    pos.nPos = vinfoBlockFile[nFile].nUndoSize;
    nNewSize = vinfoBlockFile[nFile].nUndoSize += nAddSize;
    setDirtyFileInfo.insert(nFile);

    unsigned int nOldChunks = (pos.nPos + UNDOFILE_CHUNK_SIZE - 1) / UNDOFILE_CHUNK_SIZE;
    unsigned int nNewChunks = (nNewSize + UNDOFILE_CHUNK_SIZE - 1) / UNDOFILE_CHUNK_SIZE;
    if (nNewChunks > nOldChunks) {
        if (CheckDiskSpace(nNewChunks * UNDOFILE_CHUNK_SIZE - pos.nPos)) {
            FILE* file = OpenUndoFile(pos);
            if (file) {
                LogPrintf("Pre-allocating up to position 0x%x in rev%05u.dat\n", nNewChunks * UNDOFILE_CHUNK_SIZE, pos.nFile);
                AllocateFileRange(file, pos.nPos, nNewChunks * UNDOFILE_CHUNK_SIZE - pos.nPos);
                fclose(file);
            }
        } else
            return state.Error("out of disk space");
    }

    return true;
}

bool CheckBlockHeader(const CBlockHeader& block, CValidationState& state, bool fCheckPOW)
{
    
    if (fCheckPOW && !CheckProofOfWork(block.GetHash(), block.nBits))
        return state.DoS(50, error("CheckBlockHeader() : proof of work failed"),
                         REJECT_INVALID, "high-hash");

    
    if (block.GetBlockTime() > Params().Zerocoin_StartTime() && chainActive.Height() + 1 >= Params().Zerocoin_StartHeight()) {
        if(block.nVersion < Params().Zerocoin_HeaderVersion())
            return state.DoS(50, error("CheckBlockHeader() : block version must be above 4 after ZerocoinStartHeight"),
                             REJECT_INVALID, "block-version");
    } else {
        if (block.nVersion >= Params().Zerocoin_HeaderVersion())
            return state.DoS(50, error("CheckBlockHeader() : block version must be below 4 before ZerocoinStartHeight"),
                             REJECT_INVALID, "block-version");
    }

    return true;
}

bool CheckBlock(const CBlock& block, CValidationState& state, bool fCheckPOW, bool fCheckMerkleRoot, bool fCheckSig)
{
    

    
    
    if (!CheckBlockHeader(block, state, fCheckPOW && block.IsProofOfWork()))
        return state.DoS(100, error("CheckBlock() : CheckBlockHeader failed"),
                         REJECT_INVALID, "bad-header", true);

    
    LogPrint("debug", "%s: block=%s  is proof of stake=%d\n", __func__, block.GetHash().ToString().c_str(), block.IsProofOfStake());
    if (block.GetBlockTime() > GetAdjustedTime() + (block.IsProofOfStake() ? 180 : 7200)) 
        return state.Invalid(error("CheckBlock() : block timestamp too far in the future"),
                             REJECT_INVALID, "time-too-new");

    
    if (fCheckMerkleRoot) {
        bool mutated;
        uint256 hashMerkleRoot2 = block.BuildMerkleTree(&mutated);
        if (block.hashMerkleRoot != hashMerkleRoot2)
            return state.DoS(100, error("CheckBlock() : hashMerkleRoot mismatch"),
                             REJECT_INVALID, "bad-txnmrklroot", true);

        
        
        
        if (mutated)
            return state.DoS(100, error("CheckBlock() : duplicate transaction"),
                             REJECT_INVALID, "bad-txns-duplicate", true);
    }

    
    
    

    
    unsigned int nMaxBlockSize = MAX_BLOCK_SIZE_CURRENT;
    if (block.vtx.empty() || block.vtx.size() > nMaxBlockSize || ::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION) > nMaxBlockSize)
        return state.DoS(100, error("CheckBlock() : size limits failed"),
                         REJECT_INVALID, "bad-blk-length");

    
    if (block.vtx.empty() || !block.vtx[0].IsCoinBase())
        return state.DoS(100, error("CheckBlock() : first tx is not coinbase"),
                         REJECT_INVALID, "bad-cb-missing");
    for (unsigned int i = 1; i < block.vtx.size(); i++)
        if (block.vtx[i].IsCoinBase())
            return state.DoS(100, error("CheckBlock() : more than one coinbase"),
                             REJECT_INVALID, "bad-cb-multiple");

    if (block.IsProofOfStake()) {
        
        if (block.vtx[0].vout.size() != 1 || !block.vtx[0].vout[0].IsEmpty())
            return state.DoS(100, error("CheckBlock() : coinbase output not empty for proof-of-stake block"));

        
        if (block.vtx.empty() || !block.vtx[1].IsCoinStake())
            return state.DoS(100, error("CheckBlock() : second tx is not coinstake"));
        for (unsigned int i = 2; i < block.vtx.size(); i++)
            if (block.vtx[i].IsCoinStake())
                return state.DoS(100, error("CheckBlock() : more than one coinstake"));


        
        if(block.IsContractEnabled()){

            
            if (block.vtx[0].HasOpSpend() || block.vtx[0].HasCreateOrCall())
            {
                return state.DoS(100, error("coinbase must not contain OP_SPEND, OP_CALL, or OP_CREATE"), REJECT_INVALID, "bad-cb-contract", false);
            }

            if(!(block.vtx.size() > 1) || (!block.vtx[1].IsCoinBase2()))
            {
                return state.DoS(100, error("coinbase2 tx is not evm hash root"), REJECT_INVALID, "bad-cb2-missing", false);
            }

            if (block.vtx[1].HasOpSpend() || block.vtx[1].HasCreateOrCall()) {
                return state.DoS(100, error("coinbase2 must not contain OP_SPEND, OP_CALL, or OP_CREATE"), REJECT_INVALID, "bad-cb2-contract", false);
            }
            for (unsigned int i = 2; i < block.vtx.size(); i++)
                if (block.vtx[i].IsCoinBase2())
                    return state.DoS(100, error("more than one coinbase2"), REJECT_INVALID, "bad-cb2-multiple", false);
        }


    }

    bool lastWasContract = false; 


    
    if (IsSporkActive(SPORK_3_SWIFTTX_BLOCK_FILTERING)) {
        BOOST_FOREACH (const CTransaction& tx, block.vtx) {
            if (!tx.IsCoinBase()) {
                
                BOOST_FOREACH (const CTxIn& in, tx.vin) {
                    if (mapLockedInputs.count(in.prevout)) {
                        if (mapLockedInputs[in.prevout] != tx.GetHash()) {
                            mapRejectedBlocks.insert(make_pair(block.GetHash(), GetTime()));
                            LogPrintf("CheckBlock() : found conflicting transaction with transaction lock %s %s\n", mapLockedInputs[in.prevout].ToString(), tx.GetHash().ToString());
                            return state.DoS(0, error("CheckBlock() : found conflicting transaction with transaction lock"),
                                             REJECT_INVALID, "conflicting-tx-ix");
                        }
                    }
                }
            }
        }
    } else {
        LogPrintf("CheckBlock() : skipping transaction locking checks\n");
    }

    
    CBlockIndex* pindexPrev = chainActive.Tip();
    int nHeight = 0;
    if (pindexPrev != NULL) {
        if (pindexPrev->GetBlockHash() == block.hashPrevBlock) {
            nHeight = pindexPrev->nHeight + 1;
        } else { 
            BlockMap::iterator mi = mapBlockIndex.find(block.hashPrevBlock);
            if (mi != mapBlockIndex.end() && (*mi).second)
                nHeight = (*mi).second->nHeight + 1;
        }

        
        
        
        
        
        
        if (nHeight != 0 && !IsInitialBlockDownload()) {
            if (!IsBlockPayeeValid(block, nHeight)) {
                mapRejectedBlocks.insert(make_pair(block.GetHash(), GetTime()));
                return state.DoS(0, error("CheckBlock() : Couldn't find masternode/budget payment"),
                                 REJECT_INVALID, "bad-cb-payee");
            }
        } else {
            if (fDebug)
                LogPrintf("CheckBlock(): Masternode payment check skipped on sync - skipping IsBlockPayeeValid()\n");
        }
    }

    
    bool fZerocoinActive = (block.GetBlockTime() > Params().Zerocoin_StartTime()) && (chainActive.Height() + 1 >= Params().Zerocoin_StartHeight());
    vector<CBigNum> vBlockSerials;
    for (const CTransaction& tx : block.vtx) {
        if (!CheckTransaction(tx, fZerocoinActive, false, state))
            return error("CheckBlock() : CheckTransaction failed");

        
        if (tx.IsZerocoinSpend()) {
            for (const CTxIn txIn : tx.vin) {
                if (txIn.scriptSig.IsZerocoinSpend()) {
                    libzerocoin::CoinSpend spend = TxInToZerocoinSpend(txIn);
                    if (count(vBlockSerials.begin(), vBlockSerials.end(), spend.getCoinSerialNumber()))
                        return state.DoS(100, error("%s : Double spending of zUlo serial %s in block\n Block: %s",
                                                    __func__, spend.getCoinSerialNumber().GetHex(), block.ToString()));
                    vBlockSerials.emplace_back(spend.getCoinSerialNumber());
                }
            }
        }

        
        
        
        if (tx.HasOpSpend())
        {
            if (!lastWasContract)
            {
                return state.DoS(100, error("OP_SPEND transaction without corresponding contract transaction"), REJECT_INVALID, "bad-opspend-tx", false);
            }
        }
        lastWasContract = tx.HasCreateOrCall() || tx.HasOpSpend();


    }


    unsigned int nSigOps = 0;
    BOOST_FOREACH (const CTransaction& tx, block.vtx) {
        nSigOps += GetLegacySigOpCount(tx);
    }
    unsigned int nMaxBlockSigOps = fZerocoinActive ? MAX_BLOCK_SIGOPS_CURRENT : MAX_BLOCK_SIGOPS_LEGACY;
    if (nSigOps > nMaxBlockSigOps)
        return state.DoS(100, error("CheckBlock() : out-of-bounds SigOpCount"),
                         REJECT_INVALID, "bad-blk-sigops", true);

    return true;
}

bool CheckWork(const CBlock block, CBlockIndex* const pindexPrev)
{
    if (pindexPrev == NULL)
        return error("%s : null pindexPrev for block %s", __func__, block.GetHash().ToString().c_str());

    unsigned int nBitsRequired = GetNextWorkRequired(pindexPrev, &block);

    if (block.IsProofOfWork() && (pindexPrev->nHeight + 1 <= Params().LAST_POW_BLOCK())) {
        double n1 = ConvertBitsToDouble(block.nBits);
        double n2 = ConvertBitsToDouble(nBitsRequired);

        if (abs(n1 - n2) > n1 * 0.5)
            return error("%s : incorrect proof of work (DGW pre-fork) - %f %f %f at %d", __func__, abs(n1 - n2), n1, n2, pindexPrev->nHeight + 1);

        return true;
    }

    if (block.nBits != nBitsRequired)
        return error("%s : incorrect proof of work at %d", __func__, pindexPrev->nHeight + 1);

    if (block.IsProofOfStake()) {
        uint256 hashProofOfStake;
        uint256 hash = block.GetHash();

        if(!CheckProofOfStake(block, hashProofOfStake)) {
            LogPrintf("WARNING: ProcessBlock(): check proof-of-stake failed for block %s\n", hash.ToString().c_str());
            return false;
        }
        if(!mapProofOfStake.count(hash)) 
            mapProofOfStake.insert(make_pair(hash, hashProofOfStake));
    }

    return true;
}

bool ContextualCheckBlockHeader(const CBlockHeader& block, CValidationState& state, CBlockIndex* const pindexPrev)
{
    uint256 hash = block.GetHash();

    if (hash == Params().HashGenesisBlock())
        return true;

    assert(pindexPrev);

    int nHeight = pindexPrev->nHeight + 1;

    
    int nMaxReorgDepth = GetArg("-maxreorg", Params().MaxReorganizationDepth());
    if (chainActive.Height() - nHeight >= nMaxReorgDepth)
        return state.DoS(1, error("%s: forked chain older than max reorganization depth (height %d)", __func__, nHeight));

    
    if (block.GetBlockTime() <= pindexPrev->GetMedianTimePast()) {
        LogPrintf("Block time = %d , GetMedianTimePast = %d \n", block.GetBlockTime(), pindexPrev->GetMedianTimePast());
        return state.Invalid(error("%s : block's timestamp is too early", __func__),
                             REJECT_INVALID, "time-too-old");
    }

    
    if (!Checkpoints::CheckBlock(nHeight, hash))
        return state.DoS(100, error("%s : rejected by checkpoint lock-in at %d", __func__, nHeight),
                         REJECT_CHECKPOINT, "checkpoint mismatch");

    
    CBlockIndex* pcheckpoint = Checkpoints::GetLastCheckpoint();
    if (pcheckpoint && nHeight < pcheckpoint->nHeight)
        return state.DoS(0, error("%s : forked chain older than last checkpoint (height %d)", __func__, nHeight));

    
    if (block.nVersion < 2 &&
            CBlockIndex::IsSuperMajority(2, pindexPrev, Params().RejectBlockOutdatedMajority())) {
        return state.Invalid(error("%s : rejected nVersion=1 block", __func__),
                             REJECT_OBSOLETE, "bad-version");
    }

    
    if (block.nVersion < 3 && CBlockIndex::IsSuperMajority(3, pindexPrev, Params().RejectBlockOutdatedMajority())) {
        return state.Invalid(error("%s : rejected nVersion=2 block", __func__),
                             REJECT_OBSOLETE, "bad-version");
    }

    return true;
}

bool IsBlockHashInChain(const uint256& hashBlock)
{
    if (hashBlock == 0 || !mapBlockIndex.count(hashBlock))
        return false;

    return chainActive.Contains(mapBlockIndex[hashBlock]);
}

bool IsTransactionInChain(uint256 txId, int& nHeightTx)
{
    uint256 hashBlock;
    CTransaction tx;
    GetTransaction(txId, tx, hashBlock, true);
    if (!IsBlockHashInChain(hashBlock))
        return false;

    nHeightTx = mapBlockIndex.at(hashBlock)->nHeight;
    return true;
}

bool ContextualCheckBlock(const CBlock& block, CValidationState& state, CBlockIndex* const pindexPrev)
{
    const int nHeight = pindexPrev == NULL ? 0 : pindexPrev->nHeight + 1;

    
    BOOST_FOREACH (const CTransaction& tx, block.vtx)
            if (!IsFinalTx(tx, nHeight, block.GetBlockTime())) {
        return state.DoS(10, error("%s : contains a non-final transaction", __func__), REJECT_INVALID, "bad-txns-nonfinal");
    }

    
    
    if (block.nVersion >= 2 &&
            CBlockIndex::IsSuperMajority(2, pindexPrev, Params().EnforceBlockUpgradeMajority())) {
        CScript expect = CScript() << nHeight;
        if (block.vtx[0].vin[0].scriptSig.size() < expect.size() ||
                !std::equal(expect.begin(), expect.end(), block.vtx[0].vin[0].scriptSig.begin())) {
            return state.DoS(100, error("%s : block height mismatch in coinbase", __func__), REJECT_INVALID, "bad-cb-height");
        }
    }

    return true;
}

bool AcceptBlockHeader(const CBlock& block, CValidationState& state, CBlockIndex** ppindex)
{
    AssertLockHeld(cs_main);
    
    uint256 hash = block.GetHash();
    BlockMap::iterator miSelf = mapBlockIndex.find(hash);
    CBlockIndex* pindex = NULL;

    
    if (miSelf != mapBlockIndex.end()) {
        
        pindex = miSelf->second;
        if (ppindex)
            *ppindex = pindex;
        if (pindex->nStatus & BLOCK_FAILED_MASK)
            return state.Invalid(error("%s : block is marked invalid", __func__), 0, "duplicate");
        return true;
    }

    if (!CheckBlockHeader(block, state, false)) {
        LogPrintf("AcceptBlockHeader(): CheckBlockHeader failed \n");
        return false;
    }

    
    CBlockIndex* pindexPrev = NULL;
    if (hash != Params().HashGenesisBlock()) {
        BlockMap::iterator mi = mapBlockIndex.find(block.hashPrevBlock);
        if (mi == mapBlockIndex.end())
            return state.DoS(0, error("%s : prev block %s not found", __func__, block.hashPrevBlock.ToString().c_str()), 0, "bad-prevblk");
        pindexPrev = (*mi).second;
        if (pindexPrev->nStatus & BLOCK_FAILED_MASK) {
            
            if (pindex && Checkpoints::CheckBlock(pindex->nHeight - 1, block.hashPrevBlock, true)) {
                LogPrintf("%s : Reconsidering block %s height %d\n", __func__, pindexPrev->GetBlockHash().GetHex(), pindexPrev->nHeight);
                CValidationState statePrev;
                ReconsiderBlock(statePrev, pindexPrev);
                if (statePrev.IsValid()) {
                    ActivateBestChain(statePrev);
                    return true;
                }
            }

            return state.DoS(100, error("%s : prev block height=%d hash=%s is invalid, unable to add block %s", __func__, pindexPrev->nHeight, block.hashPrevBlock.GetHex(), block.GetHash().GetHex()),
                             REJECT_INVALID, "bad-prevblk");
        }

    }

    if (!ContextualCheckBlockHeader(block, state, pindexPrev))
        return false;

    if (pindex == NULL)
        pindex = AddToBlockIndex(block);

    if (ppindex)
        *ppindex = pindex;

    return true;
}

bool AcceptBlock(CBlock& block, CValidationState& state, CBlockIndex** ppindex, CDiskBlockPos* dbp, bool fAlreadyCheckedBlock)
{
    AssertLockHeld(cs_main);

    CBlockIndex*& pindex = *ppindex;

    
    CBlockIndex* pindexPrev = NULL;
    if (block.GetHash() != Params().HashGenesisBlock()) {
        BlockMap::iterator mi = mapBlockIndex.find(block.hashPrevBlock);
        if (mi == mapBlockIndex.end())
            return state.DoS(0, error("%s : prev block %s not found", __func__, block.hashPrevBlock.ToString().c_str()), 0, "bad-prevblk");
        pindexPrev = (*mi).second;
        if (pindexPrev->nStatus & BLOCK_FAILED_MASK) {
            
            if (Checkpoints::CheckBlock(pindexPrev->nHeight, block.hashPrevBlock, true)) {
                LogPrintf("%s : Reconsidering block %s height %d\n", __func__, pindexPrev->GetBlockHash().GetHex(), pindexPrev->nHeight);
                CValidationState statePrev;
                ReconsiderBlock(statePrev, pindexPrev);
                if (statePrev.IsValid()) {
                    ActivateBestChain(statePrev);
                    return true;
                }
            }
            return state.DoS(100, error("%s : prev block %s is invalid, unable to add block %s", __func__, block.hashPrevBlock.GetHex(), block.GetHash().GetHex()),
                             REJECT_INVALID, "bad-prevblk");
        }
    }

    if (block.GetHash() != Params().HashGenesisBlock() && !CheckWork(block, pindexPrev))
        return false;

    if (!AcceptBlockHeader(block, state, &pindex))
        return false;

    if (pindex->nStatus & BLOCK_HAVE_DATA) {
        
        
        return true;
    }

    if ((!fAlreadyCheckedBlock && !CheckBlock(block, state)) || !ContextualCheckBlock(block, state, pindex->pprev)) {
        if (state.IsInvalid() && !state.CorruptionPossible()) {
            pindex->nStatus |= BLOCK_FAILED_VALID;
            setDirtyBlockIndex.insert(pindex);
        }
        return false;
    }

    int nHeight = pindex->nHeight;

    
    try {
        unsigned int nBlockSize = ::GetSerializeSize(block, SER_DISK, CLIENT_VERSION);
        CDiskBlockPos blockPos;
        if (dbp != NULL)
            blockPos = *dbp;
        if (!FindBlockPos(state, blockPos, nBlockSize + 8, nHeight, block.GetBlockTime(), dbp != NULL))
            return error("AcceptBlock() : FindBlockPos failed");
        if (dbp == NULL)
            if (!WriteBlockToDisk(block, blockPos))
                return state.Abort("Failed to write block");
        if (!ReceivedBlockTransactions(block, state, pindex, blockPos))
            return error("AcceptBlock() : ReceivedBlockTransactions failed");
    } catch (std::runtime_error& e) {
        return state.Abort(std::string("System error: ") + e.what());
    }

    return true;
}

bool CBlockIndex::IsSuperMajority(int minVersion, const CBlockIndex* pstart, unsigned int nRequired)
{
    unsigned int nToCheck = Params().ToCheckBlockUpgradeMajority();
    unsigned int nFound = 0;
    for (unsigned int i = 0; i < nToCheck && nFound < nRequired && pstart != NULL; i++) {
        if (pstart->nVersion >= minVersion)
            ++nFound;
        pstart = pstart->pprev;
    }
    return (nFound >= nRequired);
}


int static inline InvertLowestOne(int n) { return n & (n - 1); }


int static inline GetSkipHeight(int height)
{
    if (height < 2)
        return 0;

    
    
    
    return (height & 1) ? InvertLowestOne(InvertLowestOne(height - 1)) + 1 : InvertLowestOne(height);
}

CBlockIndex* CBlockIndex::GetAncestor(int height)
{
    if (height > nHeight || height < 0)
        return NULL;

    CBlockIndex* pindexWalk = this;
    int heightWalk = nHeight;
    while (heightWalk > height) {
        int heightSkip = GetSkipHeight(heightWalk);
        int heightSkipPrev = GetSkipHeight(heightWalk - 1);
        if (heightSkip == height ||
                (heightSkip > height && !(heightSkipPrev < heightSkip - 2 && heightSkipPrev >= height))) {
            
            pindexWalk = pindexWalk->pskip;
            heightWalk = heightSkip;
        } else {
            pindexWalk = pindexWalk->pprev;
            heightWalk--;
        }
    }
    return pindexWalk;
}

const CBlockIndex* CBlockIndex::GetAncestor(int height) const
{
    return const_cast<CBlockIndex*>(this)->GetAncestor(height);
}

void CBlockIndex::BuildSkip()
{
    if (pprev)
        pskip = pprev->GetAncestor(GetSkipHeight(nHeight));
}

#ifdef  POW_IN_POS_PHASE

bool GetBestTmpBlockParams(CTransaction& coinBaseTx, unsigned int& nNonce, unsigned int& nCount)
{
    uint256 blockParamHash;
    uint256 blockHeaderHash;

    LOCK(cs_main);
    
    nCount = tmpblockmempool.mapTmpBlock.size();
    
    if (nCount > 0) {
        std::map<uint256, std::pair<CTmpBlockParams,int64_t>>::const_iterator it = tmpblockmempool.mapTmpBlock.begin();
        blockParamHash = it->first;
        blockHeaderHash = it->second.first.blockheader_hash;
        for (it++; it != tmpblockmempool.mapTmpBlock.end(); it++)
        {
            if (it->second.first.blockheader_hash < blockHeaderHash)
            {
                blockParamHash = it->first;
                blockHeaderHash = it->second.first.blockheader_hash;
            }
        }

        nNonce = tmpblockmempool.mapTmpBlock.find(blockParamHash)->second.first.nNonce;
        coinBaseTx = tmpblockmempool.mapTmpBlock.find(blockParamHash)->second.first.coinBaseTx;
    }

    return true;
}

bool ProcessNewTmpBlockParam(CTmpBlockParams &tmpBlockParams, const CBlockHeader &blockHeader)
{
    LOCK(cs_main);

    uint256 blockcnhash = HashCryptoNight(BEGIN(blockHeader.nVersion), END(blockHeader.nAccumulatorCheckpoint));

    if(CheckProofOfWork(blockcnhash, blockHeader.nBits) && !tmpblockmempool.HaveTmpBlock(tmpBlockParams.GetHash())) {
        tmpBlockParams.blockheader_hash =  blockcnhash;
        tmpblockmempool.mapTmpBlock.insert(make_pair(tmpBlockParams.GetHash(),std::pair<CTmpBlockParams,int64_t>(tmpBlockParams,GetTime())));
        LogPrintf("New tmp block(%s): orig %s, nonce %lu, coinBase %s\n",
                  tmpBlockParams.GetHash().GetHex(), tmpBlockParams.ori_hash.GetHex(), tmpBlockParams.nNonce,
                  tmpBlockParams.coinBaseTx.ToString());
        BOOST_FOREACH (CNode* pnode, vNodes)
                pnode->PushMessage("tmpblock", tmpBlockParams);
    }

    return true;
}

#endif

bool ProcessNewBlock(CValidationState& state, CNode* pfrom, CBlock* pblock, CDiskBlockPos* dbp)
{
    
    int64_t nStartTime = GetTimeMillis();
    bool checked = CheckBlock(*pblock, state);

    int nMints = 0;
    int nSpends = 0;
    for (const CTransaction tx : pblock->vtx) {
        if (tx.ContainsZerocoins()) {
            for (const CTxIn in : tx.vin) {
                if (in.scriptSig.IsZerocoinSpend())
                    nSpends++;
            }
            for (const CTxOut out : tx.vout) {
                if (out.IsZerocoinMint())
                    nMints++;
            }
        }
    }
    if (nMints || nSpends)
        LogPrintf("%s : block contains %d zUlo mints and %d zUlo spends\n", __func__, nMints, nSpends);

    
    
    
    
    

    
    if (!pblock->CheckBlockSignature())
        return error("ProcessNewBlock() : bad proof-of-stake block signature");

    if (pblock->GetHash() != Params().HashGenesisBlock() && pfrom != NULL) {
        
        BlockMap::iterator mi = mapBlockIndex.find(pblock->hashPrevBlock);
        if (mi == mapBlockIndex.end()) {
            pfrom->PushMessage("getblocks", chainActive.GetLocator(), uint256(0));
            return false;
        }
    }

    {
        LOCK(cs_main);   

        MarkBlockAsReceived (pblock->GetHash ());
        if (!checked) {
            return error ("%s : CheckBlock FAILED for block %s", __func__, pblock->GetHash().GetHex());
        }

        
        CBlockIndex* pindex = NULL;
        bool ret = AcceptBlock (*pblock, state, &pindex, dbp, checked);
        if (pindex && pfrom) {
            mapBlockSource[pindex->GetBlockHash ()] = pfrom->GetId ();
        }
        CheckBlockIndex ();
        if (!ret)
            return error ("%s : AcceptBlock FAILED", __func__);
    }

    if (!ActivateBestChain(state, pblock, checked))
        return error("%s : ActivateBestChain failed", __func__);

    if (!fLiteMode) {
        if (masternodeSync.RequestedMasternodeAssets > MASTERNODE_SYNC_LIST) {
            obfuScationPool.NewBlock();
            masternodePayments.ProcessBlock(GetHeight() + 10);
            budget.NewBlock();
        }
    }

    if (pwalletMain) {
        
        if (pwalletMain->isMultiSendEnabled())
            pwalletMain->MultiSend();

        
        if (pwalletMain->fCombineDust)
            pwalletMain->AutoCombineDust();
    }

#ifdef  POW_IN_POS_PHASE
    {
        LOCK(cs_main);
        tmpblockmempool.mapTmpBlock.clear();
    }
#endif

    LogPrintf("%s : ACCEPTED in %ld milliseconds with size=%d\n", __func__, GetTimeMillis() - nStartTime,
              pblock->GetSerializeSize(SER_DISK, CLIENT_VERSION));

    return true;
}

bool TestBlockValidity(CValidationState& state, const CBlock& block, CBlockIndex* pindexPrev, bool fCheckPOW, bool fCheckMerkleRoot)
{
    AssertLockHeld(cs_main);
    assert(pindexPrev == chainActive.Tip());

    uint256 hashUTXORoot;
    uint256 hashStateRoot;

    CCoinsViewCache viewNew(pcoinsTip);
    CBlockIndex indexDummy(block);
    indexDummy.pprev = pindexPrev;
    indexDummy.nHeight = pindexPrev->nHeight + 1;

    
    if (!ContextualCheckBlockHeader(block, state, pindexPrev))
        return false;
    if (!CheckBlock(block, state, fCheckPOW, fCheckMerkleRoot))
        return false;
    if (!ContextualCheckBlock(block, state, pindexPrev))
        return false;

    GetState(hashStateRoot, hashUTXORoot);
    
    if (!ConnectBlock(block, state, &indexDummy, viewNew, true))
    {
        UpdateState(hashStateRoot, hashUTXORoot);
        ClearCacheResult();
        return false;
    }
    assert(state.IsValid());

    return true;
}


bool AbortNode(const std::string& strMessage, const std::string& userMessage)
{
    strMiscWarning = strMessage;
    
    uiInterface.ThreadSafeMessageBox(
                userMessage.empty() ? _("Error: A fatal internal error occured, see debug.log for details") : userMessage,
                "", CClientUIInterface::MSG_ERROR);
    StartShutdown();
    return false;
}

bool CheckDiskSpace(uint64_t nAdditionalBytes)
{
    uint64_t nFreeBytesAvailable = filesystem::space(GetDataDir()).available;

    
    if (nFreeBytesAvailable < nMinDiskSpace + nAdditionalBytes)
        return AbortNode("Disk space is low!", _("Error: Disk space is low!"));

    return true;
}

FILE* OpenDiskFile(const CDiskBlockPos& pos, const char* prefix, bool fReadOnly)
{
    if (pos.IsNull())
        return NULL;
    boost::filesystem::path path = GetBlockPosFilename(pos, prefix);
    boost::filesystem::create_directories(path.parent_path());
    FILE* file = fopen(path.string().c_str(), "rb+");
    if (!file && !fReadOnly)
        file = fopen(path.string().c_str(), "wb+");
    if (!file) {
        LogPrintf("Unable to open file %s\n", path.string());
        return NULL;
    }
    if (pos.nPos) {
        if (fseek(file, pos.nPos, SEEK_SET)) {
            LogPrintf("Unable to seek to position %u of %s\n", pos.nPos, path.string());
            fclose(file);
            return NULL;
        }
    }
    return file;
}

FILE* OpenBlockFile(const CDiskBlockPos& pos, bool fReadOnly)
{
    return OpenDiskFile(pos, "blk", fReadOnly);
}

FILE* OpenUndoFile(const CDiskBlockPos& pos, bool fReadOnly)
{
    return OpenDiskFile(pos, "rev", fReadOnly);
}

boost::filesystem::path GetBlockPosFilename(const CDiskBlockPos& pos, const char* prefix)
{
    return GetDataDir() / "blocks" / strprintf("%s%05u.dat", prefix, pos.nFile);
}

CBlockIndex* InsertBlockIndex(uint256 hash)
{
    if (hash == 0)
        return NULL;

    
    BlockMap::iterator mi = mapBlockIndex.find(hash);
    if (mi != mapBlockIndex.end())
        return (*mi).second;

    
    CBlockIndex* pindexNew = new CBlockIndex();
    if (!pindexNew)
        throw runtime_error("LoadBlockIndex() : new CBlockIndex failed");
    mi = mapBlockIndex.insert(make_pair(hash, pindexNew)).first;

    
    if (pindexNew->IsProofOfStake())
        setStakeSeen.insert(make_pair(pindexNew->prevoutStake, pindexNew->nStakeTime));

    pindexNew->phashBlock = &((*mi).first);

    return pindexNew;
}

bool static LoadBlockIndexDB(string& strError)
{
    if (!pblocktree->LoadBlockIndexGuts())
        return false;

    boost::this_thread::interruption_point();

    
    vector<pair<int, CBlockIndex*> > vSortedByHeight;
    vSortedByHeight.reserve(mapBlockIndex.size());
    for (const PAIRTYPE(uint256, CBlockIndex*) & item : mapBlockIndex) {
        CBlockIndex* pindex = item.second;
        vSortedByHeight.push_back(make_pair(pindex->nHeight, pindex));
    }
    sort(vSortedByHeight.begin(), vSortedByHeight.end());
    BOOST_FOREACH (const PAIRTYPE(int, CBlockIndex*) & item, vSortedByHeight) {
        CBlockIndex* pindex = item.second;
        pindex->nChainWork = (pindex->pprev ? pindex->pprev->nChainWork : 0) + GetBlockProof(*pindex);
        if (pindex->nStatus & BLOCK_HAVE_DATA) {
            if (pindex->pprev) {
                if (pindex->pprev->nChainTx) {
                    pindex->nChainTx = pindex->pprev->nChainTx + pindex->nTx;
                } else {
                    pindex->nChainTx = 0;
                    mapBlocksUnlinked.insert(std::make_pair(pindex->pprev, pindex));
                }
            } else {
                pindex->nChainTx = pindex->nTx;
            }
        }
        if (pindex->IsValid(BLOCK_VALID_TRANSACTIONS) && (pindex->nChainTx || pindex->pprev == NULL))
            setBlockIndexCandidates.insert(pindex);
        if (pindex->nStatus & BLOCK_FAILED_MASK && (!pindexBestInvalid || pindex->nChainWork > pindexBestInvalid->nChainWork))
            pindexBestInvalid = pindex;
        if (pindex->pprev)
            pindex->BuildSkip();
        if (pindex->IsValid(BLOCK_VALID_TREE) && (pindexBestHeader == NULL || CBlockIndexWorkComparator()(pindexBestHeader, pindex)))
            pindexBestHeader = pindex;
    }

    
    pblocktree->ReadLastBlockFile(nLastBlockFile);
    vinfoBlockFile.resize(nLastBlockFile + 1);
    LogPrintf("%s: last block file = %i\n", __func__, nLastBlockFile);
    for (int nFile = 0; nFile <= nLastBlockFile; nFile++) {
        pblocktree->ReadBlockFileInfo(nFile, vinfoBlockFile[nFile]);
    }
    LogPrintf("%s: last block file info: %s\n", __func__, vinfoBlockFile[nLastBlockFile].ToString());
    for (int nFile = nLastBlockFile + 1; true; nFile++) {
        CBlockFileInfo info;
        if (pblocktree->ReadBlockFileInfo(nFile, info)) {
            vinfoBlockFile.push_back(info);
        } else {
            break;
        }
    }

    
    LogPrintf("Checking all blk files are present...\n");
    set<int> setBlkDataFiles;
    for (const PAIRTYPE(uint256, CBlockIndex*) & item : mapBlockIndex) {
        CBlockIndex* pindex = item.second;
        if (pindex->nStatus & BLOCK_HAVE_DATA) {
            setBlkDataFiles.insert(pindex->nFile);
        }
    }
    for (std::set<int>::iterator it = setBlkDataFiles.begin(); it != setBlkDataFiles.end(); it++) {
        CDiskBlockPos pos(*it, 0);
        if (CAutoFile(OpenBlockFile(pos, true), SER_DISK, CLIENT_VERSION).IsNull()) {
            return false;
        }
    }

    
    bool fLastShutdownWasPrepared = true;
    pblocktree->ReadFlag("shutdown", fLastShutdownWasPrepared);
    LogPrintf("%s: Last shutdown was prepared: %s\n", __func__, fLastShutdownWasPrepared);

    
    if (!fLastShutdownWasPrepared && !GetBoolArg("-forcestart", false) && !GetBoolArg("-reindex", false)) {
        unsigned int nHeightLastBlockFile = vinfoBlockFile[nLastBlockFile].nHeightLast + 1;
        if (vSortedByHeight.size() > nHeightLastBlockFile && pcoinsTip->GetBestBlock() != vSortedByHeight[nHeightLastBlockFile].second->GetBlockHash()) {
            
            
            

            if (!mapBlockIndex.count(pcoinsTip->GetBestBlock())) {
                strError = "The wallet has been not been closed gracefully, causing the transaction database to be out of sync with the block database";
                return false;
            }
            LogPrintf("%s : pcoinstip synced to block height %d, block index height %d\n", __func__,
                      mapBlockIndex[pcoinsTip->GetBestBlock()]->nHeight, vSortedByHeight.size());

            
            CBlockIndex *pindexLastMeta = vSortedByHeight[vinfoBlockFile[nLastBlockFile].nHeightLast + 1].second;
            CBlockIndex *pindex = vSortedByHeight[0].second;
            unsigned int nSortedPos = 0;
            for (unsigned int i = 0; i < vSortedByHeight.size(); i++) {
                nSortedPos = i;
                if (vSortedByHeight[i].first == mapBlockIndex[pcoinsTip->GetBestBlock()]->nHeight + 1) {
                    pindex = vSortedByHeight[i].second;
                    break;
                }
            }

            
            
            CCoinsViewCache view(pcoinsTip);
            while (nSortedPos < vSortedByHeight.size()) {
                CBlock block;
                if (!ReadBlockFromDisk(block, pindex)) {
                    strError = "The wallet has been not been closed gracefully and has caused corruption of blocks stored to disk. Data directory is in an unusable state";
                    return false;
                }

                vector<CTxUndo> vtxundo;
                vtxundo.reserve(block.vtx.size() - 1);
                uint256 hashBlock = block.GetHash();
                for (unsigned int i = 0; i < block.vtx.size(); i++) {
                    CValidationState state;
                    CTxUndo undoDummy;
                    if (i > 0)
                        vtxundo.push_back(CTxUndo());
                    UpdateCoins(block.vtx[i], state, view, i == 0 ? undoDummy : vtxundo.back(), pindex->nHeight);
                    view.SetBestBlock(hashBlock);
                }

                if(pindex->nHeight >= pindexLastMeta->nHeight)
                    break;

                pindex = vSortedByHeight[++nSortedPos].second;
            }

            
            if (!view.Flush() || !pcoinsTip->Flush())
                LogPrintf("%s : failed to flush view\n", __func__);

            LogPrintf("%s: Last block properly recorded: #%d %s\n", __func__, pindexLastMeta->nHeight,
                      pindexLastMeta->GetBlockHash().ToString().c_str());
            LogPrintf("%s : pcoinstip=%d %s\n", __func__, mapBlockIndex[pcoinsTip->GetBestBlock()]->nHeight,
                    pcoinsTip->GetBestBlock().GetHex());
        }
    }

    
    bool fReindexing = false;
    pblocktree->ReadReindexing(fReindexing);
    fReindex |= fReindexing;

    
    pblocktree->ReadFlag("txindex", fTxIndex);
    LogPrintf("LoadBlockIndexDB(): transaction index %s\n", fTxIndex ? "enabled" : "disabled");

    pblocktree->ReadFlag("logevents", fLogEvents);
    LogPrintf("logevents %s", fLogEvents ? "enabled" : "disabled");

    
    pblocktree->WriteFlag("shutdown", false);

    
    paddressmap->ReadEnable(fAddrIndex);
    LogPrintf("LoadBlockIndexDB(): address index %s\n", fAddrIndex ? "enabled" : "disabled");

    
    BlockMap::iterator it = mapBlockIndex.find(pcoinsTip->GetBestBlock());
    if (it == mapBlockIndex.end())
        return true;
    chainActive.SetTip(it->second);

    PruneBlockIndexCandidates();

    LogPrintf("LoadBlockIndexDB(): hashBestChain=%s height=%d date=%s progress=%f\n",
              chainActive.Tip()->GetBlockHash().ToString(), chainActive.Height(),
              DateTimeStrFormat("%Y-%m-%d %H:%M:%S", chainActive.Tip()->GetBlockTime()),
              Checkpoints::GuessVerificationProgress(chainActive.Tip()));

    return true;
}

CVerifyDB::CVerifyDB()
{
    uiInterface.ShowProgress(_("Verifying blocks..."), 0);
}

CVerifyDB::~CVerifyDB()
{
    uiInterface.ShowProgress("", 100);
}

bool CVerifyDB::VerifyDB(CCoinsView* coinsview, int nCheckLevel, int nCheckDepth)
{
    LOCK(cs_main);
    if (chainActive.Tip() == NULL || chainActive.Tip()->pprev == NULL)
        return true;

    
    if (nCheckDepth <= 0)
        nCheckDepth = 1000000000; 
    if (nCheckDepth > chainActive.Height())
        nCheckDepth = chainActive.Height();
    nCheckLevel = std::max(0, std::min(4, nCheckLevel));
    LogPrintf("Verifying last %i blocks at level %i\n", nCheckDepth, nCheckLevel);
    CCoinsViewCache coins(coinsview);
    CBlockIndex* pindexState = chainActive.Tip();
    CBlockIndex* pindexFailure = NULL;
    int nGoodTransactions = 0;
    CValidationState state;

    uint256 oldHashStateRoot, oldHashUTXORoot;
    GetState(oldHashStateRoot, oldHashUTXORoot);


    for (CBlockIndex* pindex = chainActive.Tip(); pindex && pindex->pprev; pindex = pindex->pprev) {
        boost::this_thread::interruption_point();
        uiInterface.ShowProgress(_("Verifying blocks..."), std::max(1, std::min(99, (int)(((double)(chainActive.Height() - pindex->nHeight)) / (double)nCheckDepth * (nCheckLevel >= 4 ? 50 : 100)))));
        if (pindex->nHeight < chainActive.Height() - nCheckDepth)
            break;
        CBlock block;
        
        if (!ReadBlockFromDisk(block, pindex))
            return error("VerifyDB() : *** ReadBlockFromDisk failed at %d, hash=%s", pindex->nHeight, pindex->GetBlockHash().ToString());
        
        if (nCheckLevel >= 1 && !CheckBlock(block, state))
            return error("VerifyDB() : *** found bad block at %d, hash=%s\n", pindex->nHeight, pindex->GetBlockHash().ToString());
        
        if (nCheckLevel >= 2 && pindex) {
            CBlockUndo undo;
            CDiskBlockPos pos = pindex->GetUndoPos();
            if (!pos.IsNull()) {
                if (!undo.ReadFromDisk(pos, pindex->pprev->GetBlockHash()))
                    return error("VerifyDB() : *** found bad undo data at %d, hash=%s\n", pindex->nHeight, pindex->GetBlockHash().ToString());
            }
        }
        
        if (nCheckLevel >= 3 && pindex == pindexState && (coins.GetCacheSize() + pcoinsTip->GetCacheSize()) <= nCoinCacheSize) {
            bool fClean = true;
            if (!DisconnectBlock(block, state, pindex, coins, &fClean))
                return error("VerifyDB() : *** irrecoverable inconsistency in block data at %d, hash=%s", pindex->nHeight, pindex->GetBlockHash().ToString());
            pindexState = pindex->pprev;
            if (!fClean) {
                nGoodTransactions = 0;
                pindexFailure = pindex;
            } else
                nGoodTransactions += block.vtx.size();
        }
        if (ShutdownRequested())
            return true;
    }
    if (pindexFailure)
        return error("VerifyDB() : *** coin database inconsistencies found (last %i blocks, %i good transactions before that)\n", chainActive.Height() - pindexFailure->nHeight + 1, nGoodTransactions);

    
    if (nCheckLevel >= 4) {
        CBlockIndex* pindex = pindexState;
        while (pindex != chainActive.Tip()) {
            boost::this_thread::interruption_point();
            uiInterface.ShowProgress(_("Verifying blocks..."), std::max(1, std::min(99, 100 - (int)(((double)(chainActive.Height() - pindex->nHeight)) / (double)nCheckDepth * 50))));
            pindex = chainActive.Next(pindex);
            CBlock block;
            if (!ReadBlockFromDisk(block, pindex))
                return error("VerifyDB() : *** ReadBlockFromDisk failed at %d, hash=%s", pindex->nHeight, pindex->GetBlockHash().ToString());
            uint256 oldHashStateRoot, oldHashUTXORoot;
            GetState(oldHashStateRoot, oldHashUTXORoot); 
            
            if (!ConnectBlock(block, state, pindex, coins, false))
            {
                UpdateState(oldHashStateRoot, oldHashUTXORoot); 
                ClearCacheResult(); 
                return error("VerifyDB() : *** found unconnectable block at %d, hash=%s", pindex->nHeight, pindex->GetBlockHash().ToString());
            }
        }
    }
    else
    {
        UpdateState(oldHashStateRoot, oldHashUTXORoot); 
    }

    LogPrintf("No coin database inconsistencies in last %i blocks (%i transactions)\n", chainActive.Height() - pindexState->nHeight, nGoodTransactions);

    return true;
}

void UnloadBlockIndex()
{
    mapBlockIndex.clear();
    setBlockIndexCandidates.clear();
    chainActive.SetTip(NULL);
    pindexBestInvalid = NULL;
}

bool LoadBlockIndex(string& strError)
{
    
    if (!fReindex && !LoadBlockIndexDB(strError))
        return false;
    return true;
}


bool InitBlockIndex()
{
    LOCK(cs_main);
    
    if (chainActive.Genesis() != NULL)
        return true;

    
    fTxIndex = GetBoolArg("-txindex", true);
    pblocktree->WriteFlag("txindex", fTxIndex);

    
    fAddrIndex = GetBoolArg("-addrindex", false);
    paddressmap->WriteEnable(fAddrIndex);
    LogPrintf("Initializing databases...\n");

    
    if (!fReindex) {
        try {
            CBlock& block = const_cast<CBlock&>(Params().GenesisBlock());
            
            unsigned int nBlockSize = ::GetSerializeSize(block, SER_DISK, CLIENT_VERSION);
            CDiskBlockPos blockPos;
            CValidationState state;
            if (!FindBlockPos(state, blockPos, nBlockSize + 8, 0, block.GetBlockTime()))
                return error("LoadBlockIndex() : FindBlockPos failed");
            if (!WriteBlockToDisk(block, blockPos))
                return error("LoadBlockIndex() : writing genesis block to disk failed");
            CBlockIndex* pindex = AddToBlockIndex(block);
            if (!ReceivedBlockTransactions(block, state, pindex, blockPos))
                return error("LoadBlockIndex() : genesis block not accepted");
            if (!ActivateBestChain(state, &block))
                return error("LoadBlockIndex() : genesis block cannot be activated");
            
            return FlushStateToDisk(state, FLUSH_STATE_ALWAYS);
        } catch (std::runtime_error& e) {
            return error("LoadBlockIndex() : failed to initialize block database: %s", e.what());
        }
    }

    return true;
}


bool LoadExternalBlockFile(FILE* fileIn, CDiskBlockPos* dbp)
{
    
    static std::multimap<uint256, CDiskBlockPos> mapBlocksUnknownParent;
    int64_t nStart = GetTimeMillis();

    int nLoaded = 0;
    try {
        
        CBufferedFile blkdat(fileIn, 2 * MAX_BLOCK_SIZE_CURRENT, MAX_BLOCK_SIZE_CURRENT + 8, SER_DISK, CLIENT_VERSION);
        uint64_t nRewind = blkdat.GetPos();
        while (!blkdat.eof()) {
            boost::this_thread::interruption_point();

            blkdat.SetPos(nRewind);
            nRewind++;         
            blkdat.SetLimit(); 
            unsigned int nSize = 0;
            try {
                
                unsigned char buf[MESSAGE_START_SIZE];
                blkdat.FindByte(Params().MessageStart()[0]);
                nRewind = blkdat.GetPos() + 1;
                blkdat >> FLATDATA(buf);
                if (memcmp(buf, Params().MessageStart(), MESSAGE_START_SIZE))
                    continue;
                
                blkdat >> nSize;
                if (nSize < 80 || nSize > MAX_BLOCK_SIZE_CURRENT)
                    continue;
            } catch (const std::exception&) {
                
                break;
            }
            try {
                
                uint64_t nBlockPos = blkdat.GetPos();
                if (dbp)
                    dbp->nPos = nBlockPos;
                blkdat.SetLimit(nBlockPos + nSize);
                blkdat.SetPos(nBlockPos);
                CBlock block;
                blkdat >> block;
                nRewind = blkdat.GetPos();

                
                uint256 hash = block.GetHash();
                if (hash != Params().HashGenesisBlock() && mapBlockIndex.find(block.hashPrevBlock) == mapBlockIndex.end()) {
                    LogPrint("reindex", "%s: Out of order block %s, parent %s not known\n", __func__, hash.ToString(),
                             block.hashPrevBlock.ToString());
                    if (dbp)
                        mapBlocksUnknownParent.insert(std::make_pair(block.hashPrevBlock, *dbp));
                    continue;
                }

                
                if (mapBlockIndex.count(hash) == 0 || (mapBlockIndex[hash]->nStatus & BLOCK_HAVE_DATA) == 0) {
                    CValidationState state;
                    if (ProcessNewBlock(state, NULL, &block, dbp))
                        nLoaded++;
                    if (state.IsError())
                        break;
                } else if (hash != Params().HashGenesisBlock() && mapBlockIndex[hash]->nHeight % 1000 == 0) {
                    LogPrintf("Block Import: already had block %s at height %d\n", hash.ToString(), mapBlockIndex[hash]->nHeight);
                }

                
                deque<uint256> queue;
                queue.push_back(hash);
                while (!queue.empty()) {
                    uint256 head = queue.front();
                    queue.pop_front();
                    std::pair<std::multimap<uint256, CDiskBlockPos>::iterator, std::multimap<uint256, CDiskBlockPos>::iterator> range = mapBlocksUnknownParent.equal_range(head);
                    while (range.first != range.second) {
                        std::multimap<uint256, CDiskBlockPos>::iterator it = range.first;
                        if (ReadBlockFromDisk(block, it->second)) {
                            LogPrintf("%s: Processing out of order child %s of %s\n", __func__, block.GetHash().ToString(),
                                      head.ToString());
                            CValidationState dummy;
                            if (ProcessNewBlock(dummy, NULL, &block, &it->second)) {
                                nLoaded++;
                                queue.push_back(block.GetHash());
                            }
                        }
                        range.first++;
                        mapBlocksUnknownParent.erase(it);
                    }
                }
            } catch (std::exception& e) {
                LogPrintf("%s : Deserialize or I/O error - %s", __func__, e.what());
            }
        }
    } catch (std::runtime_error& e) {
        AbortNode(std::string("System error: ") + e.what());
    }
    if (nLoaded > 0)
        LogPrintf("Loaded %i blocks from external file in %dms\n", nLoaded, GetTimeMillis() - nStart);
    return nLoaded > 0;
}

void static CheckBlockIndex()
{
    if (!fCheckBlockIndex) {
        return;
    }

    LOCK(cs_main);

    
    
    
    if (chainActive.Height() < 0) {
        assert(mapBlockIndex.size() <= 1);
        return;
    }

    
    std::multimap<CBlockIndex*, CBlockIndex*> forward;
    for (BlockMap::iterator it = mapBlockIndex.begin(); it != mapBlockIndex.end(); it++) {
        forward.insert(std::make_pair(it->second->pprev, it->second));
    }

    assert(forward.size() == mapBlockIndex.size());

    std::pair<std::multimap<CBlockIndex*, CBlockIndex*>::iterator, std::multimap<CBlockIndex*, CBlockIndex*>::iterator> rangeGenesis = forward.equal_range(NULL);
    CBlockIndex* pindex = rangeGenesis.first->second;
    rangeGenesis.first++;
    assert(rangeGenesis.first == rangeGenesis.second); 

    
    
    
    size_t nNodes = 0;
    int nHeight = 0;
    CBlockIndex* pindexFirstInvalid = NULL;         
    CBlockIndex* pindexFirstMissing = NULL;         
    CBlockIndex* pindexFirstNotTreeValid = NULL;    
    CBlockIndex* pindexFirstNotChainValid = NULL;   
    CBlockIndex* pindexFirstNotScriptsValid = NULL; 
    while (pindex != NULL) {
        nNodes++;
        if (pindexFirstInvalid == NULL && pindex->nStatus & BLOCK_FAILED_VALID) pindexFirstInvalid = pindex;
        if (pindexFirstMissing == NULL && !(pindex->nStatus & BLOCK_HAVE_DATA)) pindexFirstMissing = pindex;
        if (pindex->pprev != NULL && pindexFirstNotTreeValid == NULL && (pindex->nStatus & BLOCK_VALID_MASK) < BLOCK_VALID_TREE) pindexFirstNotTreeValid = pindex;
        if (pindex->pprev != NULL && pindexFirstNotChainValid == NULL && (pindex->nStatus & BLOCK_VALID_MASK) < BLOCK_VALID_CHAIN) pindexFirstNotChainValid = pindex;
        if (pindex->pprev != NULL && pindexFirstNotScriptsValid == NULL && (pindex->nStatus & BLOCK_VALID_MASK) < BLOCK_VALID_SCRIPTS) pindexFirstNotScriptsValid = pindex;

        
        if (pindex->pprev == NULL) {
            
            assert(pindex->GetBlockHash() == Params().HashGenesisBlock()); 
            assert(pindex == chainActive.Genesis());                       
        }
        
        assert(!(pindex->nStatus & BLOCK_HAVE_DATA) == (pindex->nTx == 0));
        assert(((pindex->nStatus & BLOCK_VALID_MASK) >= BLOCK_VALID_TRANSACTIONS) == (pindex->nTx > 0));
        if (pindex->nChainTx == 0) assert(pindex->nSequenceId == 0); 
        
        assert((pindexFirstMissing != NULL) == (pindex->nChainTx == 0));                                             
        assert(pindex->nHeight == nHeight);                                                                          
        assert(pindex->pprev == NULL || pindex->nChainWork >= pindex->pprev->nChainWork);                            
        assert(nHeight < 2 || (pindex->pskip && (pindex->pskip->nHeight < nHeight)));                                
        assert(pindexFirstNotTreeValid == NULL);                                                                     
        if ((pindex->nStatus & BLOCK_VALID_MASK) >= BLOCK_VALID_TREE) assert(pindexFirstNotTreeValid == NULL);       
        if ((pindex->nStatus & BLOCK_VALID_MASK) >= BLOCK_VALID_CHAIN) assert(pindexFirstNotChainValid == NULL);     
        if ((pindex->nStatus & BLOCK_VALID_MASK) >= BLOCK_VALID_SCRIPTS) assert(pindexFirstNotScriptsValid == NULL); 
        if (pindexFirstInvalid == NULL) {
            
            assert((pindex->nStatus & BLOCK_FAILED_MASK) == 0); 
        }
        if (!CBlockIndexWorkComparator()(pindex, chainActive.Tip()) && pindexFirstMissing == NULL) {
            if (pindexFirstInvalid == NULL) { 
                assert(setBlockIndexCandidates.count(pindex));
            }
        } else { 
            assert(setBlockIndexCandidates.count(pindex) == 0);
        }
        
        std::pair<std::multimap<CBlockIndex*, CBlockIndex*>::iterator, std::multimap<CBlockIndex*, CBlockIndex*>::iterator> rangeUnlinked = mapBlocksUnlinked.equal_range(pindex->pprev);
        bool foundInUnlinked = false;
        while (rangeUnlinked.first != rangeUnlinked.second) {
            assert(rangeUnlinked.first->first == pindex->pprev);
            if (rangeUnlinked.first->second == pindex) {
                foundInUnlinked = true;
                break;
            }
            rangeUnlinked.first++;
        }
        if (pindex->pprev && pindex->nStatus & BLOCK_HAVE_DATA && pindexFirstMissing != NULL) {
            if (pindexFirstInvalid == NULL) { 
                assert(foundInUnlinked);
            }
        } else { 
            assert(!foundInUnlinked);
        }
        
        

        
        std::pair<std::multimap<CBlockIndex*, CBlockIndex*>::iterator, std::multimap<CBlockIndex*, CBlockIndex*>::iterator> range = forward.equal_range(pindex);
        if (range.first != range.second) {
            
            pindex = range.first->second;
            nHeight++;
            continue;
        }
        
        
        while (pindex) {
            
            
            if (pindex == pindexFirstInvalid) pindexFirstInvalid = NULL;
            if (pindex == pindexFirstMissing) pindexFirstMissing = NULL;
            if (pindex == pindexFirstNotTreeValid) pindexFirstNotTreeValid = NULL;
            if (pindex == pindexFirstNotChainValid) pindexFirstNotChainValid = NULL;
            if (pindex == pindexFirstNotScriptsValid) pindexFirstNotScriptsValid = NULL;
            
            CBlockIndex* pindexPar = pindex->pprev;
            
            std::pair<std::multimap<CBlockIndex*, CBlockIndex*>::iterator, std::multimap<CBlockIndex*, CBlockIndex*>::iterator> rangePar = forward.equal_range(pindexPar);
            while (rangePar.first->second != pindex) {
                assert(rangePar.first != rangePar.second); 
                rangePar.first++;
            }
            
            rangePar.first++;
            if (rangePar.first != rangePar.second) {
                
                pindex = rangePar.first->second;
                break;
            } else {
                
                pindex = pindexPar;
                nHeight--;
                continue;
            }
        }
    }

    
    assert(nNodes == forward.size());
}






string GetWarnings(string strFor)
{
    int nPriority = 0;
    string strStatusBar;
    string strRPC;

    if (!CLIENT_VERSION_IS_RELEASE)
        strStatusBar = _("This is a pre-release test build - use at your own risk - do not use for staking or merchant applications!");

    if (GetBoolArg("-testsafemode", false))
        strStatusBar = strRPC = "testsafemode enabled";

    
    if (strMiscWarning != "") {
        nPriority = 1000;
        strStatusBar = strMiscWarning;
    }

    if (fLargeWorkForkFound) {
        nPriority = 2000;
        strStatusBar = strRPC = _("Warning: The network does not appear to fully agree! Some miners appear to be experiencing issues.");
    } else if (fLargeWorkInvalidChainFound) {
        nPriority = 2000;
        strStatusBar = strRPC = _("Warning: We do not appear to fully agree with our peers! You may need to upgrade, or other nodes may need to upgrade.");
    }

    
    {
        LOCK(cs_mapAlerts);
        BOOST_FOREACH (PAIRTYPE(const uint256, CAlert) & item, mapAlerts) {
            const CAlert& alert = item.second;
            if (alert.AppliesToMe() && alert.nPriority > nPriority) {
                nPriority = alert.nPriority;
                strStatusBar = alert.strStatusBar;
            }
        }
    }

    if (strFor == "statusbar")
        return strStatusBar;
    else if (strFor == "rpc")
        return strRPC;
    assert(!"GetWarnings() : invalid parameter");
    return "error";
}








bool static AlreadyHave(const CInv& inv)
{
    switch (inv.type) {
    case MSG_TX: {
        bool txInMap = false;
        txInMap = mempool.exists(inv.hash);
        return txInMap || mapOrphanTransactions.count(inv.hash) ||
                pcoinsTip->HaveCoins(inv.hash);
    }
    case MSG_DSTX:
        return mapObfuscationBroadcastTxes.count(inv.hash);
    case MSG_BLOCK:
        return mapBlockIndex.count(inv.hash);
    case MSG_TXLOCK_REQUEST:
        return mapTxLockReq.count(inv.hash) ||
                mapTxLockReqRejected.count(inv.hash);
    case MSG_TXLOCK_VOTE:
        return mapTxLockVote.count(inv.hash);
    case MSG_SPORK:
        return mapSporks.count(inv.hash);
    case MSG_MASTERNODE_WINNER:
        if (masternodePayments.mapMasternodePayeeVotes.count(inv.hash)) {
            masternodeSync.AddedMasternodeWinner(inv.hash);
            return true;
        }
        return false;
    case MSG_BUDGET_VOTE:
        if (budget.mapSeenMasternodeBudgetVotes.count(inv.hash)) {
            masternodeSync.AddedBudgetItem(inv.hash);
            return true;
        }
        return false;
    case MSG_BUDGET_PROPOSAL:
        if (budget.mapSeenMasternodeBudgetProposals.count(inv.hash)) {
            masternodeSync.AddedBudgetItem(inv.hash);
            return true;
        }
        return false;
    case MSG_BUDGET_FINALIZED_VOTE:
        if (budget.mapSeenFinalizedBudgetVotes.count(inv.hash)) {
            masternodeSync.AddedBudgetItem(inv.hash);
            return true;
        }
        return false;
    case MSG_BUDGET_FINALIZED:
        if (budget.mapSeenFinalizedBudgets.count(inv.hash)) {
            masternodeSync.AddedBudgetItem(inv.hash);
            return true;
        }
        return false;
    case MSG_MASTERNODE_ANNOUNCE:
        if (mnodeman.mapSeenMasternodeBroadcast.count(inv.hash)) {
            masternodeSync.AddedMasternodeList(inv.hash);
            return true;
        }
        return false;
    case MSG_MASTERNODE_PING:
        return mnodeman.mapSeenMasternodePing.count(inv.hash);
    }
    
    return true;
}


void static ProcessGetData(CNode* pfrom)
{
    std::deque<CInv>::iterator it = pfrom->vRecvGetData.begin();

    vector<CInv> vNotFound;

    LOCK(cs_main);

    while (it != pfrom->vRecvGetData.end()) {
        
        if (pfrom->nSendSize >= SendBufferSize())
            break;

        const CInv& inv = *it;
        {
            boost::this_thread::interruption_point();
            it++;

            if (inv.type == MSG_BLOCK || inv.type == MSG_FILTERED_BLOCK) {
                bool send = false;
                BlockMap::iterator mi = mapBlockIndex.find(inv.hash);
                if (mi != mapBlockIndex.end()) {
                    if (chainActive.Contains(mi->second)) {
                        send = true;
                    } else {
                        
                        
                        
                        send = mi->second->IsValid(BLOCK_VALID_SCRIPTS) && (pindexBestHeader != NULL) &&
                                (chainActive.Height() - mi->second->nHeight < Params().MaxReorganizationDepth());
                        if (!send) {
                            LogPrintf("ProcessGetData(): ignoring request from peer=%i for old block that isn't in the main chain\n", pfrom->GetId());
                        }
                    }
                }
                
                if (send && (mi->second->nStatus & BLOCK_HAVE_DATA)) {
                    
                    CBlock block;
                    if (!ReadBlockFromDisk(block, (*mi).second))
                        assert(!"cannot load block from disk");
                    if (inv.type == MSG_BLOCK) {
                        if (fEnableLz4Block) {
                            CDataStream blockStream(SER_NETWORK, PROTOCOL_VERSION);
                            std::string blockString;

                            unsigned int blockSize;
                            
                            blockStream << block;
                            blockString = blockStream.str();
                            blockSize = blockStream.size();
                            blockStream.clear();
                            
                            std::vector<unsigned char> vCompress;
                            LZ4IO_Compress(blockString, blockSize, vCompress);

                            blockStream.write((const char *) vCompress.data(), vCompress.size());
                            pfrom->PushMessage("block", blockStream);
                        } else {
                            pfrom->PushMessage("block", block);
                        }
                    }
                    else 
                    {
                        LOCK(pfrom->cs_filter);
                        if (pfrom->pfilter) {
                            CMerkleBlock merkleBlock(block, *pfrom->pfilter);
                            pfrom->PushMessage("merkleblock", merkleBlock);
                            
                            
                            
                            
                            
                            
                            typedef std::pair<unsigned int, uint256> PairType;
                            BOOST_FOREACH (PairType& pair, merkleBlock.vMatchedTxn)
                                    if (!pfrom->setInventoryKnown.count(CInv(MSG_TX, pair.second)))
                                    pfrom->PushMessage("tx", block.vtx[pair.first]);
                        }
                        
                        
                    }

                    
                    if (inv.hash == pfrom->hashContinue) {
                        
                        
                        
                        vector<CInv> vInv;
                        vInv.push_back(CInv(MSG_BLOCK, chainActive.Tip()->GetBlockHash()));
                        pfrom->PushMessage("inv", vInv);
                        pfrom->hashContinue = 0;
                    }
                }
            } else if (inv.IsKnownType()) {
                
                bool pushed = false;
                {
                    LOCK(cs_mapRelay);
                    map<CInv, CDataStream>::iterator mi = mapRelay.find(inv);
                    if (mi != mapRelay.end()) {
                        pfrom->PushMessage(inv.GetCommand(), (*mi).second);
                        pushed = true;
                    }
                }

                if (!pushed && inv.type == MSG_TX) {
                    CTransaction tx;
                    if (mempool.lookup(inv.hash, tx)) {
                        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                        ss.reserve(1000);
                        ss << tx;
                        pfrom->PushMessage("tx", ss);
                        pushed = true;
                    }
                }
                if (!pushed && inv.type == MSG_TXLOCK_VOTE) {
                    if (mapTxLockVote.count(inv.hash)) {
                        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                        ss.reserve(1000);
                        ss << mapTxLockVote[inv.hash];
                        pfrom->PushMessage("txlvote", ss);
                        pushed = true;
                    }
                }
                if (!pushed && inv.type == MSG_TXLOCK_REQUEST) {
                    if (mapTxLockReq.count(inv.hash)) {
                        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                        ss.reserve(1000);
                        ss << mapTxLockReq[inv.hash];
                        pfrom->PushMessage("ix", ss);
                        pushed = true;
                    }
                }
                if (!pushed && inv.type == MSG_SPORK) {
                    if (mapSporks.count(inv.hash)) {
                        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                        ss.reserve(1000);
                        ss << mapSporks[inv.hash];
                        pfrom->PushMessage("spork", ss);
                        pushed = true;
                    }
                }
                if (!pushed && inv.type == MSG_MASTERNODE_WINNER) {
                    if (masternodePayments.mapMasternodePayeeVotes.count(inv.hash)) {
                        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                        ss.reserve(1000);
                        ss << masternodePayments.mapMasternodePayeeVotes[inv.hash];
                        pfrom->PushMessage("mnw", ss);
                        pushed = true;
                    }
                }
                if (!pushed && inv.type == MSG_BUDGET_VOTE) {
                    if (budget.mapSeenMasternodeBudgetVotes.count(inv.hash)) {
                        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                        ss.reserve(1000);
                        ss << budget.mapSeenMasternodeBudgetVotes[inv.hash];
                        pfrom->PushMessage("mvote", ss);
                        pushed = true;
                    }
                }

                if (!pushed && inv.type == MSG_BUDGET_PROPOSAL) {
                    if (budget.mapSeenMasternodeBudgetProposals.count(inv.hash)) {
                        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                        ss.reserve(1000);
                        ss << budget.mapSeenMasternodeBudgetProposals[inv.hash];
                        pfrom->PushMessage("mprop", ss);
                        pushed = true;
                    }
                }

                if (!pushed && inv.type == MSG_BUDGET_FINALIZED_VOTE) {
                    if (budget.mapSeenFinalizedBudgetVotes.count(inv.hash)) {
                        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                        ss.reserve(1000);
                        ss << budget.mapSeenFinalizedBudgetVotes[inv.hash];
                        pfrom->PushMessage("fbvote", ss);
                        pushed = true;
                    }
                }

                if (!pushed && inv.type == MSG_BUDGET_FINALIZED) {
                    if (budget.mapSeenFinalizedBudgets.count(inv.hash)) {
                        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                        ss.reserve(1000);
                        ss << budget.mapSeenFinalizedBudgets[inv.hash];
                        pfrom->PushMessage("fbs", ss);
                        pushed = true;
                    }
                }

                if (!pushed && inv.type == MSG_MASTERNODE_ANNOUNCE) {
                    if (mnodeman.mapSeenMasternodeBroadcast.count(inv.hash)) {
                        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                        ss.reserve(1000);
                        ss << mnodeman.mapSeenMasternodeBroadcast[inv.hash];
                        pfrom->PushMessage("mnb", ss);
                        pushed = true;
                    }
                }

                if (!pushed && inv.type == MSG_MASTERNODE_PING) {
                    if (mnodeman.mapSeenMasternodePing.count(inv.hash)) {
                        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                        ss.reserve(1000);
                        ss << mnodeman.mapSeenMasternodePing[inv.hash];
                        pfrom->PushMessage("mnp", ss);
                        pushed = true;
                    }
                }

                if (!pushed && inv.type == MSG_DSTX) {
                    if (mapObfuscationBroadcastTxes.count(inv.hash)) {
                        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                        ss.reserve(1000);
                        ss << mapObfuscationBroadcastTxes[inv.hash].tx << mapObfuscationBroadcastTxes[inv.hash].vin << mapObfuscationBroadcastTxes[inv.hash].vchSig << mapObfuscationBroadcastTxes[inv.hash].sigTime;

                        pfrom->PushMessage("dstx", ss);
                        pushed = true;
                    }
                }


                if (!pushed) {
                    vNotFound.push_back(inv);
                }
            }

            
            g_signals.Inventory(inv.hash);

            if (inv.type == MSG_BLOCK || inv.type == MSG_FILTERED_BLOCK)
                break;
        }
    }

    pfrom->vRecvGetData.erase(pfrom->vRecvGetData.begin(), it);

    if (!vNotFound.empty()) {
        
        
        
        
        
        
        
        pfrom->PushMessage("notfound", vNotFound);
    }
}

bool fRequestedSporksIDB = false;
bool static ProcessMessage(CNode* pfrom, string strCommand, CDataStream& vRecv, int64_t nTimeReceived)
{
    RandAddSeedPerfmon();
    if (fDebug)
        LogPrintf("received: %s (%u bytes) peer=%d\n", SanitizeString(strCommand), vRecv.size(), pfrom->id);
    if (mapArgs.count("-dropmessagestest") && GetRand(atoi(mapArgs["-dropmessagestest"])) == 0) {
        LogPrintf("dropmessagestest DROPPING RECV MESSAGE\n");
        return true;
    }

    if (strCommand == "version") {
        
        if (pfrom->nVersion != 0) {
            pfrom->PushMessage("reject", strCommand, REJECT_DUPLICATE, string("Duplicate version message"));
            Misbehaving(pfrom->GetId(), 1);
            return false;
        }

        
        
        bool fMissingSporks = !pSporkDB->SporkExists(SPORK_14_NEW_PROTOCOL_ENFORCEMENT) &&
                !pSporkDB->SporkExists(SPORK_15_NEW_PROTOCOL_ENFORCEMENT_2) &&
                !pSporkDB->SporkExists(SPORK_16_ZEROCOIN_MAINTENANCE_MODE);

        if (fMissingSporks || !fRequestedSporksIDB){
            LogPrintf("asking peer for sporks\n");
            pfrom->PushMessage("getsporks");
            fRequestedSporksIDB = true;
        }

        int64_t nTime;
        CAddress addrMe;
        CAddress addrFrom;
        uint64_t nNonce = 1;
        vRecv >> pfrom->nVersion >> pfrom->nServices >> nTime >> addrMe;
        if (pfrom->DisconnectOldProtocol(ActiveProtocol(), strCommand))
            return false;

        if (pfrom->nVersion == 10300)
            pfrom->nVersion = 300;
        if (!vRecv.empty())
            vRecv >> addrFrom >> nNonce;
        if (!vRecv.empty()) {
            vRecv >> LIMITED_STRING(pfrom->strSubVer, 256);
            pfrom->cleanSubVer = SanitizeString(pfrom->strSubVer);
        }
        if (!vRecv.empty())
            vRecv >> pfrom->nStartingHeight;
        if (!vRecv.empty())
            vRecv >> pfrom->fRelayTxes; 
        else
            pfrom->fRelayTxes = true;

        
        if (nNonce == nLocalHostNonce && nNonce > 1) {
            LogPrintf("connected to self at %s, disconnecting\n", pfrom->addr.ToString());
            pfrom->fDisconnect = true;
            return true;
        }

        pfrom->addrLocal = addrMe;
        if (pfrom->fInbound && addrMe.IsRoutable()) {
            SeenLocal(addrMe);
        }

        
        if (pfrom->fInbound)
            pfrom->PushVersion();

        pfrom->fClient = !(pfrom->nServices & NODE_NETWORK);

        
        UpdatePreferredDownload(pfrom, State(pfrom->GetId()));

        
        pfrom->PushMessage("verack");
        pfrom->ssSend.SetVersion(min(pfrom->nVersion, PROTOCOL_VERSION));

        if (!pfrom->fInbound) {
            
            if (fListen && !IsInitialBlockDownload()) {
                CAddress addr = GetLocalAddress(&pfrom->addr);
                if (addr.IsRoutable()) {
                    LogPrintf("ProcessMessages: advertizing address %s\n", addr.ToString());
                    pfrom->PushAddress(addr);
                } else if (IsPeerAddrLocalGood(pfrom)) {
                    addr.SetIP(pfrom->addrLocal);
                    LogPrintf("ProcessMessages: advertizing address %s\n", addr.ToString());
                    pfrom->PushAddress(addr);
                }
            }

            
            if (pfrom->fOneShot || pfrom->nVersion >= CADDR_TIME_VERSION || addrman.size() < 1000) {
                pfrom->PushMessage("getaddr");
                pfrom->fGetAddr = true;
            }
            addrman.Good(pfrom->addr);
        } else {
            if (((CNetAddr)pfrom->addr) == (CNetAddr)addrFrom) {
                addrman.Add(addrFrom, addrFrom);
                addrman.Good(addrFrom);
            }
        }

        
        {
            LOCK(cs_mapAlerts);
            BOOST_FOREACH (PAIRTYPE(const uint256, CAlert) & item, mapAlerts)
                    item.second.RelayTo(pfrom);
        }

        pfrom->fSuccessfullyConnected = true;

        string remoteAddr;
        if (fLogIPs)
            remoteAddr = ", peeraddr=" + pfrom->addr.ToString();

        LogPrintf("receive version message: %s: version %d, blocks=%d, us=%s, peer=%d%s\n",
                  pfrom->cleanSubVer, pfrom->nVersion,
                  pfrom->nStartingHeight, addrMe.ToString(), pfrom->id,
                  remoteAddr);

        AddTimeData(pfrom->addr, nTime);
    }


    else if (pfrom->nVersion == 0) {
        
        Misbehaving(pfrom->GetId(), 1);
        return false;
    }


    else if (strCommand == "verack") {
        pfrom->SetRecvVersion(min(pfrom->nVersion, PROTOCOL_VERSION));

        
        if (pfrom->fNetworkNode) {
            LOCK(cs_main);
            State(pfrom->GetId())->fCurrentlyConnected = true;
        }
    }


    else if (strCommand == "addr") {
        vector<CAddress> vAddr;
        vRecv >> vAddr;

        
        if (pfrom->nVersion < CADDR_TIME_VERSION && addrman.size() > 1000)
            return true;
        if (vAddr.size() > 1000) {
            Misbehaving(pfrom->GetId(), 20);
            return error("message addr size() = %u", vAddr.size());
        }

        
        vector<CAddress> vAddrOk;
        int64_t nNow = GetAdjustedTime();
        int64_t nSince = nNow - 10 * 60;
        BOOST_FOREACH (CAddress& addr, vAddr) {
            boost::this_thread::interruption_point();

            if (addr.nTime <= 100000000 || addr.nTime > nNow + 10 * 60)
                addr.nTime = nNow - 5 * 24 * 60 * 60;
            pfrom->AddAddressKnown(addr);
            bool fReachable = IsReachable(addr);
            if (addr.nTime > nSince && !pfrom->fGetAddr && vAddr.size() <= 10 && addr.IsRoutable()) {
                
                {
                    LOCK(cs_vNodes);
                    
                    
                    static uint256 hashSalt;
                    if (hashSalt == 0)
                        hashSalt = GetRandHash();
                    uint64_t hashAddr = addr.GetHash();
                    uint256 hashRand = hashSalt ^ (hashAddr << 32) ^ ((GetTime() + hashAddr) / (24 * 60 * 60));
                    hashRand = Hash(BEGIN(hashRand), END(hashRand));
                    multimap<uint256, CNode*> mapMix;
                    BOOST_FOREACH (CNode* pnode, vNodes) {
                        if (pnode->nVersion < CADDR_TIME_VERSION)
                            continue;
                        unsigned int nPointer;
                        memcpy(&nPointer, &pnode, sizeof(nPointer));
                        uint256 hashKey = hashRand ^ nPointer;
                        hashKey = Hash(BEGIN(hashKey), END(hashKey));
                        mapMix.insert(make_pair(hashKey, pnode));
                    }
                    int nRelayNodes = fReachable ? 2 : 1; 
                    for (multimap<uint256, CNode*>::iterator mi = mapMix.begin(); mi != mapMix.end() && nRelayNodes-- > 0; ++mi)
                        ((*mi).second)->PushAddress(addr);
                }
            }
            
            if (fReachable)
                vAddrOk.push_back(addr);
        }
        addrman.Add(vAddrOk, pfrom->addr, 2 * 60 * 60);
        if (vAddr.size() < 1000)
            pfrom->fGetAddr = false;
        if (pfrom->fOneShot)
            pfrom->fDisconnect = true;
    }


    else if (strCommand == "inv") {
        vector<CInv> vInv;
        vRecv >> vInv;
        if (vInv.size() > MAX_INV_SZ) {
            Misbehaving(pfrom->GetId(), 20);
            return error("message inv size() = %u", vInv.size());
        }

        LOCK(cs_main);

        std::vector<CInv> vToFetch;

        for (unsigned int nInv = 0; nInv < vInv.size(); nInv++) {
            const CInv& inv = vInv[nInv];

            boost::this_thread::interruption_point();
            pfrom->AddInventoryKnown(inv);

            bool fAlreadyHave = AlreadyHave(inv);
            LogPrint("net", "got inv: %s  %s peer=%d\n", inv.ToString(), fAlreadyHave ? "have" : "new", pfrom->id);

            if (!fAlreadyHave && !fImporting && !fReindex && inv.type != MSG_BLOCK)
                pfrom->AskFor(inv);


            if (inv.type == MSG_BLOCK) {
                UpdateBlockAvailability(pfrom->GetId(), inv.hash);
                if (!fAlreadyHave && !fImporting && !fReindex && !mapBlocksInFlight.count(inv.hash)) {
                    
                    vToFetch.push_back(inv);
                    LogPrint("net", "getblocks (%d) %s to peer=%d\n", pindexBestHeader->nHeight, inv.hash.ToString(), pfrom->id);
                }
            }

            
            g_signals.Inventory(inv.hash);

            if (pfrom->nSendSize > (SendBufferSize() * 2)) {
                Misbehaving(pfrom->GetId(), 50);
                return error("send buffer size() = %u", pfrom->nSendSize);
            }
        }

        if (!vToFetch.empty())
            pfrom->PushMessage("getdata", vToFetch);
    }


    else if (strCommand == "getdata") {
        vector<CInv> vInv;
        vRecv >> vInv;
        if (vInv.size() > MAX_INV_SZ) {
            Misbehaving(pfrom->GetId(), 20);
            return error("message getdata size() = %u", vInv.size());
        }

        if (fDebug || (vInv.size() != 1))
            LogPrint("net", "received getdata (%u invsz) peer=%d\n", vInv.size(), pfrom->id);

        if ((fDebug && vInv.size() > 0) || (vInv.size() == 1))
            LogPrint("net", "received getdata for: %s peer=%d\n", vInv[0].ToString(), pfrom->id);

        pfrom->vRecvGetData.insert(pfrom->vRecvGetData.end(), vInv.begin(), vInv.end());
        ProcessGetData(pfrom);
    }


    else if (strCommand == "getblocks" || strCommand == "getheaders") {
        CBlockLocator locator;
        uint256 hashStop;
        vRecv >> locator >> hashStop;

        LOCK(cs_main);

        
        CBlockIndex* pindex = FindForkInGlobalIndex(chainActive, locator);

        
        if (pindex)
            pindex = chainActive.Next(pindex);
        int nLimit = 500;
        LogPrint("net", "getblocks %d to %s limit %d from peer=%d\n", (pindex ? pindex->nHeight : -1), hashStop == uint256(0) ? "end" : hashStop.ToString(), nLimit, pfrom->id);
        for (; pindex; pindex = chainActive.Next(pindex)) {
            if (pindex->GetBlockHash() == hashStop) {
                LogPrint("net", "  getblocks stopping at %d %s\n", pindex->nHeight, pindex->GetBlockHash().ToString());
                break;
            }
            pfrom->PushInventory(CInv(MSG_BLOCK, pindex->GetBlockHash()));
            if (--nLimit <= 0) {
                
                
                LogPrint("net", "  getblocks stopping at limit %d %s\n", pindex->nHeight, pindex->GetBlockHash().ToString());
                pfrom->hashContinue = pindex->GetBlockHash();
                break;
            }
        }
    }


    else if (strCommand == "headers" && Params().HeadersFirstSyncingActive()) {
        CBlockLocator locator;
        uint256 hashStop;
        vRecv >> locator >> hashStop;

        LOCK(cs_main);

        if (IsInitialBlockDownload())
            return true;

        CBlockIndex* pindex = NULL;
        if (locator.IsNull()) {
            
            BlockMap::iterator mi = mapBlockIndex.find(hashStop);
            if (mi == mapBlockIndex.end())
                return true;
            pindex = (*mi).second;
        } else {
            
            pindex = FindForkInGlobalIndex(chainActive, locator);
            if (pindex)
                pindex = chainActive.Next(pindex);
        }

        
        vector<CBlock> vHeaders;
        int nLimit = MAX_HEADERS_RESULTS;
        if (fDebug)
            LogPrintf("getheaders %d to %s from peer=%d\n", (pindex ? pindex->nHeight : -1), hashStop.ToString(), pfrom->id);
        for (; pindex; pindex = chainActive.Next(pindex)) {
            vHeaders.push_back(pindex->GetBlockHeader());
            if (--nLimit <= 0 || pindex->GetBlockHash() == hashStop)
                break;
        }
        pfrom->PushMessage("headers", vHeaders);
    }


    else if (strCommand == "tx" || strCommand == "dstx") {
        vector<uint256> vWorkQueue;
        vector<uint256> vEraseQueue;
        CTransaction tx;

        
        bool ignoreFees = false;
        CTxIn vin;
        vector<unsigned char> vchSig;
        int64_t sigTime;

        if (strCommand == "tx") {
            vRecv >> tx;
        } else if (strCommand == "dstx") {
            
            vRecv >> tx >> vin >> vchSig >> sigTime;

            CMasternode* pmn = mnodeman.Find(vin);
            if (pmn != NULL) {
                if (!pmn->allowFreeTx) {
                    
                    if (fDebug) LogPrintf("dstx: Masternode sending too many transactions %s\n", tx.GetHash().ToString());
                    return true;
                }

                std::string strMessage = tx.GetHash().ToString() + boost::lexical_cast<std::string>(sigTime);

                std::string errorMessage = "";
                if (!obfuScationSigner.VerifyMessage(pmn->pubKeyMasternode, vchSig, strMessage, errorMessage)) {
                    LogPrintf("dstx: Got bad masternode address signature %s \n", vin.ToString());
                    
                    return false;
                }

                LogPrintf("dstx: Got Masternode transaction %s\n", tx.GetHash().ToString());

                ignoreFees = true;
                pmn->allowFreeTx = false;

                if (!mapObfuscationBroadcastTxes.count(tx.GetHash())) {
                    CObfuscationBroadcastTx dstx;
                    dstx.tx = tx;
                    dstx.vin = vin;
                    dstx.vchSig = vchSig;
                    dstx.sigTime = sigTime;

                    mapObfuscationBroadcastTxes.insert(make_pair(tx.GetHash(), dstx));
                }
            }
        }

        CInv inv(MSG_TX, tx.GetHash());
        pfrom->AddInventoryKnown(inv);

        LOCK(cs_main);

        bool fMissingInputs = false;
        bool fMissingZerocoinInputs = false;
        CValidationState state;

        mapAlreadyAskedFor.erase(inv);

        if (!tx.IsZerocoinSpend() && AcceptToMemoryPool(mempool, state, tx, true, &fMissingInputs, false, ignoreFees)) {
            mempool.check(pcoinsTip);
            RelayTransaction(tx);
            vWorkQueue.push_back(inv.hash);

            LogPrint("mempool", "AcceptToMemoryPool: peer=%d %s : accepted %s (poolsz %u)\n",
                     pfrom->id, pfrom->cleanSubVer,
                     tx.GetHash().ToString(),
                     mempool.mapTx.size());

            
            set<NodeId> setMisbehaving;
            for(unsigned int i = 0; i < vWorkQueue.size(); i++) {
                map<uint256, set<uint256> >::iterator itByPrev = mapOrphanTransactionsByPrev.find(vWorkQueue[i]);
                if(itByPrev == mapOrphanTransactionsByPrev.end())
                    continue;
                for(set<uint256>::iterator mi = itByPrev->second.begin();
                    mi != itByPrev->second.end();
                    ++mi) {
                    const uint256 &orphanHash = *mi;
                    const CTransaction &orphanTx = mapOrphanTransactions[orphanHash].tx;
                    NodeId fromPeer = mapOrphanTransactions[orphanHash].fromPeer;
                    bool fMissingInputs2 = false;
                    
                    
                    
                    CValidationState stateDummy;


                    if(setMisbehaving.count(fromPeer))
                        continue;
                    if(AcceptToMemoryPool(mempool, stateDummy, orphanTx, true, &fMissingInputs2)) {
                        LogPrint("mempool", "   accepted orphan tx %s\n", orphanHash.ToString());
                        RelayTransaction(orphanTx);
                        vWorkQueue.push_back(orphanHash);
                        vEraseQueue.push_back(orphanHash);
                    } else if(!fMissingInputs2) {
                        int nDos = 0;
                        if(stateDummy.IsInvalid(nDos) && nDos > 0) {
                            
                            Misbehaving(fromPeer, nDos);
                            setMisbehaving.insert(fromPeer);
                            LogPrint("mempool", "   invalid orphan tx %s\n", orphanHash.ToString());
                        }
                        
                        
                        LogPrint("mempool", "   removed orphan tx %s\n", orphanHash.ToString());
                        vEraseQueue.push_back(orphanHash);
                    }
                    mempool.check(pcoinsTip);
                }
            }

            BOOST_FOREACH (uint256 hash, vEraseQueue)EraseOrphanTx(hash);
        } else if (tx.IsZerocoinSpend() && AcceptToMemoryPool(mempool, state, tx, true, &fMissingZerocoinInputs, false, ignoreFees)) {
            
            
            RelayTransaction(tx);
            LogPrint("mempool", "AcceptToMemoryPool: Zerocoinspend peer=%d %s : accepted %s (poolsz %u)\n",
                     pfrom->id, pfrom->cleanSubVer,
                     tx.GetHash().ToString(),
                     mempool.mapTx.size());
        } else if (fMissingInputs) {
            AddOrphanTx(tx, pfrom->GetId());

            
            unsigned int nMaxOrphanTx = (unsigned int)std::max((int64_t)0, GetArg("-maxorphantx", DEFAULT_MAX_ORPHAN_TRANSACTIONS));
            unsigned int nEvicted = LimitOrphanTxSize(nMaxOrphanTx);
            if (nEvicted > 0)
                LogPrint("mempool", "mapOrphan overflow, removed %u tx\n", nEvicted);
        } else if (pfrom->fWhitelisted) {
            
            
            

            RelayTransaction(tx);
        }

        if (strCommand == "dstx") {
            CInv inv(MSG_DSTX, tx.GetHash());
            RelayInv(inv);
        }

        int nDoS = 0;
        if (state.IsInvalid(nDoS)) {
            LogPrint("mempool", "%s from peer=%d %s was not accepted into the memory pool: %s\n", tx.GetHash().ToString(),
                     pfrom->id, pfrom->cleanSubVer,
                     state.GetRejectReason());
            pfrom->PushMessage("reject", strCommand, state.GetRejectCode(),
                               state.GetRejectReason().substr(0, MAX_REJECT_MESSAGE_LENGTH), inv.hash);
            if (nDoS > 0)
                Misbehaving(pfrom->GetId(), nDoS);
        }
    }


    else if (strCommand == "headers" && Params().HeadersFirstSyncingActive() && !fImporting && !fReindex) 
    {
        std::vector<CBlockHeader> headers;

        
        unsigned int nCount = ReadCompactSize(vRecv);
        if (nCount > MAX_HEADERS_RESULTS) {
            Misbehaving(pfrom->GetId(), 20);
            return error("headers message size = %u", nCount);
        }
        headers.resize(nCount);
        for (unsigned int n = 0; n < nCount; n++) {
            vRecv >> headers[n];
            ReadCompactSize(vRecv); 
        }

        LOCK(cs_main);

        if (nCount == 0) {
            
            return true;
        }
        CBlockIndex* pindexLast = NULL;
        BOOST_FOREACH (const CBlockHeader& header, headers) {
            CValidationState state;
            if (pindexLast != NULL && header.hashPrevBlock != pindexLast->GetBlockHash()) {
                Misbehaving(pfrom->GetId(), 20);
                return error("non-continuous headers sequence");
            }

            /*TODO: this has a CBlock cast on it so that it will compile. There should be a solution for this
             * before headers are reimplemented on mainnet
             */
            if (!AcceptBlockHeader((CBlock)header, state, &pindexLast)) {
                int nDoS;
                if (state.IsInvalid(nDoS)) {
                    if (nDoS > 0)
                        Misbehaving(pfrom->GetId(), nDoS);
                    std::string strError = "invalid header received " + header.GetHash().ToString();
                    return error(strError.c_str());
                }
            }
        }

        if (pindexLast)
            UpdateBlockAvailability(pfrom->GetId(), pindexLast->GetBlockHash());

        if (nCount == MAX_HEADERS_RESULTS && pindexLast) {
            
            
            
            LogPrintf("more getheaders (%d) to end to peer=%d (startheight:%d)\n", pindexLast->nHeight, pfrom->id, pfrom->nStartingHeight);
            pfrom->PushMessage("getheaders", chainActive.GetLocator(pindexLast), uint256(0));
        }

        CheckBlockIndex();
    }


    else if(strCommand == "tmpblock" && !fImporting && !fReindex )
    {
#ifdef  POW_IN_POS_PHASE
        LOCK(cs_main);

        CBlock block;
        CBlockIndex *pindexCurrent;

        CTmpBlockParams tmpBlockParams;
        vRecv >> tmpBlockParams;

        pindexCurrent = chainActive.Tip();

        if(tmpblockmempool.HaveTmpBlock(tmpBlockParams.GetHash())) return true; 

        if(pindexCurrent->nHeight < Params().POW_Start_BLOCK_In_POS() - 1) return true; 

        if(*pindexCurrent->phashBlock != tmpBlockParams.ori_hash ||
                GetTmpBlockValue(pindexCurrent->nHeight) != tmpBlockParams.coinBaseTx.GetValueOut()) return true; 
        
        if(!ReadBlockFromDisk(block, pindexCurrent) || !block.IsProofOfStake())
            return true;

        block.vtx[0] = tmpBlockParams.coinBaseTx;
        block.hashMerkleRoot = block.BuildMerkleTree();

        CBlockHeader blockHeader = block.GetBlockHeader(); 

        blockHeader.nNonce = tmpBlockParams.nNonce; 
        blockHeader.nBits = blockHeader.nBits2; 

        ProcessNewTmpBlockParam(tmpBlockParams, blockHeader);
#endif
    }

    else if (strCommand == "block" && !fImporting && !fReindex) 
    {
        CBlock block;
        std::string blockString = vRecv.str();
        unsigned int version = (blockString[0] | (blockString[1] << 8) | (blockString[2] << 16) | (blockString[3] << 24));
        
        if (LZ4IO_MAGICNUMBER == version)
        {
            unsigned int blockSize;
            std::vector<unsigned char> vDecompress;

            blockSize = vRecv.size();
            LZ4IO_Decompress(blockString, blockSize, vDecompress);
            if (vDecompress.size())
            {
                vRecv.clear();
                vRecv.write((const char*) vDecompress.data(), vDecompress.size());
            } else {
                return error("lz4 block decompress size = %u, fEnableLz4Block = %u", vDecompress.size(), fEnableLz4Block);
            }
        }
        vRecv >> block;
        uint256 hashBlock = block.GetHash();
        CInv inv(MSG_BLOCK, hashBlock);
        LogPrint("net", "received block %s peer=%d\n", inv.hash.ToString(), pfrom->id);

        
        if (!mapBlockIndex.count(block.hashPrevBlock)) {
            if (find(pfrom->vBlockRequested.begin(), pfrom->vBlockRequested.end(), hashBlock) != pfrom->vBlockRequested.end()) {
                
                pfrom->PushMessage("getblocks", chainActive.GetLocator(), block.hashPrevBlock);
                pfrom->vBlockRequested.push_back(block.hashPrevBlock);
            } else {
                
                pfrom->PushMessage("getblocks", chainActive.GetLocator(), hashBlock);
                pfrom->vBlockRequested.push_back(hashBlock);
            }
        } else {
            pfrom->AddInventoryKnown(inv);

            CValidationState state;
            if (!mapBlockIndex.count(block.GetHash())) {
                ProcessNewBlock(state, pfrom, &block);
                int nDoS;
                if(state.IsInvalid(nDoS)) {
                    pfrom->PushMessage("reject", strCommand, state.GetRejectCode(),
                                       state.GetRejectReason().substr(0, MAX_REJECT_MESSAGE_LENGTH), inv.hash);
                    if(nDoS > 0) {
                        TRY_LOCK(cs_main, lockMain);
                        if(lockMain) Misbehaving(pfrom->GetId(), nDoS);
                    }
                }
                
                pfrom->DisconnectOldProtocol(ActiveProtocol(), strCommand);
            } else {
                LogPrint("net", "%s : Already processed block %s, skipping ProcessNewBlock()\n", __func__, block.GetHash().GetHex());
            }
        }
    }


    
    
    
    
    
    else if ((strCommand == "getaddr") && (pfrom->fInbound)) {
        pfrom->vAddrToSend.clear();
        vector<CAddress> vAddr = addrman.GetAddr();
        BOOST_FOREACH (const CAddress& addr, vAddr)
                pfrom->PushAddress(addr);
    }


    else if (strCommand == "mempool") {
        LOCK2(cs_main, pfrom->cs_filter);

        std::vector<uint256> vtxid;
        mempool.queryHashes(vtxid);
        vector<CInv> vInv;
        BOOST_FOREACH (uint256& hash, vtxid) {
            CInv inv(MSG_TX, hash);
            CTransaction tx;
            bool fInMemPool = mempool.lookup(hash, tx);
            if (!fInMemPool) continue; 
            if ((pfrom->pfilter && pfrom->pfilter->IsRelevantAndUpdate(tx)) ||
                    (!pfrom->pfilter))
                vInv.push_back(inv);
            if (vInv.size() == MAX_INV_SZ) {
                pfrom->PushMessage("inv", vInv);
                vInv.clear();
            }
        }
        if (vInv.size() > 0)
            pfrom->PushMessage("inv", vInv);
    }


    else if (strCommand == "ping") {
        if (pfrom->nVersion > BIP0031_VERSION) {
            uint64_t nonce = 0;
            vRecv >> nonce;
            
            
            
            
            
            
            
            
            
            
            
            pfrom->PushMessage("pong", nonce);
        }
    }


    else if (strCommand == "pong") {
        int64_t pingUsecEnd = nTimeReceived;
        uint64_t nonce = 0;
        size_t nAvail = vRecv.in_avail();
        bool bPingFinished = false;
        std::string sProblem;

        if (nAvail >= sizeof(nonce)) {
            vRecv >> nonce;

            
            if (pfrom->nPingNonceSent != 0) {
                if (nonce == pfrom->nPingNonceSent) {
                    
                    bPingFinished = true;
                    int64_t pingUsecTime = pingUsecEnd - pfrom->nPingUsecStart;
                    if (pingUsecTime > 0) {
                        
                        pfrom->nPingUsecTime = pingUsecTime;
                    } else {
                        
                        sProblem = "Timing mishap";
                    }
                } else {
                    
                    sProblem = "Nonce mismatch";
                    if (nonce == 0) {
                        
                        bPingFinished = true;
                        sProblem = "Nonce zero";
                    }
                }
            } else {
                sProblem = "Unsolicited pong without ping";
            }
        } else {
            
            bPingFinished = true;
            sProblem = "Short payload";
        }

        if (!(sProblem.empty())) {
            LogPrint("net", "pong peer=%d %s: %s, %x expected, %x received, %u bytes\n",
                     pfrom->id,
                     pfrom->cleanSubVer,
                     sProblem,
                     pfrom->nPingNonceSent,
                     nonce,
                     nAvail);
        }
        if (bPingFinished) {
            pfrom->nPingNonceSent = 0;
        }
    }


    else if (fAlerts && strCommand == "alert") {
        CAlert alert;
        vRecv >> alert;

        uint256 alertHash = alert.GetHash();
        if (pfrom->setKnown.count(alertHash) == 0) {
            if (alert.ProcessAlert()) {
                
                pfrom->setKnown.insert(alertHash);
                {
                    LOCK(cs_vNodes);
                    BOOST_FOREACH (CNode* pnode, vNodes)
                            alert.RelayTo(pnode);
                }
            } else {
                
                
                
                
                
                
                Misbehaving(pfrom->GetId(), 10);
            }
        }
    }

    else if (!(nLocalServices & NODE_BLOOM) &&
             (strCommand == "filterload" ||
              strCommand == "filteradd" ||
              strCommand == "filterclear")) {
        LogPrintf("bloom message=%s\n", strCommand);
        Misbehaving(pfrom->GetId(), 100);
    }

    else if (strCommand == "filterload") {
        CBloomFilter filter;
        vRecv >> filter;

        if (!filter.IsWithinSizeConstraints())
            
            Misbehaving(pfrom->GetId(), 100);
        else {
            LOCK(pfrom->cs_filter);
            delete pfrom->pfilter;
            pfrom->pfilter = new CBloomFilter(filter);
            pfrom->pfilter->UpdateEmptyFull();
        }
        pfrom->fRelayTxes = true;
    }


    else if (strCommand == "filteradd") {
        vector<unsigned char> vData;
        vRecv >> vData;

        
        
        if (vData.size() > MAX_SCRIPT_ELEMENT_SIZE) {
            Misbehaving(pfrom->GetId(), 100);
        } else {
            LOCK(pfrom->cs_filter);
            if (pfrom->pfilter)
                pfrom->pfilter->insert(vData);
            else
                Misbehaving(pfrom->GetId(), 100);
        }
    }


    else if (strCommand == "filterclear") {
        LOCK(pfrom->cs_filter);
        delete pfrom->pfilter;
        pfrom->pfilter = new CBloomFilter();
        pfrom->fRelayTxes = true;
    }


    else if (strCommand == "reject") {
        if (fDebug) {
            try {
                string strMsg;
                unsigned char ccode;
                string strReason;
                vRecv >> LIMITED_STRING(strMsg, CMessageHeader::COMMAND_SIZE) >> ccode >> LIMITED_STRING(strReason, MAX_REJECT_MESSAGE_LENGTH);

                ostringstream ss;
                ss << strMsg << " code " << itostr(ccode) << ": " << strReason;

                if (strMsg == "block" || strMsg == "tx") {
                    uint256 hash;
                    vRecv >> hash;
                    ss << ": hash " << hash.ToString();
                }
                LogPrint("net", "Reject %s\n", SanitizeString(ss.str()));
            } catch (std::ios_base::failure& e) {
                
                LogPrint("net", "Unparseable reject message received\n");
            }
        }
    } else {
        
        obfuScationPool.ProcessMessageObfuscation(pfrom, strCommand, vRecv);
        mnodeman.ProcessMessage(pfrom, strCommand, vRecv);
        budget.ProcessMessage(pfrom, strCommand, vRecv);
        masternodePayments.ProcessMessageMasternodePayments(pfrom, strCommand, vRecv);
        ProcessMessageSwiftTX(pfrom, strCommand, vRecv);
        ProcessSpork(pfrom, strCommand, vRecv);
        masternodeSync.ProcessMessage(pfrom, strCommand, vRecv);
    }


    return true;
}





int ActiveProtocol()
{

    
    

    /*    if (IsSporkActive(SPORK_14_NEW_PROTOCOL_ENFORCEMENT))
            return MIN_PEER_PROTO_VERSION_AFTER_ENFORCEMENT;
*/

    
    

    if (IsSporkActive(SPORK_15_NEW_PROTOCOL_ENFORCEMENT_2))
        return MIN_PEER_PROTO_VERSION_AFTER_ENFORCEMENT;
    return MIN_PEER_PROTO_VERSION_BEFORE_ENFORCEMENT;
}


bool ProcessMessages(CNode* pfrom)
{
    
    

    
    
    
    
    
    
    
    
    bool fOk = true;

    if (!pfrom->vRecvGetData.empty())
        ProcessGetData(pfrom);

    
    if (!pfrom->vRecvGetData.empty()) return fOk;

    std::deque<CNetMessage>::iterator it = pfrom->vRecvMsg.begin();
    while (!pfrom->fDisconnect && it != pfrom->vRecvMsg.end()) {
        
        if (pfrom->nSendSize >= SendBufferSize())
            break;

        
        CNetMessage& msg = *it;

        
        
        
        

        
        if (!msg.complete())
            break;

        
        it++;

        
        if (memcmp(msg.hdr.pchMessageStart, Params().MessageStart(), MESSAGE_START_SIZE) != 0) {
            LogPrintf("PROCESSMESSAGE: INVALID MESSAGESTART %s peer=%d\n", SanitizeString(msg.hdr.GetCommand()), pfrom->id);
            fOk = false;
            break;
        }

        
        CMessageHeader& hdr = msg.hdr;
        if (!hdr.IsValid()) {
            LogPrintf("PROCESSMESSAGE: ERRORS IN HEADER %s peer=%d\n", SanitizeString(hdr.GetCommand()), pfrom->id);
            continue;
        }
        string strCommand = hdr.GetCommand();

        
        unsigned int nMessageSize = hdr.nMessageSize;

        
        CDataStream& vRecv = msg.vRecv;
        uint256 hash = Hash(vRecv.begin(), vRecv.begin() + nMessageSize);
        unsigned int nChecksum = 0;
        memcpy(&nChecksum, &hash, sizeof(nChecksum));
        if (nChecksum != hdr.nChecksum) {
            LogPrintf("ProcessMessages(%s, %u bytes): CHECKSUM ERROR nChecksum=%08x hdr.nChecksum=%08x\n",
                      SanitizeString(strCommand), nMessageSize, nChecksum, hdr.nChecksum);
            continue;
        }

        
        bool fRet = false;
        try {
            fRet = ProcessMessage(pfrom, strCommand, vRecv, msg.nTime);
            boost::this_thread::interruption_point();
        } catch (std::ios_base::failure& e) {
            pfrom->PushMessage("reject", strCommand, REJECT_MALFORMED, string("error parsing message"));
            if (strstr(e.what(), "end of data")) {
                
                LogPrintf("ProcessMessages(%s, %u bytes): Exception '%s' caught, normally caused by a message being shorter than its stated length\n", SanitizeString(strCommand), nMessageSize, e.what());
            } else if (strstr(e.what(), "size too large")) {
                
                LogPrintf("ProcessMessages(%s, %u bytes): Exception '%s' caught\n", SanitizeString(strCommand), nMessageSize, e.what());
            } else {
                PrintExceptionContinue(&e, "ProcessMessages()");
            }
        } catch (boost::thread_interrupted) {
            throw;
        } catch (std::exception& e) {
            PrintExceptionContinue(&e, "ProcessMessages()");
        } catch (...) {
            PrintExceptionContinue(NULL, "ProcessMessages()");
        }

        if (!fRet)
            LogPrintf("ProcessMessage(%s, %u bytes) FAILED peer=%d\n", SanitizeString(strCommand), nMessageSize, pfrom->id);

        break;
    }

    
    if (!pfrom->fDisconnect)
        pfrom->vRecvMsg.erase(pfrom->vRecvMsg.begin(), it);

    return fOk;
}


bool SendMessages(CNode* pto, bool fSendTrickle)
{
    {
        
        if (pto->nVersion == 0)
            return true;

        
        
        
        bool pingSend = false;
        if (pto->fPingQueued) {
            
            pingSend = true;
        }
        if (pto->nPingNonceSent == 0 && pto->nPingUsecStart + PING_INTERVAL * 1000000 < GetTimeMicros()) {
            
            pingSend = true;
        }
        if (pingSend) {
            uint64_t nonce = 0;
            while (nonce == 0) {
                GetRandBytes((unsigned char*)&nonce, sizeof(nonce));
            }
            pto->fPingQueued = false;
            pto->nPingUsecStart = GetTimeMicros();
            if (pto->nVersion > BIP0031_VERSION) {
                pto->nPingNonceSent = nonce;
                pto->PushMessage("ping", nonce);
            } else {
                
                pto->nPingNonceSent = 0;
                pto->PushMessage("ping");
            }
        }

        TRY_LOCK(cs_main, lockMain); 
        if (!lockMain)
            return true;

        
        static int64_t nLastRebroadcast;
        if (!IsInitialBlockDownload() && (GetTime() - nLastRebroadcast > 24 * 60 * 60)) {
            LOCK(cs_vNodes);
            BOOST_FOREACH (CNode* pnode, vNodes) {
                
                if (nLastRebroadcast)
                    pnode->setAddrKnown.clear();

                
                AdvertizeLocal(pnode);
            }
            if (!vNodes.empty())
                nLastRebroadcast = GetTime();
        }

        
        
        
        if (fSendTrickle) {
            vector<CAddress> vAddr;
            vAddr.reserve(pto->vAddrToSend.size());
            BOOST_FOREACH (const CAddress& addr, pto->vAddrToSend) {
                
                if (pto->setAddrKnown.insert(addr).second) {
                    vAddr.push_back(addr);
                    
                    if (vAddr.size() >= 1000) {
                        pto->PushMessage("addr", vAddr);
                        vAddr.clear();
                    }
                }
            }
            pto->vAddrToSend.clear();
            if (!vAddr.empty())
                pto->PushMessage("addr", vAddr);
        }

        CNodeState& state = *State(pto->GetId());
        if (state.fShouldBan) {
            if (pto->fWhitelisted)
                LogPrintf("Warning: not punishing whitelisted peer %s!\n", pto->addr.ToString());
            else {
                pto->fDisconnect = true;
                if (pto->addr.IsLocal())
                    LogPrintf("Warning: not banning local peer %s!\n", pto->addr.ToString());
                else {
                    CNode::Ban(pto->addr, BanReasonNodeMisbehaving);
                }
            }
            state.fShouldBan = false;
        }

        BOOST_FOREACH (const CBlockReject& reject, state.rejects)
                pto->PushMessage("reject", (string) "block", reject.chRejectCode, reject.strRejectReason, reject.hashBlock);
        state.rejects.clear();

        
        if (pindexBestHeader == NULL)
            pindexBestHeader = chainActive.Tip();
        bool fFetch = state.fPreferredDownload || (nPreferredDownload == 0 && !pto->fClient && !pto->fOneShot); 
        if (!state.fSyncStarted && !pto->fClient && fFetch  && !fReindex) {
            
            if (nSyncStarted == 0 || pindexBestHeader->GetBlockTime() > GetAdjustedTime() - 6 * 60 * 60) { 
                state.fSyncStarted = true;
                nSyncStarted++;
                
                
                
                pto->PushMessage("getblocks", chainActive.GetLocator(chainActive.Tip()), uint256(0));
            }
        }

        
        
        
        if (!fReindex ) {
            g_signals.Broadcast();
        }

        
        
        
        vector<CInv> vInv;
        vector<CInv> vInvWait;
        {
            LOCK(pto->cs_inventory);
            vInv.reserve(pto->vInventoryToSend.size());
            vInvWait.reserve(pto->vInventoryToSend.size());
            BOOST_FOREACH (const CInv& inv, pto->vInventoryToSend) {
                if (pto->setInventoryKnown.count(inv))
                    continue;

                
                if (inv.type == MSG_TX && !fSendTrickle) {
                    
                    static uint256 hashSalt;
                    if (hashSalt == 0)
                        hashSalt = GetRandHash();
                    uint256 hashRand = inv.hash ^ hashSalt;
                    hashRand = Hash(BEGIN(hashRand), END(hashRand));
                    bool fTrickleWait = ((hashRand & 3) != 0);

                    if (fTrickleWait) {
                        vInvWait.push_back(inv);
                        continue;
                    }
                }

                
                if (pto->setInventoryKnown.insert(inv).second) {
                    vInv.push_back(inv);
                    if (vInv.size() >= 1000) {
                        pto->PushMessage("inv", vInv);
                        vInv.clear();
                    }
                }
            }
            pto->vInventoryToSend = vInvWait;
        }
        if (!vInv.empty())
            pto->PushMessage("inv", vInv);

        
        int64_t nNow = GetTimeMicros();
        if (!pto->fDisconnect && state.nStallingSince && state.nStallingSince < nNow - 1000000 * BLOCK_STALLING_TIMEOUT) {
            
            
            
            LogPrintf("Peer=%d is stalling block download, disconnecting\n", pto->id);
            pto->fDisconnect = true;
        }
        
        
        
        
        
        if (!pto->fDisconnect && state.vBlocksInFlight.size() > 0 && state.vBlocksInFlight.front().nTime < nNow - 500000 * Params().TargetSpacing() * (4 + state.vBlocksInFlight.front().nValidatedQueuedBefore)) {
            LogPrintf("Timeout downloading block %s from peer=%d, disconnecting\n", state.vBlocksInFlight.front().hash.ToString(), pto->id);
            pto->fDisconnect = true;
        }

        
        
        
        vector<CInv> vGetData;
        if (!pto->fDisconnect && !pto->fClient && fFetch && state.nBlocksInFlight < MAX_BLOCKS_IN_TRANSIT_PER_PEER) {
            vector<CBlockIndex*> vToDownload;
            NodeId staller = -1;
            FindNextBlocksToDownload(pto->GetId(), MAX_BLOCKS_IN_TRANSIT_PER_PEER - state.nBlocksInFlight, vToDownload, staller);
            BOOST_FOREACH (CBlockIndex* pindex, vToDownload) {
                vGetData.push_back(CInv(MSG_BLOCK, pindex->GetBlockHash()));
                MarkBlockAsInFlight(pto->GetId(), pindex->GetBlockHash(), pindex);
                LogPrintf("Requesting block %s (%d) peer=%d\n", pindex->GetBlockHash().ToString(),
                          pindex->nHeight, pto->id);
            }
            if (state.nBlocksInFlight == 0 && staller != -1) {
                if (State(staller)->nStallingSince == 0) {
                    State(staller)->nStallingSince = nNow;
                    LogPrint("net", "Stall started peer=%d\n", staller);
                }
            }
        }

        
        
        
        while (!pto->fDisconnect && !pto->mapAskFor.empty() && (*pto->mapAskFor.begin()).first <= nNow) {
            const CInv& inv = (*pto->mapAskFor.begin()).second;
            if (!AlreadyHave(inv)) {
                if (fDebug)
                    LogPrint("net", "Requesting %s peer=%d\n", inv.ToString(), pto->id);
                vGetData.push_back(inv);
                if (vGetData.size() >= 1000) {
                    pto->PushMessage("getdata", vGetData);
                    vGetData.clear();
                }
            }
            pto->mapAskFor.erase(pto->mapAskFor.begin());
        }
        if (!vGetData.empty())
            pto->PushMessage("getdata", vGetData);
    }
    return true;
}


bool CBlockUndo::WriteToDisk(CDiskBlockPos& pos, const uint256& hashBlock)
{
    
    CAutoFile fileout(OpenUndoFile(pos), SER_DISK, CLIENT_VERSION);
    if (fileout.IsNull())
        return error("CBlockUndo::WriteToDisk : OpenUndoFile failed");

    
    unsigned int nSize = fileout.GetSerializeSize(*this);
    fileout << FLATDATA(Params().MessageStart()) << nSize;

    
    long fileOutPos = ftell(fileout.Get());
    if (fileOutPos < 0)
        return error("CBlockUndo::WriteToDisk : ftell failed");
    pos.nPos = (unsigned int)fileOutPos;
    fileout << *this;

    
    CHashWriter hasher(SER_GETHASH, PROTOCOL_VERSION);
    hasher << hashBlock;
    hasher << *this;
    fileout << hasher.GetHash();

    return true;
}

bool CBlockUndo::ReadFromDisk(const CDiskBlockPos& pos, const uint256& hashBlock)
{
    
    CAutoFile filein(OpenUndoFile(pos, true), SER_DISK, CLIENT_VERSION);
    if (filein.IsNull())
        return error("CBlockUndo::ReadFromDisk : OpenBlockFile failed");

    
    uint256 hashChecksum;
    try {
        filein >> *this;
        filein >> hashChecksum;
    } catch (std::exception& e) {
        return error("%s : Deserialize or I/O error - %s", __func__, e.what());
    }

    
    CHashWriter hasher(SER_GETHASH, PROTOCOL_VERSION);
    hasher << hashBlock;
    hasher << *this;
    if (hashChecksum != hasher.GetHash())
        return error("CBlockUndo::ReadFromDisk : Checksum mismatch");

    return true;
}

std::string CBlockFileInfo::ToString() const
{
    return strprintf("CBlockFileInfo(blocks=%u, size=%u, heights=%u...%u, time=%s...%s)", nBlocks, nSize, nHeightFirst, nHeightLast, DateTimeStrFormat("%Y-%m-%d", nTimeFirst), DateTimeStrFormat("%Y-%m-%d", nTimeLast));
}


class CMainCleanup
{
public:
    CMainCleanup() {}
    ~CMainCleanup()
    {
        
        BlockMap::iterator it1 = mapBlockIndex.begin();
        for (; it1 != mapBlockIndex.end(); it1++)
            delete (*it1).second;
        mapBlockIndex.clear();

        
        mapOrphanTransactions.clear();
        mapOrphanTransactionsByPrev.clear();
    }
} instance_of_cmaincleanup;
