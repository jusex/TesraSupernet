




#include "chain.h"

using namespace std;

/**
 * CChain implementation
 */
void CChain::SetTip(CBlockIndex* pindex)
{
    if (pindex == NULL) {
        vChain.clear();
        return;
    }
    vChain.resize(pindex->nHeight + 1);
    while (pindex && vChain[pindex->nHeight] != pindex) {
        vChain[pindex->nHeight] = pindex;
        pindex = pindex->pprev;
    }
}

CBlockLocator CChain::GetLocator(const CBlockIndex* pindex) const
{
    int nStep = 1;
    std::vector<uint256> vHave;
    vHave.reserve(32);

    if (!pindex)
        pindex = Tip();
    while (pindex) {
        vHave.push_back(pindex->GetBlockHash());
        
        if (pindex->nHeight == 0)
            break;
        
        int nHeight = std::max(pindex->nHeight - nStep, 0);
        if (Contains(pindex)) {
            
            pindex = (*this)[nHeight];
        } else {
            
            pindex = pindex->GetAncestor(nHeight);
        }
        if (vHave.size() > 10)
            nStep *= 2;
    }

    return CBlockLocator(vHave);
}

const CBlockIndex* CChain::FindFork(const CBlockIndex* pindex) const
{
    if (pindex->nHeight > Height())
        pindex = pindex->GetAncestor(Height());
    while (pindex && !Contains(pindex))
        pindex = pindex->pprev;
    return pindex;
}

uint256 CBlockIndex::GetBlockTrust() const
{
    uint256 bnTarget;
    bnTarget.SetCompact(nBits);
    if (bnTarget <= 0)
        return 0;

    if (IsProofOfStake()) {
        
        return (uint256(1) << 256) / (bnTarget + 1);
    } else {
        
        uint256 bnPoWTrust = ((~uint256(0) >> 20) / (bnTarget + 1));
        return bnPoWTrust > 1 ? bnPoWTrust : 1;
    }
}