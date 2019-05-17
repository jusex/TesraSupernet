




#ifndef ACTIVEMASTERNODE_H
#define ACTIVEMASTERNODE_H

#include "init.h"
#include "key.h"
#include "masternode.h"
#include "net.h"
#include "obfuscation.h"
#include "sync.h"
#include "wallet.h"

#define ACTIVE_MASTERNODE_INITIAL 0 
#define ACTIVE_MASTERNODE_SYNC_IN_PROCESS 1
#define ACTIVE_MASTERNODE_INPUT_TOO_NEW 2
#define ACTIVE_MASTERNODE_NOT_CAPABLE 3
#define ACTIVE_MASTERNODE_STARTED 4


class CActiveMasternode
{
private:
    
    mutable CCriticalSection cs;

    
    bool SendMasternodePing(std::string& errorMessage);

    
    bool Register(CTxIn vin, CService service, CKey key, CPubKey pubKey, CKey keyMasternode, CPubKey pubKeyMasternode, std::string& errorMessage);

    
    bool GetMasterNodeVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey, std::string strTxHash, std::string strOutputIndex);
    bool GetVinFromOutput(COutput out, CTxIn& vin, CPubKey& pubkey, CKey& secretKey);

public:
    
    
    CPubKey pubKeyMasternode;

    
    CTxIn vin;
    CService service;

    int status;
    std::string notCapableReason;

    CActiveMasternode()
    {
        status = ACTIVE_MASTERNODE_INITIAL;
    }

    
    void ManageStatus();
    std::string GetStatus();

    
    bool Register(std::string strService, std::string strKey, std::string strTxHash, std::string strOutputIndex, std::string& errorMessage);

    
    bool GetMasterNodeVin(CTxIn& vin, CPubKey& pubkey, CKey& secretKey);
    vector<COutput> SelectCoinsMasternode();

    
    bool EnableHotColdMasterNode(CTxIn& vin, CService& addr);
};

#endif
