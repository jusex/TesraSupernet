


#include "pch.h"
#include "config.h"
#include "cryptlib.h"
#include "secblock.h"
#include "rdrand.h"
#include "cpu.h"

#if CRYPTOPP_MSC_VERSION
# pragma warning(disable: 4100)
#endif


























#if 0
#define NASM_RDRAND_ASM_AVAILABLE 1
#define NASM_RDSEED_ASM_AVAILABLE 1
#endif







#if (CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32 || CRYPTOPP_BOOL_X64)
# ifndef CRYPTOPP_CPUID_AVAILABLE
#  define CRYPTOPP_CPUID_AVAILABLE
# endif
#endif

#if defined(CRYPTOPP_CPUID_AVAILABLE)
# if defined(CRYPTOPP_MSC_VERSION)
#  define MASM_RDRAND_ASM_AVAILABLE 1
#  define MASM_RDSEED_ASM_AVAILABLE 1
# elif defined(CRYPTOPP_LLVM_CLANG_VERSION) || defined(CRYPTOPP_APPLE_CLANG_VERSION)
#  define GCC_RDRAND_ASM_AVAILABLE 1
#  define GCC_RDSEED_ASM_AVAILABLE 1
# elif defined(__SUNPRO_CC)
#  if defined(__RDRND__) && (__SUNPRO_CC >= 0x5130)
#    define ALL_RDRAND_INTRIN_AVAILABLE 1
#  elif (__SUNPRO_CC >= 0x5100)
#    define GCC_RDRAND_ASM_AVAILABLE 1
#  endif
#  if defined(__RDSEED__) && (__SUNPRO_CC >= 0x5140)
#    define ALL_RDSEED_INTRIN_AVAILABLE 1
#  elif (__SUNPRO_CC >= 0x5100)
#    define GCC_RDSEED_ASM_AVAILABLE 1
#  endif
# elif defined(CRYPTOPP_GCC_VERSION)
#  if defined(__RDRND__) && (CRYPTOPP_GCC_VERSION >= 30200)
#    define ALL_RDRAND_INTRIN_AVAILABLE 1
#  else
#    define GCC_RDRAND_ASM_AVAILABLE 1
#  endif
#  if defined(__RDSEED__) && (CRYPTOPP_GCC_VERSION >= 30200)
#    define ALL_RDSEED_INTRIN_AVAILABLE 1
#  else
#    define GCC_RDSEED_ASM_AVAILABLE 1
#  endif
# endif
#endif


#if 0
#  if MASM_RDRAND_ASM_AVAILABLE
#    pragma message ("MASM_RDRAND_ASM_AVAILABLE is 1")
#  elif NASM_RDRAND_ASM_AVAILABLE
#    pragma message ("NASM_RDRAND_ASM_AVAILABLE is 1")
#  elif GCC_RDRAND_ASM_AVAILABLE
#    pragma message ("GCC_RDRAND_ASM_AVAILABLE is 1")
#  elif ALL_RDRAND_INTRIN_AVAILABLE
#    pragma message ("ALL_RDRAND_INTRIN_AVAILABLE is 1")
#  else
#    pragma message ("RDRAND is not available")
#  endif
#  if MASM_RDSEED_ASM_AVAILABLE
#    pragma message ("MASM_RDSEED_ASM_AVAILABLE is 1")
#  elif NASM_RDSEED_ASM_AVAILABLE
#    pragma message ("NASM_RDSEED_ASM_AVAILABLE is 1")
#  elif GCC_RDSEED_ASM_AVAILABLE
#    pragma message ("GCC_RDSEED_ASM_AVAILABLE is 1")
#  elif ALL_RDSEED_INTRIN_AVAILABLE
#    pragma message ("ALL_RDSEED_INTRIN_AVAILABLE is 1")
#  else
#    pragma message ("RDSEED is not available")
#  endif
#endif




#if (ALL_RDRAND_INTRIN_AVAILABLE || ALL_RDSEED_INTRIN_AVAILABLE)
# include <immintrin.h> 
# if defined(__GNUC__) && (CRYPTOPP_GCC_VERSION >= 40600)
#  include <x86intrin.h> 
# endif
# if defined(__has_include)
#  if __has_include(<x86intrin.h>)
#   include <x86intrin.h> 
#  endif
# endif
#endif

#if MASM_RDRAND_ASM_AVAILABLE
# ifdef _M_X64
extern "C" int CRYPTOPP_FASTCALL MASM_RRA_GenerateBlock(byte*, size_t, unsigned int);

# else
extern "C" int MASM_RRA_GenerateBlock(byte*, size_t, unsigned int);

# endif
#endif

#if MASM_RDSEED_ASM_AVAILABLE
# ifdef _M_X64
extern "C" int CRYPTOPP_FASTCALL MASM_RSA_GenerateBlock(byte*, size_t, unsigned int);

# else
extern "C" int MASM_RSA_GenerateBlock(byte*, size_t, unsigned int);

# endif
#endif

#if NASM_RDRAND_ASM_AVAILABLE
extern "C" int NASM_RRA_GenerateBlock(byte*, size_t, unsigned int);
#endif

#if NASM_RDSEED_ASM_AVAILABLE
extern "C" int NASM_RSA_GenerateBlock(byte*, size_t, unsigned int);
#endif




NAMESPACE_BEGIN(CryptoPP)

#if ALL_RDRAND_INTRIN_AVAILABLE
static int ALL_RRI_GenerateBlock(byte *output, size_t size, unsigned int safety)
{
	CRYPTOPP_ASSERT((output && size) || !(output || size));
#if CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32
	word32 val;
#else
	word64 val;
#endif

	while (size >= sizeof(val))
	{
#if CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32
		if (_rdrand32_step((word32*)output))
#else
		
		if (_rdrand64_step(reinterpret_cast<unsigned long long*>(output)))
#endif
		{
			output += sizeof(val);
			size -= sizeof(val);
		}
		else
		{
			if (!safety--)
			{
				CRYPTOPP_ASSERT(0);
				return 0;
			}
		}
	}

	if (size)
	{
#if CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32
		if (_rdrand32_step(&val))
#else
		
		if (_rdrand64_step(reinterpret_cast<unsigned long long*>(&val)))
#endif
		{
			memcpy(output, &val, size);
			size = 0;
		}
		else
		{
			if (!safety--)
			{
				CRYPTOPP_ASSERT(0);
				return 0;
			}
		}
	}

	SecureWipeBuffer(&val, 1);

	return int(size == 0);
}
#endif 

#if GCC_RDRAND_ASM_AVAILABLE
static int GCC_RRA_GenerateBlock(byte *output, size_t size, unsigned int safety)
{
	CRYPTOPP_ASSERT((output && size) || !(output || size));
#if CRYPTOPP_BOOL_X64 || CRYPTOPP_BOOL_X32
	word64 val;
#else
	word32 val;
#endif
	char rc;
	while (size)
	{
		__asm__ volatile(
#if CRYPTOPP_BOOL_X64 || CRYPTOPP_BOOL_X32
			".byte 0x48, 0x0f, 0xc7, 0xf0;\n"  
#else
			".byte 0x0f, 0xc7, 0xf0;\n"        
#endif
			"setc %1; "
			: "=a" (val), "=qm" (rc)
			:
			: "cc"
		);

		if (rc)
		{
			if (size >= sizeof(val))
			{
#if defined(CRYPTOPP_ALLOW_UNALIGNED_DATA_ACCESS) && (CRYPTOPP_BOOL_X64 || CRYPTOPP_BOOL_X32)
				*((word64*)(void *)output) = val;
#elif defined(CRYPTOPP_ALLOW_UNALIGNED_DATA_ACCESS) && (CRYPTOPP_BOOL_X86)
				*((word32*)(void *)output) = val;
#else
				memcpy(output, &val, sizeof(val));
#endif
				output += sizeof(val);
				size -= sizeof(val);
			}
			else
			{
				memcpy(output, &val, size);
				size = 0;
			}
		}
		else
		{
			if (!safety--)
			{
				CRYPTOPP_ASSERT(0);
				return 0;
			}
		}
	}

	SecureWipeBuffer(&val, 1);

	return int(size == 0);
}

#endif 

#if (CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32 || CRYPTOPP_BOOL_X64)
void RDRAND::GenerateBlock(byte *output, size_t size)
{
	CRYPTOPP_UNUSED(output), CRYPTOPP_UNUSED(size);
	CRYPTOPP_ASSERT((output && size) || !(output || size));

	if(!HasRDRAND())
		throw NotImplemented("RDRAND: rdrand is not available on this platform");

	int rc; CRYPTOPP_UNUSED(rc);
#if MASM_RDRAND_ASM_AVAILABLE
	rc = MASM_RRA_GenerateBlock(output, size, m_retries);
	if (!rc) { throw RDRAND_Err("MASM_RRA_GenerateBlock"); }
#elif NASM_RDRAND_ASM_AVAILABLE
	rc = NASM_RRA_GenerateBlock(output, size, m_retries);
	if (!rc) { throw RDRAND_Err("NASM_RRA_GenerateBlock"); }
#elif ALL_RDRAND_INTRIN_AVAILABLE
	rc = ALL_RRI_GenerateBlock(output, size, m_retries);
	if (!rc) { throw RDRAND_Err("ALL_RRI_GenerateBlock"); }
#elif GCC_RDRAND_ASM_AVAILABLE
	rc = GCC_RRA_GenerateBlock(output, size, m_retries);
	if (!rc) { throw RDRAND_Err("GCC_RRA_GenerateBlock"); }
#else
	
	throw NotImplemented("RDRAND: failed to find a suitable implementation???");
#endif 
}

void RDRAND::DiscardBytes(size_t n)
{
	
	
	CRYPTOPP_ASSERT(HasRDRAND());
#if CRYPTOPP_BOOL_X64 || CRYPTOPP_BOOL_X32
	FixedSizeSecBlock<word64, 16> discard;
	n = RoundUpToMultipleOf(n, sizeof(word64));
#else
	FixedSizeSecBlock<word32, 16> discard;
	n = RoundUpToMultipleOf(n, sizeof(word32));
#endif

	size_t count = STDMIN(n, discard.SizeInBytes());
	while (count)
	{
		GenerateBlock(discard.BytePtr(), count);
		n -= count;
		count = STDMIN(n, discard.SizeInBytes());
	}
}
#endif 




#if ALL_RDSEED_INTRIN_AVAILABLE
static int ALL_RSI_GenerateBlock(byte *output, size_t size, unsigned int safety)
{
	CRYPTOPP_ASSERT((output && size) || !(output || size));
#if CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32
	word32 val;
#else
	word64 val;
#endif

	while (size >= sizeof(val))
	{
#if CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32
		if (_rdseed32_step((word32*)output))
#else
		
		if (_rdseed64_step(reinterpret_cast<unsigned long long*>(output)))
#endif
		{
			output += sizeof(val);
			size -= sizeof(val);
		}
		else
		{
			if (!safety--)
			{
				CRYPTOPP_ASSERT(0);
				return 0;
			}
		}
	}

	if (size)
	{
#if CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32
		if (_rdseed32_step(&val))
#else
		
		if (_rdseed64_step(reinterpret_cast<unsigned long long*>(&val)))
#endif
		{
			memcpy(output, &val, size);
			size = 0;
		}
		else
		{
			if (!safety--)
			{
				CRYPTOPP_ASSERT(0);
				return 0;
			}
		}
	}

	SecureWipeBuffer(&val, 1);

	return int(size == 0);
}
#endif 

#if GCC_RDSEED_ASM_AVAILABLE
static int GCC_RSA_GenerateBlock(byte *output, size_t size, unsigned int safety)
{
	CRYPTOPP_ASSERT((output && size) || !(output || size));
#if CRYPTOPP_BOOL_X64 || CRYPTOPP_BOOL_X32
	word64 val;
#else
	word32 val;
#endif
	char rc;
	while (size)
	{
		__asm__ volatile(
#if CRYPTOPP_BOOL_X64 || CRYPTOPP_BOOL_X32
			".byte 0x48, 0x0f, 0xc7, 0xf8;\n"  
#else
			".byte 0x0f, 0xc7, 0xf8;\n"        
#endif
			"setc %1; "
			: "=a" (val), "=qm" (rc)
			:
			: "cc"
		);

		if (rc)
		{
			if (size >= sizeof(val))
			{
#if defined(CRYPTOPP_ALLOW_UNALIGNED_DATA_ACCESS) && (CRYPTOPP_BOOL_X64 || CRYPTOPP_BOOL_X32)
				*((word64*)(void *)output) = val;
#elif defined(CRYPTOPP_ALLOW_UNALIGNED_DATA_ACCESS) && (CRYPTOPP_BOOL_X86)
				*((word32*)(void *)output) = val;
#else
				memcpy(output, &val, sizeof(val));
#endif
				output += sizeof(val);
				size -= sizeof(val);
			}
			else
			{
				memcpy(output, &val, size);
				size = 0;
			}
		}
		else
		{
			if (!safety--)
			{
				CRYPTOPP_ASSERT(0);
				return 0;
			}
		}
	}

	SecureWipeBuffer(&val, 1);

	return int(size == 0);
}
#endif 

#if (CRYPTOPP_BOOL_X86 || CRYPTOPP_BOOL_X32 || CRYPTOPP_BOOL_X64)
void RDSEED::GenerateBlock(byte *output, size_t size)
{
	CRYPTOPP_UNUSED(output), CRYPTOPP_UNUSED(size);
	CRYPTOPP_ASSERT((output && size) || !(output || size));

	if(!HasRDSEED())
		throw NotImplemented("RDSEED: rdseed is not available on this platform");

	int rc; CRYPTOPP_UNUSED(rc);
#if MASM_RDSEED_ASM_AVAILABLE
	rc = MASM_RSA_GenerateBlock(output, size, m_retries);
	if (!rc) { throw RDSEED_Err("MASM_RSA_GenerateBlock"); }
#elif NASM_RDSEED_ASM_AVAILABLE
	rc = NASM_RSA_GenerateBlock(output, size, m_retries);
	if (!rc) { throw RDRAND_Err("NASM_RSA_GenerateBlock"); }
#elif ALL_RDSEED_INTRIN_AVAILABLE
	rc = ALL_RSI_GenerateBlock(output, size, m_retries);
	if (!rc) { throw RDSEED_Err("ALL_RSI_GenerateBlock"); }
#elif GCC_RDSEED_ASM_AVAILABLE
	rc = GCC_RSA_GenerateBlock(output, size, m_retries);
	if (!rc) { throw RDSEED_Err("GCC_RSA_GenerateBlock"); }
#else
	
	throw NotImplemented("RDSEED: failed to find a suitable implementation???");
#endif
}

void RDSEED::DiscardBytes(size_t n)
{
	
	
	CRYPTOPP_ASSERT(HasRDSEED());
#if CRYPTOPP_BOOL_X64 || CRYPTOPP_BOOL_X32
	FixedSizeSecBlock<word64, 16> discard;
	n = RoundUpToMultipleOf(n, sizeof(word64));
#else
	FixedSizeSecBlock<word32, 16> discard;
	n = RoundUpToMultipleOf(n, sizeof(word32));
#endif

	size_t count = STDMIN(n, discard.SizeInBytes());
	while (count)
	{
		GenerateBlock(discard.BytePtr(), count);
		n -= count;
		count = STDMIN(n, discard.SizeInBytes());
	}
}
#endif 

NAMESPACE_END
