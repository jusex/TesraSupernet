



#ifndef BITCOIN_CHECKPOINTS_H
#define BITCOIN_CHECKPOINTS_H

#include "uint256.h"

#include <map>

class CBlockIndex;

/** 
 * Block-chain checkpoints are compiled-in sanity checks.
 * They are updated every release or three.
 */
namespace Checkpoints
{
typedef std::map<int, uint256> MapCheckpoints;

struct CCheckpointData {
    const MapCheckpoints* mapCheckpoints;
    int64_t nTimeLastCheckpoint;
    int64_t nTransactionsLastCheckpoint;
    double fTransactionsPerDay;
};


bool CheckBlock(int nHeight, const uint256& hash, bool fMatchesCheckpoint = false);


int GetTotalBlocksEstimate();


CBlockIndex* GetLastCheckpoint();

double GuessVerificationProgress(CBlockIndex* pindex, bool fSigchecks = true);

extern bool fEnabled;

} 

#endif 
