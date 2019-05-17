




#ifndef CRYPTOPP_SECKEY_H
#define CRYPTOPP_SECKEY_H

#include "config.h"

#if CRYPTOPP_MSC_VERSION
# pragma warning(push)
# pragma warning(disable: 4189)
#endif

#include "cryptlib.h"
#include "misc.h"
#include "simple.h"

NAMESPACE_BEGIN(CryptoPP)




inline CipherDir ReverseCipherDir(CipherDir dir)
{
	return (dir == ENCRYPTION) ? DECRYPTION : ENCRYPTION;
}




template <unsigned int N>
class FixedBlockSize
{
public:
	
	CRYPTOPP_CONSTANT(BLOCKSIZE = N)
};






template <unsigned int R>
class FixedRounds
{
public:
	
	CRYPTOPP_CONSTANT(ROUNDS = R)
};






template <unsigned int D, unsigned int N=1, unsigned int M=INT_MAX>		
class VariableRounds
{
public:
	
	CRYPTOPP_CONSTANT(DEFAULT_ROUNDS = D)
	
	CRYPTOPP_CONSTANT(MIN_ROUNDS = N)
	
	CRYPTOPP_CONSTANT(MAX_ROUNDS = M)
	
	
	
	
	CRYPTOPP_CONSTEXPR static unsigned int StaticGetDefaultRounds(size_t keylength)
	{
		
		
#if defined(CRYPTOPP_CXX11_CONSTEXPR)
		return CRYPTOPP_UNUSED(keylength), static_cast<unsigned int>(DEFAULT_ROUNDS);
#else
		CRYPTOPP_UNUSED(keylength);
		return static_cast<unsigned int>(DEFAULT_ROUNDS);
#endif
	}

protected:
	
	
	
	
	
	inline void ThrowIfInvalidRounds(int rounds, const Algorithm *alg)
	{
		if (M == INT_MAX) 
		{
			if (rounds < MIN_ROUNDS)
				throw InvalidRounds(alg ? alg->AlgorithmName() : std::string("VariableRounds"), rounds);
		}
		else
		{
			if (rounds < MIN_ROUNDS || rounds > MAX_ROUNDS)
				throw InvalidRounds(alg ? alg->AlgorithmName() : std::string("VariableRounds"), rounds);
		}
	}

	
	
	
	
	
	
	inline unsigned int GetRoundsAndThrowIfInvalid(const NameValuePairs &param, const Algorithm *alg)
	{
		int rounds = param.GetIntValueWithDefault("Rounds", DEFAULT_ROUNDS);
		ThrowIfInvalidRounds(rounds, alg);
		return (unsigned int)rounds;
	}
};









template <unsigned int N, unsigned int IV_REQ = SimpleKeyingInterface::NOT_RESYNCHRONIZABLE, unsigned int IV_L = 0>
class FixedKeyLength
{
public:
	
	
	CRYPTOPP_CONSTANT(KEYLENGTH=N)
	
	
	CRYPTOPP_CONSTANT(MIN_KEYLENGTH=N)
	
	
	CRYPTOPP_CONSTANT(MAX_KEYLENGTH=N)
	
	
	CRYPTOPP_CONSTANT(DEFAULT_KEYLENGTH=N)
	
	
	
	CRYPTOPP_CONSTANT(IV_REQUIREMENT = IV_REQ)
	
	
	CRYPTOPP_CONSTANT(IV_LENGTH = IV_L)
	
	
	
	
	CRYPTOPP_CONSTEXPR static size_t CRYPTOPP_API StaticGetValidKeyLength(size_t keylength)
	{
		
		
#if defined(CRYPTOPP_CXX11_CONSTEXPR)
		return CRYPTOPP_UNUSED(keylength), static_cast<size_t>(KEYLENGTH);
#else
		CRYPTOPP_UNUSED(keylength);
		return static_cast<size_t>(KEYLENGTH);
#endif
	}
};










template <unsigned int D, unsigned int N, unsigned int M, unsigned int Q = 1, unsigned int IV_REQ = SimpleKeyingInterface::NOT_RESYNCHRONIZABLE, unsigned int IV_L = 0>
class VariableKeyLength
{
	
	CRYPTOPP_COMPILE_ASSERT(Q > 0);
	CRYPTOPP_COMPILE_ASSERT(N % Q == 0);
	CRYPTOPP_COMPILE_ASSERT(M % Q == 0);
	CRYPTOPP_COMPILE_ASSERT(N < M);
	CRYPTOPP_COMPILE_ASSERT(D >= N);
	CRYPTOPP_COMPILE_ASSERT(M >= D);

public:
	
	
	CRYPTOPP_CONSTANT(MIN_KEYLENGTH=N)
	
	
	CRYPTOPP_CONSTANT(MAX_KEYLENGTH=M)
	
	
	CRYPTOPP_CONSTANT(DEFAULT_KEYLENGTH=D)
	
	
	CRYPTOPP_CONSTANT(KEYLENGTH_MULTIPLE=Q)
	
	
	
	CRYPTOPP_CONSTANT(IV_REQUIREMENT=IV_REQ)
	
	
	CRYPTOPP_CONSTANT(IV_LENGTH=IV_L)
	
	
	
	
	
	
	
	
	
	static size_t CRYPTOPP_API StaticGetValidKeyLength(size_t keylength)
	{
		if (keylength < (size_t)MIN_KEYLENGTH)
			return MIN_KEYLENGTH;
		else if (keylength > (size_t)MAX_KEYLENGTH)
			return (size_t)MAX_KEYLENGTH;
		else
		{
			keylength += KEYLENGTH_MULTIPLE-1;
			return keylength - keylength%KEYLENGTH_MULTIPLE;
		}
	}
};







template <class T, unsigned int IV_REQ = SimpleKeyingInterface::NOT_RESYNCHRONIZABLE, unsigned int IV_L = 0>
class SameKeyLengthAs
{
public:
	
	
	CRYPTOPP_CONSTANT(MIN_KEYLENGTH=T::MIN_KEYLENGTH)
	
	
	CRYPTOPP_CONSTANT(MAX_KEYLENGTH=T::MAX_KEYLENGTH)
	
	
	CRYPTOPP_CONSTANT(DEFAULT_KEYLENGTH=T::DEFAULT_KEYLENGTH)
	
	
	
	CRYPTOPP_CONSTANT(IV_REQUIREMENT=IV_REQ)
	
	
	CRYPTOPP_CONSTANT(IV_LENGTH=IV_L)
	
	
	
	
	
	
	
	
	CRYPTOPP_CONSTEXPR static size_t CRYPTOPP_API StaticGetValidKeyLength(size_t keylength)
		{return T::StaticGetValidKeyLength(keylength);}
};










template <class BASE, class INFO = BASE>
class CRYPTOPP_NO_VTABLE SimpleKeyingInterfaceImpl : public BASE
{
public:
	
	
	size_t MinKeyLength() const
		{return INFO::MIN_KEYLENGTH;}

	
	
	size_t MaxKeyLength() const
		{return (size_t)INFO::MAX_KEYLENGTH;}

	
	
	size_t DefaultKeyLength() const
		{return INFO::DEFAULT_KEYLENGTH;}

	
	
	
	
	
	
	
	
	size_t GetValidKeyLength(size_t keylength) const {return INFO::StaticGetValidKeyLength(keylength);}

	
	
	
	SimpleKeyingInterface::IV_Requirement IVRequirement() const
		{return (SimpleKeyingInterface::IV_Requirement)INFO::IV_REQUIREMENT;}

	
	
	unsigned int IVSize() const
		{return INFO::IV_LENGTH;}
};








template <class INFO, class BASE = BlockCipher>
class CRYPTOPP_NO_VTABLE BlockCipherImpl : public AlgorithmImpl<SimpleKeyingInterfaceImpl<TwoBases<BASE, INFO> > >
{
public:
	
	
	unsigned int BlockSize() const {return this->BLOCKSIZE;}
};





template <CipherDir DIR, class BASE>
class BlockCipherFinal : public ClonableImpl<BlockCipherFinal<DIR, BASE>, BASE>
{
public:
	
	
 	BlockCipherFinal() {}

	
	
	
	
	BlockCipherFinal(const byte *key)
		{this->SetKey(key, this->DEFAULT_KEYLENGTH);}

	
	
	
	
	
	BlockCipherFinal(const byte *key, size_t length)
		{this->SetKey(key, length);}

	
	
	
	
	
	
	BlockCipherFinal(const byte *key, size_t length, unsigned int rounds)
		{this->SetKeyWithRounds(key, length, rounds);}

	
	
	
	bool IsForwardTransformation() const {return DIR == ENCRYPTION;}
};









template <class BASE, class INFO = BASE>
class MessageAuthenticationCodeImpl : public AlgorithmImpl<SimpleKeyingInterfaceImpl<BASE, INFO>, INFO>
{
};





template <class BASE>
class MessageAuthenticationCodeFinal : public ClonableImpl<MessageAuthenticationCodeFinal<BASE>, MessageAuthenticationCodeImpl<BASE> >
{
public:
	
	
 	MessageAuthenticationCodeFinal() {}
	
	
	
	
	MessageAuthenticationCodeFinal(const byte *key)
		{this->SetKey(key, this->DEFAULT_KEYLENGTH);}
	
	
	
	
	
	MessageAuthenticationCodeFinal(const byte *key, size_t length)
		{this->SetKey(key, length);}
};









struct BlockCipherDocumentation
{
	
	typedef BlockCipher Encryption;
	
	typedef BlockCipher Decryption;
};









struct SymmetricCipherDocumentation
{
	
	typedef SymmetricCipher Encryption;
	
	typedef SymmetricCipher Decryption;
};






struct AuthenticatedSymmetricCipherDocumentation
{
	
	typedef AuthenticatedSymmetricCipher Encryption;
	
	typedef AuthenticatedSymmetricCipher Decryption;
};

NAMESPACE_END

#if CRYPTOPP_MSC_VERSION
# pragma warning(pop)
#endif

#endif
