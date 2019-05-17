







#ifndef CRYPTOPP_RNG_H
#define CRYPTOPP_RNG_H

#include "cryptlib.h"
#include "filters.h"
#include "smartptr.h"

NAMESPACE_BEGIN(CryptoPP)





class LC_RNG : public RandomNumberGenerator
{
public:
	
	
	LC_RNG(word32 init_seed)
		: seed(init_seed) {}

	void GenerateBlock(byte *output, size_t size);

	word32 GetSeed() {return seed;}

private:
	word32 seed;

	static const word32 m;
	static const word32 q;
	static const word16 a;
	static const word16 r;
};






class CRYPTOPP_DLL X917RNG : public RandomNumberGenerator, public NotCopyable
{
public:
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	X917RNG(BlockTransformation *cipher, const byte *seed, const byte *deterministicTimeVector = 0);

	void GenerateIntoBufferedTransformation(BufferedTransformation &target, const std::string &channel, lword size);

private:
	member_ptr<BlockTransformation> m_cipher;
	const unsigned int m_size;  
	SecByteBlock m_datetime;    
	SecByteBlock m_randseed, m_lastBlock, m_deterministicTimeVector;
};







class MaurerRandomnessTest : public Bufferless<Sink>
{
public:
	
	MaurerRandomnessTest();

	size_t Put2(const byte *inString, size_t length, int messageEnd, bool blocking);

	
	
	
	
	unsigned int BytesNeeded() const {return n >= (Q+K) ? 0 : Q+K-n;}

	
	
	double GetTestValue() const;

private:
	enum {L=8, V=256, Q=2000, K=2000};
	double sum;
	unsigned int n;
	unsigned int tab[V];
};

NAMESPACE_END

#endif
