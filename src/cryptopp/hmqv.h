


#ifndef CRYPTOPP_HMQV_H
#define CRYPTOPP_HMQV_H





#include "gfpcrypt.h"
#include "algebra.h"
#include "sha.h"

NAMESPACE_BEGIN(CryptoPP)






template <class GROUP_PARAMETERS, class COFACTOR_OPTION = CPP_TYPENAME GROUP_PARAMETERS::DefaultCofactorOption, class HASH = SHA512>
class HMQV_Domain: public AuthenticatedKeyAgreementDomain
{
public:
  typedef GROUP_PARAMETERS GroupParameters;
  typedef typename GroupParameters::Element Element;
  typedef HMQV_Domain<GROUP_PARAMETERS, COFACTOR_OPTION, HASH> Domain;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~HMQV_Domain() {}
#endif

  HMQV_Domain(bool clientRole = true): m_role(clientRole ? RoleClient : RoleServer) {}

  HMQV_Domain(const GroupParameters &params, bool clientRole = true)
    : m_role(clientRole ? RoleClient : RoleServer), m_groupParameters(params) {}

  HMQV_Domain(BufferedTransformation &bt, bool clientRole = true)
    : m_role(clientRole ? RoleClient : RoleServer)
  {m_groupParameters.BERDecode(bt);}

  template <class T1>
  HMQV_Domain(T1 v1, bool clientRole = true)
    : m_role(clientRole ? RoleClient : RoleServer)
  {m_groupParameters.Initialize(v1);}

  template <class T1, class T2>
  HMQV_Domain(T1 v1, T2 v2, bool clientRole = true)
    : m_role(clientRole ? RoleClient : RoleServer)
  {m_groupParameters.Initialize(v1, v2);}

  template <class T1, class T2, class T3>
  HMQV_Domain(T1 v1, T2 v2, T3 v3, bool clientRole = true)
    : m_role(clientRole ? RoleClient : RoleServer)
  {m_groupParameters.Initialize(v1, v2, v3);}

  template <class T1, class T2, class T3, class T4>
  HMQV_Domain(T1 v1, T2 v2, T3 v3, T4 v4, bool clientRole = true)
    : m_role(clientRole ? RoleClient : RoleServer)
  {m_groupParameters.Initialize(v1, v2, v3, v4);}

public:

  const GroupParameters & GetGroupParameters() const {return m_groupParameters;}
  GroupParameters & AccessGroupParameters(){return m_groupParameters;}

  CryptoParameters & AccessCryptoParameters(){return AccessAbstractGroupParameters();}

  
  unsigned int AgreedValueLength() const {return GetAbstractGroupParameters().GetEncodedElementSize(false);}
  
  unsigned int StaticPrivateKeyLength() const {return GetAbstractGroupParameters().GetSubgroupOrder().ByteCount();}
  
  unsigned int StaticPublicKeyLength() const{return GetAbstractGroupParameters().GetEncodedElementSize(true);}

  
  
  void GenerateStaticPrivateKey(RandomNumberGenerator &rng, byte *privateKey) const
  {
    Integer x(rng, Integer::One(), GetAbstractGroupParameters().GetMaxExponent());
    x.Encode(privateKey, StaticPrivateKeyLength());
  }

  
  
  void GenerateStaticPublicKey(RandomNumberGenerator &rng, const byte *privateKey, byte *publicKey) const
  {
    CRYPTOPP_UNUSED(rng);
    const DL_GroupParameters<Element> &params = GetAbstractGroupParameters();
    Integer x(privateKey, StaticPrivateKeyLength());
    Element y = params.ExponentiateBase(x);
    params.EncodeElement(true, y, publicKey);
  }

  unsigned int EphemeralPrivateKeyLength() const {return StaticPrivateKeyLength() + StaticPublicKeyLength();}
  unsigned int EphemeralPublicKeyLength() const{return StaticPublicKeyLength();}

  
  void GenerateEphemeralPrivateKey(RandomNumberGenerator &rng, byte *privateKey) const
  {
    const DL_GroupParameters<Element> &params = GetAbstractGroupParameters();
    Integer x(rng, Integer::One(), params.GetMaxExponent());
    x.Encode(privateKey, StaticPrivateKeyLength());
    Element y = params.ExponentiateBase(x);
    params.EncodeElement(true, y, privateKey+StaticPrivateKeyLength());
  }

  
  void GenerateEphemeralPublicKey(RandomNumberGenerator &rng, const byte *privateKey, byte *publicKey) const
  {
    CRYPTOPP_UNUSED(rng);
    memcpy(publicKey, privateKey+StaticPrivateKeyLength(), EphemeralPublicKeyLength());
  }

  
  /*! \note The ephemeral public key will always be validated.
  If you have previously validated the static public key, use validateStaticOtherPublicKey=false to save time.
  \pre size of agreedValue == AgreedValueLength()
  \pre length of staticPrivateKey == StaticPrivateKeyLength()
  \pre length of ephemeralPrivateKey == EphemeralPrivateKeyLength()
  \pre length of staticOtherPublicKey == StaticPublicKeyLength()
  \pre length of ephemeralOtherPublicKey == EphemeralPublicKeyLength()
  */
  bool Agree(byte *agreedValue,
    const byte *staticPrivateKey, const byte *ephemeralPrivateKey,
    const byte *staticOtherPublicKey, const byte *ephemeralOtherPublicKey,
    bool validateStaticOtherPublicKey=true) const
  {
    byte *XX = NULL, *YY = NULL, *AA = NULL, *BB = NULL;
    size_t xxs = 0, yys = 0, aas = 0, bbs = 0;

    
    
    SecByteBlock tt(StaticPublicKeyLength());

    try
    {
      const DL_GroupParameters<Element> &params = GetAbstractGroupParameters();

      if(m_role == RoleServer)
      {
        Integer b(staticPrivateKey, StaticPrivateKeyLength());
        Element B = params.ExponentiateBase(b);
        params.EncodeElement(true, B, tt);

        XX = const_cast<byte*>(ephemeralOtherPublicKey);
        xxs = EphemeralPublicKeyLength();
        YY = const_cast<byte*>(ephemeralPrivateKey) + StaticPrivateKeyLength();
        yys = EphemeralPublicKeyLength();
        AA = const_cast<byte*>(staticOtherPublicKey);
        aas = StaticPublicKeyLength();
        BB = tt.BytePtr();
        bbs = tt.SizeInBytes();
      }
      else if(m_role == RoleClient)
      {
        Integer a(staticPrivateKey, StaticPrivateKeyLength());
        Element A = params.ExponentiateBase(a);
        params.EncodeElement(true, A, tt);

        XX = const_cast<byte*>(ephemeralPrivateKey) + StaticPrivateKeyLength();
        xxs = EphemeralPublicKeyLength();
        YY = const_cast<byte*>(ephemeralOtherPublicKey);
        yys = EphemeralPublicKeyLength();
        AA = tt.BytePtr();
        aas = tt.SizeInBytes();
        BB = const_cast<byte*>(staticOtherPublicKey);
        bbs = StaticPublicKeyLength();
      }
      else
      {
        CRYPTOPP_ASSERT(0);
        return false;
      }

      
      
      
      Element VV1 = params.DecodeElement(staticOtherPublicKey, false);
      if(!params.ValidateElement(validateStaticOtherPublicKey ? 3 : 1, VV1, NULL))
        return false;

      
      
      Element VV2 = params.DecodeElement(ephemeralOtherPublicKey, false);
      if(!params.ValidateElement(3, VV2, NULL))
        return false;

      
      const Integer& q = params.GetSubgroupOrder();
      const unsigned int len  = (((q.BitCount()+1)/2 +7)/8);

      Integer d, e;
      SecByteBlock dd(len), ee(len);

      
      Hash(NULL, XX, xxs, BB, bbs, dd.BytePtr(), dd.SizeInBytes());
      d.Decode(dd.BytePtr(), dd.SizeInBytes());

      
      Hash(NULL, YY, yys, AA, aas, ee.BytePtr(), ee.SizeInBytes());
      e.Decode(ee.BytePtr(), ee.SizeInBytes());

      Element sigma;
      if(m_role == RoleServer)
      {
        Integer y(ephemeralPrivateKey, StaticPrivateKeyLength());
        Integer b(staticPrivateKey, StaticPrivateKeyLength());
        Integer s_B = (y + e * b) % q;

        Element A = params.DecodeElement(AA, false);
        Element X = params.DecodeElement(XX, false);

        Element t1 = params.ExponentiateElement(A, d);
        Element t2 = m_groupParameters.MultiplyElements(X, t1);

        
        sigma = params.ExponentiateElement(t2, s_B);
      }
      else
      {
        Integer x(ephemeralPrivateKey, StaticPrivateKeyLength());
        Integer a(staticPrivateKey, StaticPrivateKeyLength());
        Integer s_A = (x + d * a) % q;

        Element B = params.DecodeElement(BB, false);
        Element Y = params.DecodeElement(YY, false);

        Element t1 = params.ExponentiateElement(B, e);
        Element t2 = m_groupParameters.MultiplyElements(Y, t1);

        
        sigma = params.ExponentiateElement(t2, s_A);
      }
      Hash(&sigma, NULL, 0, NULL, 0, agreedValue, AgreedValueLength());
    }
    catch (DL_BadElement &)
    {
      return false;
    }
    return true;
  }

protected:
  
  

  inline void Hash(const Element* sigma,
    const byte* e1, size_t e1len, 
    const byte* s1, size_t s1len, 
    byte* digest, size_t dlen) const
  {
    HASH hash;
    size_t idx = 0, req = dlen;
    size_t blk = STDMIN(dlen, (size_t)HASH::DIGESTSIZE);

    if(sigma)
    {
      if (e1len != 0 || s1len != 0) {
	CRYPTOPP_ASSERT(0);
      }
      Integer x = GetAbstractGroupParameters().ConvertElementToInteger(*sigma);
      SecByteBlock sbb(x.MinEncodedSize());
      x.Encode(sbb.BytePtr(), sbb.SizeInBytes());
      hash.Update(sbb.BytePtr(), sbb.SizeInBytes());
    } else {
      if (e1len == 0 || s1len == 0) {
	CRYPTOPP_ASSERT(0);
      }
      hash.Update(e1, e1len);
      hash.Update(s1, s1len);
    }

    hash.TruncatedFinal(digest, blk);
    req -= blk;

    
    while(req != 0)
    {
      hash.Update(&digest[idx], (size_t)HASH::DIGESTSIZE);

      idx += (size_t)HASH::DIGESTSIZE;
      blk = STDMIN(req, (size_t)HASH::DIGESTSIZE);
      hash.TruncatedFinal(&digest[idx], blk);

      req -= blk;
    }
  }

private:

  
  enum KeyAgreementRole{ RoleServer = 1, RoleClient };

  DL_GroupParameters<Element> & AccessAbstractGroupParameters() {return m_groupParameters;}
  const DL_GroupParameters<Element> & GetAbstractGroupParameters() const{return m_groupParameters;}

  GroupParameters m_groupParameters;
  KeyAgreementRole m_role;
};






typedef HMQV_Domain<DL_GroupParameters_GFP_DefaultSafePrime> HMQV;

NAMESPACE_END

#endif
