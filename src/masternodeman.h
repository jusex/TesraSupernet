




#ifndef MASTERNODEMAN_H
#define MASTERNODEMAN_H

#include "base58.h"
#include "key.h"
#include "main.h"
#include "masternode.h"
#include "net.h"
#include "sync.h"
#include "util.h"

#define MASTERNODES_DUMP_SECONDS (15 * 60)
#define MASTERNODES_DSEG_SECONDS (3 * 60 * 60)

using namespace std;

class CMasternodeMan;

extern CMasternodeMan mnodeman;
void DumpMasternodes();

/** Access to the MN database (mncache.dat)
 */
class CMasternodeDB
{
private:
    boost::filesystem::path pathMN;
    std::string strMagicMessage;

public:
    enum ReadResult {
        Ok,
        FileError,
        HashReadError,
        IncorrectHash,
        IncorrectMagicMessage,
        IncorrectMagicNumber,
        IncorrectFormat
    };

    CMasternodeDB();
    bool Write(const CMasternodeMan& mnodemanToSave);
    ReadResult Read(CMasternodeMan& mnodemanToLoad, bool fDryRun = false);
};

class CMasternodeMan
{
private:
    
    mutable CCriticalSection cs;

    
    mutable CCriticalSection cs_process_message;

    
    std::vector<CMasternode> vMasternodes;
    
    std::map<CNetAddr, int64_t> mAskedUsForMasternodeList;
    
    std::map<CNetAddr, int64_t> mWeAskedForMasternodeList;
    
    std::map<COutPoint, int64_t> mWeAskedForMasternodeListEntry;

public:
    
    map<uint256, CMasternodeBroadcast> mapSeenMasternodeBroadcast;
    
    map<uint256, CMasternodePing> mapSeenMasternodePing;

    
    int64_t nDsqCount;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        LOCK(cs);
        READWRITE(vMasternodes);
        READWRITE(mAskedUsForMasternodeList);
        READWRITE(mWeAskedForMasternodeList);
        READWRITE(mWeAskedForMasternodeListEntry);
        READWRITE(nDsqCount);

        READWRITE(mapSeenMasternodeBroadcast);
        READWRITE(mapSeenMasternodePing);
    }

    CMasternodeMan();
    CMasternodeMan(CMasternodeMan& other);

    
    bool Add(CMasternode& mn);

    
    void AskForMN(CNode* pnode, CTxIn& vin);

    
    void Check();

    
    void CheckAndRemove(bool forceExpiredRemoval = false);

    
    void Clear();

    int CountEnabled(int protocolVersion = -1);

    void CountNetworks(int protocolVersion, int& ipv4, int& ipv6, int& onion);

    void DsegUpdate(CNode* pnode);

    
    CMasternode* Find(const CScript& payee);
    CMasternode* Find(const CTxIn& vin);
    CMasternode* Find(const CPubKey& pubKeyMasternode);

    
    CMasternode* GetNextMasternodeInQueueForPayment(int nBlockHeight, bool fFilterSigTime, int& nCount);

    
    CMasternode* FindRandomNotInVec(std::vector<CTxIn>& vecToExclude, int protocolVersion = -1);

    
    CMasternode* GetCurrentMasterNode(int mod = 1, int64_t nBlockHeight = 0, int minProtocol = 0);

    std::vector<CMasternode> GetFullMasternodeVector()
    {
        Check();
        return vMasternodes;
    }

    std::vector<pair<int, CMasternode> > GetMasternodeRanks(int64_t nBlockHeight, int minProtocol = 0);
    int GetMasternodeRank(const CTxIn& vin, int64_t nBlockHeight, int minProtocol = 0, bool fOnlyActive = true);
    CMasternode* GetMasternodeByRank(int nRank, int64_t nBlockHeight, int minProtocol = 0, bool fOnlyActive = true);

    void ProcessMasternodeConnections();

    void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv);

    
    int size() { return vMasternodes.size(); }

    
    int stable_size ();

    std::string ToString() const;

    void Remove(CTxIn vin);

    
    void UpdateMasternodeList(CMasternodeBroadcast mnb);
};

#endif
