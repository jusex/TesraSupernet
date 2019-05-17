


























#ifndef CRYPTOPP_RANDPOOL_H
#define CRYPTOPP_RANDPOOL_H

#include "cryptlib.h"
#include "filters.h"
#include "secblock.h"
#include "smartptr.h"
#include "aes.h"

NAMESPACE_BEGIN(CryptoPP)











class CRYPTOPP_DLL RandomPool : public RandomNumberGenerator, public NotCopyable
{
public:
	
	RandomPool();

	bool CanIncorporateEntropy() const {return true;}
	void IncorporateEntropy(const byte *input, size_t length);
	void GenerateIntoBufferedTransformation(BufferedTransformation &target, const std::string &channel, lword size);

	
	void Put(const byte *input, size_t length) {IncorporateEntropy(input, length);}

private:
	FixedSizeAlignedSecBlock<byte, 16, true> m_seed;
	FixedSizeAlignedSecBlock<byte, 32> m_key;
	member_ptr<BlockCipher> m_pCipher;
	bool m_keySet;
};

NAMESPACE_END

#endif
