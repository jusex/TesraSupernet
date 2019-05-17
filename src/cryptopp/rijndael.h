






#ifndef CRYPTOPP_RIJNDAEL_H
#define CRYPTOPP_RIJNDAEL_H

#include "seckey.h"
#include "secblock.h"


#if CRYPTOPP_BOOL_X32 || (defined(CRYPTOPP_LLVM_CLANG_VERSION) && (CRYPTOPP_LLVM_CLANG_VERSION < 30400))
# define CRYPTOPP_DISABLE_RIJNDAEL_ASM
#endif

NAMESPACE_BEGIN(CryptoPP)


struct Rijndael_Info : public FixedBlockSize<16>, public VariableKeyLength<16, 16, 32, 8>
{
	CRYPTOPP_DLL static const char * CRYPTOPP_API StaticAlgorithmName() {return CRYPTOPP_RIJNDAEL_NAME;}
};



class CRYPTOPP_DLL Rijndael : public Rijndael_Info, public BlockCipherDocumentation
{
	
	
	class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<Rijndael_Info>
	{
	public:
		void UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params);

	protected:
		static void FillEncTable();
		static void FillDecTable();

		
		static const byte Se[256];
		static const byte Sd[256];

		static const word32 rcon[];

		unsigned int m_rounds;
		FixedSizeAlignedSecBlock<word32, 4*15> m_key;
	};

	
	
	class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE Enc : public Base
	{
	public:
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;
#if CRYPTOPP_BOOL_X64 || CRYPTOPP_BOOL_X32 || CRYPTOPP_BOOL_X86
		Enc();
		size_t AdvancedProcessBlocks(const byte *inBlocks, const byte *xorBlocks, byte *outBlocks, size_t length, word32 flags) const;
	private:
		SecByteBlock m_aliasBlock;
#endif
	};

	
	
	class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE Dec : public Base
	{
	public:
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;
#if CRYPTOPP_BOOL_AESNI_INTRINSICS_AVAILABLE
		size_t AdvancedProcessBlocks(const byte *inBlocks, const byte *xorBlocks, byte *outBlocks, size_t length, word32 flags) const;
#endif
	};

public:
	typedef BlockCipherFinal<ENCRYPTION, Enc> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Dec> Decryption;
};

typedef Rijndael::Encryption RijndaelEncryption;
typedef Rijndael::Decryption RijndaelDecryption;

NAMESPACE_END

#endif
