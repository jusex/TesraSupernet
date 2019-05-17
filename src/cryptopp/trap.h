














#ifndef CRYPTOPP_TRAP_H
#define CRYPTOPP_TRAP_H

#include "config.h"

#if CRYPTOPP_DEBUG
#  include <iostream>
#  include <sstream>
#  if defined(CRYPTOPP_BSD_AVAILABLE) || defined(CRYPTOPP_UNIX_AVAILABLE)
#    include "ossig.h"
#  elif defined(CRYPTOPP_WIN32_AVAILABLE)
#    if (_MSC_VER >= 1400)
#      include <intrin.h>
#    endif
#  endif
#endif 



#if defined(CRYPTOPP_DOXYGEN_PROCESSING)


























#  define CRYPTOPP_ASSERT(exp) { ... }
#endif

#if CRYPTOPP_DEBUG && (defined(CRYPTOPP_BSD_AVAILABLE) || defined(CRYPTOPP_UNIX_AVAILABLE))
#  define CRYPTOPP_ASSERT(exp) {                                  \
    if (!(exp)) {                                                 \
      std::ostringstream oss;                                     \
      oss << "Assertion failed: " << (char*)(__FILE__) << "("     \
          << (int)(__LINE__) << "): " << (char*)(__func__)        \
          << std::endl;                                           \
      std::cerr << oss.str();                                     \
      raise(SIGTRAP);                                             \
    }                                                             \
  }
#elif CRYPTOPP_DEBUG && defined(CRYPTOPP_WIN32_AVAILABLE)
#  define CRYPTOPP_ASSERT(exp) {                                  \
    if (!(exp)) {                                                 \
      std::ostringstream oss;                                     \
      oss << "Assertion failed: " << (char*)(__FILE__) << "("     \
          << (int)(__LINE__) << "): " << (char*)(__FUNCTION__)    \
          << std::endl;                                           \
      std::cerr << oss.str();                                     \
      __debugbreak();                                             \
    }                                                             \
  }
#endif 



#ifndef CRYPTOPP_ASSERT
#  define CRYPTOPP_ASSERT(exp) ((void)(exp))
#endif

NAMESPACE_BEGIN(CryptoPP)



#if (CRYPTOPP_DEBUG && (defined(CRYPTOPP_BSD_AVAILABLE) || defined(CRYPTOPP_UNIX_AVAILABLE))) || defined(CRYPTOPP_DOXYGEN_PROCESSING)




































#if defined(CRYPTOPP_DOXYGEN_PROCESSING)
class DebugTrapHandler : public SignalHandler<SIGILL, false> { };
#else
typedef SignalHandler<SIGILL, false> DebugTrapHandler;
#endif

#endif  

NAMESPACE_END

#endif 
