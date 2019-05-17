






#ifndef CRYPTOPP_RSA_H
#define CRYPTOPP_RSA_H

#include "cryptlib.h"
#include "pubkey.h"
#include "integer.h"
#include "pkcspad.h"
#include "oaep.h"
#include "emsa2.h"
#include "asn.h"

NAMESPACE_BEGIN(CryptoPP)



class CRYPTOPP_DLL RSAFunction : public TrapdoorFunction, public X509PublicKey
{
	typedef RSAFunction ThisClass;

public:
	
	
	
	void Initialize(const Integer &n, const Integer &e)
		{m_n = n; m_e = e;}

	
	OID GetAlgorithmID() const;
	void BERDecodePublicKey(BufferedTransformation &bt, bool parametersPresent, size_t size);
	void DEREncodePublicKey(BufferedTransformation &bt) const;

	
	bool Validate(RandomNumberGenerator &rng, unsigned int level) const;
	bool GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const;
	void AssignFrom(const NameValuePairs &source);

	
	Integer ApplyFunction(const Integer &x) const;
	Integer PreimageBound() const {return m_n;}
	Integer ImageBound() const {return m_n;}

	
	const Integer & GetModulus() const {return m_n;}
	const Integer & GetPublicExponent() const {return m_e;}

	void SetModulus(const Integer &n) {m_n = n;}
	void SetPublicExponent(const Integer &e) {m_e = e;}

protected:
	Integer m_n, m_e;
};



class CRYPTOPP_DLL InvertibleRSAFunction : public RSAFunction, public TrapdoorFunctionInverse, public PKCS8PrivateKey
{
	typedef InvertibleRSAFunction ThisClass;

public:
	
	
	
	
	void Initialize(RandomNumberGenerator &rng, unsigned int modulusBits, const Integer &e = 17);

	
	
	
	
	
	
	
	
	
	void Initialize(const Integer &n, const Integer &e, const Integer &d, const Integer &p, const Integer &q, const Integer &dp, const Integer &dq, const Integer &u)
		{m_n = n; m_e = e; m_d = d; m_p = p; m_q = q; m_dp = dp; m_dq = dq; m_u = u;}
	
	
	
	
	
	void Initialize(const Integer &n, const Integer &e, const Integer &d);

	
	void BERDecode(BufferedTransformation &bt)
		{PKCS8PrivateKey::BERDecode(bt);}
	void DEREncode(BufferedTransformation &bt) const
		{PKCS8PrivateKey::DEREncode(bt);}
	void Load(BufferedTransformation &bt)
		{PKCS8PrivateKey::BERDecode(bt);}
	void Save(BufferedTransformation &bt) const
		{PKCS8PrivateKey::DEREncode(bt);}
	OID GetAlgorithmID() const {return RSAFunction::GetAlgorithmID();}
	void BERDecodePrivateKey(BufferedTransformation &bt, bool parametersPresent, size_t size);
	void DEREncodePrivateKey(BufferedTransformation &bt) const;

	
	Integer CalculateInverse(RandomNumberGenerator &rng, const Integer &x) const;

	
	bool Validate(RandomNumberGenerator &rng, unsigned int level) const;
	
	void GenerateRandom(RandomNumberGenerator &rng, const NameValuePairs &alg);
	bool GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const;
	void AssignFrom(const NameValuePairs &source);

	
	const Integer& GetPrime1() const {return m_p;}
	const Integer& GetPrime2() const {return m_q;}
	const Integer& GetPrivateExponent() const {return m_d;}
	const Integer& GetModPrime1PrivateExponent() const {return m_dp;}
	const Integer& GetModPrime2PrivateExponent() const {return m_dq;}
	const Integer& GetMultiplicativeInverseOfPrime2ModPrime1() const {return m_u;}

	void SetPrime1(const Integer &p) {m_p = p;}
	void SetPrime2(const Integer &q) {m_q = q;}
	void SetPrivateExponent(const Integer &d) {m_d = d;}
	void SetModPrime1PrivateExponent(const Integer &dp) {m_dp = dp;}
	void SetModPrime2PrivateExponent(const Integer &dq) {m_dq = dq;}
	void SetMultiplicativeInverseOfPrime2ModPrime1(const Integer &u) {m_u = u;}

protected:
	Integer m_d, m_p, m_q, m_dp, m_dq, m_u;
};



class CRYPTOPP_DLL RSAFunction_ISO : public RSAFunction
{
public:
	Integer ApplyFunction(const Integer &x) const;
	Integer PreimageBound() const {return ++(m_n>>1);}
};



class CRYPTOPP_DLL InvertibleRSAFunction_ISO : public InvertibleRSAFunction
{
public:
	Integer CalculateInverse(RandomNumberGenerator &rng, const Integer &x) const;
	Integer PreimageBound() const {return ++(m_n>>1);}
};



struct CRYPTOPP_DLL RSA
{
	CRYPTOPP_CONSTEXPR static const char * CRYPTOPP_API StaticAlgorithmName() {return "RSA";}
	typedef RSAFunction PublicKey;
	typedef InvertibleRSAFunction PrivateKey;
};





template <class STANDARD>
struct RSAES : public TF_ES<STANDARD, RSA>
{
};







template <class STANDARD, class H>
struct RSASS : public TF_SS<STANDARD, H, RSA>
{
};



struct CRYPTOPP_DLL RSA_ISO
{
	CRYPTOPP_CONSTEXPR static const char * CRYPTOPP_API StaticAlgorithmName() {return "RSA-ISO";}
	typedef RSAFunction_ISO PublicKey;
	typedef InvertibleRSAFunction_ISO PrivateKey;
};




template <class H>
struct RSASS_ISO : public TF_SS<P1363_EMSA2, H, RSA_ISO>
{
};



DOCUMENTED_TYPEDEF(RSAES<PKCS1v15>::Decryptor, RSAES_PKCS1v15_Decryptor);


DOCUMENTED_TYPEDEF(RSAES<PKCS1v15>::Encryptor, RSAES_PKCS1v15_Encryptor);



DOCUMENTED_TYPEDEF(RSAES<OAEP<SHA> >::Decryptor, RSAES_OAEP_SHA_Decryptor);


DOCUMENTED_TYPEDEF(RSAES<OAEP<SHA> >::Encryptor, RSAES_OAEP_SHA_Encryptor);

#ifdef CRYPTOPP_DOXYGEN_PROCESSING


class RSASSA_PKCS1v15_SHA_Signer : public RSASS<PKCS1v15,SHA>::Signer {};


class RSASSA_PKCS1v15_SHA_Verifier : public RSASS<PKCS1v15,SHA>::Verifier {};

namespace Weak {



class RSASSA_PKCS1v15_MD2_Signer : public RSASS<PKCS1v15, Weak1::MD2>::Signer {};


class RSASSA_PKCS1v15_MD2_Verifier : public RSASS<PKCS1v15, Weak1::MD2>::Verifier {};



class RSASSA_PKCS1v15_MD5_Signer : public RSASS<PKCS1v15, Weak1::MD5>::Signer {};


class RSASSA_PKCS1v15_MD5_Verifier : public RSASS<PKCS1v15, Weak1::MD5>::Verifier {};
}

#else
typedef RSASS<PKCS1v15,SHA>::Signer RSASSA_PKCS1v15_SHA_Signer;
typedef RSASS<PKCS1v15,SHA>::Verifier RSASSA_PKCS1v15_SHA_Verifier;

namespace Weak {
	typedef RSASS<PKCS1v15, Weak1::MD2>::Signer RSASSA_PKCS1v15_MD2_Signer;
	typedef RSASS<PKCS1v15, Weak1::MD2>::Verifier RSASSA_PKCS1v15_MD2_Verifier;
	typedef RSASS<PKCS1v15, Weak1::MD5>::Signer RSASSA_PKCS1v15_MD5_Signer;
	typedef RSASS<PKCS1v15, Weak1::MD5>::Verifier RSASSA_PKCS1v15_MD5_Verifier;
}
#endif 

NAMESPACE_END

#endif
