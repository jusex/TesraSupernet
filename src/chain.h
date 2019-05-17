





#ifndef BITCOIN_CHAIN_H
#define BITCOIN_CHAIN_H

#include "pow.h"
#include "primitives/block.h"
#include "tinyformat.h"
#include "uint256.h"
#include "util.h"
#include "libzerocoin/Denominations.h"

#include <vector>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

struct CDiskBlockPos {
    int nFile;
    unsigned int nPos;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(VARINT(nFile));
        READWRITE(VARINT(nPos));
    }

    CDiskBlockPos()
    {
        SetNull();
    }

    CDiskBlockPos(int nFileIn, unsigned int nPosIn)
    {
        nFile = nFileIn;
        nPos = nPosIn;
    }

    friend bool operator==(const CDiskBlockPos& a, const CDiskBlockPos& b)
    {
        return (a.nFile == b.nFile && a.nPos == b.nPos);
    }

    friend bool operator!=(const CDiskBlockPos& a, const CDiskBlockPos& b)
    {
        return !(a == b);
    }

    void SetNull()
    {
        nFile = -1;
        nPos = 0;
    }
    bool IsNull() const { return (nFile == -1); }
};

enum BlockStatus {
    
    BLOCK_VALID_UNKNOWN = 0,

    
    BLOCK_VALID_HEADER = 1,

    
    
    BLOCK_VALID_TREE = 2,

    /**
     * Only first tx is coinbase, 2 <= coinbase input script length <= 100, transactions valid, no duplicate txids,
     * sigops, size, merkle root. Implies all parents are at least TREE but not necessarily TRANSACTIONS. When all
     * parent blocks also have TRANSACTIONS, CBlockIndex::nChainTx will be set.
     */
    BLOCK_VALID_TRANSACTIONS = 3,

    
    
    BLOCK_VALID_CHAIN = 4,

    
    BLOCK_VALID_SCRIPTS = 5,

    
    BLOCK_VALID_MASK = BLOCK_VALID_HEADER | BLOCK_VALID_TREE | BLOCK_VALID_TRANSACTIONS |
                       BLOCK_VALID_CHAIN |
                       BLOCK_VALID_SCRIPTS,

    BLOCK_HAVE_DATA = 8,  
    BLOCK_HAVE_UNDO = 16, 
    BLOCK_HAVE_MASK = BLOCK_HAVE_DATA | BLOCK_HAVE_UNDO,

    BLOCK_FAILED_VALID = 32, 
    BLOCK_FAILED_CHILD = 64, 
    BLOCK_FAILED_MASK = BLOCK_FAILED_VALID | BLOCK_FAILED_CHILD,
};

/** The block chain is a tree shaped structure starting with the
 * genesis block at the root, with each block potentially having multiple
 * candidates to be the next block. A blockindex may have multiple pprev pointing
 * to it, but at most one of them can be part of the currently active branch.
 */
class CBlockIndex
{
public:
    
    const uint256* phashBlock;

    
    CBlockIndex* pprev;

    
    CBlockIndex* pnext;

    
    CBlockIndex* pskip;

    
    uint256 bnChainTrust;

    
    int nHeight;

    
    int nFile;

    
    unsigned int nDataPos;

    
    unsigned int nUndoPos;

    
    uint256 nChainWork;

    
    
    unsigned int nTx;

    
    
    
    unsigned int nChainTx;

    
    unsigned int nStatus;

    unsigned int nFlags; 
    enum {
        BLOCK_PROOF_OF_STAKE = (1 << 0), 
        BLOCK_STAKE_ENTROPY = (1 << 1),  
        BLOCK_STAKE_MODIFIER = (1 << 2), 
    };

    
    uint256 GetBlockTrust() const;
    uint64_t nStakeModifier;             
    unsigned int nStakeModifierChecksum; 
    COutPoint prevoutStake;
    unsigned int nStakeTime;
    uint256 hashProofOfStake;
    int64_t nMint;
    int64_t nMoneySupply;

    
    int nVersion;
    uint256 hashMerkleRoot;
    unsigned int nTime;
    unsigned int nBits;
    unsigned int nNonce;
    uint256 nAccumulatorCheckpoint;
#ifdef  POW_IN_POS_PHASE
    unsigned int nBits2;
#endif

    
    uint32_t nSequenceId;
    
    
    std::map<libzerocoin::CoinDenomination, int64_t> mapZerocoinSupply;
    std::vector<libzerocoin::CoinDenomination> vMintDenominationsInBlock;
    
    void SetNull()
    {
        phashBlock = NULL;
        pprev = NULL;
        pskip = NULL;
        nHeight = 0;
        nFile = 0;
        nDataPos = 0;
        nUndoPos = 0;
        nChainWork = 0;
        nTx = 0;
        nChainTx = 0;
        nStatus = 0;
        nSequenceId = 0;

        nMint = 0;
        nMoneySupply = 0;
        nFlags = 0;
        nStakeModifier = 0;
        nStakeModifierChecksum = 0;
        prevoutStake.SetNull();
        nStakeTime = 0;

        nVersion = 0;
        hashMerkleRoot = uint256();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        nAccumulatorCheckpoint = 0;
#ifdef  POW_IN_POS_PHASE
        nBits2 = 0;
#endif
        
        for (auto& denom : libzerocoin::zerocoinDenomList) {
            mapZerocoinSupply.insert(make_pair(denom, 0));
        }
        vMintDenominationsInBlock.clear();
    }

    CBlockIndex()
    {
        SetNull();
    }

    CBlockIndex(const CBlock& block)
    {
        SetNull();

        nVersion = block.nVersion;
        hashMerkleRoot = block.hashMerkleRoot;
        nTime = block.nTime;
        nBits = block.nBits;
        nNonce = block.nNonce;
        if(block.nVersion > POS_VERSION)
            nAccumulatorCheckpoint = block.nAccumulatorCheckpoint;
#ifdef  POW_IN_POS_PHASE
        nBits2 = block.nBits2;
#endif
        
        bnChainTrust = uint256();
        nMint = 0;
        nMoneySupply = 0;
        nFlags = 0;
        nStakeModifier = 0;
        nStakeModifierChecksum = 0;
        hashProofOfStake = uint256();

        if (block.IsProofOfStake()) {
            SetProofOfStake();
            prevoutStake = block.vtx[1].vin[0].prevout;
            nStakeTime = block.nTime;
        } else {
            prevoutStake.SetNull();
            nStakeTime = 0;
        }
    }
    

    CDiskBlockPos GetBlockPos() const
    {
        CDiskBlockPos ret;
        if (nStatus & BLOCK_HAVE_DATA) {
            ret.nFile = nFile;
            ret.nPos = nDataPos;
        }
        return ret;
    }

    CDiskBlockPos GetUndoPos() const
    {
        CDiskBlockPos ret;
        if (nStatus & BLOCK_HAVE_UNDO) {
            ret.nFile = nFile;
            ret.nPos = nUndoPos;
        }
        return ret;
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion = nVersion;
        if (pprev)
            block.hashPrevBlock = pprev->GetBlockHash();
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime = nTime;
        block.nBits = nBits;
        block.nNonce = nNonce;
        block.nAccumulatorCheckpoint = nAccumulatorCheckpoint;
#ifdef  POW_IN_POS_PHASE
        block.nBits2 = nBits2;
#endif
        return block;
    }

    bool IsContractEnabled() const
    {
        return (nVersion > ZEROCOIN_VERSION);
    }

    int64_t GetZerocoinSupply() const
    {
        int64_t nTotal = 0;
        for (auto& denom : libzerocoin::zerocoinDenomList) {
            nTotal += libzerocoin::ZerocoinDenominationToAmount(denom) * mapZerocoinSupply.at(denom);
        }
        return nTotal;
    }

    bool MintedDenomination(libzerocoin::CoinDenomination denom) const
    {
        return std::find(vMintDenominationsInBlock.begin(), vMintDenominationsInBlock.end(), denom) != vMintDenominationsInBlock.end();
    }

    uint256 GetBlockHash() const
    {
        return *phashBlock;
    }

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }

    enum { nMedianTimeSpan = 11 };

    int64_t GetMedianTimePast() const
    {
        int64_t pmedian[nMedianTimeSpan];
        int64_t* pbegin = &pmedian[nMedianTimeSpan];
        int64_t* pend = &pmedian[nMedianTimeSpan];

        const CBlockIndex* pindex = this;
        for (int i = 0; i < nMedianTimeSpan && pindex; i++, pindex = pindex->pprev)
            *(--pbegin) = pindex->GetBlockTime();

        std::sort(pbegin, pend);
        return pbegin[(pend - pbegin) / 2];
    }

    bool IsProofOfWork() const
    {
        return !(nFlags & BLOCK_PROOF_OF_STAKE);
    }

    bool IsProofOfStake() const
    {
        return (nFlags & BLOCK_PROOF_OF_STAKE);
    }

    void SetProofOfStake()
    {
        nFlags |= BLOCK_PROOF_OF_STAKE;
    }

    unsigned int GetStakeEntropyBit() const
    {
        unsigned int nEntropyBit = ((GetBlockHash().Get64()) & 1);
        if (fDebug || GetBoolArg("-printstakemodifier", false))
            LogPrintf("GetStakeEntropyBit: nHeight=%u hashBlock=%s nEntropyBit=%u\n", nHeight, GetBlockHash().ToString().c_str(), nEntropyBit);

        return nEntropyBit;
    }

    bool SetStakeEntropyBit(unsigned int nEntropyBit)
    {
        if (nEntropyBit > 1)
            return false;
        nFlags |= (nEntropyBit ? BLOCK_STAKE_ENTROPY : 0);
        return true;
    }

    bool GeneratedStakeModifier() const
    {
        return (nFlags & BLOCK_STAKE_MODIFIER);
    }

    void SetStakeModifier(uint64_t nModifier, bool fGeneratedStakeModifier)
    {
        nStakeModifier = nModifier;
        if (fGeneratedStakeModifier)
            nFlags |= BLOCK_STAKE_MODIFIER;
    }

    /**
     * Returns true if there are nRequired or more blocks of minVersion or above
     * in the last Params().ToCheckBlockUpgradeMajority() blocks, starting at pstart 
     * and going backwards.
     */
    static bool IsSuperMajority(int minVersion, const CBlockIndex* pstart, unsigned int nRequired);

    std::string ToString() const
    {
        return strprintf("CBlockIndex(pprev=%p, nHeight=%d, merkle=%s, hashBlock=%s)",
            pprev, nHeight,
            hashMerkleRoot.ToString(),
            GetBlockHash().ToString());
    }

    
    bool IsValid(enum BlockStatus nUpTo = BLOCK_VALID_TRANSACTIONS) const
    {
        assert(!(nUpTo & ~BLOCK_VALID_MASK)); 
        if (nStatus & BLOCK_FAILED_MASK)
            return false;
        return ((nStatus & BLOCK_VALID_MASK) >= nUpTo);
    }

    
    
    bool RaiseValidity(enum BlockStatus nUpTo)
    {
        assert(!(nUpTo & ~BLOCK_VALID_MASK)); 
        if (nStatus & BLOCK_FAILED_MASK)
            return false;
        if ((nStatus & BLOCK_VALID_MASK) < nUpTo) {
            nStatus = (nStatus & ~BLOCK_VALID_MASK) | nUpTo;
            return true;
        }
        return false;
    }

    
    void BuildSkip();

    
    CBlockIndex* GetAncestor(int height);
    const CBlockIndex* GetAncestor(int height) const;
};


class CDiskBlockIndex : public CBlockIndex
{
public:
    uint256 hashPrev;
    uint256 hashNext;

    CDiskBlockIndex()
    {
        hashPrev = uint256();
        hashNext = uint256();
    }

    explicit CDiskBlockIndex(CBlockIndex* pindex) : CBlockIndex(*pindex)
    {
        hashPrev = (pprev ? pprev->GetBlockHash() : uint256());
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        if (!(nType & SER_GETHASH))
            READWRITE(VARINT(nVersion));

        READWRITE(VARINT(nHeight));
        READWRITE(VARINT(nStatus));
        READWRITE(VARINT(nTx));
        if (nStatus & (BLOCK_HAVE_DATA | BLOCK_HAVE_UNDO))
            READWRITE(VARINT(nFile));
        if (nStatus & BLOCK_HAVE_DATA)
            READWRITE(VARINT(nDataPos));
        if (nStatus & BLOCK_HAVE_UNDO)
            READWRITE(VARINT(nUndoPos));


        READWRITE(nMint);
        READWRITE(nMoneySupply);
        READWRITE(nFlags);
        READWRITE(nStakeModifier);
        if (IsProofOfStake()) {
            READWRITE(prevoutStake);
            READWRITE(nStakeTime);
        } else {
            const_cast<CDiskBlockIndex*>(this)->prevoutStake.SetNull();
            const_cast<CDiskBlockIndex*>(this)->nStakeTime = 0;
            const_cast<CDiskBlockIndex*>(this)->hashProofOfStake = uint256();
        }

        
        READWRITE(this->nVersion);
        READWRITE(hashPrev);
        READWRITE(hashNext);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
        if(this->nVersion > POS_VERSION) {
            READWRITE(nAccumulatorCheckpoint);
#ifdef  POW_IN_POS_PHASE
            if (IsProofOfStake())
                READWRITE(nBits2);
#endif
            READWRITE(mapZerocoinSupply);
            READWRITE(vMintDenominationsInBlock);
        }

    }

    uint256 GetBlockHash() const
    {
        CBlockHeader block;
        block.nVersion = nVersion;
        block.hashPrevBlock = hashPrev;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime = nTime;
        block.nBits = nBits;
        block.nNonce = nNonce;
        block.nAccumulatorCheckpoint = nAccumulatorCheckpoint;
#ifdef  POW_IN_POS_PHASE
        block.nBits2 = nBits2;
#endif
        return block.GetHash();
    }


    std::string ToString() const
    {
        std::string str = "CDiskBlockIndex(";
        str += CBlockIndex::ToString();
        str += strprintf("\n                hashBlock=%s, hashPrev=%s)",
            GetBlockHash().ToString(),
            hashPrev.ToString());
        return str;
    }
};


class CChain
{
private:
    std::vector<CBlockIndex*> vChain;

public:
    
    CBlockIndex* Genesis() const
    {
        return vChain.size() > 0 ? vChain[0] : NULL;
    }

    
    CBlockIndex* Tip(bool fProofOfStake = false) const
    {
        if (vChain.size() < 1)
            return NULL;

        CBlockIndex* pindex = vChain[vChain.size() - 1];

        if (fProofOfStake) {
            while (pindex && pindex->pprev && !pindex->IsProofOfStake())
                pindex = pindex->pprev;
        }
        return pindex;
    }

    
    CBlockIndex* operator[](int nHeight) const
    {
        if (nHeight < 0 || nHeight >= (int)vChain.size())
            return NULL;
        return vChain[nHeight];
    }

    
    friend bool operator==(const CChain& a, const CChain& b)
    {
        return a.vChain.size() == b.vChain.size() &&
               a.vChain[a.vChain.size() - 1] == b.vChain[b.vChain.size() - 1];
    }

    
    bool Contains(const CBlockIndex* pindex) const
    {
        return (*this)[pindex->nHeight] == pindex;
    }

    
    CBlockIndex* Next(const CBlockIndex* pindex) const
    {
        if (Contains(pindex))
            return (*this)[pindex->nHeight + 1];
        else
            return NULL;
    }

    
    int Height() const
    {
        return vChain.size() - 1;
    }

    
    void SetTip(CBlockIndex* pindex);

    
    CBlockLocator GetLocator(const CBlockIndex* pindex = NULL) const;

    
    const CBlockIndex* FindFork(const CBlockIndex* pindex) const;
};

#endif 
