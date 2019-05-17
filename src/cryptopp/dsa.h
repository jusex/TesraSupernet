




#ifndef CRYPTOPP_DSA_H
#define CRYPTOPP_DSA_H

#include "cryptlib.h"
#include "gfpcrypt.h"

NAMESPACE_BEGIN(CryptoPP)




enum DSASignatureFormat {
	
	DSA_P1363,
	
	DSA_DER,
	
	DSA_OPENPGP
};











size_t DSAConvertSignatureFormat(byte *buffer, size_t bufferSize, DSASignatureFormat toFormat,
	const byte *signature, size_t signatureLen, DSASignatureFormat fromFormat);

NAMESPACE_END

#endif
