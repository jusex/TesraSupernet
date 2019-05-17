





#ifndef CRYPTOPP_HASH_KEY_DERIVATION_FUNCTION_H
#define CRYPTOPP_HASH_KEY_DERIVATION_FUNCTION_H

#include "cryptlib.h"
#include "hrtimer.h"
#include "secblock.h"
#include "hmac.h"

NAMESPACE_BEGIN(CryptoPP)


class KeyDerivationFunction
{
public:
	
	virtual size_t MaxDerivedKeyLength() const =0;
	virtual bool Usesinfo() const =0;
	
	virtual unsigned int DeriveKey(byte *derived, size_t derivedLen, const byte *secret, size_t secretLen, const byte *salt, size_t saltLen, const byte* info=NULL, size_t infoLen=0) const =0;

	virtual ~KeyDerivationFunction() {}
};






template <class T>
class HKDF : public KeyDerivationFunction
{
public:
	CRYPTOPP_CONSTANT(DIGESTSIZE = T::DIGESTSIZE)
	CRYPTOPP_CONSTANT(SALTSIZE = T::DIGESTSIZE)
	static const char* StaticAlgorithmName () {
		static const std::string name(std::string("HKDF(") + std::string(T::StaticAlgorithmName()) + std::string(")"));
		return name.c_str();
	}
	size_t MaxDerivedKeyLength() const {return static_cast<size_t>(T::DIGESTSIZE) * 255;}
	bool Usesinfo() const {return true;}
	unsigned int DeriveKey(byte *derived, size_t derivedLen, const byte *secret, size_t secretLen, const byte *salt, size_t saltLen, const byte* info, size_t infoLen) const;

protected:
	
	
	typedef byte NullVectorType[SALTSIZE];
	static const NullVectorType& GetNullVector() {
		static const NullVectorType s_NullVector = {0};
		return s_NullVector;
	}
};

template <class T>
unsigned int HKDF<T>::DeriveKey(byte *derived, size_t derivedLen, const byte *secret, size_t secretLen, const byte *salt, size_t saltLen, const byte* info, size_t infoLen) const
{
	static const size_t DIGEST_SIZE = static_cast<size_t>(T::DIGESTSIZE);
	const unsigned int req = static_cast<unsigned int>(derivedLen);

	CRYPTOPP_ASSERT(secret && secretLen);
	CRYPTOPP_ASSERT(derived && derivedLen);
	CRYPTOPP_ASSERT(derivedLen <= MaxDerivedKeyLength());

	if (derivedLen > MaxDerivedKeyLength())
		throw InvalidArgument("HKDF: derivedLen must be less than or equal to MaxDerivedKeyLength");

	HMAC<T> hmac;
	FixedSizeSecBlock<byte, DIGEST_SIZE> prk, buffer;

	
	const byte* key = (salt ? salt : GetNullVector());
	const size_t klen = (salt ? saltLen : DIGEST_SIZE);

	hmac.SetKey(key, klen);
	hmac.CalculateDigest(prk, secret, secretLen);

	
	hmac.SetKey(prk.data(), prk.size());
	byte block = 0;

	while (derivedLen > 0)
	{
		if (block++) {hmac.Update(buffer, buffer.size());}
		if (info && infoLen) {hmac.Update(info, infoLen);}
		hmac.CalculateDigest(buffer, &block, 1);

#if CRYPTOPP_MSC_VERSION
		const size_t segmentLen = STDMIN(derivedLen, DIGEST_SIZE);
		memcpy_s(derived, segmentLen, buffer, segmentLen);
#else
		const size_t segmentLen = STDMIN(derivedLen, DIGEST_SIZE);
		std::memcpy(derived, buffer, segmentLen);
#endif

		derived += segmentLen;
		derivedLen -= segmentLen;
	}

	return req;
}

NAMESPACE_END

#endif 
