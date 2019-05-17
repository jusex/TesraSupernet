




#ifndef BITCOIN_WALLET_ISMINE_H
#define BITCOIN_WALLET_ISMINE_H

#include "key.h"
#include "script/standard.h"

class CKeyStore;
class CScript;


enum isminetype {
    ISMINE_NO = 0,
    
    ISMINE_WATCH_ONLY = 1,
    
    ISMINE_MULTISIG = 2,
    ISMINE_SPENDABLE  = 4,
    ISMINE_ALL = ISMINE_WATCH_ONLY | ISMINE_SPENDABLE
};

typedef uint8_t isminefilter;

isminetype IsMine(const CKeyStore& keystore, const CScript& scriptPubKey);
isminetype IsMine(const CKeyStore& keystore, const CTxDestination& dest);
void IsMine_Each(const CKeyStore& keystore, const CScript& scriptPubKey);

#endif 
