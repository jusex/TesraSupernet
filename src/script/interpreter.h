




#ifndef BITCOIN_SCRIPT_INTERPRETER_H
#define BITCOIN_SCRIPT_INTERPRETER_H

#include "script_error.h"
#include "primitives/transaction.h"

#include <vector>
#include <stdint.h>
#include <string>

class CPubKey;
class CScript;
class CTransaction;
class uint256;


enum
{
    SIGHASH_ALL = 1,
    SIGHASH_NONE = 2,
    SIGHASH_SINGLE = 3,
    SIGHASH_ANYONECANPAY = 0x80,
};


enum
{
    SCRIPT_VERIFY_NONE      = 0,

    
    SCRIPT_VERIFY_P2SH      = (1U << 0),

    
    
    
    SCRIPT_VERIFY_STRICTENC = (1U << 1),

    
    SCRIPT_VERIFY_DERSIG    = (1U << 2),

    
    
    SCRIPT_VERIFY_LOW_S     = (1U << 3),

    
    SCRIPT_VERIFY_NULLDUMMY = (1U << 4),

    
    SCRIPT_VERIFY_SIGPUSHONLY = (1U << 5),

    
    
    
    
    
    SCRIPT_VERIFY_MINIMALDATA = (1U << 6),

    
    
    
    
    
    
    
    
    SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS  = (1U << 7),

    
    
    SCRIPT_EXEC_BYTE_CODE = (1U << 30),
};

uint256 SignatureHash(const CScript &scriptCode, const CTransaction& txTo, unsigned int nIn, int nHashType);

class BaseSignatureChecker
{
public:
    virtual bool CheckSig(const std::vector<unsigned char>& scriptSig, const std::vector<unsigned char>& vchPubKey, const CScript& scriptCode) const
    {
        return false;
    }

    virtual ~BaseSignatureChecker() {}
};

class TransactionSignatureChecker : public BaseSignatureChecker
{
private:
    const CTransaction* txTo;
    unsigned int nIn;

protected:
    virtual bool VerifySignature(const std::vector<unsigned char>& vchSig, const CPubKey& vchPubKey, const uint256& sighash) const;

public:
    TransactionSignatureChecker(const CTransaction* txToIn, unsigned int nInIn) : txTo(txToIn), nIn(nInIn) {}
    bool CheckSig(const std::vector<unsigned char>& scriptSig, const std::vector<unsigned char>& vchPubKey, const CScript& scriptCode) const;
};

class MutableTransactionSignatureChecker : public TransactionSignatureChecker
{
private:
    const CTransaction txTo;

public:
    MutableTransactionSignatureChecker(const CMutableTransaction* txToIn, unsigned int nInIn) : TransactionSignatureChecker(&txTo, nInIn), txTo(*txToIn) {}
};

bool EvalScript(std::vector<std::vector<unsigned char> >& stack, const CScript& script, unsigned int flags, const BaseSignatureChecker& checker, ScriptError* error = NULL);
bool VerifyScript(const CScript& scriptSig, const CScript& scriptPubKey, unsigned int flags, const BaseSignatureChecker& checker, ScriptError* error = NULL);

#endif 
