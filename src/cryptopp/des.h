




#ifndef CRYPTOPP_DES_H
#define CRYPTOPP_DES_H

#include "seckey.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)



class CRYPTOPP_DLL RawDES
{
public:
	void RawSetKey(CipherDir direction, const byte *userKey);
	void RawProcessBlock(word32 &l, word32 &r) const;

protected:
	static const word32 Spbox[8][64];

	FixedSizeSecBlock<word32, 32> k;
};



struct DES_Info : public FixedBlockSize<8>, public FixedKeyLength<8>
{
	
	CRYPTOPP_CONSTEXPR static const char *StaticAlgorithmName() {return "DES";}
};







class DES : public DES_Info, public BlockCipherDocumentation
{
	
	
	class CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<DES_Info>, public RawDES
	{
	public:
		void UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params);
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;
	};

public:
	
	static bool CheckKeyParityBits(const byte *key);
	
	static void CorrectKeyParityBits(byte *key);

	typedef BlockCipherFinal<ENCRYPTION, Base> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Base> Decryption;
};



struct DES_EDE2_Info : public FixedBlockSize<8>, public FixedKeyLength<16>
{
	CRYPTOPP_DLL static const char * CRYPTOPP_API StaticAlgorithmName() {return "DES-EDE2";}
};




class DES_EDE2 : public DES_EDE2_Info, public BlockCipherDocumentation
{
	
	
	class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<DES_EDE2_Info>
	{
	public:
		void UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params);
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;

	protected:
		RawDES m_des1, m_des2;
	};

public:
	typedef BlockCipherFinal<ENCRYPTION, Base> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Base> Decryption;
};



struct DES_EDE3_Info : public FixedBlockSize<8>, public FixedKeyLength<24>
{
	CRYPTOPP_DLL static const char * CRYPTOPP_API StaticAlgorithmName() {return "DES-EDE3";}
};




class DES_EDE3 : public DES_EDE3_Info, public BlockCipherDocumentation
{
	
	
	class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<DES_EDE3_Info>
	{
	public:
		void UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params);
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;

	protected:
		RawDES m_des1, m_des2, m_des3;
	};

public:
	typedef BlockCipherFinal<ENCRYPTION, Base> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Base> Decryption;
};



struct DES_XEX3_Info : public FixedBlockSize<8>, public FixedKeyLength<24>
{
	CRYPTOPP_CONSTEXPR static const char *StaticAlgorithmName() {return "DES-XEX3";}
};




class DES_XEX3 : public DES_XEX3_Info, public BlockCipherDocumentation
{
	
	
	class CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<DES_XEX3_Info>
	{
	public:
		void UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params);
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;

	protected:
		FixedSizeSecBlock<byte, BLOCKSIZE> m_x1, m_x3;
		
		
		value_ptr<DES::Encryption> m_des;
	};

public:
	typedef BlockCipherFinal<ENCRYPTION, Base> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Base> Decryption;
};

typedef DES::Encryption DESEncryption;
typedef DES::Decryption DESDecryption;

typedef DES_EDE2::Encryption DES_EDE2_Encryption;
typedef DES_EDE2::Decryption DES_EDE2_Decryption;

typedef DES_EDE3::Encryption DES_EDE3_Encryption;
typedef DES_EDE3::Decryption DES_EDE3_Decryption;

typedef DES_XEX3::Encryption DES_XEX3_Encryption;
typedef DES_XEX3::Decryption DES_XEX3_Decryption;

NAMESPACE_END

#endif
