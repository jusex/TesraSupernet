




#ifndef BITCOIN_POW_H
#define BITCOIN_POW_H

#include <stdint.h>

class CBlockHeader;
class CBlockIndex;
class uint256;
class arith_uint256;


enum DiffMode {
    DIFF_DEFAULT = 0, 
    DIFF_BTC = 1,     
    DIFF_KGW = 2,     
    DIFF_DGW = 3,     
};

unsigned int GetNextPowWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader* pblock);

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader* pblock);


bool CheckProofOfWork(uint256 hash, unsigned int nBits);
uint256 GetBlockProof(const CBlockIndex& block);

#endif 
