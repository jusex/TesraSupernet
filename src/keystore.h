




#ifndef BITCOIN_KEYSTORE_H
#define BITCOIN_KEYSTORE_H

#include "key.h"
#include "pubkey.h"
#include "sync.h"
#include "script/standard.h"
#include <boost/signals2/signal.hpp>

class CScript;
class CScriptID;


class CKeyStore
{
protected:
    mutable CCriticalSection cs_KeyStore;

public:
    virtual ~CKeyStore() {}

    
    virtual bool AddKeyPubKey(const CKey& key, const CPubKey& pubkey) = 0;
    virtual bool AddKey(const CKey& key);

    
    virtual bool HaveKey(const CKeyID& address) const = 0;
    virtual bool GetKey(const CKeyID& address, CKey& keyOut) const = 0;
    virtual void GetKeys(std::set<CKeyID>& setAddress) const = 0;
    virtual bool GetPubKey(const CKeyID& address, CPubKey& vchPubKeyOut) const;

    
    virtual bool AddCScript(const CScript& redeemScript) = 0;
    virtual bool HaveCScript(const CScriptID& hash) const = 0;
    virtual bool GetCScript(const CScriptID& hash, CScript& redeemScriptOut) const = 0;

    
    virtual bool AddWatchOnly(const CScript& dest) = 0;
    virtual bool RemoveWatchOnly(const CScript& dest) = 0;
    virtual bool HaveWatchOnly(const CScript& dest) const = 0;
    virtual bool HaveWatchOnly() const = 0;

    
    virtual bool AddMultiSig(const CScript& dest) = 0;
    virtual bool RemoveMultiSig(const CScript& dest) = 0;
    virtual bool HaveMultiSig(const CScript& dest) const = 0;
    virtual bool HaveMultiSig() const = 0;
};

typedef std::map<CKeyID, CKey> KeyMap;
typedef std::map<CScriptID, CScript> ScriptMap;
typedef std::set<CScript> WatchOnlySet;
typedef std::set<CScript> MultiSigScriptSet;


class CBasicKeyStore : public CKeyStore
{
protected:
    KeyMap mapKeys;
    ScriptMap mapScripts;
    WatchOnlySet setWatchOnly;
    MultiSigScriptSet setMultiSig;

public:
    bool AddKeyPubKey(const CKey& key, const CPubKey& pubkey);
    bool HaveKey(const CKeyID& address) const;
    void GetKeys(std::set<CKeyID>& setAddress) const;
    bool GetKey(const CKeyID& address, CKey& keyOut) const;

    virtual bool AddCScript(const CScript& redeemScript);
    virtual bool HaveCScript(const CScriptID& hash) const;
    virtual bool GetCScript(const CScriptID& hash, CScript& redeemScriptOut) const;

    virtual bool AddWatchOnly(const CScript& dest);
    virtual bool RemoveWatchOnly(const CScript& dest);
    virtual bool HaveWatchOnly(const CScript& dest) const;
    virtual bool HaveWatchOnly() const;

    virtual bool AddMultiSig(const CScript& dest);
    virtual bool RemoveMultiSig(const CScript& dest);
    virtual bool HaveMultiSig(const CScript& dest) const;
    virtual bool HaveMultiSig() const;
};

typedef std::vector<unsigned char, secure_allocator<unsigned char> > CKeyingMaterial;
typedef std::map<CKeyID, std::pair<CPubKey, std::vector<unsigned char> > > CryptedKeyMap;
CKeyID GetKeyForDestination(const CKeyStore& store, const CTxDestination& dest);

#endif 
