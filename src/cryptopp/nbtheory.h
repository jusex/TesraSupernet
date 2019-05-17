




#ifndef CRYPTOPP_NBTHEORY_H
#define CRYPTOPP_NBTHEORY_H

#include "cryptlib.h"
#include "integer.h"
#include "algparam.h"

NAMESPACE_BEGIN(CryptoPP)


CRYPTOPP_DLL const word16 * CRYPTOPP_API GetPrimeTable(unsigned int &size);







CRYPTOPP_DLL Integer CRYPTOPP_API MaurerProvablePrime(RandomNumberGenerator &rng, unsigned int bits);






CRYPTOPP_DLL Integer CRYPTOPP_API MihailescuProvablePrime(RandomNumberGenerator &rng, unsigned int bits);







CRYPTOPP_DLL bool CRYPTOPP_API IsSmallPrime(const Integer &p);





CRYPTOPP_DLL bool CRYPTOPP_API TrialDivision(const Integer &p, unsigned bound);


CRYPTOPP_DLL bool CRYPTOPP_API SmallDivisorsTest(const Integer &p);


CRYPTOPP_DLL bool CRYPTOPP_API IsFermatProbablePrime(const Integer &n, const Integer &b);
CRYPTOPP_DLL bool CRYPTOPP_API IsLucasProbablePrime(const Integer &n);

CRYPTOPP_DLL bool CRYPTOPP_API IsStrongProbablePrime(const Integer &n, const Integer &b);
CRYPTOPP_DLL bool CRYPTOPP_API IsStrongLucasProbablePrime(const Integer &n);



CRYPTOPP_DLL bool CRYPTOPP_API RabinMillerTest(RandomNumberGenerator &rng, const Integer &w, unsigned int rounds);






CRYPTOPP_DLL bool CRYPTOPP_API IsPrime(const Integer &p);









CRYPTOPP_DLL bool CRYPTOPP_API VerifyPrime(RandomNumberGenerator &rng, const Integer &p, unsigned int level = 1);



class CRYPTOPP_DLL PrimeSelector
{
public:
	const PrimeSelector *GetSelectorPointer() const {return this;}
	virtual bool IsAcceptable(const Integer &candidate) const =0;
};











CRYPTOPP_DLL bool CRYPTOPP_API FirstPrime(Integer &p, const Integer &max, const Integer &equiv, const Integer &mod, const PrimeSelector *pSelector);

CRYPTOPP_DLL unsigned int CRYPTOPP_API PrimeSearchInterval(const Integer &max);

CRYPTOPP_DLL AlgorithmParameters CRYPTOPP_API MakeParametersForTwoPrimesOfEqualSize(unsigned int productBitLength);



inline Integer GCD(const Integer &a, const Integer &b)
	{return Integer::Gcd(a,b);}
inline bool RelativelyPrime(const Integer &a, const Integer &b)
	{return Integer::Gcd(a,b) == Integer::One();}
inline Integer LCM(const Integer &a, const Integer &b)
	{return a/Integer::Gcd(a,b)*b;}
inline Integer EuclideanMultiplicativeInverse(const Integer &a, const Integer &b)
	{return a.InverseMod(b);}


CRYPTOPP_DLL Integer CRYPTOPP_API CRT(const Integer &xp, const Integer &p, const Integer &xq, const Integer &q, const Integer &u);



CRYPTOPP_DLL int CRYPTOPP_API Jacobi(const Integer &a, const Integer &b);


CRYPTOPP_DLL Integer CRYPTOPP_API Lucas(const Integer &e, const Integer &p, const Integer &n);

CRYPTOPP_DLL Integer CRYPTOPP_API InverseLucas(const Integer &e, const Integer &m, const Integer &p, const Integer &q, const Integer &u);

inline Integer ModularExponentiation(const Integer &a, const Integer &e, const Integer &m)
	{return a_exp_b_mod_c(a, e, m);}

CRYPTOPP_DLL Integer CRYPTOPP_API ModularSquareRoot(const Integer &a, const Integer &p);




CRYPTOPP_DLL Integer CRYPTOPP_API ModularRoot(const Integer &a, const Integer &dp, const Integer &dq, const Integer &p, const Integer &q, const Integer &u);



CRYPTOPP_DLL bool CRYPTOPP_API SolveModularQuadraticEquation(Integer &r1, Integer &r2, const Integer &a, const Integer &b, const Integer &c, const Integer &p);


CRYPTOPP_DLL unsigned int CRYPTOPP_API DiscreteLogWorkFactor(unsigned int bitlength);
CRYPTOPP_DLL unsigned int CRYPTOPP_API FactoringWorkFactor(unsigned int bitlength);




class CRYPTOPP_DLL PrimeAndGenerator
{
public:
	PrimeAndGenerator() {}
	
	
	
	PrimeAndGenerator(signed int delta, RandomNumberGenerator &rng, unsigned int pbits)
		{Generate(delta, rng, pbits, pbits-1);}
	
	
	PrimeAndGenerator(signed int delta, RandomNumberGenerator &rng, unsigned int pbits, unsigned qbits)
		{Generate(delta, rng, pbits, qbits);}

	void Generate(signed int delta, RandomNumberGenerator &rng, unsigned int pbits, unsigned qbits);

	const Integer& Prime() const {return p;}
	const Integer& SubPrime() const {return q;}
	const Integer& Generator() const {return g;}

private:
	Integer p, q, g;
};

NAMESPACE_END

#endif
