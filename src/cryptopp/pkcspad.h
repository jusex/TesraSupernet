





#ifndef CRYPTOPP_PKCSPAD_H
#define CRYPTOPP_PKCSPAD_H

#include "cryptlib.h"
#include "pubkey.h"

#ifdef CRYPTOPP_IS_DLL
#include "sha.h"
#endif

NAMESPACE_BEGIN(CryptoPP)




class PKCS_EncryptionPaddingScheme : public PK_EncryptionMessageEncodingMethod
{
public:
	CRYPTOPP_CONSTEXPR static const char *StaticAlgorithmName() {return "EME-PKCS1-v1_5";}

	size_t MaxUnpaddedLength(size_t paddedLength) const;
	void Pad(RandomNumberGenerator &rng, const byte *raw, size_t inputLength, byte *padded, size_t paddedLength, const NameValuePairs &parameters) const;
	DecodingResult Unpad(const byte *padded, size_t paddedLength, byte *raw, const NameValuePairs &parameters) const;
};



template <class H> class PKCS_DigestDecoration
{
public:
	static const byte decoration[];
	static const unsigned int length;
};



class SHA1;
class SHA224;
class SHA256;
class SHA384;
class SHA512;
class Tiger;
class RIPEMD160;
namespace Weak1 {
class MD2;
class MD5;
}


#ifdef CRYPTOPP_IS_DLL
CRYPTOPP_DLL_TEMPLATE_CLASS PKCS_DigestDecoration<SHA1>;
CRYPTOPP_DLL_TEMPLATE_CLASS PKCS_DigestDecoration<SHA224>;
CRYPTOPP_DLL_TEMPLATE_CLASS PKCS_DigestDecoration<SHA256>;
CRYPTOPP_DLL_TEMPLATE_CLASS PKCS_DigestDecoration<SHA384>;
CRYPTOPP_DLL_TEMPLATE_CLASS PKCS_DigestDecoration<SHA512>;
#endif




class CRYPTOPP_DLL PKCS1v15_SignatureMessageEncodingMethod : public PK_DeterministicSignatureMessageEncodingMethod
{
public:
	CRYPTOPP_CONSTEXPR static const char * CRYPTOPP_API StaticAlgorithmName() {return "EMSA-PKCS1-v1_5";}

	size_t MinRepresentativeBitLength(size_t hashIdentifierSize, size_t digestSize) const
		{return 8 * (digestSize + hashIdentifierSize + 10);}

	void ComputeMessageRepresentative(RandomNumberGenerator &rng,
		const byte *recoverableMessage, size_t recoverableMessageLength,
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength) const;

	struct HashIdentifierLookup
	{
		template <class H> struct HashIdentifierLookup2
		{
			static HashIdentifier Lookup()
			{
				return HashIdentifier(PKCS_DigestDecoration<H>::decoration, PKCS_DigestDecoration<H>::length);
			}
		};
	};
};


/*! Only the following hash functions are supported by this signature standard:
	\dontinclude pkcspad.h
	\skip can be instantiated
	\until end of list
*/
struct PKCS1v15 : public SignatureStandard, public EncryptionStandard
{
	typedef PKCS_EncryptionPaddingScheme EncryptionMessageEncodingMethod;
	typedef PKCS1v15_SignatureMessageEncodingMethod SignatureMessageEncodingMethod;
};

NAMESPACE_END

#endif
