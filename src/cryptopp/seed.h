





#ifndef CRYPTOPP_SEED_H
#define CRYPTOPP_SEED_H

#include "seckey.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)




struct SEED_Info : public FixedBlockSize<16>, public FixedKeyLength<16>, public FixedRounds<16>
{
	CRYPTOPP_CONSTEXPR static const char *StaticAlgorithmName() {return "SEED";}
};





class SEED : public SEED_Info, public BlockCipherDocumentation
{
	class CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<SEED_Info>
	{
	public:
		void UncheckedSetKey(const byte *key, unsigned int length, const NameValuePairs &params);
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;

	protected:
		FixedSizeSecBlock<word32, 32> m_k;
	};

public:
	typedef BlockCipherFinal<ENCRYPTION, Base> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Base> Decryption;
};

NAMESPACE_END

#endif
