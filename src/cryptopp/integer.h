










#ifndef CRYPTOPP_INTEGER_H
#define CRYPTOPP_INTEGER_H

#include "cryptlib.h"
#include "secblock.h"
#include "stdcpp.h"

#include <iosfwd>

NAMESPACE_BEGIN(CryptoPP)



struct InitializeInteger
{
	InitializeInteger();
};


#if defined(CRYPTOPP_WORD128_AVAILABLE)
typedef SecBlock<word, AllocatorWithCleanup<word, true> > IntegerSecBlock;
#else
typedef SecBlock<word, AllocatorWithCleanup<word, CRYPTOPP_BOOL_X86> > IntegerSecBlock;
#endif









class CRYPTOPP_DLL Integer : private InitializeInteger, public ASN1Object
{
public:
	
	
		
		class DivideByZero : public Exception
		{
		public:
			DivideByZero() : Exception(OTHER_ERROR, "Integer: division by zero") {}
		};

		
		
		class RandomNumberNotFound : public Exception
		{
		public:
			RandomNumberNotFound() : Exception(OTHER_ERROR, "Integer: no integer satisfies the given parameters") {}
		};

		
		
		
		
		enum Sign {
			
			POSITIVE=0,
			
			NEGATIVE=1};

		
		
		
		
		enum Signedness {
			
			UNSIGNED,
			
			SIGNED};

		
		
		enum RandomNumberType {
			
			ANY,
			
			PRIME};
	

	
	
		
		Integer();

		
		Integer(const Integer& t);

		
		Integer(signed long value);

		
		
		
		Integer(Sign sign, lword value);

		
		
		
		
		Integer(Sign sign, word highWord, word lowWord);

		
		
		
		
		
		
		
		explicit Integer(const char *str, ByteOrder order = BIG_ENDIAN_ORDER);

		
		
		
		
		
		
		
		explicit Integer(const wchar_t *str, ByteOrder order = BIG_ENDIAN_ORDER);

		
		
		
		
		
		
		
		Integer(const byte *encodedInteger, size_t byteCount, Signedness sign=UNSIGNED, ByteOrder order = BIG_ENDIAN_ORDER);

		
		
		
		
		
		
		
		Integer(BufferedTransformation &bt, size_t byteCount, Signedness sign=UNSIGNED, ByteOrder order = BIG_ENDIAN_ORDER);

		
		
		explicit Integer(BufferedTransformation &bt);

		
		
		
		
		Integer(RandomNumberGenerator &rng, size_t bitCount);

		
		
		
		static const Integer & CRYPTOPP_API Zero();
		
		
		
		static const Integer & CRYPTOPP_API One();
		
		
		
		static const Integer & CRYPTOPP_API Two();

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		Integer(RandomNumberGenerator &rng, const Integer &min, const Integer &max, RandomNumberType rnType=ANY, const Integer &equiv=Zero(), const Integer &mod=One());

		
		
		
		static Integer CRYPTOPP_API Power2(size_t e);
	

	
	
		
		
		
		size_t MinEncodedSize(Signedness sign=UNSIGNED) const;

		
		
		
		
		
		
		
		void Encode(byte *output, size_t outputLen, Signedness sign=UNSIGNED) const;

		
		
		
		
		
		
		
		void Encode(BufferedTransformation &bt, size_t outputLen, Signedness sign=UNSIGNED) const;

		
		
		
		
		void DEREncode(BufferedTransformation &bt) const;

		
		
		
		void DEREncodeAsOctetString(BufferedTransformation &bt, size_t length) const;

		
		
		
		
		
		
		size_t OpenPGPEncode(byte *output, size_t bufferSize) const;

		
		
		
		
		
		size_t OpenPGPEncode(BufferedTransformation &bt) const;

		
		
		
		
		void Decode(const byte *input, size_t inputLen, Signedness sign=UNSIGNED);

		
		
		
		
		
		void Decode(BufferedTransformation &bt, size_t inputLen, Signedness sign=UNSIGNED);

		
		
		
		void BERDecode(const byte *input, size_t inputLen);

		
		
		void BERDecode(BufferedTransformation &bt);

		
		
		
		void BERDecodeAsOctetString(BufferedTransformation &bt, size_t length);

		
		class OpenPGPDecodeErr : public Exception
		{
		public:
			OpenPGPDecodeErr() : Exception(INVALID_DATA_FORMAT, "OpenPGP decode error") {}
		};

		
		
		
		void OpenPGPDecode(const byte *input, size_t inputLen);
		
		
		void OpenPGPDecode(BufferedTransformation &bt);
	

	
	
		
		
		
		bool IsConvertableToLong() const;
		
		
		
		signed long ConvertToLong() const;

		
		
		unsigned int BitCount() const;
		
		
		unsigned int ByteCount() const;
		
		
		unsigned int WordCount() const;

		
		
		bool GetBit(size_t i) const;
		
		
		byte GetByte(size_t i) const;
		
		
		lword GetBits(size_t i, size_t n) const;

		
		
		bool IsZero() const {return !*this;}
		
		
		bool NotZero() const {return !IsZero();}
		
		
		bool IsNegative() const {return sign == NEGATIVE;}
		
		
		bool NotNegative() const {return !IsNegative();}
		
		
		bool IsPositive() const {return NotNegative() && NotZero();}
		
		
		bool NotPositive() const {return !IsPositive();}
		
		
		bool IsEven() const {return GetBit(0) == 0;}
		
		
		bool IsOdd() const	{return GetBit(0) == 1;}
	

	
	
		
		Integer&  operator=(const Integer& t);

		
		Integer&  operator+=(const Integer& t);
		
		Integer&  operator-=(const Integer& t);
		
		
		Integer&  operator*=(const Integer& t)	{return *this = Times(t);}
		
		Integer&  operator/=(const Integer& t)	{return *this = DividedBy(t);}
		
		
		Integer&  operator%=(const Integer& t)	{return *this = Modulo(t);}
		
		Integer&  operator/=(word t)  {return *this = DividedBy(t);}
		
		
		Integer&  operator%=(word t)  {return *this = Integer(POSITIVE, 0, Modulo(t));}

		
		Integer&  operator<<=(size_t);
		
		Integer&  operator>>=(size_t);

		
		
		
		
		void Randomize(RandomNumberGenerator &rng, size_t bitCount);

		
		
		
		
		
		void Randomize(RandomNumberGenerator &rng, const Integer &min, const Integer &max);

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		bool Randomize(RandomNumberGenerator &rng, const Integer &min, const Integer &max, RandomNumberType rnType, const Integer &equiv=Zero(), const Integer &mod=One());

		bool GenerateRandomNoThrow(RandomNumberGenerator &rng, const NameValuePairs &params = g_nullNameValuePairs);
		void GenerateRandom(RandomNumberGenerator &rng, const NameValuePairs &params = g_nullNameValuePairs)
		{
			if (!GenerateRandomNoThrow(rng, params))
				throw RandomNumberNotFound();
		}

		
		
		void SetBit(size_t n, bool value=1);

		
		
		void SetByte(size_t n, byte value);

		
		void Negate();

		
		void SetPositive() {sign = POSITIVE;}

		
		void SetNegative() {if (!!(*this)) sign = NEGATIVE;}

		
		void swap(Integer &a);
	

	
	
		
		bool		operator!() const;
		
		Integer 	operator+() const {return *this;}
		
		Integer 	operator-() const;
		
		Integer&	operator++();
		
		Integer&	operator--();
		
		Integer 	operator++(int) {Integer temp = *this; ++*this; return temp;}
		
		Integer 	operator--(int) {Integer temp = *this; --*this; return temp;}
	

	
	
		
		
		
		
		
		int Compare(const Integer& a) const;

		
		Integer Plus(const Integer &b) const;
		
		Integer Minus(const Integer &b) const;
		
		
		Integer Times(const Integer &b) const;
		
		Integer DividedBy(const Integer &b) const;
		
		
		Integer Modulo(const Integer &b) const;
		
		Integer DividedBy(word b) const;
		
		
		word Modulo(word b) const;

		
		Integer operator>>(size_t n) const	{return Integer(*this)>>=n;}
		
		Integer operator<<(size_t n) const	{return Integer(*this)<<=n;}
	

	
	
		
		Integer AbsoluteValue() const;
		
		Integer Doubled() const {return Plus(*this);}
		
		
		Integer Squared() const {return Times(*this);}
		
		Integer SquareRoot() const;
		
		bool IsSquare() const;

		
		bool IsUnit() const;
		
		Integer MultiplicativeInverse() const;

		
		static void CRYPTOPP_API Divide(Integer &r, Integer &q, const Integer &a, const Integer &d);
		
		static void CRYPTOPP_API Divide(word &r, Integer &q, const Integer &a, word d);

		
		static void CRYPTOPP_API DivideByPowerOf2(Integer &r, Integer &q, const Integer &a, unsigned int n);

		
		static Integer CRYPTOPP_API Gcd(const Integer &a, const Integer &n);
		
		
		Integer InverseMod(const Integer &n) const;
		
		
		word InverseMod(word n) const;
	

	
	
		
		
		
		
		friend CRYPTOPP_DLL std::istream& CRYPTOPP_API operator>>(std::istream& in, Integer &a);
		
		
		
		
		
		
		
		
		
		
		
		friend CRYPTOPP_DLL std::ostream& CRYPTOPP_API operator<<(std::ostream& out, const Integer &a);
	

#ifndef CRYPTOPP_DOXYGEN_PROCESSING
	
	CRYPTOPP_DLL friend Integer CRYPTOPP_API a_times_b_mod_c(const Integer &x, const Integer& y, const Integer& m);
	
	CRYPTOPP_DLL friend Integer CRYPTOPP_API a_exp_b_mod_c(const Integer &x, const Integer& e, const Integer& m);
#endif

private:

	Integer(word value, size_t length);
	int PositiveCompare(const Integer &t) const;

	IntegerSecBlock reg;
	Sign sign;

#ifndef CRYPTOPP_DOXYGEN_PROCESSING
	friend class ModularArithmetic;
	friend class MontgomeryRepresentation;
	friend class HalfMontgomeryRepresentation;

	friend void PositiveAdd(Integer &sum, const Integer &a, const Integer &b);
	friend void PositiveSubtract(Integer &diff, const Integer &a, const Integer &b);
	friend void PositiveMultiply(Integer &product, const Integer &a, const Integer &b);
	friend void PositiveDivide(Integer &remainder, Integer &quotient, const Integer &dividend, const Integer &divisor);
#endif
};


inline bool operator==(const CryptoPP::Integer& a, const CryptoPP::Integer& b) {return a.Compare(b)==0;}

inline bool operator!=(const CryptoPP::Integer& a, const CryptoPP::Integer& b) {return a.Compare(b)!=0;}

inline bool operator> (const CryptoPP::Integer& a, const CryptoPP::Integer& b) {return a.Compare(b)> 0;}

inline bool operator>=(const CryptoPP::Integer& a, const CryptoPP::Integer& b) {return a.Compare(b)>=0;}

inline bool operator< (const CryptoPP::Integer& a, const CryptoPP::Integer& b) {return a.Compare(b)< 0;}

inline bool operator<=(const CryptoPP::Integer& a, const CryptoPP::Integer& b) {return a.Compare(b)<=0;}

inline CryptoPP::Integer operator+(const CryptoPP::Integer &a, const CryptoPP::Integer &b) {return a.Plus(b);}

inline CryptoPP::Integer operator-(const CryptoPP::Integer &a, const CryptoPP::Integer &b) {return a.Minus(b);}


inline CryptoPP::Integer operator*(const CryptoPP::Integer &a, const CryptoPP::Integer &b) {return a.Times(b);}

inline CryptoPP::Integer operator/(const CryptoPP::Integer &a, const CryptoPP::Integer &b) {return a.DividedBy(b);}


inline CryptoPP::Integer operator%(const CryptoPP::Integer &a, const CryptoPP::Integer &b) {return a.Modulo(b);}

inline CryptoPP::Integer operator/(const CryptoPP::Integer &a, CryptoPP::word b) {return a.DividedBy(b);}


inline CryptoPP::word    operator%(const CryptoPP::Integer &a, CryptoPP::word b) {return a.Modulo(b);}

NAMESPACE_END

#ifndef __BORLANDC__
NAMESPACE_BEGIN(std)
inline void swap(CryptoPP::Integer &a, CryptoPP::Integer &b)
{
	a.swap(b);
}
NAMESPACE_END
#endif

#endif
