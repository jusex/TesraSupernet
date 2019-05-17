#ifndef CRYPTOPP_GF2N_H
#define CRYPTOPP_GF2N_H



#include "cryptlib.h"
#include "secblock.h"
#include "algebra.h"
#include "misc.h"
#include "asn.h"

#include <iosfwd>

NAMESPACE_BEGIN(CryptoPP)



class CRYPTOPP_DLL PolynomialMod2
{
public:
	
	
		
		class DivideByZero : public Exception
		{
		public:
			DivideByZero() : Exception(OTHER_ERROR, "PolynomialMod2: division by zero") {}
		};

		typedef unsigned int RandomizationParameter;
	

	
	
		
		PolynomialMod2();
		
		PolynomialMod2(const PolynomialMod2& t);

		
		/*! value should be encoded with the least significant bit as coefficient to x^0
			and most significant bit as coefficient to x^(WORD_BITS-1)
			bitLength denotes how much memory to allocate initially
		*/
		PolynomialMod2(word value, size_t bitLength=WORD_BITS);

		
		PolynomialMod2(const byte *encodedPoly, size_t byteCount)
			{Decode(encodedPoly, byteCount);}

		
		PolynomialMod2(BufferedTransformation &encodedPoly, size_t byteCount)
			{Decode(encodedPoly, byteCount);}

		
		PolynomialMod2(RandomNumberGenerator &rng, size_t bitcount)
			{Randomize(rng, bitcount);}

		
		static PolynomialMod2 CRYPTOPP_API Monomial(size_t i);
		
		static PolynomialMod2 CRYPTOPP_API Trinomial(size_t t0, size_t t1, size_t t2);
		
		static PolynomialMod2 CRYPTOPP_API Pentanomial(size_t t0, size_t t1, size_t t2, size_t t3, size_t t4);
		
		static PolynomialMod2 CRYPTOPP_API AllOnes(size_t n);

		
		static const PolynomialMod2 & CRYPTOPP_API Zero();
		
		static const PolynomialMod2 & CRYPTOPP_API One();
	

	
	
		
		
		unsigned int MinEncodedSize() const {return STDMAX(1U, ByteCount());}

		
		/*! if outputLen < MinEncodedSize, the most significant bytes will be dropped
			if outputLen > MinEncodedSize, the most significant bytes will be padded
		*/
		void Encode(byte *output, size_t outputLen) const;
		
		void Encode(BufferedTransformation &bt, size_t outputLen) const;

		
		void Decode(const byte *input, size_t inputLen);
		
		
		void Decode(BufferedTransformation &bt, size_t inputLen);

		
		void DEREncodeAsOctetString(BufferedTransformation &bt, size_t length) const;
		
		void BERDecodeAsOctetString(BufferedTransformation &bt, size_t length);
	

	
	
		
		unsigned int BitCount() const;
		
		unsigned int ByteCount() const;
		
		unsigned int WordCount() const;

		
		bool GetBit(size_t n) const {return GetCoefficient(n)!=0;}
		
		byte GetByte(size_t n) const;

		
		signed int Degree() const {return (signed int)(BitCount()-1U);}
		
		unsigned int CoefficientCount() const {return BitCount();}
		
		int GetCoefficient(size_t i) const
			{return (i/WORD_BITS < reg.size()) ? int(reg[i/WORD_BITS] >> (i % WORD_BITS)) & 1 : 0;}
		
		int operator[](unsigned int i) const {return GetCoefficient(i);}

		
		bool IsZero() const {return !*this;}
		
		bool Equals(const PolynomialMod2 &rhs) const;
	

	
	
		
		PolynomialMod2&  operator=(const PolynomialMod2& t);
		
		PolynomialMod2&  operator&=(const PolynomialMod2& t);
		
		PolynomialMod2&  operator^=(const PolynomialMod2& t);
		
		PolynomialMod2&  operator+=(const PolynomialMod2& t) {return *this ^= t;}
		
		PolynomialMod2&  operator-=(const PolynomialMod2& t) {return *this ^= t;}
		
		PolynomialMod2&  operator*=(const PolynomialMod2& t);
		
		PolynomialMod2&  operator/=(const PolynomialMod2& t);
		
		PolynomialMod2&  operator%=(const PolynomialMod2& t);
		
		PolynomialMod2&  operator<<=(unsigned int);
		
		PolynomialMod2&  operator>>=(unsigned int);

		
		void Randomize(RandomNumberGenerator &rng, size_t bitcount);

		
		void SetBit(size_t i, int value = 1);
		
		void SetByte(size_t n, byte value);

		
		void SetCoefficient(size_t i, int value) {SetBit(i, value);}

		
		void swap(PolynomialMod2 &a) {reg.swap(a.reg);}
	

	
	
		
		bool			operator!() const;
		
		PolynomialMod2	operator+() const {return *this;}
		
		PolynomialMod2	operator-() const {return *this;}
	

	
	
		
		PolynomialMod2 And(const PolynomialMod2 &b) const;
		
		PolynomialMod2 Xor(const PolynomialMod2 &b) const;
		
		PolynomialMod2 Plus(const PolynomialMod2 &b) const {return Xor(b);}
		
		PolynomialMod2 Minus(const PolynomialMod2 &b) const {return Xor(b);}
		
		PolynomialMod2 Times(const PolynomialMod2 &b) const;
		
		PolynomialMod2 DividedBy(const PolynomialMod2 &b) const;
		
		PolynomialMod2 Modulo(const PolynomialMod2 &b) const;

		
		PolynomialMod2 operator>>(unsigned int n) const;
		
		PolynomialMod2 operator<<(unsigned int n) const;
	

	
	
		
		unsigned int Parity() const;

		
		bool IsIrreducible() const;

		
		PolynomialMod2 Doubled() const {return Zero();}
		
		PolynomialMod2 Squared() const;

		
		bool IsUnit() const {return Equals(One());}
		
		PolynomialMod2 MultiplicativeInverse() const {return IsUnit() ? One() : Zero();}

		
		static PolynomialMod2 CRYPTOPP_API Gcd(const PolynomialMod2 &a, const PolynomialMod2 &n);
		
		PolynomialMod2 InverseMod(const PolynomialMod2 &) const;

		
		static void CRYPTOPP_API Divide(PolynomialMod2 &r, PolynomialMod2 &q, const PolynomialMod2 &a, const PolynomialMod2 &d);
	

	
	
		
		friend std::ostream& operator<<(std::ostream& out, const PolynomialMod2 &a);
	

private:
	friend class GF2NT;

	SecWordBlock reg;
};


inline bool operator==(const CryptoPP::PolynomialMod2 &a, const CryptoPP::PolynomialMod2 &b)
{return a.Equals(b);}

inline bool operator!=(const CryptoPP::PolynomialMod2 &a, const CryptoPP::PolynomialMod2 &b)
{return !(a==b);}

inline bool operator> (const CryptoPP::PolynomialMod2 &a, const CryptoPP::PolynomialMod2 &b)
{return a.Degree() > b.Degree();}

inline bool operator>=(const CryptoPP::PolynomialMod2 &a, const CryptoPP::PolynomialMod2 &b)
{return a.Degree() >= b.Degree();}

inline bool operator< (const CryptoPP::PolynomialMod2 &a, const CryptoPP::PolynomialMod2 &b)
{return a.Degree() < b.Degree();}

inline bool operator<=(const CryptoPP::PolynomialMod2 &a, const CryptoPP::PolynomialMod2 &b)
{return a.Degree() <= b.Degree();}

inline CryptoPP::PolynomialMod2 operator&(const CryptoPP::PolynomialMod2 &a, const CryptoPP::PolynomialMod2 &b) {return a.And(b);}

inline CryptoPP::PolynomialMod2 operator^(const CryptoPP::PolynomialMod2 &a, const CryptoPP::PolynomialMod2 &b) {return a.Xor(b);}

inline CryptoPP::PolynomialMod2 operator+(const CryptoPP::PolynomialMod2 &a, const CryptoPP::PolynomialMod2 &b) {return a.Plus(b);}

inline CryptoPP::PolynomialMod2 operator-(const CryptoPP::PolynomialMod2 &a, const CryptoPP::PolynomialMod2 &b) {return a.Minus(b);}

inline CryptoPP::PolynomialMod2 operator*(const CryptoPP::PolynomialMod2 &a, const CryptoPP::PolynomialMod2 &b) {return a.Times(b);}

inline CryptoPP::PolynomialMod2 operator/(const CryptoPP::PolynomialMod2 &a, const CryptoPP::PolynomialMod2 &b) {return a.DividedBy(b);}

inline CryptoPP::PolynomialMod2 operator%(const CryptoPP::PolynomialMod2 &a, const CryptoPP::PolynomialMod2 &b) {return a.Modulo(b);}



CRYPTOPP_DLL_TEMPLATE_CLASS AbstractGroup<PolynomialMod2>;
CRYPTOPP_DLL_TEMPLATE_CLASS AbstractRing<PolynomialMod2>;
CRYPTOPP_DLL_TEMPLATE_CLASS AbstractEuclideanDomain<PolynomialMod2>;
CRYPTOPP_DLL_TEMPLATE_CLASS EuclideanDomainOf<PolynomialMod2>;
CRYPTOPP_DLL_TEMPLATE_CLASS QuotientRing<EuclideanDomainOf<PolynomialMod2> >;


class CRYPTOPP_DLL GF2NP : public QuotientRing<EuclideanDomainOf<PolynomialMod2> >
{
public:
	GF2NP(const PolynomialMod2 &modulus);

	virtual GF2NP * Clone() const {return new GF2NP(*this);}
	virtual void DEREncode(BufferedTransformation &bt) const
		{CRYPTOPP_UNUSED(bt); CRYPTOPP_ASSERT(false);}	

	void DEREncodeElement(BufferedTransformation &out, const Element &a) const;
	void BERDecodeElement(BufferedTransformation &in, Element &a) const;

	bool Equal(const Element &a, const Element &b) const
		{CRYPTOPP_ASSERT(a.Degree() < m_modulus.Degree() && b.Degree() < m_modulus.Degree()); return a.Equals(b);}

	bool IsUnit(const Element &a) const
		{CRYPTOPP_ASSERT(a.Degree() < m_modulus.Degree()); return !!a;}

	unsigned int MaxElementBitLength() const
		{return m;}

	unsigned int MaxElementByteLength() const
		{return (unsigned int)BitsToBytes(MaxElementBitLength());}

	Element SquareRoot(const Element &a) const;

	Element HalfTrace(const Element &a) const;

	
	Element SolveQuadraticEquation(const Element &a) const;

protected:
	unsigned int m;
};


class CRYPTOPP_DLL GF2NT : public GF2NP
{
public:
	
	GF2NT(unsigned int t0, unsigned int t1, unsigned int t2);

	GF2NP * Clone() const {return new GF2NT(*this);}
	void DEREncode(BufferedTransformation &bt) const;

	const Element& Multiply(const Element &a, const Element &b) const;

	const Element& Square(const Element &a) const
		{return Reduced(a.Squared());}

	const Element& MultiplicativeInverse(const Element &a) const;

private:
	const Element& Reduced(const Element &a) const;

	unsigned int t0, t1;
	mutable PolynomialMod2 result;
};


class CRYPTOPP_DLL GF2NPP : public GF2NP
{
public:
	
	GF2NPP(unsigned int t0, unsigned int t1, unsigned int t2, unsigned int t3, unsigned int t4)
		: GF2NP(PolynomialMod2::Pentanomial(t0, t1, t2, t3, t4)), t0(t0), t1(t1), t2(t2), t3(t3) {}

	GF2NP * Clone() const {return new GF2NPP(*this);}
	void DEREncode(BufferedTransformation &bt) const;

private:
	unsigned int t0, t1, t2, t3;
};


CRYPTOPP_DLL GF2NP * CRYPTOPP_API BERDecodeGF2NP(BufferedTransformation &bt);

NAMESPACE_END

#ifndef __BORLANDC__
NAMESPACE_BEGIN(std)
template<> inline void swap(CryptoPP::PolynomialMod2 &a, CryptoPP::PolynomialMod2 &b)
{
	a.swap(b);
}
NAMESPACE_END
#endif

#endif
