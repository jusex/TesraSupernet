



#ifndef BITCOIN_NETBASE_H
#define BITCOIN_NETBASE_H

#if defined(HAVE_CONFIG_H)
#include "config/tesra-config.h"
#endif

#include "compat.h"
#include "serialize.h"

#include <stdint.h>
#include <string>
#include <vector>

extern int nConnectTimeout;
extern bool fNameLookup;


static const int DEFAULT_CONNECT_TIMEOUT = 5000;

static const int DEFAULT_NAME_LOOKUP = true;

#ifdef WIN32

#undef SetPort
#endif

enum Network {
    NET_UNROUTABLE = 0,
    NET_IPV4,
    NET_IPV6,
    NET_TOR,

    NET_MAX,
};


class CNetAddr
{
protected:
    unsigned char ip[16]; 

public:
    CNetAddr();
    CNetAddr(const struct in_addr& ipv4Addr);
    explicit CNetAddr(const char* pszIp, bool fAllowLookup = false);
    explicit CNetAddr(const std::string& strIp, bool fAllowLookup = false);
    void Init();
    void SetIP(const CNetAddr& ip);

    /**
         * Set raw IPv4 or IPv6 address (in network byte order)
         * @note Only NET_IPV4 and NET_IPV6 are allowed for network.
         */
    void SetRaw(Network network, const uint8_t* data);

    bool SetSpecial(const std::string& strName); 
    bool IsIPv4() const;                         
    bool IsIPv6() const;                         
    bool IsRFC1918() const;                      
    bool IsRFC2544() const;                      
    bool IsRFC6598() const;                      
    bool IsRFC5737() const;                      
    bool IsRFC3849() const;                      
    bool IsRFC3927() const;                      
    bool IsRFC3964() const;                      
    bool IsRFC4193() const;                      
    bool IsRFC4380() const;                      
    bool IsRFC4843() const;                      
    bool IsRFC4862() const;                      
    bool IsRFC6052() const;                      
    bool IsRFC6145() const;                      
    bool IsTor() const;
    bool IsLocal() const;
    bool IsRoutable() const;
    bool IsValid() const;
    bool IsMulticast() const;
    enum Network GetNetwork() const;
    std::string ToString() const;
    std::string ToStringIP() const;
    unsigned int GetByte(int n) const;
    uint64_t GetHash() const;
    bool GetInAddr(struct in_addr* pipv4Addr) const;
    std::vector<unsigned char> GetGroup() const;
    int GetReachabilityFrom(const CNetAddr* paddrPartner = NULL) const;

    CNetAddr(const struct in6_addr& pipv6Addr);
    bool GetIn6Addr(struct in6_addr* pipv6Addr) const;

    friend bool operator==(const CNetAddr& a, const CNetAddr& b);
    friend bool operator!=(const CNetAddr& a, const CNetAddr& b);
    friend bool operator<(const CNetAddr& a, const CNetAddr& b);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(FLATDATA(ip));
    }

    friend class CSubNet;
};

class CSubNet
{
protected:
    
    CNetAddr network;
    
    uint8_t netmask[16];
    
    bool valid;

public:
    CSubNet();
    explicit CSubNet(const std::string& strSubnet, bool fAllowLookup = false);

    
    explicit CSubNet(const CNetAddr &addr);

    bool Match(const CNetAddr& addr) const;

    std::string ToString() const;
    bool IsValid() const;

    friend bool operator==(const CSubNet& a, const CSubNet& b);
    friend bool operator!=(const CSubNet& a, const CSubNet& b);
    friend bool operator<(const CSubNet& a, const CSubNet& b);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(network);
        READWRITE(FLATDATA(netmask));
        READWRITE(FLATDATA(valid));
    }
};


class CService : public CNetAddr
{
protected:
    unsigned short port; 

public:
    CService();
    CService(const CNetAddr& ip, unsigned short port);
    CService(const struct in_addr& ipv4Addr, unsigned short port);
    CService(const struct sockaddr_in& addr);
    explicit CService(const char* pszIpPort, int portDefault, bool fAllowLookup = false);
    explicit CService(const char* pszIpPort, bool fAllowLookup = false);
    explicit CService(const std::string& strIpPort, int portDefault, bool fAllowLookup = false);
    explicit CService(const std::string& strIpPort, bool fAllowLookup = false);
    void Init();
    void SetPort(unsigned short portIn);
    unsigned short GetPort() const;
    bool GetSockAddr(struct sockaddr* paddr, socklen_t* addrlen) const;
    bool SetSockAddr(const struct sockaddr* paddr);
    friend bool operator==(const CService& a, const CService& b);
    friend bool operator!=(const CService& a, const CService& b);
    friend bool operator<(const CService& a, const CService& b);
    std::vector<unsigned char> GetKey() const;
    std::string ToString() const;
    std::string ToStringPort() const;
    std::string ToStringIPPort() const;

    CService(const struct in6_addr& ipv6Addr, unsigned short port);
    CService(const struct sockaddr_in6& addr);

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(FLATDATA(ip));
        unsigned short portN = htons(port);
        READWRITE(portN);
        if (ser_action.ForRead())
            port = ntohs(portN);
    }
};

class proxyType
{
public:
    proxyType(): randomize_credentials(false) {}
    proxyType(const CService &proxy, bool randomize_credentials=false): proxy(proxy), randomize_credentials(randomize_credentials) {}

    bool IsValid() const { return proxy.IsValid(); }

    CService proxy;
    bool randomize_credentials;
};

enum Network ParseNetwork(std::string net);
std::string GetNetworkName(enum Network net);
void SplitHostPort(std::string in, int& portOut, std::string& hostOut);
bool SetProxy(enum Network net, const proxyType &addrProxy);
bool GetProxy(enum Network net, proxyType& proxyInfoOut);
bool IsProxy(const CNetAddr& addr);
bool SetNameProxy(const proxyType &addrProxy);
bool HaveNameProxy();
bool LookupHost(const char* pszName, std::vector<CNetAddr>& vIP, unsigned int nMaxSolutions = 0, bool fAllowLookup = true);
bool Lookup(const char* pszName, CService& addr, int portDefault = 0, bool fAllowLookup = true);
bool Lookup(const char* pszName, std::vector<CService>& vAddr, int portDefault = 0, bool fAllowLookup = true, unsigned int nMaxSolutions = 0);
bool LookupNumeric(const char* pszName, CService& addr, int portDefault = 0);
bool ConnectSocket(const CService& addr, SOCKET& hSocketRet, int nTimeout, bool* outProxyConnectionFailed = 0);
bool ConnectSocketByName(CService& addr, SOCKET& hSocketRet, const char* pszDest, int portDefault, int nTimeout, bool* outProxyConnectionFailed = 0);

std::string NetworkErrorString(int err);

bool CloseSocket(SOCKET& hSocket);

bool SetSocketNonBlocking(SOCKET& hSocket, bool fNonBlocking);
/**
 * Convert milliseconds to a struct timeval for e.g. select.
 */
struct timeval MillisToTimeval(int64_t nTimeout);

#endif 
