




#ifndef CRYPTOPP_GOST_H
#define CRYPTOPP_GOST_H

#include "seckey.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)



struct GOST_Info : public FixedBlockSize<8>, public FixedKeyLength<32>
{
	CRYPTOPP_CONSTEXPR static const char *StaticAlgorithmName() {return "GOST";}
};




class GOST : public GOST_Info, public BlockCipherDocumentation
{
	
	
	class CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<GOST_Info>
	{
	public:
		void UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params);

	protected:
		static void PrecalculateSTable();

		static const byte sBox[8][16];
		static volatile bool sTableCalculated;
		static word32 sTable[4][256];

		FixedSizeSecBlock<word32, 8> m_key;
	};

	
	
	class CRYPTOPP_NO_VTABLE Enc : public Base
	{
	public:
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;
	};

	
	
	class CRYPTOPP_NO_VTABLE Dec : public Base
	{
	public:
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;
	};

public:
	typedef BlockCipherFinal<ENCRYPTION, Enc> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Dec> Decryption;
};

typedef GOST::Encryption GOSTEncryption;
typedef GOST::Decryption GOSTDecryption;

NAMESPACE_END

#endif
