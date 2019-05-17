









#ifndef CRYPTOPP_SHA3_H
#define CRYPTOPP_SHA3_H

#include "cryptlib.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)










class SHA3 : public HashTransformation
{
public:
	
	
	
	
	
	SHA3(unsigned int digestSize) : m_digestSize(digestSize) {Restart();}
	unsigned int DigestSize() const {return m_digestSize;}
	std::string AlgorithmName() const {return "SHA3-" + IntToString(m_digestSize*8);}
	CRYPTOPP_CONSTEXPR static const char* StaticAlgorithmName() { return "SHA3"; }
	unsigned int OptimalDataAlignment() const {return GetAlignmentOf<word64>();}

	void Update(const byte *input, size_t length);
	void Restart();
	void TruncatedFinal(byte *hash, size_t size);

	
protected:
	inline unsigned int r() const {return 200 - 2 * m_digestSize;}

	FixedSizeSecBlock<word64, 25> m_state;
	unsigned int m_digestSize, m_counter;
};





template<unsigned int T_DigestSize>
class SHA3_Final : public SHA3
{
public:
	CRYPTOPP_CONSTANT(DIGESTSIZE = T_DigestSize)
	CRYPTOPP_CONSTANT(BLOCKSIZE = 200 - 2 * DIGESTSIZE)

	
	SHA3_Final() : SHA3(DIGESTSIZE) {}
	static std::string StaticAlgorithmName() { return "SHA3-" + IntToString(DIGESTSIZE * 8); }
	unsigned int BlockSize() const { return BLOCKSIZE; }
private:
	CRYPTOPP_COMPILE_ASSERT(BLOCKSIZE < 200); 
	CRYPTOPP_COMPILE_ASSERT(BLOCKSIZE > (int)T_DigestSize); 
};




typedef SHA3_Final<28> SHA3_224;



typedef SHA3_Final<32> SHA3_256;



typedef SHA3_Final<48> SHA3_384;



typedef SHA3_Final<64> SHA3_512;

NAMESPACE_END

#endif
