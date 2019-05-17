#ifndef CRYPTOPP_DLL_ONLY
# define CRYPTOPP_DEFAULT_NO_DLL
#endif

#include "dll.h"
#include "cryptlib.h"
#include "filters.h"

USING_NAMESPACE(CryptoPP)
USING_NAMESPACE(std)

void FIPS140_SampleApplication()
{
	if (!FIPS_140_2_ComplianceEnabled())
	{
		cerr << "FIPS 140-2 compliance was turned off at compile time.\n";
		abort();
	}

	
	if (GetPowerUpSelfTestStatus() != POWER_UP_SELF_TEST_PASSED)
	{
		cerr << "Automatic power-up self test failed.\n";
		abort();
	}
	cout << "0. Automatic power-up self test passed.\n";

	
	SimulatePowerUpSelfTestFailure();
	try
	{
		
		AES::Encryption aes;

		
		cerr << "Use of AES failed to cause an exception after power-up self test error.\n";
		abort();
	}
	catch (SelfTestFailure &e)
	{
		cout << "1. Caught expected exception when simulating self test failure. Exception message follows: ";
		cout << e.what() << endl;
	}

	
	DoDllPowerUpSelfTest();
	if (GetPowerUpSelfTestStatus() != POWER_UP_SELF_TEST_PASSED)
	{
		cerr << "Re-do power-up self test failed.\n";
		abort();
	}
	cout << "2. Re-do power-up self test passed.\n";

	
	const byte key[] = {0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef, 0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef, 0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef};
	const byte iv[] = {0x12,0x34,0x56,0x78,0x90,0xab,0xcd,0xef};
	const byte plaintext[] = {	
		0x4e,0x6f,0x77,0x20,0x69,0x73,0x20,0x74,
		0x68,0x65,0x20,0x74,0x69,0x6d,0x65,0x20,
		0x66,0x6f,0x72,0x20,0x61,0x6c,0x6c,0x20};
	byte ciphertext[24];
	byte decrypted[24];

	CFB_FIPS_Mode<DES_EDE3>::Encryption encryption_DES_EDE3_CFB;
	encryption_DES_EDE3_CFB.SetKeyWithIV(key, sizeof(key), iv);
	encryption_DES_EDE3_CFB.ProcessString(ciphertext, plaintext, 24);

	CFB_FIPS_Mode<DES_EDE3>::Decryption decryption_DES_EDE3_CFB;
	decryption_DES_EDE3_CFB.SetKeyWithIV(key, sizeof(key), iv);
	decryption_DES_EDE3_CFB.ProcessString(decrypted, ciphertext, 24);

	if (memcmp(plaintext, decrypted, 24) != 0)
	{
		cerr << "DES-EDE3-CFB Encryption/decryption failed.\n";
		abort();
	}
	cout << "3. DES-EDE3-CFB Encryption/decryption succeeded.\n";

	
	const byte message[] = {'a', 'b', 'c'};
	const byte expectedDigest[] = {0xA9,0x99,0x3E,0x36,0x47,0x06,0x81,0x6A,0xBA,0x3E,0x25,0x71,0x78,0x50,0xC2,0x6C,0x9C,0xD0,0xD8,0x9D};
	byte digest[20];

	SHA1 sha;
	sha.Update(message, 3);
	sha.Final(digest);

	if (memcmp(digest, expectedDigest, 20) != 0)
	{
		cerr << "SHA-1 hash failed.\n";
		abort();
	}
	cout << "4. SHA-1 hash succeeded.\n";

	
#ifdef OS_RNG_AVAILABLE
	AutoSeededX917RNG<AES> rng;
#else
	
	RandomNumberGenerator &rng(NullRNG());
#endif

	
	DSA::PrivateKey dsaPrivateKey;
	dsaPrivateKey.GenerateRandomWithKeySize(rng, 1024);
	DSA::PublicKey dsaPublicKey;
	dsaPublicKey.AssignFrom(dsaPrivateKey);
	if (!dsaPrivateKey.Validate(rng, 3) || !dsaPublicKey.Validate(rng, 3))
	{
		cerr << "DSA key generation failed.\n";
		abort();
	}
	cout << "5. DSA key generation succeeded.\n";

	
	std::string encodedDsaPublicKey, encodedDsaPrivateKey;
	dsaPublicKey.DEREncode(StringSink(encodedDsaPublicKey).Ref());
	dsaPrivateKey.DEREncode(StringSink(encodedDsaPrivateKey).Ref());

	
	DSA::PrivateKey decodedDsaPrivateKey;
	decodedDsaPrivateKey.BERDecode(StringStore(encodedDsaPrivateKey).Ref());
	DSA::PublicKey decodedDsaPublicKey;
	decodedDsaPublicKey.BERDecode(StringStore(encodedDsaPublicKey).Ref());

	if (!decodedDsaPrivateKey.Validate(rng, 3) || !decodedDsaPublicKey.Validate(rng, 3))
	{
		cerr << "DSA key encode/decode failed.\n";
		abort();
	}
	cout << "6. DSA key encode/decode succeeded.\n";

	
	byte signature[40];
	DSA::Signer signer(dsaPrivateKey);
	CRYPTOPP_ASSERT(signer.SignatureLength() == 40);
	signer.SignMessage(rng, message, 3, signature);

	DSA::Verifier verifier(dsaPublicKey);
	if (!verifier.VerifyMessage(message, 3, signature, sizeof(signature)))
	{
		cerr << "DSA signature and verification failed.\n";
		abort();
	}
	cout << "7. DSA signature and verification succeeded.\n";


	
	signature[0] ^= 1;
	if (verifier.VerifyMessage(message, 3, signature, sizeof(signature)))
	{
		cerr << "DSA signature verification failed to detect bad signature.\n";
		abort();
	}
	cout << "8. DSA signature verification successfully detected bad signature.\n";

	
	try
	{
		ECB_Mode<DES_EDE3>::Encryption encryption_DES_EDE3_ECB;
		encryption_DES_EDE3_ECB.SetKey(key, 5);

		
		cerr << "DES-EDE3 implementation did not detect use of invalid key length.\n";
		abort();
	}
	catch (InvalidArgument &e)
	{
		cout << "9. Caught expected exception when using invalid key length. Exception message follows: ";
		cout << e.what() << endl;
	}

	cout << "\nFIPS 140-2 Sample Application completed normally.\n";
}

#ifdef CRYPTOPP_IMPORTS

static PNew s_pNew = NULL;
static PDelete s_pDelete = NULL;

extern "C" __declspec(dllexport) void __cdecl SetNewAndDeleteFromCryptoPP(PNew pNew, PDelete pDelete, PSetNewHandler pSetNewHandler)
{
	(void)(pSetNewHandler);
	s_pNew = pNew;
	s_pDelete = pDelete;
}

void * __cdecl operator new (size_t size)
{
	return s_pNew(size);
}

void __cdecl operator delete (void * p)
{
	s_pDelete(p);
}

#endif

#ifdef CRYPTOPP_DLL_ONLY

int __cdecl main()
{
	FIPS140_SampleApplication();
	return 0;
}

#endif
