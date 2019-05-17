




#ifndef CRYPTOPP_SOSEMANUK_H
#define CRYPTOPP_SOSEMANUK_H

#include "strciphr.h"
#include "secblock.h"



#if CRYPTOPP_BOOL_X32 || defined(CRYPTOPP_DISABLE_INTEL_ASM)
# define CRYPTOPP_DISABLE_SOSEMANUK_ASM
#endif

NAMESPACE_BEGIN(CryptoPP)


struct SosemanukInfo : public VariableKeyLength<16, 1, 32, 1, SimpleKeyingInterface::UNIQUE_IV, 16>
{
	CRYPTOPP_CONSTEXPR static const char *StaticAlgorithmName() {return "Sosemanuk";}
};


class SosemanukPolicy : public AdditiveCipherConcretePolicy<word32, 20>, public SosemanukInfo
{
protected:
	void CipherSetKey(const NameValuePairs &params, const byte *key, size_t length);
	void OperateKeystream(KeystreamOperation operation, byte *output, const byte *input, size_t iterationCount);
	void CipherResynchronize(byte *keystreamBuffer, const byte *iv, size_t length);
	bool CipherIsRandomAccess() const {return false;}
#if (CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32 || CRYPTOPP_BOOL_X64) && !defined(CRYPTOPP_DISABLE_SOSEMANUK_ASM)
	unsigned int GetAlignment() const;
	unsigned int GetOptimalBlockSize() const;
#endif

	FixedSizeSecBlock<word32, 25*4> m_key;
	FixedSizeAlignedSecBlock<word32, 12> m_state;
};


struct Sosemanuk : public SosemanukInfo, public SymmetricCipherDocumentation
{
	typedef SymmetricCipherFinal<ConcretePolicyHolder<SosemanukPolicy, AdditiveCipherTemplate<> >, SosemanukInfo> Encryption;
	typedef Encryption Decryption;
};

NAMESPACE_END

#endif
