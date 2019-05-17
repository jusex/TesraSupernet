





#include "merkleblock.h"

#include "hash.h"
#include "primitives/block.h" 
#include "utilstrencodings.h"

using namespace std;

CMerkleBlock::CMerkleBlock(const CBlock& block, CBloomFilter& filter)
{
    header = block.GetBlockHeader();

    vector<bool> vMatch;
    vector<uint256> vHashes;

    vMatch.reserve(block.vtx.size());
    vHashes.reserve(block.vtx.size());

    for (unsigned int i = 0; i < block.vtx.size(); i++) {
        const uint256& hash = block.vtx[i].GetHash();
        if (filter.IsRelevantAndUpdate(block.vtx[i])) {
            vMatch.push_back(true);
            vMatchedTxn.push_back(make_pair(i, hash));
        } else
            vMatch.push_back(false);
        vHashes.push_back(hash);
    }

    txn = CPartialMerkleTree(vHashes, vMatch);
}

uint256 CPartialMerkleTree::CalcHash(int height, unsigned int pos, const std::vector<uint256>& vTxid)
{
    if (height == 0) {
        
        return vTxid[pos];
    } else {
        
        uint256 left = CalcHash(height - 1, pos * 2, vTxid), right;
        
        if (pos * 2 + 1 < CalcTreeWidth(height - 1))
            right = CalcHash(height - 1, pos * 2 + 1, vTxid);
        else
            right = left;
        
        return Hash(BEGIN(left), END(left), BEGIN(right), END(right));
    }
}

void CPartialMerkleTree::TraverseAndBuild(int height, unsigned int pos, const std::vector<uint256>& vTxid, const std::vector<bool>& vMatch)
{
    
    bool fParentOfMatch = false;
    for (unsigned int p = pos << height; p < (pos + 1) << height && p < nTransactions; p++)
        fParentOfMatch |= vMatch[p];
    
    vBits.push_back(fParentOfMatch);
    if (height == 0 || !fParentOfMatch) {
        
        vHash.push_back(CalcHash(height, pos, vTxid));
    } else {
        
        TraverseAndBuild(height - 1, pos * 2, vTxid, vMatch);
        if (pos * 2 + 1 < CalcTreeWidth(height - 1))
            TraverseAndBuild(height - 1, pos * 2 + 1, vTxid, vMatch);
    }
}

uint256 CPartialMerkleTree::TraverseAndExtract(int height, unsigned int pos, unsigned int& nBitsUsed, unsigned int& nHashUsed, std::vector<uint256>& vMatch)
{
    if (nBitsUsed >= vBits.size()) {
        
        fBad = true;
        return 0;
    }
    bool fParentOfMatch = vBits[nBitsUsed++];
    if (height == 0 || !fParentOfMatch) {
        
        if (nHashUsed >= vHash.size()) {
            
            fBad = true;
            return 0;
        }
        const uint256& hash = vHash[nHashUsed++];
        if (height == 0 && fParentOfMatch) 
            vMatch.push_back(hash);
        return hash;
    } else {
        
        uint256 left = TraverseAndExtract(height - 1, pos * 2, nBitsUsed, nHashUsed, vMatch), right;
        if (pos * 2 + 1 < CalcTreeWidth(height - 1))
            right = TraverseAndExtract(height - 1, pos * 2 + 1, nBitsUsed, nHashUsed, vMatch);
        else
            right = left;
        
        return Hash(BEGIN(left), END(left), BEGIN(right), END(right));
    }
}

CPartialMerkleTree::CPartialMerkleTree(const std::vector<uint256>& vTxid, const std::vector<bool>& vMatch) : nTransactions(vTxid.size()), fBad(false)
{
    
    vBits.clear();
    vHash.clear();

    
    int nHeight = 0;
    while (CalcTreeWidth(nHeight) > 1)
        nHeight++;

    
    TraverseAndBuild(nHeight, 0, vTxid, vMatch);
}

CPartialMerkleTree::CPartialMerkleTree() : nTransactions(0), fBad(true) {}

uint256 CPartialMerkleTree::ExtractMatches(std::vector<uint256>& vMatch)
{
    vMatch.clear();
    
    if (nTransactions == 0)
        return 0;
    
    if (nTransactions > MAX_BLOCK_SIZE_CURRENT / 60) 
        return 0;
    
    if (vHash.size() > nTransactions)
        return 0;
    
    if (vBits.size() < vHash.size())
        return 0;
    
    int nHeight = 0;
    while (CalcTreeWidth(nHeight) > 1)
        nHeight++;
    
    unsigned int nBitsUsed = 0, nHashUsed = 0;
    uint256 hashMerkleRoot = TraverseAndExtract(nHeight, 0, nBitsUsed, nHashUsed, vMatch);
    
    if (fBad)
        return 0;
    
    if ((nBitsUsed + 7) / 8 != (vBits.size() + 7) / 8)
        return 0;
    
    if (nHashUsed != vHash.size())
        return 0;
    return hashMerkleRoot;
}
