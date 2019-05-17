






#ifndef BITCOIN_MAIN_H
#define BITCOIN_MAIN_H

#if defined(HAVE_CONFIG_H)
#include "config/tesra-config.h"
#endif

#include "amount.h"
#include "chain.h"
#include "chainparams.h"
#include "coins.h"
#include "net.h"
#include "pow.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "primitives/zerocoin.h"
#include "script/script.h"
#include "script/sigcache.h"
#include "script/standard.h"
#include "sync.h"
#include "tinyformat.h"
#include "txmempool.h"
#include "tmpblocksmempool.h"
#include "uint256.h"
#include "undo.h"
#include "contractconfig.h"

#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

#include "libzerocoin/CoinSpend.h"
#include "contract_api/contractcomponent.h"

#include <boost/unordered_map.hpp>

class CBlockIndex;
class CBlockTreeDB;
class CZerocoinDB;
class CSporkDB;
class CBloomFilter;
class CInv;
class CScriptCheck;
class CValidationInterface;
class CValidationState;
class CAddressDB;

struct CBlockTemplate;
struct CNodeStateStats;


static const unsigned int DEFAULT_BLOCK_MAX_SIZE = 750000;
static const unsigned int DEFAULT_BLOCK_MIN_SIZE = 0;

static const unsigned int DEFAULT_BLOCK_PRIORITY_SIZE = 50000;

static const bool DEFAULT_ALERTS = true;

static const unsigned int MAX_STANDARD_TX_SIZE = 100000;
static const unsigned int MAX_ZEROCOIN_TX_SIZE = 150000;

static const unsigned int MAX_BLOCK_SIGOPS_CURRENT = MAX_BLOCK_SIZE_CURRENT / 50;
static const unsigned int MAX_BLOCK_SIGOPS_LEGACY = MAX_BLOCK_SIZE_LEGACY / 50;

static const unsigned int MAX_P2SH_SIGOPS = 15;

static const unsigned int MAX_TX_SIGOPS_CURRENT = MAX_BLOCK_SIGOPS_CURRENT / 5;
static const unsigned int MAX_TX_SIGOPS_LEGACY = MAX_BLOCK_SIGOPS_LEGACY / 5;

static const unsigned int DEFAULT_MAX_ORPHAN_TRANSACTIONS = 100;

static const unsigned int MAX_BLOCKFILE_SIZE = 0x8000000; 

static const unsigned int BLOCKFILE_CHUNK_SIZE = 0x1000000; 

static const unsigned int UNDOFILE_CHUNK_SIZE = 0x100000; 

static const int COINBASE_MATURITY = 100;

static const unsigned int LOCKTIME_THRESHOLD = 500000000; 

static const int MAX_SCRIPTCHECK_THREADS = 16;

static const int DEFAULT_SCRIPTCHECK_THREADS = 0;

static const int MAX_BLOCKS_IN_TRANSIT_PER_PEER = 16;

static const unsigned int BLOCK_STALLING_TIMEOUT = 2;
/** Number of headers sent in one getheaders result. We rely on the assumption that if a peer sends
 *  less than this number, we reached their tip. Changing this value is a protocol upgrade. */
static const unsigned int MAX_HEADERS_RESULTS = 2000;
/** Size of the "block download window": how far ahead of our current height do we fetch?
 *  Larger windows tolerate larger download speed differences between peer, but increase the potential
 *  degree of disordering of blocks on disk (which make reindexing and in the future perhaps pruning
 *  harder). We'll probably want to make this a per-peer adaptive value at some point. */
static const unsigned int BLOCK_DOWNLOAD_WINDOW = 1024;

static const unsigned int DATABASE_WRITE_INTERVAL = 3600;

static const unsigned int MAX_REJECT_MESSAGE_LENGTH = 111;


 static const bool DEFAULT_PEERBLOOMFILTERS = true;


static const unsigned char REJECT_MALFORMED = 0x01;
static const unsigned char REJECT_INVALID = 0x10;
static const unsigned char REJECT_OBSOLETE = 0x11;
static const unsigned char REJECT_DUPLICATE = 0x12;
static const unsigned char REJECT_NONSTANDARD = 0x40;
static const unsigned char REJECT_DUST = 0x41;
static const unsigned char REJECT_INSUFFICIENTFEE = 0x42;
static const unsigned char REJECT_CHECKPOINT = 0x43;

static const unsigned int REJECT_HIGHFEE = 0x100;

struct BlockHasher {
    size_t operator()(const uint256& hash) const { return hash.GetLow64(); }
};

extern CScript COINBASE_FLAGS;
extern CCriticalSection cs_main;
extern CTxMemPool mempool;

extern bool fTestNet;


#ifdef  POW_IN_POS_PHASE
extern TmpBlocksMempool tmpblockmempool;
#endif

typedef boost::unordered_map<uint256, CBlockIndex*, BlockHasher> BlockMap;
extern BlockMap mapBlockIndex;
extern uint64_t nLastBlockTx;
extern uint64_t nLastBlockSize;
extern const std::string strMessageMagic;
extern int64_t nTimeBestReceived;
extern CWaitableCriticalSection csBestBlock;
extern CConditionVariable cvBlockChange;
extern bool fImporting;
extern bool fReindex;
extern int nScriptCheckThreads;
extern bool fTxIndex;
extern bool fAddrIndex;
extern bool fLogEvents;
extern bool fIsBareMultisigStd;
extern bool fCheckBlockIndex;
extern unsigned int nCoinCacheSize;
extern CFeeRate minRelayTxFee;
extern bool fAlerts;
extern bool fVerifyingBlocks;

extern bool fLargeWorkForkFound;
extern bool fLargeWorkInvalidChainFound;

extern unsigned int nStakeMinAge;
extern int64_t nLastCoinStakeSearchInterval;
extern int64_t nLastCoinStakeSearchTime;
extern int64_t nReserveBalance;

extern std::map<uint256, int64_t> mapRejectedBlocks;
extern std::map<unsigned int, unsigned int> mapHashedBlocks;
extern std::map<COutPoint, COutPoint> mapInvalidOutPoints;
extern std::map<CBigNum, CAmount> mapInvalidSerials;
extern std::set<std::pair<COutPoint, unsigned int> > setStakeSeen;


extern CBlockIndex* pindexBestHeader;


static const uint64_t nMinDiskSpace = 52428800;


void RegisterValidationInterface(CValidationInterface* pwalletIn);

void UnregisterValidationInterface(CValidationInterface* pwalletIn);

void UnregisterAllValidationInterfaces();

void SyncWithWallets(const CTransaction& tx, const CBlock* pblock = NULL);


void RegisterNodeSignals(CNodeSignals& nodeSignals);

void UnregisterNodeSignals(CNodeSignals& nodeSignals);

#ifdef  POW_IN_POS_PHASE

bool GetBestTmpBlockParams(CTransaction& coinBaseTx, unsigned int& nNonce, unsigned int& nCount);

bool ProcessNewTmpBlockParam(CTmpBlockParams &tmpBlockParams, const CBlockHeader &blockHeader);

#endif

/** 
 * Process an incoming block. This only returns after the best known valid
 * block is made active. Note that it does not, however, guarantee that the
 * specific block passed to it has been checked for validity!
 * 
 * @param[out]  state   This may be set to an Error state if any error occurred processing it, including during validation/connection/etc of otherwise unrelated blocks during reorganisation; or it may be set to an Invalid state if pblock is itself invalid (but this is not guaranteed even when the block is checked). If you want to *possibly* get feedback on whether pblock is valid, you must also install a CValidationInterface - this will have its BlockChecked method called whenever *any* block completes validation.
 * @param[in]   pfrom   The node which we are receiving the block from; it is added to mapBlockSource and may be penalised if the block is invalid.
 * @param[in]   pblock  The block we want to process.
 * @param[out]  dbp     If pblock is stored to disk (or already there), this will be set to its location.
 * @return True if state.IsValid()
 */
bool ProcessNewBlock(CValidationState& state, CNode* pfrom, CBlock* pblock, CDiskBlockPos* dbp = NULL);

bool CheckDiskSpace(uint64_t nAdditionalBytes = 0);

FILE* OpenBlockFile(const CDiskBlockPos& pos, bool fReadOnly = false);

FILE* OpenUndoFile(const CDiskBlockPos& pos, bool fReadOnly = false);

boost::filesystem::path GetBlockPosFilename(const CDiskBlockPos& pos, const char* prefix);

bool LoadExternalBlockFile(FILE* fileIn, CDiskBlockPos* dbp = NULL);

bool InitBlockIndex();

bool LoadBlockIndex(std::string& strError);

void UnloadBlockIndex();
int LoadLogEvents();

int ActiveProtocol();

bool ProcessMessages(CNode* pfrom);
/**
 * Send queued protocol messages to be sent to a give node.
 *
 * @param[in]   pto             The node which we are sending messages to.
 * @param[in]   fSendTrickle    When true send the trickled data, otherwise trickle the data until true.
 */
bool SendMessages(CNode* pto, bool fSendTrickle);

void ThreadScriptCheck();



bool CheckProofOfWork(uint256 hash, unsigned int nBits);


bool IsInitialBlockDownload();

std::string GetWarnings(std::string strFor);

bool GetTransaction(const uint256& hash, CTransaction& tx, uint256& hashBlock, bool fAllowSlow = false);


bool DisconnectBlocksAndReprocess(int blocks);


double ConvertBitsToDouble(unsigned int nBits);
int64_t GetMasternodePayment(int nHeight, int64_t blockValue, int nMasternodeCount = 0);
unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader* pblock, bool fProofOfStake);

bool ActivateBestChain(CValidationState& state, CBlock* pblock = NULL, bool fAlreadyChecked = false);
CAmount GetBlockValue(int nHeight);
CAmount GetTmpBlockValue(int nHeight);


CBlockIndex* InsertBlockIndex(uint256 hash);

bool AbortNode(const std::string& msg, const std::string& userMessage = "");

bool GetNodeStateStats(NodeId nodeid, CNodeStateStats& stats);

void Misbehaving(NodeId nodeid, int howmuch);

void FlushStateToDisk();



bool AcceptToMemoryPool(CTxMemPool& pool, CValidationState& state, const CTransaction& tx, bool fLimitFree, bool* pfMissingInputs, bool fRejectInsaneFee = false, bool ignoreFees = false);

bool AcceptableInputs(CTxMemPool& pool, CValidationState& state, const CTransaction& tx, bool fLimitFree, bool* pfMissingInputs, bool fRejectInsaneFee = false, bool isDSTX = false);

int GetInputAge(CTxIn& vin);
int GetInputAgeIX(uint256 nTXHash, CTxIn& vin);
bool GetCoinAge(const CTransaction& tx, unsigned int nTxTime, uint64_t& nCoinAge);
int GetIXConfirmations(uint256 nTXHash);

struct CNodeStateStats {
    int nMisbehavior;
    int nSyncHeight;
    int nCommonHeight;
    std::vector<int> vHeightInFlight;
};

struct CDiskTxPos : public CDiskBlockPos {
    unsigned int nTxOffset; 

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(*(CDiskBlockPos*)this);
        READWRITE(VARINT(nTxOffset));
    }

    CDiskTxPos(const CDiskBlockPos& blockIn, unsigned int nTxOffsetIn) : CDiskBlockPos(blockIn.nFile, blockIn.nPos), nTxOffset(nTxOffsetIn)
    {
    }

    CDiskTxPos()
    {
        SetNull();
    }

    void SetNull()
    {
        CDiskBlockPos::SetNull();
        nTxOffset = 0;
    }
};


CAmount GetMinRelayFee(const CTransaction& tx, unsigned int nBytes, bool fAllowFree);
bool MoneyRange(CAmount nValueOut);

/**
 * Check transaction inputs, and make sure any
 * pay-to-script-hash transactions are evaluating IsStandard scripts
 * 
 * Why bother? To avoid denial-of-service attacks; an attacker
 * can submit a standard HASH... OP_EQUAL transaction,
 * which will get accepted into blocks. The redemption
 * script can be anything; an attacker could use a very
 * expensive-to-check-upon-redemption script like:
 *   DUP CHECKSIG DROP ... repeated 100 times... OP_1
 */

/** 
 * Check for standard transaction types
 * @param[in] mapInputs    Map of previous transactions that have outputs we're spending
 * @return True if all inputs (scriptSigs) use only standard transaction forms
 */
bool AreInputsStandard(const CTransaction& tx, const CCoinsViewCache& mapInputs);

/** 
 * Count ECDSA signature operations the old-fashioned (pre-0.6) way
 * @return number of sigops this transaction's outputs will produce when spent
 * @see CTransaction::FetchInputs
 */
unsigned int GetLegacySigOpCount(const CTransaction& tx);

/**
 * Count ECDSA signature operations in pay-to-script-hash inputs.
 * 
 * @param[in] mapInputs Map of previous transactions that have outputs we're spending
 * @return maximum number of sigops required to validate this transaction's inputs
 * @see CTransaction::FetchInputs
 */
unsigned int GetP2SHSigOpCount(const CTransaction& tx, const CCoinsViewCache& mapInputs);


/**
 * Check whether all inputs of this transaction are valid (no double spends, scripts & sigs, amounts)
 * This does not modify the UTXO set. If pvChecks is not NULL, script checks are pushed onto it
 * instead of being performed inline.
 */
bool CheckInputs(const CTransaction& tx, CValidationState& state, const CCoinsViewCache& view, bool fScriptChecks, unsigned int flags, bool cacheStore, std::vector<CScriptCheck>* pvChecks = NULL);


void UpdateCoins(const CTransaction& tx, CValidationState& state, CCoinsViewCache& inputs, CTxUndo& txundo, int nHeight);


bool CheckTransaction(const CTransaction& tx, bool fZerocoinActive, bool fRejectBadUTXO, CValidationState& state);
bool CheckZerocoinMint(const uint256& txHash, const CTxOut& txout, CValidationState& state, bool fCheckOnly = false);
bool CheckZerocoinSpend(const CTransaction tx, bool fVerifySignature, CValidationState& state);
libzerocoin::CoinSpend TxInToZerocoinSpend(const CTxIn& txin);
bool TxOutToPublicCoin(const CTxOut txout, libzerocoin::PublicCoin& pubCoin, CValidationState& state);
bool BlockToPubcoinList(const CBlock& block, list<libzerocoin::PublicCoin>& listPubcoins, bool fFilterInvalid);
bool BlockToZerocoinMintList(const CBlock& block, std::list<CZerocoinMint>& vMints, bool fFilterInvalid);
bool BlockToMintValueVector(const CBlock& block, const libzerocoin::CoinDenomination denom, std::vector<CBigNum>& vValues);
std::list<libzerocoin::CoinDenomination> ZerocoinSpendListFromBlock(const CBlock& block, bool fFilterInvalid);
void FindMints(vector<CZerocoinMint> vMintsToFind, vector<CZerocoinMint>& vMintsToUpdate, vector<CZerocoinMint>& vMissingMints, bool fExtendedSearch);
bool GetZerocoinMint(const CBigNum& bnPubcoin, uint256& txHash);
bool IsSerialKnown(const CBigNum& bnSerial);
bool IsSerialInBlockchain(const CBigNum& bnSerial, int& nHeightTx);
bool RemoveSerialFromDB(const CBigNum& bnSerial);
int GetZerocoinStartHeight();
bool IsTransactionInChain(uint256 txId, int& nHeightTx);
bool IsBlockHashInChain(const uint256& hashBlock);
void PopulateInvalidOutPointMap();
bool ValidOutPoint(const COutPoint out, int nHeight);
void RecalculateZULOSpent();
void RecalculateZULOMinted();
bool RecalculateULOSupply(int nHeightStart);
bool ReindexAccumulators(list<uint256>& listMissingCheckpoints, string& strError);


/**
 * Check if transaction will be final in the next block to be created.
 *
 * Calls IsFinalTx() with current block height and appropriate block time.
 *
 * See consensus/consensus.h for flag definitions.
 */
bool CheckFinalTx(const CTransaction& tx, int flags = -1);

/** Check for standard transaction types
 * @return True if all outputs (scriptPubKeys) use only standard transaction forms
 */
bool IsStandardTx(const CTransaction& tx, std::string& reason);

bool IsFinalTx(const CTransaction& tx, int nBlockHeight = 0, int64_t nBlockTime = 0);


class CBlockUndo
{
public:
    std::vector<CTxUndo> vtxundo; 

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(vtxundo);
    }

    bool WriteToDisk(CDiskBlockPos& pos, const uint256& hashBlock);
    bool ReadFromDisk(const CDiskBlockPos& pos, const uint256& hashBlock);
};


/** 
 * Closure representing one script verification
 * Note that this stores references to the spending transaction 
 */
class CScriptCheck
{
private:
    CScript scriptPubKey;
    const CTransaction* ptxTo;
    unsigned int nIn;
    unsigned int nFlags;
    bool cacheStore;
    ScriptError error;

public:
    CScriptCheck() : ptxTo(0), nIn(0), nFlags(0), cacheStore(false), error(SCRIPT_ERR_UNKNOWN_ERROR) {}
    CScriptCheck(const CCoins& txFromIn, const CTransaction& txToIn, unsigned int nInIn, unsigned int nFlagsIn, bool cacheIn) : scriptPubKey(txFromIn.vout[txToIn.vin[nInIn].prevout.n].scriptPubKey),
                                                                                                                                ptxTo(&txToIn), nIn(nInIn), nFlags(nFlagsIn), cacheStore(cacheIn), error(SCRIPT_ERR_UNKNOWN_ERROR) {}

    bool operator()();

    void swap(CScriptCheck& check)
    {
        scriptPubKey.swap(check.scriptPubKey);
        std::swap(ptxTo, check.ptxTo);
        std::swap(nIn, check.nIn);
        std::swap(nFlags, check.nFlags);
        std::swap(cacheStore, check.cacheStore);
        std::swap(error, check.error);
    }

    ScriptError GetScriptError() const { return error; }
};



bool WriteBlockToDisk(CBlock& block, CDiskBlockPos& pos);
bool ReadBlockFromDisk(CBlock& block, const CDiskBlockPos& pos);
bool ReadBlockFromDisk(CBlock& block, const CBlockIndex* pindex);




/** Undo the effects of this block (with given index) on the UTXO set represented by coins.
 *  In case pfClean is provided, operation will try to be tolerant about errors, and *pfClean
 *  will be true if no problems were found. Otherwise, the return value will be false in case
 *  of problems. Note that in any case, coins may be modified. */
bool DisconnectBlock(CBlock& block, CValidationState& state, CBlockIndex* pindex, CCoinsViewCache& coins, bool* pfClean = NULL);


bool DisconnectBlocksAndReprocess(int blocks);


bool ConnectBlock(const CBlock& block, CValidationState& state, CBlockIndex* pindex, CCoinsViewCache& coins, bool fJustCheck, bool fAlreadyChecked = false);


bool CheckBlockHeader(const CBlockHeader& block, CValidationState& state, bool fCheckPOW = true);
bool CheckBlock(const CBlock& block, CValidationState& state, bool fCheckPOW = true, bool fCheckMerkleRoot = true, bool fCheckSig = true);
bool CheckWork(const CBlock block, CBlockIndex* const pindexPrev);


bool ContextualCheckBlockHeader(const CBlockHeader& block, CValidationState& state, CBlockIndex* pindexPrev);
bool ContextualCheckBlock(const CBlock& block, CValidationState& state, CBlockIndex* pindexPrev);


bool TestBlockValidity(CValidationState& state, const CBlock& block, CBlockIndex* pindexPrev, bool fCheckPOW = true, bool fCheckMerkleRoot = true);


bool AcceptBlock(CBlock& block, CValidationState& state, CBlockIndex** pindex, CDiskBlockPos* dbp = NULL, bool fAlreadyCheckedBlock = false);
bool AcceptBlockHeader(const CBlockHeader& block, CValidationState& state, CBlockIndex** ppindex = NULL);


class CBlockFileInfo
{
public:
    unsigned int nBlocks;      
    unsigned int nSize;        
    unsigned int nUndoSize;    
    unsigned int nHeightFirst; 
    unsigned int nHeightLast;  
    uint64_t nTimeFirst;       
    uint64_t nTimeLast;        

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(VARINT(nBlocks));
        READWRITE(VARINT(nSize));
        READWRITE(VARINT(nUndoSize));
        READWRITE(VARINT(nHeightFirst));
        READWRITE(VARINT(nHeightLast));
        READWRITE(VARINT(nTimeFirst));
        READWRITE(VARINT(nTimeLast));
    }

    void SetNull()
    {
        nBlocks = 0;
        nSize = 0;
        nUndoSize = 0;
        nHeightFirst = 0;
        nHeightLast = 0;
        nTimeFirst = 0;
        nTimeLast = 0;
    }

    CBlockFileInfo()
    {
        SetNull();
    }

    std::string ToString() const;

    
    void AddBlock(unsigned int nHeightIn, uint64_t nTimeIn)
    {
        if (nBlocks == 0 || nHeightFirst > nHeightIn)
            nHeightFirst = nHeightIn;
        if (nBlocks == 0 || nTimeFirst > nTimeIn)
            nTimeFirst = nTimeIn;
        nBlocks++;
        if (nHeightIn > nHeightLast)
            nHeightLast = nHeightIn;
        if (nTimeIn > nTimeLast)
            nTimeLast = nTimeIn;
    }
};


class CValidationState
{
private:
    enum mode_state {
        MODE_VALID,   
        MODE_INVALID, 
        MODE_ERROR,   
    } mode;
    int nDoS;
    std::string strRejectReason;
    unsigned char chRejectCode;
    bool corruptionPossible;

public:
    CValidationState() : mode(MODE_VALID), nDoS(0), chRejectCode(0), corruptionPossible(false) {}
    bool DoS(int level, bool ret = false, unsigned int chRejectCodeIn = 0, std::string strRejectReasonIn = "", bool corruptionIn = false)
    {
        chRejectCode = chRejectCodeIn;
        strRejectReason = strRejectReasonIn;
        corruptionPossible = corruptionIn;
        if (mode == MODE_ERROR)
            return ret;
        nDoS += level;
        mode = MODE_INVALID;
        return ret;
    }
    bool Invalid(bool ret = false,
        unsigned char _chRejectCode = 0,
        std::string _strRejectReason = "")
    {
        return DoS(0, ret, _chRejectCode, _strRejectReason);
    }
    bool Error(std::string strRejectReasonIn = "")
    {
        if (mode == MODE_VALID)
            strRejectReason = strRejectReasonIn;
        mode = MODE_ERROR;
        return false;
    }
    bool Abort(const std::string& msg)
    {
        AbortNode(msg);
        return Error(msg);
    }
    bool IsValid() const
    {
        return mode == MODE_VALID;
    }
    bool IsInvalid() const
    {
        return mode == MODE_INVALID;
    }
    bool IsError() const
    {
        return mode == MODE_ERROR;
    }
    bool IsInvalid(int& nDoSOut) const
    {
        if (IsInvalid()) {
            nDoSOut = nDoS;
            return true;
        }
        return false;
    }
    bool CorruptionPossible() const
    {
        return corruptionPossible;
    }
    unsigned char GetRejectCode() const { return chRejectCode; }
    std::string GetRejectReason() const { return strRejectReason; }
};


class CVerifyDB
{
public:
    CVerifyDB();
    ~CVerifyDB();
    bool VerifyDB(CCoinsView* coinsview, int nCheckLevel, int nCheckDepth);
};


CBlockIndex* FindForkInGlobalIndex(const CChain& chain, const CBlockLocator& locator);


bool InvalidateBlock(CValidationState& state, CBlockIndex* pindex);


bool ReconsiderBlock(CValidationState& state, CBlockIndex* pindex);


extern CChain chainActive;


extern CCoinsViewCache* pcoinsTip;


extern CBlockTreeDB* pblocktree;


extern CZerocoinDB* zerocoinDB;


extern CSporkDB* pSporkDB;


extern CAddressDB *paddressmap;

struct CBlockTemplate {
    CBlock block;
    std::vector<CAmount> vTxFees;
    std::vector<int64_t> vTxSigOps;
};

/*
class CValidationInterface
{
protected:
    virtual void SyncTransaction(const CTransaction& tx, const CBlock* pblock){};
    virtual void EraseFromWallet(const uint256& hash){};
    virtual void SetBestChain(const CBlockLocator& locator){};
    virtual bool UpdatedTransaction(const uint256& hash) { return false; };
    virtual void Inventory(const uint256& hash){};
    virtual void ResendWalletTransactions(){};
    virtual void BlockChecked(const CBlock&, const CValidationState&){};
    friend void ::RegisterValidationInterface(CValidationInterface*);
    friend void ::UnregisterValidationInterface(CValidationInterface*);
    friend void ::UnregisterAllValidationInterfaces();
};
*/
#endif 
