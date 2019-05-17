





#ifndef CRYPTOPP_BLUMSHUB_H
#define CRYPTOPP_BLUMSHUB_H

#include "cryptlib.h"
#include "modarith.h"
#include "integer.h"

NAMESPACE_BEGIN(CryptoPP)


class PublicBlumBlumShub : public RandomNumberGenerator,
						   public StreamTransformation
{
public:
	PublicBlumBlumShub(const Integer &n, const Integer &seed);

	unsigned int GenerateBit();
	byte GenerateByte();
	void GenerateBlock(byte *output, size_t size);
	void ProcessData(byte *outString, const byte *inString, size_t length);

	bool IsSelfInverting() const {return true;}
	bool IsForwardTransformation() const {return true;}

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~PublicBlumBlumShub() {}
#endif

protected:
	ModularArithmetic modn;
	Integer current;
	word maxBits, bitsLeft;
};


class BlumBlumShub : public PublicBlumBlumShub
{
public:
	
	
	BlumBlumShub(const Integer &p, const Integer &q, const Integer &seed);

	bool IsRandomAccess() const {return true;}
	void Seek(lword index);

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~BlumBlumShub() {}
#endif

protected:
	const Integer p, q;
	const Integer x0;
};

NAMESPACE_END

#endif
