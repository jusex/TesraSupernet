




#include "script/standard.h"

#include "pubkey.h"
#include "script/script.h"
#include "util.h"
#include "utilstrencodings.h"

#include <boost/foreach.hpp>
#include "contract_api/contractcomponent.h"
#include "contract_api/contractconfig.h"

using namespace std;

typedef vector<unsigned char> valtype;

unsigned nMaxDatacarrierBytes = MAX_OP_RETURN_RELAY;

CScriptID::CScriptID(const CScript& in) : uint160(Hash160(in.begin(), in.end())) {}

const char* GetTxnOutputType(txnouttype t)
{
    switch (t)
    {
    case TX_NONSTANDARD: return "nonstandard";
    case TX_PUBKEY: return "pubkey";
    case TX_PUBKEYHASH: return "pubkeyhash";
    case TX_SCRIPTHASH: return "scripthash";
    case TX_MULTISIG: return "multisig";
    case TX_NULL_DATA: return "nulldata";
    case TX_ZEROCOINMINT: return "zerocoinmint";
    
    case TX_CREATE: return "create";
    case TX_CALL: return "call";
    case TX_VM_STATE: return "vm_state";
    case TX_EXT_DATA: return "ext_data";
    }
    return NULL;
}

/**
 * Return public keys or hashes from scriptPubKey, for 'standard' transaction types.
 */
bool Solver(const CScript& scriptPubKey, txnouttype& typeRet, vector<vector<unsigned char> >& vSolutionsRet,bool contractConsensus)
{

    
    
    
    

    
    static multimap<txnouttype, CScript> mTemplates;
    if (mTemplates.empty())
    {
        
        mTemplates.insert(make_pair(TX_PUBKEY, CScript() << OP_PUBKEY << OP_CHECKSIG));

        
        mTemplates.insert(make_pair(TX_PUBKEYHASH, CScript() << OP_DUP << OP_HASH160 << OP_PUBKEYHASH << OP_EQUALVERIFY << OP_CHECKSIG));

        
        mTemplates.insert(make_pair(TX_MULTISIG, CScript() << OP_SMALLINTEGER << OP_PUBKEYS << OP_SMALLINTEGER << OP_CHECKMULTISIG));


        
        mTemplates.insert(make_pair(TX_CREATE, CScript() << OP_VERSION << OP_GAS_LIMIT << OP_GAS_PRICE
                                    << OP_DATA << OP_CREATE));

        
        mTemplates.insert(make_pair(TX_CALL, CScript() << OP_VERSION << OP_GAS_LIMIT << OP_GAS_PRICE
                                    << OP_DATA << OP_PUBKEYHASH << OP_CALL));

        
        mTemplates.insert(make_pair(TX_VM_STATE, CScript() << OP_HASH_STATE_ROOT << OP_HASH_UTXO_ROOT
                                    << OP_VM_STATE));
    }

    vSolutionsRet.clear();
    
    
    if (scriptPubKey.IsPayToScriptHash())
    {
        typeRet = TX_SCRIPTHASH;
        vector<unsigned char> hashBytes(scriptPubKey.begin()+2, scriptPubKey.begin()+22);
        vSolutionsRet.push_back(hashBytes);
        return true;
    }

    
    if (scriptPubKey.IsZerocoinMint()){
        typeRet = TX_ZEROCOINMINT;
        if(scriptPubKey.size() > 150) return false;
        vector<unsigned char> hashBytes(scriptPubKey.begin()+2, scriptPubKey.end());
        vSolutionsRet.push_back(hashBytes);
        return true;
    }

    if (scriptPubKey.HasOpExtData() && scriptPubKey.size() > 5) {
        typeRet = TX_EXT_DATA;

        uint8_t     u8Offset;
        uint32_t    u32VecSize;
        uint32_t    u32DataSize;

        u8Offset = 1;
        if (scriptPubKey[0] < OP_PUSHDATA1) {
            u32VecSize = scriptPubKey[0];
            u32DataSize = (scriptPubKey[1] << 8) | scriptPubKey[2];
        } else if (scriptPubKey[0] == OP_PUSHDATA1) {
            u8Offset = 2;
            u32VecSize = scriptPubKey[1];
            u32DataSize = (scriptPubKey[2] << 8) | scriptPubKey[3];
        } else if (scriptPubKey[0] == OP_PUSHDATA2) {            
            u8Offset = 3;
            u32VecSize = *(unsigned short *)(scriptPubKey.data() + 1);
            u32DataSize = (scriptPubKey[3] << 8) | scriptPubKey[4];
        } else {
            return false;
        }

        if (u32VecSize == u32DataSize && u32VecSize < scriptPubKey.size()) {
            std::vector<unsigned char> vExtData;

            vExtData.resize(u32VecSize);
            memcpy(vExtData.data(), scriptPubKey.data() + u8Offset, u32VecSize);
            vSolutionsRet.push_back(vExtData);
        }

        return (u32VecSize == u32DataSize && u32VecSize < scriptPubKey.size());
    }

    
    
    
    
    
    if (scriptPubKey.size() >= 1 && scriptPubKey[0] == OP_RETURN && scriptPubKey.IsPushOnly(scriptPubKey.begin()+1)) {
        typeRet = TX_NULL_DATA;
        return true;
    }

    
    const CScript& script1 = scriptPubKey;
    BOOST_FOREACH(const PAIRTYPE(txnouttype, CScript)& tplate, mTemplates)
    {
        const CScript& script2 = tplate.second;
        vSolutionsRet.clear();

        opcodetype opcode1, opcode2;
        vector<unsigned char> vch1, vch2;

        VersionVM version;      
        version.rootVM = 20;    

        
        CScript::const_iterator pc1 = script1.begin();
        CScript::const_iterator pc2 = script2.begin();
        while (true)
        {
            if (pc1 == script1.end() && pc2 == script2.end())
            {
                
                typeRet = tplate.first;
                if (typeRet == TX_MULTISIG)
                {
                    
                    unsigned char m = vSolutionsRet.front()[0];
                    unsigned char n = vSolutionsRet.back()[0];
                    if (m < 1 || n < 1 || m > n || vSolutionsRet.size()-2 != n)
                        return false;
                }
                return true;
            }
            if (!script1.GetOp(pc1, opcode1, vch1))
                break;
            if (!script2.GetOp(pc2, opcode2, vch2))
                break;

            
            if (opcode2 == OP_PUBKEYS)
            {
                while (vch1.size() >= 33 && vch1.size() <= 65)
                {
                    vSolutionsRet.push_back(vch1);
                    if (!script1.GetOp(pc1, opcode1, vch1))
                        break;
                }
                if (!script2.GetOp(pc2, opcode2, vch2))
                    break;
                
                
            }

            if (opcode2 == OP_PUBKEY)
            {
                if (vch1.size() < 33 || vch1.size() > 65)
                    break;
                vSolutionsRet.push_back(vch1);
            }
            else if (opcode2 == OP_PUBKEYHASH)
            {
                if (vch1.size() != sizeof(uint160))
                    break;
                vSolutionsRet.push_back(vch1);
            }
            else if (opcode2 == OP_SMALLINTEGER)
            {   
                if (opcode1 == OP_0 ||
                    (opcode1 >= OP_1 && opcode1 <= OP_16))
                {
                    char n = (char)CScript::DecodeOP_N(opcode1);
                    vSolutionsRet.push_back(valtype(1, n));
                }
                else
                    break;
            }
            
            else if (opcode2 == OP_VERSION)
            {
                if (0 <= opcode1 && opcode1 <= OP_PUSHDATA4)
                {
                    if (vch1.empty() || vch1.size() > 4 || (vch1.back() & 0x80))
                        return false;

                    version = VersionVM::fromRaw(CScriptNum::vch_to_uint64(vch1));
                    if (!(version.toRaw() == VersionVM::GetEVMDefault().toRaw() ||
                          version.toRaw() == VersionVM::GetNoExec().toRaw()))
                    {
                        
                        return false;
                    }
                }
            } else if (opcode2 == OP_GAS_LIMIT)
            {
                try
                {
                    uint64_t val = CScriptNum::vch_to_uint64(vch1);
                    if (contractConsensus)
                    {
                        
                        if (version.rootVM != 0 && val < 1)
                        {
                            return false;
                        }
                        if (val > MAX_BLOCK_GAS_LIMIT_DGP)
                        {
                            
                            return false;
                        }
                    } else
                    {
                        
                        
                        if (version.rootVM != 0 && val < STANDARD_MINIMUM_GAS_LIMIT)
                        {
                            return false;
                        }
                        if (val > DEFAULT_BLOCK_GAS_LIMIT_DGP / 2)
                        {
                            
                            return false;
                        }

                    }
                }
                catch (const scriptnum_error &err)
                {
                    return false;
                }
            } else if (opcode2 == OP_GAS_PRICE)
            {
                try
                {
                    uint64_t val = CScriptNum::vch_to_uint64(vch1);
                    if (contractConsensus)
                    {
                        
                        if (version.rootVM != 0 && val < 1)
                        {
                            return false;
                        }
                    } else
                    {
                        
                        if (version.rootVM != 0 && val < STANDARD_MINIMUM_GAS_PRICE)
                        {
                            return false;
                        }
                    }
                }
                catch (const scriptnum_error &err)
                {
                    return false;
                }
            } else if (opcode2 == OP_DATA)
            {
                if (0 <= opcode1 && opcode1 <= OP_PUSHDATA4)
                {
                    if (vch1.empty())
                        break;
                }
            } else if (opcode2 == OP_HASH_STATE_ROOT)
            {
                if (vch1.size() != sizeof(uint256))
                    break;
                vSolutionsRet.push_back(vch1);
            } else if (opcode2 == OP_HASH_UTXO_ROOT)
            {
                if (vch1.size() != sizeof(uint256))
                    break;
                vSolutionsRet.push_back(vch1);
            }
            

            else if (opcode1 != opcode2 || vch1 != vch2)
            {
                
                break;
            }
        }
    }

    vSolutionsRet.clear();
    typeRet = TX_NONSTANDARD;
    return false;
}

int ScriptSigArgsExpected(txnouttype t, const std::vector<std::vector<unsigned char> >& vSolutions)
{
    switch (t)
    {
    case TX_NONSTANDARD:
    case TX_NULL_DATA:
    case TX_ZEROCOINMINT:
        return -1;
    case TX_PUBKEY:
        return 1;
    case TX_PUBKEYHASH:
        return 2;
    case TX_MULTISIG:
        if (vSolutions.size() < 1 || vSolutions[0].size() < 1)
            return -1;
        return vSolutions[0][0] + 1;
    case TX_SCRIPTHASH:
        return 1; 
    }
    return -1;
}

bool IsStandard(const CScript& scriptPubKey, txnouttype& whichType)
{
    vector<valtype> vSolutions;
    if (!Solver(scriptPubKey, whichType, vSolutions))
        return false;

    if (whichType == TX_MULTISIG)
    {
        unsigned char m = vSolutions.front()[0];
        unsigned char n = vSolutions.back()[0];
        
        if (n < 1 || n > 3)
            return false;
        if (m < 1 || m > n)
            return false;
    } else if (whichType == TX_NULL_DATA &&
                (!GetBoolArg("-datacarrier", true) || scriptPubKey.size() > nMaxDatacarrierBytes))
        return false;

    return whichType != TX_NONSTANDARD;
}

bool ExtractDestination(const CScript& scriptPubKey, CTxDestination& addressRet, txnouttype *typeRet)    

{
    vector<valtype> vSolutions;
    txnouttype whichType;
    if (!Solver(scriptPubKey, whichType, vSolutions))
        return false;

    
    if (typeRet)
    {
        *typeRet = whichType;
    }
    

    if (whichType == TX_PUBKEY)
    {
        CPubKey pubKey(vSolutions[0]);
        if (!pubKey.IsValid())
            return false;

        addressRet = pubKey.GetID();
        return true;
    }
    else if (whichType == TX_PUBKEYHASH)
    {
        addressRet = CKeyID(uint160(vSolutions[0]));
        return true;
    }
    else if (whichType == TX_SCRIPTHASH)
    {
        addressRet = CScriptID(uint160(vSolutions[0]));
        return true;
    }
    
    return false;
}

bool ExtractDestinations(const CScript& scriptPubKey, txnouttype& typeRet, vector<CTxDestination>& addressRet, int& nRequiredRet)
{
    addressRet.clear();
    typeRet = TX_NONSTANDARD;
    vector<valtype> vSolutions;
    if (!Solver(scriptPubKey, typeRet, vSolutions))
        return false;
    if (typeRet == TX_NULL_DATA){
        
        return false;
    }

    if (typeRet == TX_MULTISIG)
    {
        nRequiredRet = vSolutions.front()[0];
        for (unsigned int i = 1; i < vSolutions.size()-1; i++)
        {
            CPubKey pubKey(vSolutions[i]);
            if (!pubKey.IsValid())
                continue;

            CTxDestination address = pubKey.GetID();
            addressRet.push_back(address);
        }

        if (addressRet.empty())
            return false;
    }
    else
    {
        nRequiredRet = 1;
        CTxDestination address;
        if (!ExtractDestination(scriptPubKey, address))
           return false;
        addressRet.push_back(address);
    }

    return true;
}

namespace
{
class CScriptVisitor : public boost::static_visitor<bool>
{
private:
    CScript *script;
public:
    CScriptVisitor(CScript *scriptin) { script = scriptin; }

    bool operator()(const CNoDestination &dest) const {
        script->clear();
        return false;
    }

    bool operator()(const CKeyID &keyID) const {
        script->clear();
        *script << OP_DUP << OP_HASH160 << ToByteVector(keyID) << OP_EQUALVERIFY << OP_CHECKSIG;
        return true;
    }

    bool operator()(const CScriptID &scriptID) const {
        script->clear();
        *script << OP_HASH160 << ToByteVector(scriptID) << OP_EQUAL;
        return true;
    }
};
}

CScript GetScriptForDestination(const CTxDestination& dest)
{
    CScript script;

    boost::apply_visitor(CScriptVisitor(&script), dest);
    return script;
}

CScript GetScriptForMultisig(int nRequired, const std::vector<CPubKey>& keys)
{
    CScript script;

    script << CScript::EncodeOP_N(nRequired);
    BOOST_FOREACH(const CPubKey& key, keys)
        script << ToByteVector(key);
    script << CScript::EncodeOP_N(keys.size()) << OP_CHECKMULTISIG;
    return script;
}


bool IsValidContractSenderAddress(const CTxDestination &dest)
{
    const CKeyID *keyID = boost::get<CKeyID>(&dest);
    return keyID != 0;
}

