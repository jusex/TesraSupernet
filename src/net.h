





#ifndef BITCOIN_NET_H
#define BITCOIN_NET_H

#include "bloom.h"
#include "compat.h"
#include "hash.h"
#include "limitedmap.h"
#include "mruset.h"
#include "netbase.h"
#include "protocol.h"
#include "random.h"
#include "streams.h"
#include "sync.h"
#include "uint256.h"
#include "utilstrencodings.h"

#include <deque>
#include <stdint.h>

#ifndef WIN32
#include <arpa/inet.h>
#endif

#include <boost/filesystem/path.hpp>
#include <boost/foreach.hpp>
#include <boost/signals2/signal.hpp>

class CAddrMan;
class CBlockIndex;
class CNode;

namespace boost
{
class thread_group;
} 


static const int PING_INTERVAL = 2 * 60;

static const int TIMEOUT_INTERVAL = 20 * 60;

static const unsigned int MAX_INV_SZ = 50000;

static const unsigned int MAX_ADDR_TO_SEND = 1000;

static const unsigned int MAX_PROTOCOL_MESSAGE_LENGTH = 2 * 1024 * 1024;

static const bool DEFAULT_LISTEN = true;

#ifdef USE_UPNP
static const bool DEFAULT_UPNP = USE_UPNP;
#else
static const bool DEFAULT_UPNP = false;
#endif

static const size_t MAPASKFOR_MAX_SZ = MAX_INV_SZ;

unsigned int ReceiveFloodSize();
unsigned int SendBufferSize();

void AddOneShot(std::string strDest);
bool RecvLine(SOCKET hSocket, std::string& strLine);
void AddressCurrentlyConnected(const CService& addr);
CNode* FindNode(const CNetAddr& ip);
CNode* FindNode(const CSubNet& subNet);
CNode* FindNode(const std::string& addrName);
CNode* FindNode(const CService& ip);
CNode* ConnectNode(CAddress addrConnect, const char* pszDest = NULL, bool obfuScationMaster = false);
bool OpenNetworkConnection(const CAddress& addrConnect, CSemaphoreGrant* grantOutbound = NULL, const char* strDest = NULL, bool fOneShot = false);
void MapPort(bool fUseUPnP);
unsigned short GetListenPort();
bool BindListenPort(const CService& bindAddr, std::string& strError, bool fWhitelisted = false);
void StartNode(boost::thread_group& threadGroup);
bool StopNode();
void SocketSendData(CNode* pnode);

typedef int NodeId;


struct CNodeSignals {
    boost::signals2::signal<int()> GetHeight;
    boost::signals2::signal<bool(CNode*)> ProcessMessages;
    boost::signals2::signal<bool(CNode*, bool)> SendMessages;
    boost::signals2::signal<void(NodeId, const CNode*)> InitializeNode;
    boost::signals2::signal<void(NodeId)> FinalizeNode;
};


CNodeSignals& GetNodeSignals();


enum {
    LOCAL_NONE,   
    LOCAL_IF,     
    LOCAL_BIND,   
    LOCAL_UPNP,   
    LOCAL_MANUAL, 

    LOCAL_MAX
};

bool IsPeerAddrLocalGood(CNode* pnode);
void AdvertizeLocal(CNode* pnode);
void SetLimited(enum Network net, bool fLimited = true);
bool IsLimited(enum Network net);
bool IsLimited(const CNetAddr& addr);
bool AddLocal(const CService& addr, int nScore = LOCAL_NONE);
bool AddLocal(const CNetAddr& addr, int nScore = LOCAL_NONE);
bool RemoveLocal(const CService& addr);
bool SeenLocal(const CService& addr);
bool IsLocal(const CService& addr);
bool GetLocal(CService& addr, const CNetAddr* paddrPeer = NULL);
bool IsReachable(enum Network net);
bool IsReachable(const CNetAddr& addr);
void SetReachable(enum Network net, bool fFlag = true);
CAddress GetLocalAddress(const CNetAddr* paddrPeer = NULL);


extern bool fDiscover;
extern bool fListen;
extern uint64_t nLocalServices;
extern uint64_t nLocalHostNonce;
extern CAddrMan addrman;
extern int nMaxConnections;

extern std::vector<CNode*> vNodes;
extern CCriticalSection cs_vNodes;
extern std::map<CInv, CDataStream> mapRelay;
extern std::deque<std::pair<int64_t, CInv> > vRelayExpiration;
extern CCriticalSection cs_mapRelay;
extern limitedmap<CInv, int64_t> mapAlreadyAskedFor;

extern std::vector<std::string> vAddedNodes;
extern CCriticalSection cs_vAddedNodes;

extern NodeId nLastNodeId;
extern CCriticalSection cs_nLastNodeId;

struct LocalServiceInfo {
    int nScore;
    int nPort;
};

extern CCriticalSection cs_mapLocalHost;
extern std::map<CNetAddr, LocalServiceInfo> mapLocalHost;

class CNodeStats
{
public:
    NodeId nodeid;
    uint64_t nServices;
    int64_t nLastSend;
    int64_t nLastRecv;
    int64_t nTimeConnected;
    std::string addrName;
    int nVersion;
    std::string cleanSubVer;
    bool fInbound;
    int nStartingHeight;
    uint64_t nSendBytes;
    uint64_t nRecvBytes;
    bool fWhitelisted;
    double dPingTime;
    double dPingWait;
    std::string addrLocal;
};


class CNetMessage
{
public:
    bool in_data; 

    CDataStream hdrbuf; 
    CMessageHeader hdr; 
    unsigned int nHdrPos;

    CDataStream vRecv; 
    unsigned int nDataPos;

    int64_t nTime; 

    CNetMessage(int nTypeIn, int nVersionIn) : hdrbuf(nTypeIn, nVersionIn), vRecv(nTypeIn, nVersionIn)
    {
        hdrbuf.resize(24);
        in_data = false;
        nHdrPos = 0;
        nDataPos = 0;
        nTime = 0;
    }

    bool complete() const
    {
        if (!in_data)
            return false;
        return (hdr.nMessageSize == nDataPos);
    }

    void SetVersion(int nVersionIn)
    {
        hdrbuf.SetVersion(nVersionIn);
        vRecv.SetVersion(nVersionIn);
    }

    int readHeader(const char* pch, unsigned int nBytes);
    int readData(const char* pch, unsigned int nBytes);
};


typedef enum BanReason
{
    BanReasonUnknown          = 0,
    BanReasonNodeMisbehaving  = 1,
    BanReasonManuallyAdded    = 2
} BanReason;

class CBanEntry
{
public:
    static const int CURRENT_VERSION=1;
    int nVersion;
    int64_t nCreateTime;
    int64_t nBanUntil;
    uint8_t banReason;

    CBanEntry()
    {
        SetNull();
    }

    CBanEntry(int64_t nCreateTimeIn)
    {
        SetNull();
        nCreateTime = nCreateTimeIn;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(nCreateTime);
        READWRITE(nBanUntil);
        READWRITE(banReason);
    }

    void SetNull()
    {
        nVersion = CBanEntry::CURRENT_VERSION;
        nCreateTime = 0;
        nBanUntil = 0;
        banReason = BanReasonUnknown;
    }

    std::string banReasonToString()
    {
        switch (banReason) {
        case BanReasonNodeMisbehaving:
            return "node misbehabing";
        case BanReasonManuallyAdded:
            return "manually added";
        default:
            return "unknown";
        }
    }
};

typedef std::map<CSubNet, CBanEntry> banmap_t;



class CNode
{
public:
    
    uint64_t nServices;
    SOCKET hSocket;
    CDataStream ssSend;
    size_t nSendSize;   
    size_t nSendOffset; 
    uint64_t nSendBytes;
    std::deque<CSerializeData> vSendMsg;
    CCriticalSection cs_vSend;

    std::deque<CInv> vRecvGetData;
    std::deque<CNetMessage> vRecvMsg;
    CCriticalSection cs_vRecvMsg;
    uint64_t nRecvBytes;
    int nRecvVersion;

    int64_t nLastSend;
    int64_t nLastRecv;
    int64_t nTimeConnected;
    CAddress addr;
    std::string addrName;
    CService addrLocal;
    int nVersion;
    
    
    
    
    std::string strSubVer, cleanSubVer;
    bool fWhitelisted; 
    bool fOneShot;
    bool fClient;
    bool fInbound;
    bool fNetworkNode;
    bool fSuccessfullyConnected;
    bool fDisconnect;
    
    
    
    
    bool fRelayTxes;
    
    
    
    
    
    bool fObfuScationMaster;
    CSemaphoreGrant grantOutbound;
    CCriticalSection cs_filter;
    CBloomFilter* pfilter;
    int nRefCount;
    NodeId id;

protected:
    
    
    static banmap_t setBanned;
    static CCriticalSection cs_setBanned;
    static bool setBannedIsDirty;

    std::vector<std::string> vecRequestsFulfilled; 

    
    
    static std::vector<CSubNet> vWhitelistedRange;
    static CCriticalSection cs_vWhitelistedRange;

    
    void Fuzz(int nChance); 

public:
    uint256 hashContinue;
    int nStartingHeight;

    
    std::vector<CAddress> vAddrToSend;
    mruset<CAddress> setAddrKnown;
    bool fGetAddr;
    std::set<uint256> setKnown;

    
    mruset<CInv> setInventoryKnown;
    std::vector<CInv> vInventoryToSend;
    CCriticalSection cs_inventory;
    std::multimap<int64_t, CInv> mapAskFor;
    std::vector<uint256> vBlockRequested;

    
    
    uint64_t nPingNonceSent;
    
    int64_t nPingUsecStart;
    
    int64_t nPingUsecTime;
    
    bool fPingQueued;

    CNode(SOCKET hSocketIn, CAddress addrIn, std::string addrNameIn = "", bool fInboundIn = false);
    ~CNode();

private:
    
    static CCriticalSection cs_totalBytesRecv;
    static CCriticalSection cs_totalBytesSent;
    static uint64_t nTotalBytesRecv;
    static uint64_t nTotalBytesSent;

    CNode(const CNode&);
    void operator=(const CNode&);

public:
    NodeId GetId() const
    {
        return id;
    }

    int GetRefCount()
    {
        assert(nRefCount >= 0);
        return nRefCount;
    }

    
    unsigned int GetTotalRecvSize()
    {
        unsigned int total = 0;
        BOOST_FOREACH (const CNetMessage& msg, vRecvMsg)
            total += msg.vRecv.size() + 24;
        return total;
    }

    
    bool ReceiveMsgBytes(const char* pch, unsigned int nBytes);

    
    void SetRecvVersion(int nVersionIn)
    {
        nRecvVersion = nVersionIn;
        BOOST_FOREACH (CNetMessage& msg, vRecvMsg)
            msg.SetVersion(nVersionIn);
    }

    CNode* AddRef()
    {
        nRefCount++;
        return this;
    }

    void Release()
    {
        nRefCount--;
    }


    void AddAddressKnown(const CAddress& addr)
    {
        setAddrKnown.insert(addr);
    }

    void PushAddress(const CAddress& addr)
    {
        
        
        
        if (addr.IsValid() && !setAddrKnown.count(addr)) {
            if (vAddrToSend.size() >= MAX_ADDR_TO_SEND) {
                vAddrToSend[insecure_rand() % vAddrToSend.size()] = addr;
            } else {
                vAddrToSend.push_back(addr);
            }
        }
    }


    void AddInventoryKnown(const CInv& inv)
    {
        {
            LOCK(cs_inventory);
            setInventoryKnown.insert(inv);
        }
    }

    void PushInventory(const CInv& inv)
    {
        {
            LOCK(cs_inventory);
            if (!setInventoryKnown.count(inv))
                vInventoryToSend.push_back(inv);
        }
    }

    void AskFor(const CInv& inv);

    
    void BeginMessage(const char* pszCommand) EXCLUSIVE_LOCK_FUNCTION(cs_vSend);

    
    void AbortMessage() UNLOCK_FUNCTION(cs_vSend);

    
    void EndMessage() UNLOCK_FUNCTION(cs_vSend);

    void PushVersion();


    void PushMessage(const char* pszCommand)
    {
        try {
            BeginMessage(pszCommand);
            EndMessage();
        } catch (...) {
            AbortMessage();
            throw;
        }
    }

    template <typename T1>
    void PushMessage(const char* pszCommand, const T1& a1)
    {
        try {
            BeginMessage(pszCommand);
            ssSend << a1;
            EndMessage();
        } catch (...) {
            AbortMessage();
            throw;
        }
    }

    template <typename T1, typename T2>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2)
    {
        try {
            BeginMessage(pszCommand);
            ssSend << a1 << a2;
            EndMessage();
        } catch (...) {
            AbortMessage();
            throw;
        }
    }

    template <typename T1, typename T2, typename T3>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3)
    {
        try {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3;
            EndMessage();
        } catch (...) {
            AbortMessage();
            throw;
        }
    }

    template <typename T1, typename T2, typename T3, typename T4>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4)
    {
        try {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4;
            EndMessage();
        } catch (...) {
            AbortMessage();
            throw;
        }
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5)
    {
        try {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5;
            EndMessage();
        } catch (...) {
            AbortMessage();
            throw;
        }
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6)
    {
        try {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6;
            EndMessage();
        } catch (...) {
            AbortMessage();
            throw;
        }
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7)
    {
        try {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7;
            EndMessage();
        } catch (...) {
            AbortMessage();
            throw;
        }
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8)
    {
        try {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8;
            EndMessage();
        } catch (...) {
            AbortMessage();
            throw;
        }
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8, const T9& a9)
    {
        try {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9;
            EndMessage();
        } catch (...) {
            AbortMessage();
            throw;
        }
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8, const T9& a9, const T10& a10)
    {
        try {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10;
            EndMessage();
        } catch (...) {
            AbortMessage();
            throw;
        }
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8, const T9& a9, const T10& a10, const T11& a11)
    {
        try {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10 << a11;
            EndMessage();
        } catch (...) {
            AbortMessage();
            throw;
        }
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
    void PushMessage(const char* pszCommand, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7, const T8& a8, const T9& a9, const T10& a10, const T11& a11, const T12& a12)
    {
        try {
            BeginMessage(pszCommand);
            ssSend << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10 << a11 << a12;
            EndMessage();
        } catch (...) {
            AbortMessage();
            throw;
        }
    }

    bool HasFulfilledRequest(std::string strRequest)
    {
        BOOST_FOREACH (std::string& type, vecRequestsFulfilled) {
            if (type == strRequest) return true;
        }
        return false;
    }

    void ClearFulfilledRequest(std::string strRequest)
    {
        std::vector<std::string>::iterator it = vecRequestsFulfilled.begin();
        while (it != vecRequestsFulfilled.end()) {
            if ((*it) == strRequest) {
                vecRequestsFulfilled.erase(it);
                return;
            }
            ++it;
        }
    }

    void FulfilledRequest(std::string strRequest)
    {
        if (HasFulfilledRequest(strRequest)) return;
        vecRequestsFulfilled.push_back(strRequest);
    }

    bool IsSubscribed(unsigned int nChannel);
    void Subscribe(unsigned int nChannel, unsigned int nHops = 0);
    void CancelSubscribe(unsigned int nChannel);
    void CloseSocketDisconnect();
    bool DisconnectOldProtocol(int nVersionRequired, std::string strLastCommand = "");

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    static void ClearBanned(); 
    static bool IsBanned(CNetAddr ip);
    static bool IsBanned(CSubNet subnet);
    static void Ban(const CNetAddr &ip, const BanReason &banReason, int64_t bantimeoffset = 0, bool sinceUnixEpoch = false);
    static void Ban(const CSubNet &subNet, const BanReason &banReason, int64_t bantimeoffset = 0, bool sinceUnixEpoch = false);
    static bool Unban(const CNetAddr &ip);
    static bool Unban(const CSubNet &ip);
    static void GetBanned(banmap_t &banmap);
    static void SetBanned(const banmap_t &banmap);

    
    static bool BannedSetIsDirty();
    
    static void SetBannedSetDirty(bool dirty=true);
    
    static void SweepBanned();

    void copyStats(CNodeStats& stats);

    static bool IsWhitelistedRange(const CNetAddr& ip);
    static void AddWhitelistedRange(const CSubNet& subnet);

    
    static void RecordBytesRecv(uint64_t bytes);
    static void RecordBytesSent(uint64_t bytes);

    static uint64_t GetTotalBytesRecv();
    static uint64_t GetTotalBytesSent();
};

class CExplicitNetCleanup
{
public:
    static void callCleanup();
};

class CTransaction;
void RelayTransaction(const CTransaction& tx);
void RelayTransaction(const CTransaction& tx, const CDataStream& ss);
void RelayTransactionLockReq(const CTransaction& tx, bool relayToAll = false);
void RelayInv(CInv& inv);


class CAddrDB
{
private:
    boost::filesystem::path pathAddr;

public:
    CAddrDB();
    bool Write(const CAddrMan& addr);
    bool Read(CAddrMan& addr);
};


class CBanDB
{
private:
    boost::filesystem::path pathBanlist;
public:
    CBanDB();
    bool Write(const banmap_t& banSet);
    bool Read(banmap_t& banSet);
};

void DumpBanlist();

#endif 
