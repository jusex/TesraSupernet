#ifndef CRYPTOPP_MD4_H
#define CRYPTOPP_MD4_H

#include "iterhash.h"

NAMESPACE_BEGIN(CryptoPP)

namespace Weak1 {


/*! \warning MD4 is considered insecure, and should not be used
	unless you absolutely need it for compatibility. */
class MD4 : public IteratedHashWithStaticTransform<word32, LittleEndian, 64, 16, MD4>
{
public:
	static void InitState(HashWordType *state);
	static void Transform(word32 *digest, const word32 *data);
	CRYPTOPP_CONSTEXPR static const char *StaticAlgorithmName() {return "MD4";}
};

}
#if CRYPTOPP_ENABLE_NAMESPACE_WEAK >= 1
namespace Weak {using namespace Weak1;}		
#else
using namespace Weak1;	
#ifdef __GNUC__
#warning "You may be using a weak algorithm that has been retained for backwards compatibility. Please '#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1' before including this .h file and prepend the class name with 'Weak::' to remove this warning."
#else
#pragma message("You may be using a weak algorithm that has been retained for backwards compatibility. Please '#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1' before including this .h file and prepend the class name with 'Weak::' to remove this warning.")
#endif
#endif

NAMESPACE_END

#endif
