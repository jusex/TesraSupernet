




#ifndef CRYPTOPP_IDEA_H
#define CRYPTOPP_IDEA_H

#include "seckey.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)



struct IDEA_Info : public FixedBlockSize<8>, public FixedKeyLength<16>, public FixedRounds<8>
{
	CRYPTOPP_CONSTEXPR static const char *StaticAlgorithmName() {return "IDEA";}
};




class IDEA : public IDEA_Info, public BlockCipherDocumentation
{
public:		
#ifdef CRYPTOPP_NATIVE_DWORD_AVAILABLE
	typedef word Word;
#else
	typedef hword Word;
#endif

private:
	class CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<IDEA_Info>
	{
	public:
		unsigned int OptimalDataAlignment() const {return 2;}
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;

		void UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params);

	private:
		void EnKey(const byte *);
		void DeKey();
		FixedSizeSecBlock<Word, 6*ROUNDS+4> m_key;

	#ifdef IDEA_LARGECACHE
		static inline void LookupMUL(word &a, word b);
		void LookupKeyLogs();
		static void BuildLogTables();
		static volatile bool tablesBuilt;
		static word16 log[0x10000], antilog[0x10000];
	#endif
	};

public:
	typedef BlockCipherFinal<ENCRYPTION, Base> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Base> Decryption;
};

typedef IDEA::Encryption IDEAEncryption;
typedef IDEA::Decryption IDEADecryption;

NAMESPACE_END

#endif
