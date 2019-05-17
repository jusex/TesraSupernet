



#ifndef BITCOIN_KERNEL_H
#define BITCOIN_KERNEL_H

#include "main.h"



static const unsigned int MODIFIER_INTERVAL = 60;
static const unsigned int MODIFIER_INTERVAL_TESTNET = 60;
extern unsigned int nModifierInterval;
extern unsigned int getIntervalVersion(bool fTestNet);



static const int MODIFIER_INTERVAL_RATIO = 3;


bool ComputeNextStakeModifier(const CBlockIndex* pindexPrev, uint64_t& nStakeModifier, bool& fGeneratedStakeModifier);



uint256 stakeHash(unsigned int nTimeTx, CDataStream ss, unsigned int prevoutIndex, uint256 prevoutHash, unsigned int nTimeBlockFrom);
bool stakeTargetHit(uint256 hashProofOfStake, int64_t nValueIn, uint256 bnTargetPerCoinDay);
bool CheckStakeKernelHash(unsigned int nBits, const CBlock blockFrom, const CTransaction txPrev, const COutPoint prevout, unsigned int& nTimeTx, unsigned int nHashDrift, bool fCheck, uint256& hashProofOfStake, bool fPrintProofOfStake = false);



bool CheckProofOfStake(const CBlock block, uint256& hashProofOfStake);


bool CheckCoinStakeTimestamp(int64_t nTimeBlock, int64_t nTimeTx);


unsigned int GetStakeModifierChecksum(const CBlockIndex* pindex);


bool CheckStakeModifierCheckpoints(int nHeight, unsigned int nStakeModifierChecksum);


int64_t GetWeight(int64_t nIntervalBeginning, int64_t nIntervalEnd);

#endif 
