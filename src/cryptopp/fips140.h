









#ifndef CRYPTOPP_FIPS140_H
#define CRYPTOPP_FIPS140_H

#include "cryptlib.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)





class CRYPTOPP_DLL SelfTestFailure : public Exception
{
public:
	explicit SelfTestFailure(const std::string &s) : Exception(OTHER_ERROR, s) {}
};







CRYPTOPP_DLL bool CRYPTOPP_API FIPS_140_2_ComplianceEnabled();


enum PowerUpSelfTestStatus {

	
	POWER_UP_SELF_TEST_NOT_DONE,
	
	
	POWER_UP_SELF_TEST_FAILED,
	
	
	POWER_UP_SELF_TEST_PASSED
};








CRYPTOPP_DLL void CRYPTOPP_API DoPowerUpSelfTest(const char *moduleFilename, const byte *expectedModuleMac);







CRYPTOPP_DLL void CRYPTOPP_API DoDllPowerUpSelfTest();



CRYPTOPP_DLL void CRYPTOPP_API SimulatePowerUpSelfTestFailure();



CRYPTOPP_DLL PowerUpSelfTestStatus CRYPTOPP_API GetPowerUpSelfTestStatus();

#ifndef CRYPTOPP_DOXYGEN_PROCESSING
typedef PowerUpSelfTestStatus (CRYPTOPP_API * PGetPowerUpSelfTestStatus)();
#endif



CRYPTOPP_DLL MessageAuthenticationCode * CRYPTOPP_API NewIntegrityCheckingMAC();







CRYPTOPP_DLL bool CRYPTOPP_API IntegrityCheckModule(const char *moduleFilename, const byte *expectedModuleMac, SecByteBlock *pActualMac = NULL, unsigned long *pMacFileLocation = NULL);

#ifndef CRYPTOPP_DOXYGEN_PROCESSING

bool PowerUpSelfTestInProgressOnThisThread();

void SetPowerUpSelfTestInProgressOnThisThread(bool inProgress);

void SignaturePairwiseConsistencyTest(const PK_Signer &signer, const PK_Verifier &verifier);
void EncryptionPairwiseConsistencyTest(const PK_Encryptor &encryptor, const PK_Decryptor &decryptor);

void SignaturePairwiseConsistencyTest_FIPS_140_Only(const PK_Signer &signer, const PK_Verifier &verifier);
void EncryptionPairwiseConsistencyTest_FIPS_140_Only(const PK_Encryptor &encryptor, const PK_Decryptor &decryptor);
#endif






#define CRYPTOPP_DUMMY_DLL_MAC "MAC_51f34b8db820ae8"

NAMESPACE_END

#endif
