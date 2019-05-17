



#include "bip38.h"
#include "base58.h"
#include "hash.h"
#include "pubkey.h"
#include "util.h"
#include "utilstrencodings.h"

#include <openssl/aes.h>
#include <openssl/sha.h>
#include <secp256k1.h>
#include <string>


/** 39 bytes - 78 characters
 * 1) Prefix - 2 bytes - 4 chars - strKey[0..3]
 * 2) Flagbyte - 1 byte - 2 chars - strKey[4..5]
 * 3) addresshash - 4 bytes - 8 chars - strKey[6..13]
 * 4) Owner Entropy - 8 bytes - 16 chars - strKey[14..29]
 * 5) Encrypted Part 1 - 8 bytes - 16 chars - strKey[30..45]
 * 6) Encrypted Part 2 - 16 bytes - 32 chars - strKey[46..77]
 */

void DecryptAES(uint256 encryptedIn, uint256 decryptionKey, uint256& output)
{
    AES_KEY key;
    AES_set_decrypt_key(decryptionKey.begin(), 256, &key);
    AES_decrypt(encryptedIn.begin(), output.begin(), &key);
}

void ComputePreFactor(std::string strPassphrase, std::string strSalt, uint256& prefactor)
{
    
    uint64_t s = uint256(ReverseEndianString(strSalt)).Get64();
    scrypt_hash(strPassphrase.c_str(), strPassphrase.size(), BEGIN(s), strSalt.size() / 2, BEGIN(prefactor), 16384, 8, 8, 32);
}

void ComputePassfactor(std::string ownersalt, uint256 prefactor, uint256& passfactor)
{
    
    uint512 temp(ReverseEndianString(HexStr(prefactor) + ownersalt));
    Hash(temp.begin(), 40, passfactor.begin()); 
    Hash(passfactor.begin(), 32, passfactor.begin());
}

bool ComputePasspoint(uint256 passfactor, CPubKey& passpoint)
{
    size_t clen = 65;
    secp256k1_context *ctx = NULL;
    secp256k1_pubkey pubkey;

    
    if (!secp256k1_ec_pubkey_create(ctx, &pubkey, passfactor.begin()))
        return false;

    secp256k1_ec_pubkey_serialize(ctx, (unsigned char*)passpoint.begin(), &clen, &pubkey, SECP256K1_EC_COMPRESSED);

    if (passpoint.size() != clen)
        return false;

    if (!passpoint.IsValid())
        return false;

    return true;
}

void ComputeSeedBPass(CPubKey passpoint, std::string strAddressHash, std::string strOwnerSalt, uint512& seedBPass)
{
    
    string salt = ReverseEndianString(strAddressHash + strOwnerSalt);
    uint256 s2(salt);
    scrypt_hash(BEGIN(passpoint), HexStr(passpoint).size() / 2, BEGIN(s2), salt.size() / 2, BEGIN(seedBPass), 1024, 1, 1, 64);
}

void ComputeFactorB(uint256 seedB, uint256& factorB)
{
    
    Hash(seedB.begin(), 24, factorB.begin()); 
    Hash(factorB.begin(), 32, factorB.begin());
}

std::string AddressToBip38Hash(std::string address)
{
    uint256 addrCheck;
    Hash((void*)address.c_str(), address.size(), addrCheck.begin());
    Hash(addrCheck.begin(), 32, addrCheck.begin());

    return HexStr(addrCheck).substr(0, 8);
}

std::string BIP38_Encrypt(std::string strAddress, std::string strPassphrase, uint256 privKey, bool fCompressed)
{
    string strAddressHash = AddressToBip38Hash(strAddress);

    uint512 hashed;
    uint64_t salt = uint256(ReverseEndianString(strAddressHash)).Get64();
    scrypt_hash(strPassphrase.c_str(), strPassphrase.size(), BEGIN(salt), strAddressHash.size() / 2, BEGIN(hashed), 16384, 8, 8, 64);

    uint256 derivedHalf1(hashed.ToString().substr(64, 64));
    uint256 derivedHalf2(hashed.ToString().substr(0, 64));

    
    uint256 block1 = uint256((privKey << 128) ^ (derivedHalf1 << 128)) >> 128;

    
    uint512 encrypted1;
    AES_KEY key;
    AES_set_encrypt_key(derivedHalf2.begin(), 256, &key);
    AES_encrypt(block1.begin(), encrypted1.begin(), &key);

    
    uint256 p2 = privKey >> 128;
    uint256 dh12 = derivedHalf1 >> 128;
    uint256 block2 = uint256(p2 ^ dh12);

    
    uint512 encrypted2;
    AES_encrypt(block2.begin(), encrypted2.begin(), &key);

    string strPrefix = "0142";
    strPrefix += (fCompressed ? "E0" : "C0");

    uint512 encryptedKey(ReverseEndianString(strPrefix + strAddressHash));

    
    encryptedKey = encryptedKey | (encrypted1 << 56);

    
    encryptedKey = encryptedKey | (encrypted2 << (56 + 128));

    
    uint256 hashChecksum = Hash(encryptedKey.begin(), encryptedKey.begin() + 39);
    uint512 b58Checksum(hashChecksum.ToString().substr(64 - 8, 8));

    
    encryptedKey = encryptedKey | (b58Checksum << 312);

    
    return EncodeBase58(encryptedKey.begin(), encryptedKey.begin() + 43);
}

bool BIP38_Decrypt(std::string strPassphrase, std::string strEncryptedKey, uint256& privKey, bool& fCompressed)
{
    std::string strKey = DecodeBase58(strEncryptedKey.c_str());

    
    if (strKey.size() != (78 + 8))
        return false;

    
    if (uint256(ReverseEndianString(strKey.substr(0, 2))) != uint256(0x01))
        return false;

    uint256 type(ReverseEndianString(strKey.substr(2, 2)));
    uint256 flag(ReverseEndianString(strKey.substr(4, 2)));
    std::string strAddressHash = strKey.substr(6, 8);
    std::string ownersalt = strKey.substr(14, 16);
    uint256 encryptedPart1(ReverseEndianString(strKey.substr(30, 16)));
    uint256 encryptedPart2(ReverseEndianString(strKey.substr(46, 32)));

    fCompressed = (flag & uint256(0x20)) != 0;

    
    if (type == uint256(0x42)) {
        uint512 hashed;
        encryptedPart1 = uint256(ReverseEndianString(strKey.substr(14, 32)));
        uint64_t salt = uint256(ReverseEndianString(strAddressHash)).Get64();
        scrypt_hash(strPassphrase.c_str(), strPassphrase.size(), BEGIN(salt), strAddressHash.size() / 2, BEGIN(hashed), 16384, 8, 8, 64);

        uint256 derivedHalf1(hashed.ToString().substr(64, 64));
        uint256 derivedHalf2(hashed.ToString().substr(0, 64));

        uint256 decryptedPart1;
        DecryptAES(encryptedPart1, derivedHalf2, decryptedPart1);

        uint256 decryptedPart2;
        DecryptAES(encryptedPart2, derivedHalf2, decryptedPart2);

        
        uint256 temp1 = decryptedPart2 << 128;
        temp1 = temp1 | decryptedPart1;

        
        privKey = temp1 ^ derivedHalf1;

        return true;
    } else if (type != uint256(0x43)) 
        return false;

    bool fLotSequence = (flag & 0x04) != 0;

    std::string prefactorSalt = ownersalt;
    if (fLotSequence)
        prefactorSalt = ownersalt.substr(0, 8);

    uint256 prefactor;
    ComputePreFactor(strPassphrase, prefactorSalt, prefactor);

    uint256 passfactor;
    if (fLotSequence)
        ComputePassfactor(ownersalt, prefactor, passfactor);
    else
        passfactor = prefactor;

    CPubKey passpoint;
    if (!ComputePasspoint(passfactor, passpoint))
        return false;

    uint512 seedBPass;
    ComputeSeedBPass(passpoint, strAddressHash, ownersalt, seedBPass);

    
    uint256 derivedHalf1(seedBPass.ToString().substr(64, 64));
    uint256 derivedHalf2(seedBPass.ToString().substr(0, 64));

    
    uint256 decryptedPart2;
    DecryptAES(encryptedPart2, derivedHalf2, decryptedPart2);

    
    uint256 x0 = derivedHalf1 >> 128; 
    uint256 x1 = decryptedPart2 ^ x0;
    uint256 seedbPart2 = x1 >> 64;

    
    uint256 decryptedPart1;
    uint256 x2 = x1 & uint256("0xffffffffffffffff"); 
    x2 = x2 << 64;                                   
    x2 = encryptedPart1 | x2;                        
    DecryptAES(x2, derivedHalf2, decryptedPart1);

    
    uint256 x3 = derivedHalf1 & uint256("0xffffffffffffffffffffffffffffffff");
    uint256 seedbPart1 = decryptedPart1 ^ x3;
    uint256 seedB = seedbPart1 | (seedbPart2 << 128);

    uint256 factorB;
    ComputeFactorB(seedB, factorB);

    
    secp256k1_context *ctx = NULL;
    privKey = factorB;
    if (!secp256k1_ec_privkey_tweak_mul(ctx, privKey.begin(), passfactor.begin()))
        return false;

    
    CKey k;
    k.Set(privKey.begin(), privKey.end(), fCompressed);
    CPubKey pubkey = k.GetPubKey();
    string address = CBitcoinAddress(pubkey.GetID()).ToString();

    return strAddressHash == AddressToBip38Hash(address);
}
