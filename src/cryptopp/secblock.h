




#ifndef CRYPTOPP_SECBLOCK_H
#define CRYPTOPP_SECBLOCK_H

#include "config.h"
#include "stdcpp.h"
#include "misc.h"

#if CRYPTOPP_MSC_VERSION
# pragma warning(push)
# pragma warning(disable: 4700)
# if (CRYPTOPP_MSC_VERSION >= 1400)
#  pragma warning(disable: 6386)
# endif
#endif

NAMESPACE_BEGIN(CryptoPP)






template<class T>
class AllocatorBase
{
public:
	typedef T value_type;
	typedef size_t size_type;
#ifdef CRYPTOPP_MSVCRT6
	typedef ptrdiff_t difference_type;
#else
	typedef std::ptrdiff_t difference_type;
#endif
	typedef T * pointer;
	typedef const T * const_pointer;
	typedef T & reference;
	typedef const T & const_reference;

	pointer address(reference r) const {return (&r);}
	const_pointer address(const_reference r) const {return (&r); }
	void construct(pointer p, const T& val) {new (p) T(val);}
	void destroy(pointer p) {CRYPTOPP_UNUSED(p); p->~T();}

	
	
	
	
	
	
	CRYPTOPP_CONSTEXPR size_type max_size() const {return (SIZE_MAX/sizeof(T));}

#if defined(CRYPTOPP_CXX11_VARIADIC_TEMPLATES) || defined(CRYPTOPP_DOXYGEN_PROCESSING)

	
	
	
	
	
	
	
    template<typename U, typename... Args>
    void construct(U* ptr, Args&&... args) {::new ((void*)ptr) U(std::forward<Args>(args)...);}

	
	
	
	
    template<typename U>
    void destroy(U* ptr) {if(ptr) ptr->~U();}

#endif

protected:

	
	
	
	
	
	
	
	
	
	
	
	static void CheckSize(size_t size)
	{
		
		if (size > (SIZE_MAX/sizeof(T)))
			throw InvalidArgument("AllocatorBase: requested size would cause integer overflow");
	}
};

#define CRYPTOPP_INHERIT_ALLOCATOR_TYPES	\
typedef typename AllocatorBase<T>::value_type value_type;\
typedef typename AllocatorBase<T>::size_type size_type;\
typedef typename AllocatorBase<T>::difference_type difference_type;\
typedef typename AllocatorBase<T>::pointer pointer;\
typedef typename AllocatorBase<T>::const_pointer const_pointer;\
typedef typename AllocatorBase<T>::reference reference;\
typedef typename AllocatorBase<T>::const_reference const_reference;











template <class T, class A>
typename A::pointer StandardReallocate(A& alloc, T *oldPtr, typename A::size_type oldSize, typename A::size_type newSize, bool preserve)
{
	CRYPTOPP_ASSERT((oldPtr && oldSize) || !(oldPtr || oldSize));
	if (oldSize == newSize)
		return oldPtr;

	if (preserve)
	{
		typename A::pointer newPointer = alloc.allocate(newSize, NULL);
		const size_t copySize = STDMIN(oldSize, newSize) * sizeof(T);

		if (oldPtr && newPointer) {memcpy_s(newPointer, copySize, oldPtr, copySize);}
		alloc.deallocate(oldPtr, oldSize);
		return newPointer;
	}
	else
	{
		alloc.deallocate(oldPtr, oldSize);
		return alloc.allocate(newSize, NULL);
	}
}










template <class T, bool T_Align16 = false>
class AllocatorWithCleanup : public AllocatorBase<T>
{
public:
	CRYPTOPP_INHERIT_ALLOCATOR_TYPES

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	pointer allocate(size_type size, const void *ptr = NULL)
	{
		CRYPTOPP_UNUSED(ptr); CRYPTOPP_ASSERT(ptr == NULL);
		this->CheckSize(size);
		if (size == 0)
			return NULL;

#if CRYPTOPP_BOOL_ALIGN16
		
		if (T_Align16 && size*sizeof(T) >= 16)
			return (pointer)AlignedAllocate(size*sizeof(T));
#endif

		return (pointer)UnalignedAllocate(size*sizeof(T));
	}

	
	
	
	
	
	
	
	
	void deallocate(void *ptr, size_type size)
	{
		CRYPTOPP_ASSERT((ptr && size) || !(ptr || size));
		SecureWipeArray((pointer)ptr, size);

#if CRYPTOPP_BOOL_ALIGN16
		if (T_Align16 && size*sizeof(T) >= 16)
			return AlignedDeallocate(ptr);
#endif

		UnalignedDeallocate(ptr);
	}

	
	
	
	
	
	
	
	
	
	
	
	
	
	pointer reallocate(T *oldPtr, size_type oldSize, size_type newSize, bool preserve)
	{
		CRYPTOPP_ASSERT((oldPtr && oldSize) || !(oldPtr || oldSize));
		return StandardReallocate(*this, oldPtr, oldSize, newSize, preserve);
	}

	
	
	
	
	
	
	
	
	
    template <class U> struct rebind { typedef AllocatorWithCleanup<U, T_Align16> other; };
#if _MSC_VER >= 1500
	AllocatorWithCleanup() {}
	template <class U, bool A> AllocatorWithCleanup(const AllocatorWithCleanup<U, A> &) {}
#endif
};

CRYPTOPP_DLL_TEMPLATE_CLASS AllocatorWithCleanup<byte>;
CRYPTOPP_DLL_TEMPLATE_CLASS AllocatorWithCleanup<word16>;
CRYPTOPP_DLL_TEMPLATE_CLASS AllocatorWithCleanup<word32>;
CRYPTOPP_DLL_TEMPLATE_CLASS AllocatorWithCleanup<word64>;
#if defined(CRYPTOPP_WORD128_AVAILABLE)
CRYPTOPP_DLL_TEMPLATE_CLASS AllocatorWithCleanup<word128, true>; 
#endif
#if CRYPTOPP_BOOL_X86
CRYPTOPP_DLL_TEMPLATE_CLASS AllocatorWithCleanup<word, true>;	 
#endif









template <class T>
class NullAllocator : public AllocatorBase<T>
{
public:
	
	CRYPTOPP_INHERIT_ALLOCATOR_TYPES

	
	
	
	pointer allocate(size_type n, const void* unused = NULL)
	{
		CRYPTOPP_UNUSED(n); CRYPTOPP_UNUSED(unused);
		CRYPTOPP_ASSERT(false); return NULL;
	}

	void deallocate(void *p, size_type n)
	{
		CRYPTOPP_UNUSED(p); CRYPTOPP_UNUSED(n);
		CRYPTOPP_ASSERT(false);
	}

	CRYPTOPP_CONSTEXPR size_type max_size() const {return 0;}
	
};













template <class T, size_t S, class A = NullAllocator<T>, bool T_Align16 = false>
class FixedSizeAllocatorWithCleanup : public AllocatorBase<T>
{
public:
	CRYPTOPP_INHERIT_ALLOCATOR_TYPES

	
	FixedSizeAllocatorWithCleanup() : m_allocated(false) {}

	
	
	
	
	
	
	
	
	
	
	
	
	pointer allocate(size_type size)
	{
		CRYPTOPP_ASSERT(IsAlignedOn(m_array, 8));

		if (size <= S && !m_allocated)
		{
			m_allocated = true;
			return GetAlignedArray();
		}
		else
			return m_fallbackAllocator.allocate(size);
	}

	
	
	
	
	
	
	
	
	
	
	
	
	
	pointer allocate(size_type size, const void *hint)
	{
		if (size <= S && !m_allocated)
		{
			m_allocated = true;
			return GetAlignedArray();
		}
		else
			return m_fallbackAllocator.allocate(size, hint);
	}

	
	
	
	
	
	
	
	
	void deallocate(void *ptr, size_type size)
	{
		if (ptr == GetAlignedArray())
		{
			CRYPTOPP_ASSERT(size <= S);
			CRYPTOPP_ASSERT(m_allocated);
			m_allocated = false;
			SecureWipeArray((pointer)ptr, size);
		}
		else
			m_fallbackAllocator.deallocate(ptr, size);
	}

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	pointer reallocate(pointer oldPtr, size_type oldSize, size_type newSize, bool preserve)
	{
		if (oldPtr == GetAlignedArray() && newSize <= S)
		{
			CRYPTOPP_ASSERT(oldSize <= S);
			if (oldSize > newSize)
				SecureWipeArray(oldPtr+newSize, oldSize-newSize);
			return oldPtr;
		}

		pointer newPointer = allocate(newSize, NULL);
		if (preserve && newSize)
		{
			const size_t copySize = STDMIN(oldSize, newSize);
			memcpy_s(newPointer, sizeof(T)*newSize, oldPtr, sizeof(T)*copySize);
		}
		deallocate(oldPtr, oldSize);
		return newPointer;
	}

	CRYPTOPP_CONSTEXPR size_type max_size() const {return STDMAX(m_fallbackAllocator.max_size(), S);}

private:

#ifdef __BORLANDC__
	T* GetAlignedArray() {return m_array;}
	T m_array[S];
#else
	T* GetAlignedArray() {return (CRYPTOPP_BOOL_ALIGN16 && T_Align16) ? (T*)(void *)(((byte *)m_array) + (0-(size_t)m_array)%16) : m_array;}
	CRYPTOPP_ALIGN_DATA(8) T m_array[(CRYPTOPP_BOOL_ALIGN16 && T_Align16) ? S+8/sizeof(T) : S];
#endif

	A m_fallbackAllocator;
	bool m_allocated;
};





template <class T, class A = AllocatorWithCleanup<T> >
class SecBlock
{
public:
	typedef typename A::value_type value_type;
	typedef typename A::pointer iterator;
	typedef typename A::const_pointer const_iterator;
	typedef typename A::size_type size_type;

	
	
	
	
	
	explicit SecBlock(size_type size=0)
		: m_size(size), m_ptr(m_alloc.allocate(size, NULL)) { }

	
	
	
	SecBlock(const SecBlock<T, A> &t)
		: m_size(t.m_size), m_ptr(m_alloc.allocate(t.m_size, NULL)) {
			CRYPTOPP_ASSERT((!t.m_ptr && !m_size) || (t.m_ptr && m_size));
			if (t.m_ptr) {memcpy_s(m_ptr, m_size*sizeof(T), t.m_ptr, t.m_size*sizeof(T));}
		}

	
	
	
	
	
	
	
	
	SecBlock(const T *ptr, size_type len)
		: m_size(len), m_ptr(m_alloc.allocate(len, NULL)) {
			CRYPTOPP_ASSERT((!m_ptr && !m_size) || (m_ptr && m_size));
			if (ptr && m_ptr)
				memcpy_s(m_ptr, m_size*sizeof(T), ptr, len*sizeof(T));
			else if (m_size)
				memset(m_ptr, 0, m_size*sizeof(T));
		}

	~SecBlock()
		{m_alloc.deallocate(m_ptr, m_size);}

#ifdef __BORLANDC__
	operator T *() const
		{return (T*)m_ptr;}
#else
	operator const void *() const
		{return m_ptr;}
	operator void *()
		{return m_ptr;}

	operator const T *() const
		{return m_ptr;}
	operator T *()
		{return m_ptr;}
#endif

	
	
	iterator begin()
		{return m_ptr;}
	
	
	const_iterator begin() const
		{return m_ptr;}
	
	
	iterator end()
		{return m_ptr+m_size;}
	
	
	const_iterator end() const
		{return m_ptr+m_size;}

	
	
	typename A::pointer data() {return m_ptr;}
	
	
	typename A::const_pointer data() const {return m_ptr;}

	
	
	
	size_type size() const {return m_size;}
	
	
	bool empty() const {return m_size == 0;}

	
	
	byte * BytePtr() {return (byte *)m_ptr;}
	
	
	const byte * BytePtr() const {return (const byte *)m_ptr;}
	
	
	
	size_type SizeInBytes() const {return m_size*sizeof(T);}

	
	
	
	
	void Assign(const T *ptr, size_type len)
	{
		New(len);
		if (m_ptr && ptr && len)
			{memcpy_s(m_ptr, m_size*sizeof(T), ptr, len*sizeof(T));}
	}

	
	
	
	
	void Assign(const SecBlock<T, A> &t)
	{
		if (this != &t)
		{
			New(t.m_size);
			if (m_ptr && t.m_ptr && t.m_size)
				{memcpy_s(m_ptr, m_size*sizeof(T), t, t.m_size*sizeof(T));}
		}
	}

	
	
	
	
	SecBlock<T, A>& operator=(const SecBlock<T, A> &t)
	{
		
		Assign(t);
		return *this;
	}

	
	
	
	SecBlock<T, A>& operator+=(const SecBlock<T, A> &t)
	{
		CRYPTOPP_ASSERT((!t.m_ptr && !t.m_size) || (t.m_ptr && t.m_size));

		if(t.m_size)
		{
			const size_type oldSize = m_size;
			if(this != &t)  
			{
				Grow(m_size+t.m_size);
				memcpy_s(m_ptr+oldSize, (m_size-oldSize)*sizeof(T), t.m_ptr, t.m_size*sizeof(T));
			}
			else            
			{
				Grow(m_size*2);
				memcpy_s(m_ptr+oldSize, (m_size-oldSize)*sizeof(T), m_ptr, oldSize*sizeof(T));
			}
		}
		return *this;
	}

	
	
	
	
	SecBlock<T, A> operator+(const SecBlock<T, A> &t)
	{
		CRYPTOPP_ASSERT((!m_ptr && !m_size) || (m_ptr && m_size));
		CRYPTOPP_ASSERT((!t.m_ptr && !t.m_size) || (t.m_ptr && t.m_size));
		if(!t.m_size) return SecBlock(*this);

		SecBlock<T, A> result(m_size+t.m_size);
		if(m_size) {memcpy_s(result.m_ptr, result.m_size*sizeof(T), m_ptr, m_size*sizeof(T));}
		memcpy_s(result.m_ptr+m_size, (result.m_size-m_size)*sizeof(T), t.m_ptr, t.m_size*sizeof(T));
		return result;
	}

	
	
	
	
	
	
	bool operator==(const SecBlock<T, A> &t) const
	{
		return m_size == t.m_size &&
			VerifyBufsEqual(reinterpret_cast<const byte*>(m_ptr), reinterpret_cast<const byte*>(t.m_ptr), m_size*sizeof(T));
	}

	
	
	
	
	
	
	
	bool operator!=(const SecBlock<T, A> &t) const
	{
		return !operator==(t);
	}

	
	
	
	
	
	
	
	void New(size_type newSize)
	{
		m_ptr = m_alloc.reallocate(m_ptr, m_size, newSize, false);
		m_size = newSize;
	}

	
	
	
	
	
	
	
	void CleanNew(size_type newSize)
	{
		New(newSize);
		if (m_ptr) {memset_z(m_ptr, 0, m_size*sizeof(T));}
	}

	
	
	
	
	
	
	
	void Grow(size_type newSize)
	{
		if (newSize > m_size)
		{
			m_ptr = m_alloc.reallocate(m_ptr, m_size, newSize, true);
			m_size = newSize;
		}
	}

	
	
	
	
	
	
	
	void CleanGrow(size_type newSize)
	{
		if (newSize > m_size)
		{
			m_ptr = m_alloc.reallocate(m_ptr, m_size, newSize, true);
			memset_z(m_ptr+m_size, 0, (newSize-m_size)*sizeof(T));
			m_size = newSize;
		}
	}

	
	
	
	
	
	
	void resize(size_type newSize)
	{
		m_ptr = m_alloc.reallocate(m_ptr, m_size, newSize, true);
		m_size = newSize;
	}

	
	
	
	void swap(SecBlock<T, A> &b)
	{
		
		std::swap(m_alloc, b.m_alloc);
		std::swap(m_size, b.m_size);
		std::swap(m_ptr, b.m_ptr);
	}


	A m_alloc;
	size_type m_size;
	T *m_ptr;
};

#ifdef CRYPTOPP_DOXYGEN_PROCESSING


class SecByteBlock : public SecBlock<byte> {};


class SecWordBlock : public SecBlock<word> {};


class AlignedSecByteBlock : public SecBlock<byte, AllocatorWithCleanup<byte, true> > {};
#else
typedef SecBlock<byte> SecByteBlock;
typedef SecBlock<word> SecWordBlock;
typedef SecBlock<byte, AllocatorWithCleanup<byte, true> > AlignedSecByteBlock;
#endif









template <class T, unsigned int S, class A = FixedSizeAllocatorWithCleanup<T, S> >
class FixedSizeSecBlock : public SecBlock<T, A>
{
public:
	
	explicit FixedSizeSecBlock() : SecBlock<T, A>(S) {}
};






template <class T, unsigned int S, bool T_Align16 = true>
class FixedSizeAlignedSecBlock : public FixedSizeSecBlock<T, S, FixedSizeAllocatorWithCleanup<T, S, NullAllocator<T>, T_Align16> >
{
};






template <class T, unsigned int S, class A = FixedSizeAllocatorWithCleanup<T, S, AllocatorWithCleanup<T> > >
class SecBlockWithHint : public SecBlock<T, A>
{
public:
	
	explicit SecBlockWithHint(size_t size) : SecBlock<T, A>(size) {}
};

template<class T, bool A, class U, bool B>
inline bool operator==(const CryptoPP::AllocatorWithCleanup<T, A>&, const CryptoPP::AllocatorWithCleanup<U, B>&) {return (true);}
template<class T, bool A, class U, bool B>
inline bool operator!=(const CryptoPP::AllocatorWithCleanup<T, A>&, const CryptoPP::AllocatorWithCleanup<U, B>&) {return (false);}

NAMESPACE_END

NAMESPACE_BEGIN(std)
template <class T, class A>
inline void swap(CryptoPP::SecBlock<T, A> &a, CryptoPP::SecBlock<T, A> &b)
{
	a.swap(b);
}

#if defined(_STLP_DONT_SUPPORT_REBIND_MEMBER_TEMPLATE) || (defined(_STLPORT_VERSION) && !defined(_STLP_MEMBER_TEMPLATE_CLASSES))

template <class _Tp1, class _Tp2>
inline CryptoPP::AllocatorWithCleanup<_Tp2>&
__stl_alloc_rebind(CryptoPP::AllocatorWithCleanup<_Tp1>& __a, const _Tp2*)
{
	return (CryptoPP::AllocatorWithCleanup<_Tp2>&)(__a);
}
#endif

NAMESPACE_END

#if CRYPTOPP_MSC_VERSION
# pragma warning(pop)
#endif

#endif
