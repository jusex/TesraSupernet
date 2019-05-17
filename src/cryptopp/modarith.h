




#ifndef CRYPTOPP_MODARITH_H
#define CRYPTOPP_MODARITH_H



#include "cryptlib.h"
#include "integer.h"
#include "algebra.h"
#include "secblock.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

CRYPTOPP_DLL_TEMPLATE_CLASS AbstractGroup<Integer>;
CRYPTOPP_DLL_TEMPLATE_CLASS AbstractRing<Integer>;
CRYPTOPP_DLL_TEMPLATE_CLASS AbstractEuclideanDomain<Integer>;












class CRYPTOPP_DLL ModularArithmetic : public AbstractRing<Integer>
{
public:

	typedef int RandomizationParameter;
	typedef Integer Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~ModularArithmetic() {}
#endif

	
	
	ModularArithmetic(const Integer &modulus = Integer::One())
		: AbstractRing<Integer>(), m_modulus(modulus), m_result((word)0, modulus.reg.size()) {}

	
	
	ModularArithmetic(const ModularArithmetic &ma)
		: AbstractRing<Integer>(), m_modulus(ma.m_modulus), m_result((word)0, ma.m_modulus.reg.size()) {}

	
	
	ModularArithmetic(BufferedTransformation &bt);	

	
	
	
	
	virtual ModularArithmetic * Clone() const {return new ModularArithmetic(*this);}

	
	
	void DEREncode(BufferedTransformation &bt) const;

	
	
	
	void DEREncodeElement(BufferedTransformation &out, const Element &a) const;

	
	
	
	void BERDecodeElement(BufferedTransformation &in, Element &a) const;

	
	
	const Integer& GetModulus() const {return m_modulus;}

	
	
	void SetModulus(const Integer &newModulus)
		{m_modulus = newModulus; m_result.reg.resize(m_modulus.reg.size());}

	
	
	virtual bool IsMontgomeryRepresentation() const {return false;}

	
	
	
	
	
	virtual Integer ConvertIn(const Integer &a) const
		{return a%m_modulus;}

	
	
	
	
	
	virtual Integer ConvertOut(const Integer &a) const
		{return a;}

	
	
	const Integer& Half(const Integer &a) const;

	
	
	
	
	
	bool Equal(const Integer &a, const Integer &b) const
		{return a==b;}

	
	
	const Integer& Identity() const
		{return Integer::Zero();}

	
	
	
	
	const Integer& Add(const Integer &a, const Integer &b) const;

	
	
	
	
	Integer& Accumulate(Integer &a, const Integer &b) const;

	
	
	
	const Integer& Inverse(const Integer &a) const;

	
	
	
	
	const Integer& Subtract(const Integer &a, const Integer &b) const;

	
	
	
	
	Integer& Reduce(Integer &a, const Integer &b) const;

	
	
	
	
	const Integer& Double(const Integer &a) const
		{return Add(a, a);}

	
	
	
	const Integer& MultiplicativeIdentity() const
		{return Integer::One();}

	
	
	
	
	
	const Integer& Multiply(const Integer &a, const Integer &b) const
		{return m_result1 = a*b%m_modulus;}

	
	
	
	
	const Integer& Square(const Integer &a) const
		{return m_result1 = a.Squared()%m_modulus;}

	
	
	
	bool IsUnit(const Integer &a) const
		{return Integer::Gcd(a, m_modulus).IsUnit();}

	
	
	
	
	const Integer& MultiplicativeInverse(const Integer &a) const
		{return m_result1 = a.InverseMod(m_modulus);}

	
	
	
	
	
	const Integer& Divide(const Integer &a, const Integer &b) const
		{return Multiply(a, MultiplicativeInverse(b));}

	
	
	
	
	
	
	Integer CascadeExponentiate(const Integer &x, const Integer &e1, const Integer &y, const Integer &e2) const;

	
	
	
	
	
	
	
	
	
	
	void SimultaneousExponentiate(Element *results, const Element &base, const Integer *exponents, unsigned int exponentsCount) const;

	
	
	unsigned int MaxElementBitLength() const
		{return (m_modulus-1).BitCount();}

	
	
	unsigned int MaxElementByteLength() const
		{return (m_modulus-1).ByteCount();}

	
	
	
	
	
	
	
	Element RandomElement( RandomNumberGenerator &rng , const RandomizationParameter &ignore_for_now = 0) const
		
	{
		CRYPTOPP_UNUSED(ignore_for_now);
		return Element(rng, Integer::Zero(), m_modulus - Integer::One()) ;
	}

	
	
	
	
	bool operator==(const ModularArithmetic &rhs) const
		{return m_modulus == rhs.m_modulus;}

	static const RandomizationParameter DefaultRandomizationParameter ;

protected:
	Integer m_modulus;
	mutable Integer m_result, m_result1;
};














class CRYPTOPP_DLL MontgomeryRepresentation : public ModularArithmetic
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~MontgomeryRepresentation() {}
#endif

	
	
	
	MontgomeryRepresentation(const Integer &modulus);

	
	
	
	
	virtual ModularArithmetic * Clone() const {return new MontgomeryRepresentation(*this);}

	bool IsMontgomeryRepresentation() const {return true;}

	Integer ConvertIn(const Integer &a) const
		{return (a<<(WORD_BITS*m_modulus.reg.size()))%m_modulus;}

	Integer ConvertOut(const Integer &a) const;

	const Integer& MultiplicativeIdentity() const
		{return m_result1 = Integer::Power2(WORD_BITS*m_modulus.reg.size())%m_modulus;}

	const Integer& Multiply(const Integer &a, const Integer &b) const;

	const Integer& Square(const Integer &a) const;

	const Integer& MultiplicativeInverse(const Integer &a) const;

	Integer CascadeExponentiate(const Integer &x, const Integer &e1, const Integer &y, const Integer &e2) const
		{return AbstractRing<Integer>::CascadeExponentiate(x, e1, y, e2);}

	void SimultaneousExponentiate(Element *results, const Element &base, const Integer *exponents, unsigned int exponentsCount) const
		{AbstractRing<Integer>::SimultaneousExponentiate(results, base, exponents, exponentsCount);}

private:
	Integer m_u;
	mutable IntegerSecBlock m_workspace;
};

NAMESPACE_END

#endif
