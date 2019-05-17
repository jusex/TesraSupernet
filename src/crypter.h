



#ifndef BITCOIN_CRYPTER_H
#define BITCOIN_CRYPTER_H

#include "allocators.h"
#include "keystore.h"
#include "serialize.h"

class uint256;

const unsigned int WALLET_CRYPTO_KEY_SIZE = 32;
const unsigned int WALLET_CRYPTO_SALT_SIZE = 8;

/**
 * Private key encryption is done based on a CMasterKey,
 * which holds a salt and random encryption key.
 * 
 * CMasterKeys are encrypted using AES-256-CBC using a key
 * derived using derivation method nDerivationMethod
 * (0 == EVP_sha512()) and derivation iterations nDeriveIterations.
 * vchOtherDerivationParameters is provided for alternative algorithms
 * which may require more parameters (such as scrypt).
 * 
 * Wallet Private Keys are then encrypted using AES-256-CBC
 * with the double-sha256 of the public key as the IV, and the
 * master key's key as the encryption key (see keystore.[ch]).
 */


class CMasterKey
{
public:
    std::vector<unsigned char> vchCryptedKey;
    std::vector<unsigned char> vchSalt;
    
    
    unsigned int nDerivationMethod;
    unsigned int nDeriveIterations;
    
    
    std::vector<unsigned char> vchOtherDerivationParameters;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion)
    {
        READWRITE(vchCryptedKey);
        READWRITE(vchSalt);
        READWRITE(nDerivationMethod);
        READWRITE(nDeriveIterations);
        READWRITE(vchOtherDerivationParameters);
    }

    CMasterKey()
    {
        
        
        nDeriveIterations = 25000;
        nDerivationMethod = 0;
        vchOtherDerivationParameters = std::vector<unsigned char>(0);
    }
};

typedef std::vector<unsigned char, secure_allocator<unsigned char> > CKeyingMaterial;


class CCrypter
{
private:
    unsigned char chKey[WALLET_CRYPTO_KEY_SIZE];
    unsigned char chIV[WALLET_CRYPTO_KEY_SIZE];
    bool fKeySet;

public:
    bool SetKeyFromPassphrase(const SecureString& strKeyData, const std::vector<unsigned char>& chSalt, const unsigned int nRounds, const unsigned int nDerivationMethod);
    bool Encrypt(const CKeyingMaterial& vchPlaintext, std::vector<unsigned char>& vchCiphertext);
    bool Decrypt(const std::vector<unsigned char>& vchCiphertext, CKeyingMaterial& vchPlaintext);
    bool SetKey(const CKeyingMaterial& chNewKey, const std::vector<unsigned char>& chNewIV);

    void CleanKey()
    {
        OPENSSL_cleanse(chKey, sizeof(chKey));
        OPENSSL_cleanse(chIV, sizeof(chIV));
        fKeySet = false;
    }

    CCrypter()
    {
        fKeySet = false;

        
        
        
        LockedPageManager::Instance().LockRange(&chKey[0], sizeof chKey);
        LockedPageManager::Instance().LockRange(&chIV[0], sizeof chIV);
    }

    ~CCrypter()
    {
        CleanKey();

        LockedPageManager::Instance().UnlockRange(&chKey[0], sizeof chKey);
        LockedPageManager::Instance().UnlockRange(&chIV[0], sizeof chIV);
    }
};

bool EncryptSecret(const CKeyingMaterial& vMasterKey, const CKeyingMaterial& vchPlaintext, const uint256& nIV, std::vector<unsigned char>& vchCiphertext);
bool DecryptSecret(const CKeyingMaterial& vMasterKey, const std::vector<unsigned char>& vchCiphertext, const uint256& nIV, CKeyingMaterial& vchPlaintext);

bool EncryptAES256(const SecureString& sKey, const SecureString& sPlaintext, const std::string& sIV, std::string& sCiphertext);
bool DecryptAES256(const SecureString& sKey, const std::string& sCiphertext, const std::string& sIV, SecureString& sPlaintext);


/** Keystore which keeps the private keys encrypted.
 * It derives from the basic key store, which is used if no encryption is active.
 */
class CCryptoKeyStore : public CBasicKeyStore
{
private:
    CryptedKeyMap mapCryptedKeys;

    CKeyingMaterial vMasterKey;

    
    
    bool fUseCrypto;

    
    bool fDecryptionThoroughlyChecked;

protected:
    bool SetCrypted();

    
    bool EncryptKeys(CKeyingMaterial& vMasterKeyIn);

    bool Unlock(const CKeyingMaterial& vMasterKeyIn);

public:
    CCryptoKeyStore() : fUseCrypto(false), fDecryptionThoroughlyChecked(false)
    {
    }

    bool IsCrypted() const
    {
        return fUseCrypto;
    }

    bool IsLocked() const
    {
        if (!IsCrypted())
            return false;
        bool result;
        {
            LOCK(cs_KeyStore);
            result = vMasterKey.empty();
        }
        return result;
    }

    bool Lock();

    virtual bool AddCryptedKey(const CPubKey& vchPubKey, const std::vector<unsigned char>& vchCryptedSecret);
    bool AddKeyPubKey(const CKey& key, const CPubKey& pubkey);
    bool HaveKey(const CKeyID& address) const
    {
        {
            LOCK(cs_KeyStore);
            if (!IsCrypted())
                return CBasicKeyStore::HaveKey(address);
            return mapCryptedKeys.count(address) > 0;
        }
        return false;
    }
    bool GetKey(const CKeyID& address, CKey& keyOut) const;
    bool GetPubKey(const CKeyID& address, CPubKey& vchPubKeyOut) const;
    void GetKeys(std::set<CKeyID>& setAddress) const
    {
        if (!IsCrypted()) {
            CBasicKeyStore::GetKeys(setAddress);
            return;
        }
        setAddress.clear();
        CryptedKeyMap::const_iterator mi = mapCryptedKeys.begin();
        while (mi != mapCryptedKeys.end()) {
            setAddress.insert((*mi).first);
            mi++;
        }
    }

    /**
     * Wallet status (encrypted, locked) changed.
     * Note: Called without locks held.
     */
    boost::signals2::signal<void(CCryptoKeyStore* wallet)> NotifyStatusChanged;
};

#endif 
