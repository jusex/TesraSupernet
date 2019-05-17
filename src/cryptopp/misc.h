





#ifndef CRYPTOPP_MISC_H
#define CRYPTOPP_MISC_H

#include "config.h"

#if !CRYPTOPP_DOXYGEN_PROCESSING

#if CRYPTOPP_MSC_VERSION
# pragma warning(push)
# pragma warning(disable: 4146 4514)
# if (CRYPTOPP_MSC_VERSION >= 1400)
#  pragma warning(disable: 6326)
# endif
#endif

#include "cryptlib.h"
#include "stdcpp.h"
#include "smartptr.h"

#ifdef _MSC_VER
	#if _MSC_VER >= 1400
		
		#define _interlockedbittestandset CRYPTOPP_DISABLED_INTRINSIC_1
		#define _interlockedbittestandreset CRYPTOPP_DISABLED_INTRINSIC_2
		#define _interlockedbittestandset64 CRYPTOPP_DISABLED_INTRINSIC_3
		#define _interlockedbittestandreset64 CRYPTOPP_DISABLED_INTRINSIC_4
		#include <intrin.h>
		#undef _interlockedbittestandset
		#undef _interlockedbittestandreset
		#undef _interlockedbittestandset64
		#undef _interlockedbittestandreset64
		#define CRYPTOPP_FAST_ROTATE(x) 1
	#elif _MSC_VER >= 1300
		#define CRYPTOPP_FAST_ROTATE(x) ((x) == 32 | (x) == 64)
	#else
		#define CRYPTOPP_FAST_ROTATE(x) ((x) == 32)
	#endif
#elif (defined(__MWERKS__) && TARGET_CPU_PPC) || \
	(defined(__GNUC__) && (defined(_ARCH_PWR2) || defined(_ARCH_PWR) || defined(_ARCH_PPC) || defined(_ARCH_PPC64) || defined(_ARCH_COM)))
	#define CRYPTOPP_FAST_ROTATE(x) ((x) == 32)
#elif defined(__GNUC__) && (CRYPTOPP_BOOL_X64 || CRYPTOPP_BOOL_X32 || CRYPTOPP_BOOL_X86)	
	#define CRYPTOPP_FAST_ROTATE(x) 1
#else
	#define CRYPTOPP_FAST_ROTATE(x) 0
#endif

#ifdef __BORLANDC__
#include <mem.h>
#include <stdlib.h>
#endif

#if defined(__GNUC__) && defined(__linux__)
#define CRYPTOPP_BYTESWAP_AVAILABLE
#include <byteswap.h>
#endif

#if defined(__GNUC__) && defined(__BMI__)
# include <immintrin.h>
# if defined(__clang__)
#  ifndef _tzcnt_u32
#   define _tzcnt_u32(x) __tzcnt_u32(x)
#  endif
#  ifndef _blsr_u32
#    define  _blsr_u32(x)  __blsr_u32(x)
#  endif
#  ifdef __x86_64__
#   ifndef _tzcnt_u64
#    define _tzcnt_u64(x) __tzcnt_u64(x)
#   endif
#   ifndef _blsr_u64
#     define  _blsr_u64(x)  __blsr_u64(x)
#   endif
#  endif  
# endif  
#endif  

#endif 

#if CRYPTOPP_DOXYGEN_PROCESSING









#  define SIZE_MAX ...
#else



#ifndef SIZE_MAX
# if defined(__SIZE_MAX__) && (__SIZE_MAX__ > 0)
#  define SIZE_MAX __SIZE_MAX__
# elif defined(SIZE_T_MAX) && (SIZE_T_MAX > 0)
#  define SIZE_MAX SIZE_T_MAX
# else
#  define SIZE_MAX ((std::numeric_limits<size_t>::max)())
# endif
#endif

#endif 

NAMESPACE_BEGIN(CryptoPP)


class Integer;



#if CRYPTOPP_DOXYGEN_PROCESSING



#define CRYPTOPP_COMPILE_ASSERT(expr) { ... }
#else 
template <bool b>
struct CompileAssert
{
	static char dummy[2*b-1];
};


#define CRYPTOPP_COMPILE_ASSERT(assertion) CRYPTOPP_COMPILE_ASSERT_INSTANCE(assertion, __LINE__)
#if defined(CRYPTOPP_EXPORTS) || defined(CRYPTOPP_IMPORTS)
#define CRYPTOPP_COMPILE_ASSERT_INSTANCE(assertion, instance)
#else
# if defined(__GNUC__)
#  define CRYPTOPP_COMPILE_ASSERT_INSTANCE(assertion, instance) \
		static CompileAssert<(assertion)> \
		CRYPTOPP_ASSERT_JOIN(cryptopp_CRYPTOPP_ASSERT_, instance) __attribute__ ((unused))
# else
#  define CRYPTOPP_COMPILE_ASSERT_INSTANCE(assertion, instance) \
		static CompileAssert<(assertion)> \
		CRYPTOPP_ASSERT_JOIN(cryptopp_CRYPTOPP_ASSERT_, instance)
# endif 
#endif
#define CRYPTOPP_ASSERT_JOIN(X, Y) CRYPTOPP_DO_ASSERT_JOIN(X, Y)
#define CRYPTOPP_DO_ASSERT_JOIN(X, Y) X##Y

#endif 



#if CRYPTOPP_DOXYGEN_PROCESSING







# define COUNTOF(arr)
#else

#ifndef COUNTOF
# if defined(_MSC_VER) && (_MSC_VER >= 1400)
#  define COUNTOF(x) _countof(x)
# else
#  define COUNTOF(x) (sizeof(x)/sizeof(x[0]))
# endif
#endif 
#endif 





class CRYPTOPP_DLL Empty
{
};

#if !CRYPTOPP_DOXYGEN_PROCESSING
template <class BASE1, class BASE2>
class CRYPTOPP_NO_VTABLE TwoBases : public BASE1, public BASE2
{
};

template <class BASE1, class BASE2, class BASE3>
class CRYPTOPP_NO_VTABLE ThreeBases : public BASE1, public BASE2, public BASE3
{
};
#endif 





template <class T>
class ObjectHolder
{
protected:
	T m_object;
};







class NotCopyable
{
public:
	NotCopyable() {}
private:
    NotCopyable(const NotCopyable &);
    void operator=(const NotCopyable &);
};




template <class T>
struct NewObject
{
	T* operator()() const {return new T;}
};

#if CRYPTOPP_DOXYGEN_PROCESSING








#define MEMORY_BARRIER ...
#else
#if defined(CRYPTOPP_CXX11_ATOMICS)
# define MEMORY_BARRIER() std::atomic_thread_fence(std::memory_order_acq_rel)
#elif (_MSC_VER >= 1400)
# pragma intrinsic(_ReadWriteBarrier)
# define MEMORY_BARRIER() _ReadWriteBarrier()
#elif defined(__INTEL_COMPILER)
# define MEMORY_BARRIER() __memory_barrier()
#elif defined(__GNUC__) || defined(__clang__)
# define MEMORY_BARRIER() __asm__ __volatile__ ("" ::: "memory")
#else
# define MEMORY_BARRIER()
#endif
#endif 














template <class T, class F = NewObject<T>, int instance=0>
class Singleton
{
public:
	Singleton(F objectFactory = F()) : m_objectFactory(objectFactory) {}

	
	CRYPTOPP_NOINLINE const T & Ref(CRYPTOPP_NOINLINE_DOTDOTDOT) const;

private:
	F m_objectFactory;
};





#if defined(CRYPTOPP_CXX11_ATOMICS) && defined(CRYPTOPP_CXX11_SYNCHRONIZATION) && defined(CRYPTOPP_CXX11_DYNAMIC_INIT)
template <class T, class F, int instance>
  const T & Singleton<T, F, instance>::Ref(CRYPTOPP_NOINLINE_DOTDOTDOT) const
{
	static std::mutex s_mutex;
	static std::atomic<T*> s_pObject;

	T *p = s_pObject.load(std::memory_order_relaxed);
	std::atomic_thread_fence(std::memory_order_acquire);

	if (p)
		return *p;

	std::lock_guard<std::mutex> lock(s_mutex);
	p = s_pObject.load(std::memory_order_relaxed);
	std::atomic_thread_fence(std::memory_order_acquire);

	if (p)
		return *p;

	T *newObject = m_objectFactory();
	s_pObject.store(newObject, std::memory_order_relaxed);
	std::atomic_thread_fence(std::memory_order_release);

	return *newObject;
}
#else
template <class T, class F, int instance>
const T & Singleton<T, F, instance>::Ref(CRYPTOPP_NOINLINE_DOTDOTDOT) const
{
	static volatile simple_ptr<T> s_pObject;
	T *p = s_pObject.m_p;
	MEMORY_BARRIER();

	if (p)
		return *p;

	T *newObject = m_objectFactory();
	p = s_pObject.m_p;
	MEMORY_BARRIER();

	if (p)
	{
		delete newObject;
		return *p;
	}

	s_pObject.m_p = newObject;
	MEMORY_BARRIER();

	return *newObject;
}
#endif



#if (!__STDC_WANT_SECURE_LIB__ && !defined(_MEMORY_S_DEFINED)) || defined(CRYPTOPP_WANT_SECURE_LIB)



















inline void memcpy_s(void *dest, size_t sizeInBytes, const void *src, size_t count)
{
	

	
	CRYPTOPP_ASSERT(dest != NULL); CRYPTOPP_ASSERT(src != NULL);
	
	CRYPTOPP_ASSERT(sizeInBytes >= count);
	if (count > sizeInBytes)
		throw InvalidArgument("memcpy_s: buffer overflow");

#if CRYPTOPP_MSC_VERSION
# pragma warning(push)
# pragma warning(disable: 4996)
# if (CRYPTOPP_MSC_VERSION >= 1400)
#  pragma warning(disable: 6386)
# endif
#endif
	memcpy(dest, src, count);
#if CRYPTOPP_MSC_VERSION
# pragma warning(pop)
#endif
}



















inline void memmove_s(void *dest, size_t sizeInBytes, const void *src, size_t count)
{
	

	
	CRYPTOPP_ASSERT(dest != NULL); CRYPTOPP_ASSERT(src != NULL);
	
	CRYPTOPP_ASSERT(sizeInBytes >= count);
	if (count > sizeInBytes)
		throw InvalidArgument("memmove_s: buffer overflow");

#if CRYPTOPP_MSC_VERSION
# pragma warning(push)
# pragma warning(disable: 4996)
# if (CRYPTOPP_MSC_VERSION >= 1400)
#  pragma warning(disable: 6386)
# endif
#endif
	memmove(dest, src, count);
#if CRYPTOPP_MSC_VERSION
# pragma warning(pop)
#endif
}

#if __BORLANDC__ >= 0x620

# define memcpy_s CryptoPP::memcpy_s
# define memmove_s CryptoPP::memmove_s
#endif

#endif 









template <class T>
inline void vec_swap(T& a, T& b)
{
	T t;
	t=a, a=b, b=t;
}







inline void * memset_z(void *ptr, int value, size_t num)
{

#if CRYPTOPP_GCC_VERSION >= 30001
	if (__builtin_constant_p(num) && num==0)
		return ptr;
#endif
	volatile void* x = memset(ptr, value, num);
	return const_cast<void*>(x);
}






template <class T> inline const T& STDMIN(const T& a, const T& b)
{
	return b < a ? b : a;
}






template <class T> inline const T& STDMAX(const T& a, const T& b)
{
	
	return a < b ? b : a;
}

#if CRYPTOPP_MSC_VERSION
# pragma warning(push)
# pragma warning(disable: 4389)
#endif

#if CRYPTOPP_GCC_DIAGNOSTIC_AVAILABLE
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wsign-compare"
# if (CRYPTOPP_LLVM_CLANG_VERSION >= 20800) || (CRYPTOPP_APPLE_CLANG_VERSION >= 30000)
#  pragma GCC diagnostic ignored "-Wtautological-compare"
# elif (CRYPTOPP_GCC_VERSION >= 40300)
#  pragma GCC diagnostic ignored "-Wtype-limits"
# endif
#endif






template <class T1, class T2> inline const T1 UnsignedMin(const T1& a, const T2& b)
{
	CRYPTOPP_COMPILE_ASSERT((sizeof(T1)<=sizeof(T2) && T2(-1)>0) || (sizeof(T1)>sizeof(T2) && T1(-1)>0));
	if (sizeof(T1)<=sizeof(T2))
		return b < (T2)a ? (T1)b : a;
	else
		return (T1)b < a ? (T1)b : a;
}





template <class T1, class T2>
inline bool SafeConvert(T1 from, T2 &to)
{
	to = (T2)from;
	if (from != to || (from > 0) != (to > 0))
		return false;
	return true;
}





template <class T>
std::string IntToString(T value, unsigned int base = 10)
{
	
	static const unsigned int HIGH_BIT = (1U << 31);
	const char CH = !!(base & HIGH_BIT) ? 'A' : 'a';
	base &= ~HIGH_BIT;

	CRYPTOPP_ASSERT(base >= 2);
	if (value == 0)
		return "0";

	bool negate = false;
	if (value < 0)
	{
		negate = true;
		value = 0-value;	
	}
	std::string result;
	while (value > 0)
	{
		T digit = value % base;
		result = char((digit < 10 ? '0' : (CH - 10)) + digit) + result;
		value /= base;
	}
	if (negate)
		result = "-" + result;
	return result;
}







template <> CRYPTOPP_DLL
std::string IntToString<word64>(word64 value, unsigned int base);




















template <> CRYPTOPP_DLL
std::string IntToString<Integer>(Integer value, unsigned int base);

#if CRYPTOPP_MSC_VERSION
# pragma warning(pop)
#endif

#if CRYPTOPP_GCC_DIAGNOSTIC_AVAILABLE
# pragma GCC diagnostic pop
#endif

#define RETURN_IF_NONZERO(x) size_t returnedValue = x; if (returnedValue) return returnedValue


#define GETBYTE(x, y) (unsigned int)byte((x)>>(8*(y)))




#define CRYPTOPP_GET_BYTE_AS_BYTE(x, y) byte((x)>>(8*(y)))




template <class T>
unsigned int Parity(T value)
{
	for (unsigned int i=8*sizeof(value)/2; i>0; i/=2)
		value ^= value >> i;
	return (unsigned int)value&1;
}




template <class T>
unsigned int BytePrecision(const T &value)
{
	if (!value)
		return 0;

	unsigned int l=0, h=8*sizeof(value);
	while (h-l > 8)
	{
		unsigned int t = (l+h)/2;
		if (value >> t)
			l = t;
		else
			h = t;
	}

	return h/8;
}




template <class T>
unsigned int BitPrecision(const T &value)
{
	if (!value)
		return 0;

	unsigned int l=0, h=8*sizeof(value);

	while (h-l > 1)
	{
		unsigned int t = (l+h)/2;
		if (value >> t)
			l = t;
		else
			h = t;
	}

	return h;
}







inline unsigned int TrailingZeros(word32 v)
{
	
	
	
	CRYPTOPP_ASSERT(v != 0);
#if defined(__GNUC__) && defined(__BMI__)
	return (unsigned int)_tzcnt_u32(v);
#elif defined(__GNUC__) && (CRYPTOPP_GCC_VERSION >= 30400)
	return (unsigned int)__builtin_ctz(v);
#elif defined(_MSC_VER) && (_MSC_VER >= 1400)
	unsigned long result;
	_BitScanForward(&result, v);
	return (unsigned int)result;
#else
	
	static const int MultiplyDeBruijnBitPosition[32] =
	{
	  0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
	  31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};
	return MultiplyDeBruijnBitPosition[((word32)((v & -v) * 0x077CB531U)) >> 27];
#endif
}







inline unsigned int TrailingZeros(word64 v)
{
	
	
	
	CRYPTOPP_ASSERT(v != 0);
#if defined(__GNUC__) && defined(__BMI__) && defined(__x86_64__)
	return (unsigned int)_tzcnt_u64(v);
#elif defined(__GNUC__) && (CRYPTOPP_GCC_VERSION >= 30400)
	return (unsigned int)__builtin_ctzll(v);
#elif defined(_MSC_VER) && (_MSC_VER >= 1400) && (defined(_M_X64) || defined(_M_IA64))
	unsigned long result;
	_BitScanForward64(&result, v);
	return (unsigned int)result;
#else
	return word32(v) ? TrailingZeros(word32(v)) : 32 + TrailingZeros(word32(v>>32));
#endif
}








template <class T>
inline T Crop(T value, size_t bits)
{
	if (bits < 8*sizeof(value))
    	return T(value & ((T(1) << bits) - 1));
	else
		return value;
}





inline size_t BitsToBytes(size_t bitCount)
{
	return ((bitCount+7)/(8));
}






inline size_t BytesToWords(size_t byteCount)
{
	return ((byteCount+WORD_SIZE-1)/WORD_SIZE);
}






inline size_t BitsToWords(size_t bitCount)
{
	return ((bitCount+WORD_BITS-1)/(WORD_BITS));
}






inline size_t BitsToDwords(size_t bitCount)
{
	return ((bitCount+2*WORD_BITS-1)/(2*WORD_BITS));
}







CRYPTOPP_DLL void CRYPTOPP_API xorbuf(byte *buf, const byte *mask, size_t count);








CRYPTOPP_DLL void CRYPTOPP_API xorbuf(byte *output, const byte *input, const byte *mask, size_t count);










CRYPTOPP_DLL bool CRYPTOPP_API VerifyBufsEqual(const byte *buf1, const byte *buf2, size_t count);






template <class T>
inline bool IsPowerOf2(const T &value)
{
	return value > 0 && (value & (value-1)) == 0;
}

#if defined(__GNUC__) && defined(__BMI__)
template <>
inline bool IsPowerOf2<word32>(const word32 &value)
{
	return value > 0 && _blsr_u32(value) == 0;
}

# if defined(__x86_64__)
template <>
inline bool IsPowerOf2<word64>(const word64 &value)
{
	return value > 0 && _blsr_u64(value) == 0;
}
# endif
#endif








template <class T1, class T2>
inline T1 SaturatingSubtract(const T1 &a, const T2 &b)
{
	
	return T1((a > b) ? (a - b) : 0);
}









template <class T1, class T2>
inline T1 SaturatingSubtract1(const T1 &a, const T2 &b)
{
	
	return T1((a > b) ? (a - b) : 1);
}







template <class T1, class T2>
inline T2 ModPowerOf2(const T1 &a, const T2 &b)
{
	CRYPTOPP_ASSERT(IsPowerOf2(b));
	
	return T2(a) & SaturatingSubtract(b,1U);
}







template <class T1, class T2>
inline T1 RoundDownToMultipleOf(const T1 &n, const T2 &m)
{
	if (IsPowerOf2(m))
		return n - ModPowerOf2(n, m);
	else
		return n - n%m;
}








template <class T1, class T2>
inline T1 RoundUpToMultipleOf(const T1 &n, const T2 &m)
{
	if (n > (SIZE_MAX/sizeof(T1))-m-1)
		throw InvalidArgument("RoundUpToMultipleOf: integer overflow");
	return RoundDownToMultipleOf(T1(n+m-1), m);
}











template <class T>
inline unsigned int GetAlignmentOf(T *dummy=NULL)	
{

#if defined(CRYPTOPP_ALLOW_UNALIGNED_DATA_ACCESS)
	if (sizeof(T) < 16)
		return 1;
#endif
	CRYPTOPP_UNUSED(dummy);
#if defined(CRYPTOPP_CXX11_ALIGNOF)
	return alignof(T);
#elif (_MSC_VER >= 1300)
	return __alignof(T);
#elif defined(__GNUC__)
	return __alignof__(T);
#elif CRYPTOPP_BOOL_SLOW_WORD64
	return UnsignedMin(4U, sizeof(T));
#else
# if __BIGGEST_ALIGNMENT__
	if (__BIGGEST_ALIGNMENT__ < sizeof(T))
		return __BIGGEST_ALIGNMENT__;
	else
# endif
	return sizeof(T);
#endif
}







inline bool IsAlignedOn(const void *ptr, unsigned int alignment)
{
	return alignment==1 || (IsPowerOf2(alignment) ? ModPowerOf2((size_t)ptr, alignment) == 0 : (size_t)ptr % alignment == 0);
}






template <class T>
inline bool IsAligned(const void *ptr, T *dummy=NULL)	
{
	CRYPTOPP_UNUSED(dummy);
	return IsAlignedOn(ptr, GetAlignmentOf<T>());
}

#if defined(IS_LITTLE_ENDIAN)
	typedef LittleEndian NativeByteOrder;
#elif defined(IS_BIG_ENDIAN)
	typedef BigEndian NativeByteOrder;
#else
# error "Unable to determine endian-ness"
#endif



	

	
	

	
inline ByteOrder GetNativeByteOrder()
{
	return NativeByteOrder::ToEnum();
}




inline bool NativeByteOrderIs(ByteOrder order)
{
	return order == GetNativeByteOrder();
}










template <class T>
inline CipherDir GetCipherDir(const T &obj)
{
	return obj.IsForwardTransformation() ? ENCRYPTION : DECRYPTION;
}









CRYPTOPP_DLL void CRYPTOPP_API CallNewHandler();







inline void IncrementCounterByOne(byte *inout, unsigned int size)
{
	CRYPTOPP_ASSERT(inout != NULL); CRYPTOPP_ASSERT(size < INT_MAX);
	for (int i=int(size-1), carry=1; i>=0 && carry; i--)
		carry = !++inout[i];
}








inline void IncrementCounterByOne(byte *output, const byte *input, unsigned int size)
{
	CRYPTOPP_ASSERT(output != NULL); CRYPTOPP_ASSERT(input != NULL); CRYPTOPP_ASSERT(size < INT_MAX);

	int i, carry;
	for (i=int(size-1), carry=1; i>=0 && carry; i--)
		carry = ((output[i] = input[i]+1) == 0);
	memcpy_s(output, size, input, size_t(i)+1);
}





template <class T>
inline void ConditionalSwap(bool c, T &a, T &b)
{
	T t = c * (a ^ b);
	a ^= t;
	b ^= t;
}





template <class T>
inline void ConditionalSwapPointers(bool c, T &a, T &b)
{
	ptrdiff_t t = size_t(c) * (a - b);
	a -= t;
	b += t;
}








template <class T>
void SecureWipeBuffer(T *buf, size_t n)
{
	
	volatile T *p = buf+n;
	while (n--)
		*((volatile T*)(--p)) = 0;
}

#if (_MSC_VER >= 1400 || defined(__GNUC__)) && (CRYPTOPP_BOOL_X64 || CRYPTOPP_BOOL_X86)





template<> inline void SecureWipeBuffer(byte *buf, size_t n)
{
	volatile byte *p = buf;
#ifdef __GNUC__
	asm volatile("rep stosb" : "+c"(n), "+D"(p) : "a"(0) : "memory");
#else
	__stosb((byte *)(size_t)p, 0, n);
#endif
}





template<> inline void SecureWipeBuffer(word16 *buf, size_t n)
{
	volatile word16 *p = buf;
#ifdef __GNUC__
	asm volatile("rep stosw" : "+c"(n), "+D"(p) : "a"(0) : "memory");
#else
	__stosw((word16 *)(size_t)p, 0, n);
#endif
}





template<> inline void SecureWipeBuffer(word32 *buf, size_t n)
{
	volatile word32 *p = buf;
#ifdef __GNUC__
	asm volatile("rep stosl" : "+c"(n), "+D"(p) : "a"(0) : "memory");
#else
	__stosd((unsigned long *)(size_t)p, 0, n);
#endif
}





template<> inline void SecureWipeBuffer(word64 *buf, size_t n)
{
#if CRYPTOPP_BOOL_X64
	volatile word64 *p = buf;
#ifdef __GNUC__
	asm volatile("rep stosq" : "+c"(n), "+D"(p) : "a"(0) : "memory");
#else
	__stosq((word64 *)(size_t)p, 0, n);
#endif
#else
	SecureWipeBuffer((word32 *)buf, 2*n);
#endif
}

#endif	

#if (_MSC_VER >= 1700) && defined(_M_ARM)
template<> inline void SecureWipeBuffer(byte *buf, size_t n)
{
	char *p = reinterpret_cast<char*>(buf+n);
	while (n--)
		__iso_volatile_store8(--p, 0);
}

template<> inline void SecureWipeBuffer(word16 *buf, size_t n)
{
	short *p = reinterpret_cast<short*>(buf+n);
	while (n--)
		__iso_volatile_store16(--p, 0);
}

template<> inline void SecureWipeBuffer(word32 *buf, size_t n)
{
	int *p = reinterpret_cast<int*>(buf+n);
	while (n--)
		__iso_volatile_store32(--p, 0);
}

template<> inline void SecureWipeBuffer(word64 *buf, size_t n)
{
	__int64 *p = reinterpret_cast<__int64*>(buf+n);
	while (n--)
		__iso_volatile_store64(--p, 0);
}
#endif





template <class T>
inline void SecureWipeArray(T *buf, size_t n)
{
	if (sizeof(T) % 8 == 0 && GetAlignmentOf<T>() % GetAlignmentOf<word64>() == 0)
		SecureWipeBuffer((word64 *)(void *)buf, n * (sizeof(T)/8));
	else if (sizeof(T) % 4 == 0 && GetAlignmentOf<T>() % GetAlignmentOf<word32>() == 0)
		SecureWipeBuffer((word32 *)(void *)buf, n * (sizeof(T)/4));
	else if (sizeof(T) % 2 == 0 && GetAlignmentOf<T>() % GetAlignmentOf<word16>() == 0)
		SecureWipeBuffer((word16 *)(void *)buf, n * (sizeof(T)/2));
	else
		SecureWipeBuffer((byte *)(void *)buf, n * sizeof(T));
}













#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
std::string StringNarrow(const wchar_t *str, bool throwOnError = true);
#else
static std::string StringNarrow(const wchar_t *str, bool throwOnError = true)
{
	CRYPTOPP_ASSERT(str);
	std::string result;

	
#if (CRYPTOPP_MSC_VERSION >= 1400)
	size_t len=0, size=0;
	errno_t err = 0;

	
	
	len = wcslen(str)+1;

	err = wcstombs_s(&size, NULL, 0, str, len*sizeof(wchar_t));
	CRYPTOPP_ASSERT(err == 0);
	if (err != 0) {goto CONVERSION_ERROR;}

	result.resize(size);
	err = wcstombs_s(&size, &result[0], size, str, len*sizeof(wchar_t));
	CRYPTOPP_ASSERT(err == 0);

	if (err != 0)
	{
CONVERSION_ERROR:
		if (throwOnError)
			throw InvalidArgument("StringNarrow: wcstombs_s() call failed with error " + IntToString(err));
		else
			return std::string();
	}

	
	if (!result.empty() && result[size - 1] == '\0')
		result.erase(size - 1);
#else
	size_t size = wcstombs(NULL, str, 0);
	CRYPTOPP_ASSERT(size != (size_t)-1);
	if (size == (size_t)-1) {goto CONVERSION_ERROR;}

	result.resize(size);
	size = wcstombs(&result[0], str, size);
	CRYPTOPP_ASSERT(size != (size_t)-1);

	if (size == (size_t)-1)
	{
CONVERSION_ERROR:
		if (throwOnError)
			throw InvalidArgument("StringNarrow: wcstombs() call failed");
		else
			return std::string();
	}
#endif

	return result;
}
#endif 

#ifdef CRYPTOPP_DOXYGEN_PROCESSING









CRYPTOPP_DLL void* CRYPTOPP_API AlignedAllocate(size_t size);





CRYPTOPP_DLL void CRYPTOPP_API AlignedDeallocate(void *ptr);

#endif 

#if CRYPTOPP_BOOL_ALIGN16
CRYPTOPP_DLL void* CRYPTOPP_API AlignedAllocate(size_t size);
CRYPTOPP_DLL void CRYPTOPP_API AlignedDeallocate(void *ptr);
#endif 



CRYPTOPP_DLL void * CRYPTOPP_API UnalignedAllocate(size_t size);



CRYPTOPP_DLL void CRYPTOPP_API UnalignedDeallocate(void *ptr);













template <class T> inline T rotlFixed(T x, unsigned int y)
{
	
	
	
	
	static const unsigned int THIS_SIZE = sizeof(T)*8;
	static const unsigned int MASK = THIS_SIZE-1;
	CRYPTOPP_ASSERT(y < THIS_SIZE);
	return T((x<<y)|(x>>(-y&MASK)));
}











template <class T> inline T rotrFixed(T x, unsigned int y)
{
	
	
	
	
	static const unsigned int THIS_SIZE = sizeof(T)*8;
	static const unsigned int MASK = THIS_SIZE-1;
	CRYPTOPP_ASSERT(y < THIS_SIZE);
	return T((x >> y)|(x<<(-y&MASK)));
}











template <class T> inline T rotlVariable(T x, unsigned int y)
{
	static const unsigned int THIS_SIZE = sizeof(T)*8;
	static const unsigned int MASK = THIS_SIZE-1;
	CRYPTOPP_ASSERT(y < THIS_SIZE);
	return T((x<<y)|(x>>(-y&MASK)));
}











template <class T> inline T rotrVariable(T x, unsigned int y)
{
	static const unsigned int THIS_SIZE = sizeof(T)*8;
	static const unsigned int MASK = THIS_SIZE-1;
	CRYPTOPP_ASSERT(y < THIS_SIZE);
	return T((x>>y)|(x<<(-y&MASK)));
}








template <class T> inline T rotlMod(T x, unsigned int y)
{
	static const unsigned int THIS_SIZE = sizeof(T)*8;
	static const unsigned int MASK = THIS_SIZE-1;
	return T((x<<(y&MASK))|(x>>(-y&MASK)));
}








template <class T> inline T rotrMod(T x, unsigned int y)
{
	static const unsigned int THIS_SIZE = sizeof(T)*8;
	static const unsigned int MASK = THIS_SIZE-1;
	return T((x>>(y&MASK))|(x<<(-y&MASK)));
}

#ifdef _MSC_VER









template<> inline word32 rotlFixed<word32>(word32 x, unsigned int y)
{
	
	CRYPTOPP_ASSERT(y < 8*sizeof(x));
	return y ? _lrotl(x, static_cast<byte>(y)) : x;
}









template<> inline word32 rotrFixed<word32>(word32 x, unsigned int y)
{
	
	CRYPTOPP_ASSERT(y < 8*sizeof(x));
	return y ? _lrotr(x, static_cast<byte>(y)) : x;
}









template<> inline word32 rotlVariable<word32>(word32 x, unsigned int y)
{
	CRYPTOPP_ASSERT(y < 8*sizeof(x));
	return _lrotl(x, static_cast<byte>(y));
}









template<> inline word32 rotrVariable<word32>(word32 x, unsigned int y)
{
	CRYPTOPP_ASSERT(y < 8*sizeof(x));
	return _lrotr(x, static_cast<byte>(y));
}








template<> inline word32 rotlMod<word32>(word32 x, unsigned int y)
{
	y %= 8*sizeof(x);
	return _lrotl(x, static_cast<byte>(y));
}








template<> inline word32 rotrMod<word32>(word32 x, unsigned int y)
{
	y %= 8*sizeof(x);
	return _lrotr(x, static_cast<byte>(y));
}

#endif 

#if _MSC_VER >= 1300 && !defined(__INTEL_COMPILER)










template<> inline word64 rotlFixed<word64>(word64 x, unsigned int y)
{
	
	CRYPTOPP_ASSERT(y < 8*sizeof(x));
	return y ? _rotl64(x, static_cast<byte>(y)) : x;
}









template<> inline word64 rotrFixed<word64>(word64 x, unsigned int y)
{
	
	CRYPTOPP_ASSERT(y < 8*sizeof(x));
	return y ? _rotr64(x, static_cast<byte>(y)) : x;
}









template<> inline word64 rotlVariable<word64>(word64 x, unsigned int y)
{
	CRYPTOPP_ASSERT(y < 8*sizeof(x));
	return _rotl64(x, static_cast<byte>(y));
}









template<> inline word64 rotrVariable<word64>(word64 x, unsigned int y)
{
	CRYPTOPP_ASSERT(y < 8*sizeof(x));
	return y ? _rotr64(x, static_cast<byte>(y)) : x;
}








template<> inline word64 rotlMod<word64>(word64 x, unsigned int y)
{
	CRYPTOPP_ASSERT(y < 8*sizeof(x));
	return y ? _rotl64(x, static_cast<byte>(y)) : x;
}








template<> inline word64 rotrMod<word64>(word64 x, unsigned int y)
{
	CRYPTOPP_ASSERT(y < 8*sizeof(x));
	return y ? _rotr64(x, static_cast<byte>(y)) : x;
}

#endif 

#if _MSC_VER >= 1400 && !defined(__INTEL_COMPILER)


template<> inline word16 rotlFixed<word16>(word16 x, unsigned int y)
{
	
	return _rotl16(x, static_cast<byte>(y));
}

template<> inline word16 rotrFixed<word16>(word16 x, unsigned int y)
{
	
	return _rotr16(x, static_cast<byte>(y));
}

template<> inline word16 rotlVariable<word16>(word16 x, unsigned int y)
{
	return _rotl16(x, static_cast<byte>(y));
}

template<> inline word16 rotrVariable<word16>(word16 x, unsigned int y)
{
	return _rotr16(x, static_cast<byte>(y));
}

template<> inline word16 rotlMod<word16>(word16 x, unsigned int y)
{
	return _rotl16(x, static_cast<byte>(y));
}

template<> inline word16 rotrMod<word16>(word16 x, unsigned int y)
{
	return _rotr16(x, static_cast<byte>(y));
}

template<> inline byte rotlFixed<byte>(byte x, unsigned int y)
{
	
	return _rotl8(x, static_cast<byte>(y));
}

template<> inline byte rotrFixed<byte>(byte x, unsigned int y)
{
	
	return _rotr8(x, static_cast<byte>(y));
}

template<> inline byte rotlVariable<byte>(byte x, unsigned int y)
{
	return _rotl8(x, static_cast<byte>(y));
}

template<> inline byte rotrVariable<byte>(byte x, unsigned int y)
{
	return _rotr8(x, static_cast<byte>(y));
}

template<> inline byte rotlMod<byte>(byte x, unsigned int y)
{
	return _rotl8(x, static_cast<byte>(y));
}

template<> inline byte rotrMod<byte>(byte x, unsigned int y)
{
	return _rotr8(x, static_cast<byte>(y));
}

#endif 

#if (defined(__MWERKS__) && TARGET_CPU_PPC)

template<> inline word32 rotlFixed<word32>(word32 x, unsigned int y)
{
	CRYPTOPP_ASSERT(y < 32);
	return y ? __rlwinm(x,y,0,31) : x;
}

template<> inline word32 rotrFixed<word32>(word32 x, unsigned int y)
{
	CRYPTOPP_ASSERT(y < 32);
	return y ? __rlwinm(x,32-y,0,31) : x;
}

template<> inline word32 rotlVariable<word32>(word32 x, unsigned int y)
{
	CRYPTOPP_ASSERT(y < 32);
	return (__rlwnm(x,y,0,31));
}

template<> inline word32 rotrVariable<word32>(word32 x, unsigned int y)
{
	CRYPTOPP_ASSERT(y < 32);
	return (__rlwnm(x,32-y,0,31));
}

template<> inline word32 rotlMod<word32>(word32 x, unsigned int y)
{
	return (__rlwnm(x,y,0,31));
}

template<> inline word32 rotrMod<word32>(word32 x, unsigned int y)
{
	return (__rlwnm(x,32-y,0,31));
}

#endif 







template <class T>
inline unsigned int GetByte(ByteOrder order, T value, unsigned int index)
{
	if (order == LITTLE_ENDIAN_ORDER)
		return GETBYTE(value, index);
	else
		return GETBYTE(value, sizeof(T)-index-1);
}




inline byte ByteReverse(byte value)
{
	return value;
}





inline word16 ByteReverse(word16 value)
{
#ifdef CRYPTOPP_BYTESWAP_AVAILABLE
	return bswap_16(value);
#elif defined(_MSC_VER) && _MSC_VER >= 1300
	return _byteswap_ushort(value);
#else
	return rotlFixed(value, 8U);
#endif
}





inline word32 ByteReverse(word32 value)
{
#if defined(__GNUC__) && defined(CRYPTOPP_X86_ASM_AVAILABLE)
	__asm__ ("bswap %0" : "=r" (value) : "0" (value));
	return value;
#elif defined(CRYPTOPP_BYTESWAP_AVAILABLE)
	return bswap_32(value);
#elif defined(__MWERKS__) && TARGET_CPU_PPC
	return (word32)__lwbrx(&value,0);
#elif _MSC_VER >= 1400 || (_MSC_VER >= 1300 && !defined(_DLL))
	return _byteswap_ulong(value);
#elif CRYPTOPP_FAST_ROTATE(32)
	
	return (rotrFixed(value, 8U) & 0xff00ff00) | (rotlFixed(value, 8U) & 0x00ff00ff);
#else
	
	value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
	return rotlFixed(value, 16U);
#endif
}





inline word64 ByteReverse(word64 value)
{
#if defined(__GNUC__) && defined(CRYPTOPP_X86_ASM_AVAILABLE) && defined(__x86_64__)
	__asm__ ("bswap %0" : "=r" (value) : "0" (value));
	return value;
#elif defined(CRYPTOPP_BYTESWAP_AVAILABLE)
	return bswap_64(value);
#elif defined(_MSC_VER) && _MSC_VER >= 1300
	return _byteswap_uint64(value);
#elif CRYPTOPP_BOOL_SLOW_WORD64
	return (word64(ByteReverse(word32(value))) << 32) | ByteReverse(word32(value>>32));
#else
	value = ((value & W64LIT(0xFF00FF00FF00FF00)) >> 8) | ((value & W64LIT(0x00FF00FF00FF00FF)) << 8);
	value = ((value & W64LIT(0xFFFF0000FFFF0000)) >> 16) | ((value & W64LIT(0x0000FFFF0000FFFF)) << 16);
	return rotlFixed(value, 32U);
#endif
}




inline byte BitReverse(byte value)
{
	value = byte((value & 0xAA) >> 1) | byte((value & 0x55) << 1);
	value = byte((value & 0xCC) >> 2) | byte((value & 0x33) << 2);
	return rotlFixed(value, 4U);
}




inline word16 BitReverse(word16 value)
{
	value = word16((value & 0xAAAA) >> 1) | word16((value & 0x5555) << 1);
	value = word16((value & 0xCCCC) >> 2) | word16((value & 0x3333) << 2);
	value = word16((value & 0xF0F0) >> 4) | word16((value & 0x0F0F) << 4);
	return ByteReverse(value);
}




inline word32 BitReverse(word32 value)
{
	value = word32((value & 0xAAAAAAAA) >> 1) | word32((value & 0x55555555) << 1);
	value = word32((value & 0xCCCCCCCC) >> 2) | word32((value & 0x33333333) << 2);
	value = word32((value & 0xF0F0F0F0) >> 4) | word32((value & 0x0F0F0F0F) << 4);
	return ByteReverse(value);
}




inline word64 BitReverse(word64 value)
{
#if CRYPTOPP_BOOL_SLOW_WORD64
	return (word64(BitReverse(word32(value))) << 32) | BitReverse(word32(value>>32));
#else
	value = word64((value & W64LIT(0xAAAAAAAAAAAAAAAA)) >> 1) | word64((value & W64LIT(0x5555555555555555)) << 1);
	value = word64((value & W64LIT(0xCCCCCCCCCCCCCCCC)) >> 2) | word64((value & W64LIT(0x3333333333333333)) << 2);
	value = word64((value & W64LIT(0xF0F0F0F0F0F0F0F0)) >> 4) | word64((value & W64LIT(0x0F0F0F0F0F0F0F0F)) << 4);
	return ByteReverse(value);
#endif
}







template <class T>
inline T BitReverse(T value)
{
	if (sizeof(T) == 1)
		return (T)BitReverse((byte)value);
	else if (sizeof(T) == 2)
		return (T)BitReverse((word16)value);
	else if (sizeof(T) == 4)
		return (T)BitReverse((word32)value);
	else
	{
		CRYPTOPP_ASSERT(sizeof(T) == 8);
		return (T)BitReverse((word64)value);
	}
}








template <class T>
inline T ConditionalByteReverse(ByteOrder order, T value)
{
	return NativeByteOrderIs(order) ? value : ByteReverse(value);
}




































template <class T>
void ByteReverse(T *out, const T *in, size_t byteCount)
{
	CRYPTOPP_ASSERT(byteCount % sizeof(T) == 0);
	size_t count = byteCount/sizeof(T);
	for (size_t i=0; i<count; i++)
		out[i] = ByteReverse(in[i]);
}














template <class T>
inline void ConditionalByteReverse(ByteOrder order, T *out, const T *in, size_t byteCount)
{
	if (!NativeByteOrderIs(order))
		ByteReverse(out, in, byteCount);
	else if (in != out)
		memcpy_s(out, byteCount, in, byteCount);
}

template <class T>
inline void GetUserKey(ByteOrder order, T *out, size_t outlen, const byte *in, size_t inlen)
{
	const size_t U = sizeof(T);
	CRYPTOPP_ASSERT(inlen <= outlen*U);
	memcpy_s(out, outlen*U, in, inlen);
	memset_z((byte *)out+inlen, 0, outlen*U-inlen);
	ConditionalByteReverse(order, out, out, RoundUpToMultipleOf(inlen, U));
}

#ifndef CRYPTOPP_ALLOW_UNALIGNED_DATA_ACCESS
inline byte UnalignedGetWordNonTemplate(ByteOrder order, const byte *block, const byte *)
{
	CRYPTOPP_UNUSED(order);
	return block[0];
}

inline word16 UnalignedGetWordNonTemplate(ByteOrder order, const byte *block, const word16 *)
{
	return (order == BIG_ENDIAN_ORDER)
		? block[1] | (block[0] << 8)
		: block[0] | (block[1] << 8);
}

inline word32 UnalignedGetWordNonTemplate(ByteOrder order, const byte *block, const word32 *)
{
	return (order == BIG_ENDIAN_ORDER)
		? word32(block[3]) | (word32(block[2]) << 8) | (word32(block[1]) << 16) | (word32(block[0]) << 24)
		: word32(block[0]) | (word32(block[1]) << 8) | (word32(block[2]) << 16) | (word32(block[3]) << 24);
}

inline word64 UnalignedGetWordNonTemplate(ByteOrder order, const byte *block, const word64 *)
{
	return (order == BIG_ENDIAN_ORDER)
		?
		(word64(block[7]) |
		(word64(block[6]) <<  8) |
		(word64(block[5]) << 16) |
		(word64(block[4]) << 24) |
		(word64(block[3]) << 32) |
		(word64(block[2]) << 40) |
		(word64(block[1]) << 48) |
		(word64(block[0]) << 56))
		:
		(word64(block[0]) |
		(word64(block[1]) <<  8) |
		(word64(block[2]) << 16) |
		(word64(block[3]) << 24) |
		(word64(block[4]) << 32) |
		(word64(block[5]) << 40) |
		(word64(block[6]) << 48) |
		(word64(block[7]) << 56));
}

inline void UnalignedbyteNonTemplate(ByteOrder order, byte *block, byte value, const byte *xorBlock)
{
	CRYPTOPP_UNUSED(order);
	block[0] = (byte)(xorBlock ? (value ^ xorBlock[0]) : value);
}

inline void UnalignedbyteNonTemplate(ByteOrder order, byte *block, word16 value, const byte *xorBlock)
{
	if (order == BIG_ENDIAN_ORDER)
	{
		if (xorBlock)
		{
			block[0] = xorBlock[0] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 1);
			block[1] = xorBlock[1] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 0);
		}
		else
		{
			block[0] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 1);
			block[1] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 0);
		}
	}
	else
	{
		if (xorBlock)
		{
			block[0] = xorBlock[0] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 0);
			block[1] = xorBlock[1] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 1);
		}
		else
		{
			block[0] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 0);
			block[1] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 1);
		}
	}
}

inline void UnalignedbyteNonTemplate(ByteOrder order, byte *block, word32 value, const byte *xorBlock)
{
	if (order == BIG_ENDIAN_ORDER)
	{
		if (xorBlock)
		{
			block[0] = xorBlock[0] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 3);
			block[1] = xorBlock[1] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 2);
			block[2] = xorBlock[2] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 1);
			block[3] = xorBlock[3] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 0);
		}
		else
		{
			block[0] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 3);
			block[1] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 2);
			block[2] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 1);
			block[3] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 0);
		}
	}
	else
	{
		if (xorBlock)
		{
			block[0] = xorBlock[0] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 0);
			block[1] = xorBlock[1] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 1);
			block[2] = xorBlock[2] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 2);
			block[3] = xorBlock[3] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 3);
		}
		else
		{
			block[0] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 0);
			block[1] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 1);
			block[2] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 2);
			block[3] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 3);
		}
	}
}

inline void UnalignedbyteNonTemplate(ByteOrder order, byte *block, word64 value, const byte *xorBlock)
{
	if (order == BIG_ENDIAN_ORDER)
	{
		if (xorBlock)
		{
			block[0] = xorBlock[0] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 7);
			block[1] = xorBlock[1] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 6);
			block[2] = xorBlock[2] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 5);
			block[3] = xorBlock[3] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 4);
			block[4] = xorBlock[4] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 3);
			block[5] = xorBlock[5] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 2);
			block[6] = xorBlock[6] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 1);
			block[7] = xorBlock[7] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 0);
		}
		else
		{
			block[0] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 7);
			block[1] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 6);
			block[2] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 5);
			block[3] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 4);
			block[4] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 3);
			block[5] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 2);
			block[6] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 1);
			block[7] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 0);
		}
	}
	else
	{
		if (xorBlock)
		{
			block[0] = xorBlock[0] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 0);
			block[1] = xorBlock[1] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 1);
			block[2] = xorBlock[2] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 2);
			block[3] = xorBlock[3] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 3);
			block[4] = xorBlock[4] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 4);
			block[5] = xorBlock[5] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 5);
			block[6] = xorBlock[6] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 6);
			block[7] = xorBlock[7] ^ CRYPTOPP_GET_BYTE_AS_BYTE(value, 7);
		}
		else
		{
			block[0] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 0);
			block[1] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 1);
			block[2] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 2);
			block[3] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 3);
			block[4] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 4);
			block[5] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 5);
			block[6] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 6);
			block[7] = CRYPTOPP_GET_BYTE_AS_BYTE(value, 7);
		}
	}
}
#endif	

template <class T>
inline T GetWord(bool assumeAligned, ByteOrder order, const byte *block)
{






	CRYPTOPP_UNUSED(assumeAligned);
#ifdef CRYPTOPP_ALLOW_UNALIGNED_DATA_ACCESS
	return ConditionalByteReverse(order, *reinterpret_cast<const T *>((const void *)block));
#else
	T temp;
	memcpy(&temp, block, sizeof(T));
	return ConditionalByteReverse(order, temp);
#endif
}

template <class T>
inline void GetWord(bool assumeAligned, ByteOrder order, T &result, const byte *block)
{
	result = GetWord<T>(assumeAligned, order, block);
}

template <class T>
inline void PutWord(bool assumeAligned, ByteOrder order, byte *block, T value, const byte *xorBlock = NULL)
{







	CRYPTOPP_UNUSED(assumeAligned);
#ifdef CRYPTOPP_ALLOW_UNALIGNED_DATA_ACCESS
	*reinterpret_cast<T *>((void *)block) = ConditionalByteReverse(order, value) ^ (xorBlock ? *reinterpret_cast<const T *>((const void *)xorBlock) : 0);
#else
	T t1, t2 = 0;
	t1 = ConditionalByteReverse(order, value);
	if (xorBlock) memcpy(&t2, xorBlock, sizeof(T));
	memmove(block, &(t1 ^= t2), sizeof(T));
#endif
}

















template <class T, class B, bool A=false>
class GetBlock
{
public:
	
	
	GetBlock(const void *block)
		: m_block((const byte *)block) {}

	
	
	
	
	template <class U>
	inline GetBlock<T, B, A> & operator()(U &x)
	{
		CRYPTOPP_COMPILE_ASSERT(sizeof(U) >= sizeof(T));
		x = GetWord<T>(A, B::ToEnum(), m_block);
		m_block += sizeof(T);
		return *this;
	}

private:
	const byte *m_block;
};

















template <class T, class B, bool A=false>
class PutBlock
{
public:
	
	
	
	PutBlock(const void *xorBlock, void *block)
		: m_xorBlock((const byte *)xorBlock), m_block((byte *)block) {}

	
	
	
	
	template <class U>
	inline PutBlock<T, B, A> & operator()(U x)
	{
		PutWord(A, B::ToEnum(), m_block, (T)x, m_xorBlock);
		m_block += sizeof(T);
		if (m_xorBlock)
			m_xorBlock += sizeof(T);
		return *this;
	}

private:
	const byte *m_xorBlock;
	byte *m_block;
};










template <class T, class B, bool GA=false, bool PA=false>
struct BlockGetAndPut
{
	
	static inline GetBlock<T, B, GA> Get(const void *block) {return GetBlock<T, B, GA>(block);}
	typedef PutBlock<T, B, PA> Put;
};

template <class T>
std::string WordToString(T value, ByteOrder order = BIG_ENDIAN_ORDER)
{
	if (!NativeByteOrderIs(order))
		value = ByteReverse(value);

	return std::string((char *)&value, sizeof(value));
}

template <class T>
T StringToWord(const std::string &str, ByteOrder order = BIG_ENDIAN_ORDER)
{
	T value = 0;
	memcpy_s(&value, sizeof(value), str.data(), UnsignedMin(str.size(), sizeof(value)));
	return NativeByteOrderIs(order) ? value : ByteReverse(value);
}










template <bool overflow> struct SafeShifter;





template<> struct SafeShifter<true>
{
	
	
	
	
	
	template <class T>
	static inline T RightShift(T value, unsigned int bits)
	{
		CRYPTOPP_UNUSED(value); CRYPTOPP_UNUSED(bits);
		return 0;
	}

	
	
	
	
	
	template <class T>
	static inline T LeftShift(T value, unsigned int bits)
	{
		CRYPTOPP_UNUSED(value); CRYPTOPP_UNUSED(bits);
		return 0;
	}
};





template<> struct SafeShifter<false>
{
	
	
	
	
	
	template <class T>
	static inline T RightShift(T value, unsigned int bits)
	{
		return value >> bits;
	}

	
	
	
	
	
	template <class T>
	static inline T LeftShift(T value, unsigned int bits)
	{
		return value << bits;
	}
};










template <unsigned int bits, class T>
inline T SafeRightShift(T value)
{
	return SafeShifter<(bits>=(8*sizeof(T)))>::RightShift(value, bits);
}










template <unsigned int bits, class T>
inline T SafeLeftShift(T value)
{
	return SafeShifter<(bits>=(8*sizeof(T)))>::LeftShift(value, bits);
}



#define CRYPTOPP_BLOCK_1(n, t, s) t* m_##n() {return (t *)(void *)(m_aggregate+0);}     size_t SS1() {return       sizeof(t)*(s);} size_t m_##n##Size() {return (s);}
#define CRYPTOPP_BLOCK_2(n, t, s) t* m_##n() {return (t *)(void *)(m_aggregate+SS1());} size_t SS2() {return SS1()+sizeof(t)*(s);} size_t m_##n##Size() {return (s);}
#define CRYPTOPP_BLOCK_3(n, t, s) t* m_##n() {return (t *)(void *)(m_aggregate+SS2());} size_t SS3() {return SS2()+sizeof(t)*(s);} size_t m_##n##Size() {return (s);}
#define CRYPTOPP_BLOCK_4(n, t, s) t* m_##n() {return (t *)(void *)(m_aggregate+SS3());} size_t SS4() {return SS3()+sizeof(t)*(s);} size_t m_##n##Size() {return (s);}
#define CRYPTOPP_BLOCK_5(n, t, s) t* m_##n() {return (t *)(void *)(m_aggregate+SS4());} size_t SS5() {return SS4()+sizeof(t)*(s);} size_t m_##n##Size() {return (s);}
#define CRYPTOPP_BLOCK_6(n, t, s) t* m_##n() {return (t *)(void *)(m_aggregate+SS5());} size_t SS6() {return SS5()+sizeof(t)*(s);} size_t m_##n##Size() {return (s);}
#define CRYPTOPP_BLOCK_7(n, t, s) t* m_##n() {return (t *)(void *)(m_aggregate+SS6());} size_t SS7() {return SS6()+sizeof(t)*(s);} size_t m_##n##Size() {return (s);}
#define CRYPTOPP_BLOCK_8(n, t, s) t* m_##n() {return (t *)(void *)(m_aggregate+SS7());} size_t SS8() {return SS7()+sizeof(t)*(s);} size_t m_##n##Size() {return (s);}
#define CRYPTOPP_BLOCKS_END(i) size_t SST() {return SS##i();} void AllocateBlocks() {m_aggregate.New(SST());} AlignedSecByteBlock m_aggregate;

NAMESPACE_END

#if CRYPTOPP_MSC_VERSION
# pragma warning(pop)
#endif

#endif
