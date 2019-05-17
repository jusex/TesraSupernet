












#ifndef CRYPTOPP_CHACHA_H
#define CRYPTOPP_CHACHA_H

#include "strciphr.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)




template <unsigned int R>
struct ChaCha_Info : public VariableKeyLength<32, 16, 32, 16, SimpleKeyingInterface::UNIQUE_IV, 8>, public FixedRounds<R>
{
	CRYPTOPP_CONSTEXPR static const char *StaticAlgorithmName() {
		return (R==8?"ChaCha8":(R==12?"ChaCha12":(R==20?"ChaCha20":"ChaCha")));
	}
};




template <unsigned int R>
class CRYPTOPP_NO_VTABLE ChaCha_Policy : public AdditiveCipherConcretePolicy<word32, 16>
{
protected:
	CRYPTOPP_CONSTANT(ROUNDS=FixedRounds<R>::ROUNDS)

	void CipherSetKey(const NameValuePairs &params, const byte *key, size_t length);
	void OperateKeystream(KeystreamOperation operation, byte *output, const byte *input, size_t iterationCount);
	void CipherResynchronize(byte *keystreamBuffer, const byte *IV, size_t length);
	bool CipherIsRandomAccess() const {return false;} 
	void SeekToIteration(lword iterationCount);
	unsigned int GetAlignment() const;
	unsigned int GetOptimalBlockSize() const;

	FixedSizeAlignedSecBlock<word32, 16> m_state;
};





struct ChaCha8 : public ChaCha_Info<8>, public SymmetricCipherDocumentation
{
	typedef SymmetricCipherFinal<ConcretePolicyHolder<ChaCha_Policy<8>, AdditiveCipherTemplate<> >, ChaCha_Info<8> > Encryption;
	typedef Encryption Decryption;
};








struct ChaCha12 : public ChaCha_Info<12>, public SymmetricCipherDocumentation
{
	typedef SymmetricCipherFinal<ConcretePolicyHolder<ChaCha_Policy<12>, AdditiveCipherTemplate<> >, ChaCha_Info<12> > Encryption;
	typedef Encryption Decryption;
};








struct ChaCha20 : public ChaCha_Info<20>, public SymmetricCipherDocumentation
{
	typedef SymmetricCipherFinal<ConcretePolicyHolder<ChaCha_Policy<20>, AdditiveCipherTemplate<> >, ChaCha_Info<20> > Encryption;
	typedef Encryption Decryption;
};

NAMESPACE_END

#endif  
