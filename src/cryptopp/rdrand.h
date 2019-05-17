






#ifndef CRYPTOPP_RDRAND_H
#define CRYPTOPP_RDRAND_H

#include "cryptlib.h"












NAMESPACE_BEGIN(CryptoPP)




class RDRAND_Err : public Exception
{
public:
	RDRAND_Err(const std::string &operation)
		: Exception(OTHER_ERROR, "RDRAND: " + operation + " operation failed") {}
};




class RDRAND : public RandomNumberGenerator
{
public:
	std::string AlgorithmName() const {return "RDRAND";}

	
	
	
	
	
	
	
	
	
	
	RDRAND(unsigned int retries = 4) : m_retries(retries) {}

	virtual ~RDRAND() {}

	
	
	unsigned int GetRetries() const
	{
		return m_retries;
	}

	
	
	void SetRetries(unsigned int retries)
	{
		m_retries = retries;
	}

	
	
	
#if (CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32 || CRYPTOPP_BOOL_X64)
	virtual void GenerateBlock(byte *output, size_t size);
#else
	virtual void GenerateBlock(byte *output, size_t size) {
		CRYPTOPP_UNUSED(output), CRYPTOPP_UNUSED(size);
		throw NotImplemented("RDRAND: rdrand is not available on this platform");
	}
#endif

	
	
	
	
	
#if (CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32 || CRYPTOPP_BOOL_X64)
	virtual void DiscardBytes(size_t n);
#else
	virtual void DiscardBytes(size_t n) {
		CRYPTOPP_UNUSED(n);
		throw NotImplemented("RDRAND: rdrand is not available on this platform");
	}
#endif

	
	
	
	
	virtual void IncorporateEntropy(const byte *input, size_t length)
	{
		
		CRYPTOPP_UNUSED(input); CRYPTOPP_UNUSED(length);
		
	}

private:
	unsigned int m_retries;
};




class RDSEED_Err : public Exception
{
public:
	RDSEED_Err(const std::string &operation)
		: Exception(OTHER_ERROR, "RDSEED: " + operation + " operation failed") {}
};




class RDSEED : public RandomNumberGenerator
{
public:
	std::string AlgorithmName() const {return "RDSEED";}

	
	
	
	
	
	
	
	RDSEED(unsigned int retries = 64) : m_retries(retries) {}

	virtual ~RDSEED() {}

	
	
	unsigned int GetRetries() const
	{
		return m_retries;
	}

	
	
	void SetRetries(unsigned int retries)
	{
		m_retries = retries;
	}

	
	
	
#if (CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32 || CRYPTOPP_BOOL_X64)
	virtual void GenerateBlock(byte *output, size_t size);
#else
	virtual void GenerateBlock(byte *output, size_t size) {
		CRYPTOPP_UNUSED(output), CRYPTOPP_UNUSED(size);
		throw NotImplemented("RDSEED: rdseed is not available on this platform");
	}
#endif

	
	
	
	
	
#if (CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32 || CRYPTOPP_BOOL_X64)
	virtual void DiscardBytes(size_t n);
#else
	virtual void DiscardBytes(size_t n) {
		CRYPTOPP_UNUSED(n);
		throw NotImplemented("RDSEED: rdseed is not available on this platform");
	}
#endif

	
	
	
	
	virtual void IncorporateEntropy(const byte *input, size_t length)
	{
		
		CRYPTOPP_UNUSED(input); CRYPTOPP_UNUSED(length);
		
	}

private:
	unsigned int m_retries;
};

NAMESPACE_END

#endif 
