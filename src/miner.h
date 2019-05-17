




#ifndef BITCOIN_MINER_H
#define BITCOIN_MINER_H

#include <stdint.h>
#include "primitives/transaction.h"

class CBlock;
class CBlockHeader;
class CBlockIndex;
class CReserveKey;
class CScript;
class CWallet;

struct CBlockTemplate;

static const int32_t BYTECODE_TIME_BUFFER = 6;


void GenerateBitcoins(bool fGenerate, CWallet* pwallet, int nThreads);

CBlockTemplate* CreateNewPowBlock(CBlockIndex* pindexPrev, CWallet* pwallet);
CBlockTemplate* CreateNewBlock(const CScript& scriptPubKeyIn, CWallet* pwallet, bool fProofOfStake);
CBlockTemplate* CreateNewBlockWithKey(CReserveKey& reservekey, CWallet* pwallet, bool fProofOfStake);

void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce);

void UpdateTime(CBlockHeader* block, const CBlockIndex* pindexPrev);

void BitcoinMiner(CWallet* pwallet, bool fProofOfStake);

void RebuildRefundTransaction(CBlock *pblock, CAmount &nFees);
bool AttemptToAddContractToBlock(const CTransaction &iter, uint64_t minGasPrice, CBlockTemplate *pblockTemplate, uint64_t &nBlockSize, int &nBlockSigOps, uint64_t &nBlockTx, CCoinsViewCache &view, CAmount &nFees);



extern double dHashesPerSec;
extern int64_t nHPSTimerStart;

#endif 
