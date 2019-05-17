










#ifndef CRYPTOPP_KECCAK_H
#define CRYPTOPP_KECCAK_H

#include "cryptlib.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)






















class Keccak : public HashTransformation
{
public:
	
	
	
	
	
	
	Keccak(unsigned int digestSize) : m_digestSize(digestSize) {Restart();}
	unsigned int DigestSize() const {return m_digestSize;}
	std::string AlgorithmName() const {return "Keccak-" + IntToString(m_digestSize*8);}
	CRYPTOPP_CONSTEXPR static const char* StaticAlgorithmName() { return "Keccak"; }
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
class Keccak_Final : public Keccak
{
public:
	CRYPTOPP_CONSTANT(DIGESTSIZE = T_DigestSize)
	CRYPTOPP_CONSTANT(BLOCKSIZE = 200 - 2 * DIGESTSIZE)

		
	Keccak_Final() : Keccak(DIGESTSIZE) {}
	static std::string StaticAlgorithmName() { return "Keccak-" + IntToString(DIGESTSIZE * 8); }
	unsigned int BlockSize() const { return BLOCKSIZE; }
private:
	CRYPTOPP_COMPILE_ASSERT(BLOCKSIZE < 200); 
	CRYPTOPP_COMPILE_ASSERT(BLOCKSIZE > (int)T_DigestSize); 
};




typedef Keccak_Final<28> Keccak_224;



typedef Keccak_Final<32> Keccak_256;



typedef Keccak_Final<48> Keccak_384;



typedef Keccak_Final<64> Keccak_512;

NAMESPACE_END

#endif
