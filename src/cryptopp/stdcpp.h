#ifndef CRYPTOPP_STDCPP_H
#define CRYPTOPP_STDCPP_H

#if _MSC_VER >= 1500
#define _DO_NOT_DECLARE_INTERLOCKED_INTRINSICS_IN_MEMORY
#include <intrin.h>
#endif

#include <string>
#include <memory>
#include <exception>
#include <typeinfo>
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>
#include <limits>
#include <deque>
#include <list>
#include <map>
#include <new>


#if defined(_MSC_VER) && (_MSC_VER < 1900) && defined(_HAS_EXCEPTIONS) && (_HAS_EXCEPTIONS == 0)
namespace std {
  using ::type_info;
}
#endif


#if _MSC_VER >= 1600
#include <iterator>
#endif

#if defined(CRYPTOPP_CXX11_ATOMICS)
#include <atomic>
#endif

#if defined(CRYPTOPP_CXX11_SYNCHRONIZATION)
#include <mutex>
#endif

#if defined(CRYPTOPP_CXX11_RVALUES)
# include <utility>
#endif

#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <climits>


#if (__cplusplus < 201103L) && (!defined(_MSC_VER) || (_MSC_VER >= 1700))
# include <stdint.h>
#elif defined(_MSC_VER) && (_MSC_VER < 1700)
# include <stddef.h>
#endif


#ifdef CRYPTOPP_INCLUDE_VECTOR_CC
# include <vector.cc>
#endif


#if defined(CRYPTOPP_BSD_AVAILABLE)
# include <stdlib.h>
#elif defined(CRYPTOPP_UNIX_AVAILABLE) || defined(__sun) || defined(QNX)
# include <alloca.h>
#elif defined(CRYPTOPP_WIN32_AVAILABLE) || defined(__MINGW32__) || defined(__BORLANDC__)
# include <malloc.h>
#endif

#ifdef _MSC_VER
# pragma warning(disable: 4231)	
# ifdef _CRTAPI1
#  define CRYPTOPP_MSVCRT6
# endif
#endif

#endif
