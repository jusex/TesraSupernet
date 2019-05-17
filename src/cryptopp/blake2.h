
















#ifndef CRYPTOPP_BLAKE2_H
#define CRYPTOPP_BLAKE2_H

#include "cryptlib.h"
#include "secblock.h"
#include "seckey.h"

NAMESPACE_BEGIN(CryptoPP)





template <bool T_64bit>
struct BLAKE2_Info : public VariableKeyLength<(T_64bit ? 64 : 32),0,(T_64bit ? 64 : 32),1,SimpleKeyingInterface::NOT_RESYNCHRONIZABLE>
{
	typedef VariableKeyLength<(T_64bit ? 64 : 32),0,(T_64bit ? 64 : 32),1,SimpleKeyingInterface::NOT_RESYNCHRONIZABLE> KeyBase;
	CRYPTOPP_CONSTANT(MIN_KEYLENGTH = KeyBase::MIN_KEYLENGTH)
	CRYPTOPP_CONSTANT(MAX_KEYLENGTH = KeyBase::MAX_KEYLENGTH)
	CRYPTOPP_CONSTANT(DEFAULT_KEYLENGTH = KeyBase::DEFAULT_KEYLENGTH)

	CRYPTOPP_CONSTANT(BLOCKSIZE = (T_64bit ? 128 : 64))
	CRYPTOPP_CONSTANT(DIGESTSIZE = (T_64bit ? 64 : 32))
	CRYPTOPP_CONSTANT(SALTSIZE = (T_64bit ? 16 : 8))
	CRYPTOPP_CONSTANT(PERSONALIZATIONSIZE = (T_64bit ? 16 : 8))

	CRYPTOPP_CONSTEXPR static const	char *StaticAlgorithmName() {return (T_64bit ? "BLAKE2b" : "BLAKE2s");}
};







template <bool T_64bit>
struct CRYPTOPP_NO_VTABLE BLAKE2_ParameterBlock
{
};


template<>
struct CRYPTOPP_NO_VTABLE BLAKE2_ParameterBlock<true>
{
	CRYPTOPP_CONSTANT(SALTSIZE = BLAKE2_Info<true>::SALTSIZE)
	CRYPTOPP_CONSTANT(DIGESTSIZE = BLAKE2_Info<true>::DIGESTSIZE)
	CRYPTOPP_CONSTANT(PERSONALIZATIONSIZE = BLAKE2_Info<true>::PERSONALIZATIONSIZE)

	BLAKE2_ParameterBlock()
	{
		memset(this, 0x00, sizeof(*this));
		digestLength = DIGESTSIZE;
		fanout = depth = 1;
	}

	BLAKE2_ParameterBlock(size_t digestSize)
	{
		CRYPTOPP_ASSERT(digestSize <= DIGESTSIZE);
		memset(this, 0x00, sizeof(*this));
		digestLength = (byte)digestSize;
		fanout = depth = 1;
	}

	BLAKE2_ParameterBlock(size_t digestSize, size_t keyLength, const byte* salt, size_t saltLength,
		const byte* personalization, size_t personalizationLength);

	byte digestLength;
	byte keyLength, fanout, depth;
	byte leafLength[4];
	byte nodeOffset[8];
	byte nodeDepth, innerLength, rfu[14];
	byte salt[SALTSIZE];
	byte personalization[PERSONALIZATIONSIZE];
};


template<>
struct CRYPTOPP_NO_VTABLE BLAKE2_ParameterBlock<false>
{
	CRYPTOPP_CONSTANT(SALTSIZE = BLAKE2_Info<false>::SALTSIZE)
	CRYPTOPP_CONSTANT(DIGESTSIZE = BLAKE2_Info<false>::DIGESTSIZE)
	CRYPTOPP_CONSTANT(PERSONALIZATIONSIZE = BLAKE2_Info<false>::PERSONALIZATIONSIZE)

	BLAKE2_ParameterBlock()
	{
		memset(this, 0x00, sizeof(*this));
		digestLength = DIGESTSIZE;
		fanout = depth = 1;
	}

	BLAKE2_ParameterBlock(size_t digestSize)
	{
		CRYPTOPP_ASSERT(digestSize <= DIGESTSIZE);
		memset(this, 0x00, sizeof(*this));
		digestLength = (byte)digestSize;
		fanout = depth = 1;
	}

	BLAKE2_ParameterBlock(size_t digestSize, size_t keyLength, const byte* salt, size_t saltLength,
		const byte* personalization, size_t personalizationLength);

	byte digestLength;
	byte keyLength, fanout, depth;
	byte leafLength[4];
	byte nodeOffset[6];
	byte nodeDepth, innerLength;
	byte salt[SALTSIZE];
	byte personalization[PERSONALIZATIONSIZE];
};








template <class W, bool T_64bit>
struct CRYPTOPP_NO_VTABLE BLAKE2_State
{
	CRYPTOPP_CONSTANT(BLOCKSIZE = BLAKE2_Info<T_64bit>::BLOCKSIZE)

	BLAKE2_State()
	{
		
		h[0]=h[1]=h[2]=h[3]=h[4]=h[5]=h[6]=h[7] = 0;
		t[0]=t[1]=f[0]=f[1] = 0;
		length = 0;
	}

	
	W h[8], t[2], f[2];
	byte  buffer[BLOCKSIZE];
	size_t length;
};








template <class W, bool T_64bit>
class BLAKE2_Base : public SimpleKeyingInterfaceImpl<MessageAuthenticationCode, BLAKE2_Info<T_64bit> >
{
public:
	CRYPTOPP_CONSTANT(DEFAULT_KEYLENGTH = BLAKE2_Info<T_64bit>::DEFAULT_KEYLENGTH)
	CRYPTOPP_CONSTANT(MIN_KEYLENGTH = BLAKE2_Info<T_64bit>::MIN_KEYLENGTH)
	CRYPTOPP_CONSTANT(MAX_KEYLENGTH = BLAKE2_Info<T_64bit>::MAX_KEYLENGTH)

	CRYPTOPP_CONSTANT(DIGESTSIZE = BLAKE2_Info<T_64bit>::DIGESTSIZE)
	CRYPTOPP_CONSTANT(BLOCKSIZE = BLAKE2_Info<T_64bit>::BLOCKSIZE)
	CRYPTOPP_CONSTANT(SALTSIZE = BLAKE2_Info<T_64bit>::SALTSIZE)
	CRYPTOPP_CONSTANT(PERSONALIZATIONSIZE = BLAKE2_Info<T_64bit>::PERSONALIZATIONSIZE)

	typedef BLAKE2_State<W, T_64bit> State;
	typedef BLAKE2_ParameterBlock<T_64bit> ParameterBlock;
	typedef SecBlock<State, AllocatorWithCleanup<State, true> > AlignedState;
	typedef SecBlock<ParameterBlock, AllocatorWithCleanup<ParameterBlock, true> > AlignedParameterBlock;

	virtual ~BLAKE2_Base() {}

	
	
	CRYPTOPP_CONSTEXPR static const char *StaticAlgorithmName() {return BLAKE2_Info<T_64bit>::StaticAlgorithmName();}

	
	
	
	
	
	std::string AlgorithmName() const {return std::string(StaticAlgorithmName()) + "-" + IntToString(this->DigestSize()*8);}

	unsigned int DigestSize() const {return m_digestSize;}
	unsigned int OptimalDataAlignment() const {return (CRYPTOPP_BOOL_ALIGN16 ? 16 : GetAlignmentOf<W>());}

	void Update(const byte *input, size_t length);
	void Restart();

	
	
	
	
	void Restart(const BLAKE2_ParameterBlock<T_64bit>& block, const W counter[2]);

	
	
	
	
	
	
	void SetTreeMode(bool mode) {m_treeMode=mode;}

	
	
	
	bool GetTreeMode() const {return m_treeMode;}

	void TruncatedFinal(byte *hash, size_t size);

protected:
	BLAKE2_Base();
	BLAKE2_Base(bool treeMode, unsigned int digestSize);
	BLAKE2_Base(const byte *key, size_t keyLength, const byte* salt, size_t saltLength,
		const byte* personalization, size_t personalizationLength,
		bool treeMode, unsigned int digestSize);

	
	void Compress(const byte *input);
	inline void IncrementCounter(size_t count=BLOCKSIZE);

	void UncheckedSetKey(const byte* key, unsigned int length, const CryptoPP::NameValuePairs& params);

private:
	AlignedState m_state;
	AlignedParameterBlock m_block;
	AlignedSecByteBlock m_key;
	word32 m_digestSize;
	bool m_treeMode;
};










class BLAKE2b : public BLAKE2_Base<word64, true>
{
public:
	typedef BLAKE2_Base<word64, true> ThisBase; 
	typedef BLAKE2_ParameterBlock<true> ParameterBlock;
	CRYPTOPP_COMPILE_ASSERT(sizeof(ParameterBlock) == 64);

	
	
	
	BLAKE2b(bool treeMode=false, unsigned int digestSize = DIGESTSIZE) : ThisBase(treeMode, digestSize) {}

	
	
	
	
	
	
	
	
	
	BLAKE2b(const byte *key, size_t keyLength, const byte* salt = NULL, size_t saltLength = 0,
		const byte* personalization = NULL, size_t personalizationLength = 0,
		bool treeMode=false, unsigned int digestSize = DIGESTSIZE)
		: ThisBase(key, keyLength, salt, saltLength, personalization, personalizationLength, treeMode, digestSize) {}
};










class BLAKE2s : public BLAKE2_Base<word32, false>
{
public:
	typedef BLAKE2_Base<word32, false> ThisBase; 
	typedef BLAKE2_ParameterBlock<false> ParameterBlock;
	CRYPTOPP_COMPILE_ASSERT(sizeof(ParameterBlock) == 32);

	
	
	
	BLAKE2s(bool treeMode=false, unsigned int digestSize = DIGESTSIZE) : ThisBase(treeMode, digestSize) {}

	
	
	
	
	
	
	
	
	
	BLAKE2s(const byte *key, size_t keyLength, const byte* salt = NULL, size_t saltLength = 0,
		const byte* personalization = NULL, size_t personalizationLength = 0,
		bool treeMode=false, unsigned int digestSize = DIGESTSIZE)
		: ThisBase(key, keyLength, salt, saltLength, personalization, personalizationLength, treeMode, digestSize) {}
};

NAMESPACE_END

#endif
