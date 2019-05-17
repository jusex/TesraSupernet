




/*!	\mainpage Crypto++ Library 5.6.5 API Reference
<dl>
<dt>Abstract Base Classes<dd>
	cryptlib.h
<dt>Authenticated Encryption Modes<dd>
	CCM, EAX, \ref GCM "GCM (2K tables)", \ref GCM "GCM (64K tables)"
<dt>Block Ciphers<dd>
	\ref Rijndael "AES", Weak::ARC4, Blowfish, BTEA, Camellia, CAST128, CAST256, DES, \ref DES_EDE2 "2-key Triple-DES", \ref DES_EDE3 "3-key Triple-DES",
	\ref DES_XEX3 "DESX", GOST, IDEA, \ref LR "Luby-Rackoff", MARS, RC2, RC5, RC6, \ref SAFER_K "SAFER-K", \ref SAFER_SK "SAFER-SK", SEED, Serpent,
	\ref SHACAL2 "SHACAL-2", SHARK, SKIPJACK,
Square, TEA, \ref ThreeWay "3-Way", Twofish, XTEA
<dt>Stream Ciphers<dd>
	ChaCha8, ChaCha12, ChaCha20, \ref Panama "Panama-LE", \ref Panama "Panama-BE", Salsa20, \ref SEAL "SEAL-LE", \ref SEAL "SEAL-BE", WAKE, XSalsa20
<dt>Hash Functions<dd>
	BLAKE2s, BLAKE2b, \ref Keccak "Keccak (F1600)", SHA1, SHA224, SHA256, SHA384, SHA512, \ref SHA3 "SHA-3", Tiger, Whirlpool, RIPEMD160, RIPEMD320, RIPEMD128, RIPEMD256, Weak::MD2, Weak::MD4, Weak::MD5
<dt>Non-Cryptographic Checksums<dd>
	CRC32, Adler32
<dt>Message Authentication Codes<dd>
	VMAC, HMAC, CBC_MAC, CMAC, DMAC, TTMAC, \ref GCM "GCM (GMAC)", BLAKE2
<dt>Random Number Generators<dd>
	NullRNG(), LC_RNG, RandomPool, BlockingRng, NonblockingRng, AutoSeededRandomPool, AutoSeededX917RNG,
	\ref MersenneTwister "MersenneTwister (MT19937 and MT19937-AR)", RDRAND, RDSEED
<dt>Key Derivation and Password-based Cryptography<dd>
	HKDF, \ref PKCS12_PBKDF "PBKDF (PKCS #12)", \ref PKCS5_PBKDF1 "PBKDF-1 (PKCS #5)", \ref PKCS5_PBKDF2_HMAC "PBKDF-2/HMAC (PKCS #5)"
<dt>Public Key Cryptosystems<dd>
	DLIES, ECIES, LUCES, RSAES, RabinES, LUC_IES
<dt>Public Key Signature Schemes<dd>
	DSA2, GDSA, ECDSA, NR, ECNR, LUCSS, RSASS, RSASS_ISO, RabinSS, RWSS, ESIGN
<dt>Key Agreement<dd>
	DH, DH2, \ref MQV_Domain "MQV", \ref HMQV_Domain "HMQV", \ref FHMQV_Domain "FHMQV", ECDH, ECMQV, ECHMQV, ECFHMQV, XTR_DH
<dt>Algebraic Structures<dd>
	Integer, PolynomialMod2, PolynomialOver, RingOfPolynomialsOver,
	ModularArithmetic, MontgomeryRepresentation, GFP2_ONB, GF2NP, GF256, GF2_32, EC2N, ECP
<dt>Secret Sharing and Information Dispersal<dd>
	SecretSharing, SecretRecovery, InformationDispersal, InformationRecovery
<dt>Compression<dd>
	Deflator, Inflator, Gzip, Gunzip, ZlibCompressor, ZlibDecompressor
<dt>Input Source Classes<dd>
	StringSource, ArraySource, FileSource, SocketSource, WindowsPipeSource, RandomNumberSource
<dt>Output Sink Classes<dd>
	StringSinkTemplate, StringSink, ArraySink, FileSink, SocketSink, WindowsPipeSink, RandomNumberSink
<dt>Filter Wrappers<dd>
	StreamTransformationFilter, HashFilter, HashVerificationFilter, SignerFilter, SignatureVerificationFilter
<dt>Binary to Text Encoders and Decoders<dd>
	HexEncoder, HexDecoder, Base64Encoder, Base64Decoder, Base64URLEncoder, Base64URLDecoder, Base32Encoder, Base32Decoder
<dt>Wrappers for OS features<dd>
	Timer, Socket, WindowsHandle, ThreadLocalStorage, ThreadUserTimer
<dt>FIPS 140 validated cryptography<dd>
	fips140.h
</dl>

In the DLL version of Crypto++, only the following implementation class are available.
<dl>
<dt>Block Ciphers<dd>
	AES, \ref DES_EDE2 "2-key Triple-DES", \ref DES_EDE3 "3-key Triple-DES", SKIPJACK
<dt>Cipher Modes (replace template parameter BC with one of the block ciphers above)<dd>
	\ref ECB_Mode "ECB_Mode<BC>", \ref CTR_Mode "CTR_Mode<BC>", \ref CBC_Mode "CBC_Mode<BC>", \ref CFB_FIPS_Mode "CFB_FIPS_Mode<BC>", \ref OFB_Mode "OFB_Mode<BC>", \ref GCM "GCM<AES>"
<dt>Hash Functions<dd>
	SHA1, SHA224, SHA256, SHA384, SHA512
<dt>Public Key Signature Schemes (replace template parameter H with one of the hash functions above)<dd>
	RSASS\<PKCS1v15, H\>, RSASS\<PSS, H\>, RSASS_ISO\<H\>, RWSS\<P1363_EMSA2, H\>, DSA, ECDSA\<ECP, H\>, ECDSA\<EC2N, H\>
<dt>Message Authentication Codes (replace template parameter H with one of the hash functions above)<dd>
	HMAC\<H\>, CBC_MAC\<DES_EDE2\>, CBC_MAC\<DES_EDE3\>, GCM\<AES\>
<dt>Random Number Generators<dd>
	DefaultAutoSeededRNG (AutoSeededX917RNG\<AES\>)
<dt>Key Agreement<dd>
	DH, DH2
<dt>Public Key Cryptosystems<dd>
	RSAES\<OAEP\<SHA1\> \>
</dl>

<p>This reference manual is a work in progress. Some classes are lack detailed descriptions.
<p>Click <a href="CryptoPPRef.zip">here</a> to download a zip archive containing this manual.
<p>Thanks to Ryan Phillips for providing the Doxygen configuration file
and getting us started on the manual.
*/

#ifndef CRYPTOPP_CRYPTLIB_H
#define CRYPTOPP_CRYPTLIB_H

#include "config.h"
#include "stdcpp.h"
#include "trap.h"

#if defined(CRYPTOPP_BSD_AVAILABLE) || defined(CRYPTOPP_UNIX_AVAILABLE)
# include <signal.h>
#endif

#if CRYPTOPP_MSC_VERSION
# pragma warning(push)
# pragma warning(disable: 4127 4189 4702)
#endif

NAMESPACE_BEGIN(CryptoPP)


class Integer;
class RandomNumberGenerator;
class BufferedTransformation;



enum CipherDir {
	
	ENCRYPTION,
	
	DECRYPTION};


const unsigned long INFINITE_TIME = ULONG_MAX;



template <typename ENUM_TYPE, int VALUE>
struct EnumToType
{
	static ENUM_TYPE ToEnum() {return (ENUM_TYPE)VALUE;}
};




enum ByteOrder {
	
	LITTLE_ENDIAN_ORDER = 0,
	
	BIG_ENDIAN_ORDER = 1};


typedef EnumToType<ByteOrder, LITTLE_ENDIAN_ORDER> LittleEndian;

typedef EnumToType<ByteOrder, BIG_ENDIAN_ORDER> BigEndian;






class CRYPTOPP_DLL Exception : public std::exception
{
public:
	
	
	enum ErrorType {
		
		NOT_IMPLEMENTED,
		
		INVALID_ARGUMENT,
		
		CANNOT_FLUSH,
		
		DATA_INTEGRITY_CHECK_FAILED,
		
		INVALID_DATA_FORMAT,
		
		IO_ERROR,
		
		OTHER_ERROR
	};

	
	explicit Exception(ErrorType errorType, const std::string &s) : m_errorType(errorType), m_what(s) {}
	virtual ~Exception() throw() {}

	
	const char *what() const throw() {return (m_what.c_str());}
	
	const std::string &GetWhat() const {return m_what;}
	
	void SetWhat(const std::string &s) {m_what = s;}
	
	ErrorType GetErrorType() const {return m_errorType;}
	
	void SetErrorType(ErrorType errorType) {m_errorType = errorType;}

private:
	ErrorType m_errorType;
	std::string m_what;
};


class CRYPTOPP_DLL InvalidArgument : public Exception
{
public:
	explicit InvalidArgument(const std::string &s) : Exception(INVALID_ARGUMENT, s) {}
};


class CRYPTOPP_DLL InvalidDataFormat : public Exception
{
public:
	explicit InvalidDataFormat(const std::string &s) : Exception(INVALID_DATA_FORMAT, s) {}
};


class CRYPTOPP_DLL InvalidCiphertext : public InvalidDataFormat
{
public:
	explicit InvalidCiphertext(const std::string &s) : InvalidDataFormat(s) {}
};


class CRYPTOPP_DLL NotImplemented : public Exception
{
public:
	explicit NotImplemented(const std::string &s) : Exception(NOT_IMPLEMENTED, s) {}
};


class CRYPTOPP_DLL CannotFlush : public Exception
{
public:
	explicit CannotFlush(const std::string &s) : Exception(CANNOT_FLUSH, s) {}
};


class CRYPTOPP_DLL OS_Error : public Exception
{
public:
	OS_Error(ErrorType errorType, const std::string &s, const std::string& operation, int errorCode)
		: Exception(errorType, s), m_operation(operation), m_errorCode(errorCode) {}
	~OS_Error() throw() {}

	
	const std::string & GetOperation() const {return m_operation;}
	
	int GetErrorCode() const {return m_errorCode;}

protected:
	std::string m_operation;
	int m_errorCode;
};



struct CRYPTOPP_DLL DecodingResult
{
	
	
	explicit DecodingResult() : isValidCoding(false), messageLength(0) {}
	
	
	
	explicit DecodingResult(size_t len) : isValidCoding(true), messageLength(len) {}

	
	
	
	bool operator==(const DecodingResult &rhs) const {return isValidCoding == rhs.isValidCoding && messageLength == rhs.messageLength;}
	
	
	
	
	bool operator!=(const DecodingResult &rhs) const {return !operator==(rhs);}

	
	bool isValidCoding;
	
	size_t messageLength;

#ifdef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY
	operator size_t() const {return isValidCoding ? messageLength : 0;}
#endif
};












class CRYPTOPP_NO_VTABLE NameValuePairs
{
public:
	virtual ~NameValuePairs() {}

	
	
	
	class CRYPTOPP_DLL ValueTypeMismatch : public InvalidArgument
	{
	public:
		
		
		
		
		ValueTypeMismatch(const std::string &name, const std::type_info &stored, const std::type_info &retrieving)
			: InvalidArgument("NameValuePairs: type mismatch for '" + name + "', stored '" + stored.name() + "', trying to retrieve '" + retrieving.name() + "'")
			, m_stored(stored), m_retrieving(retrieving) {}

		
		
		const std::type_info & GetStoredTypeInfo() const {return m_stored;}

		
		
		const std::type_info & GetRetrievingTypeInfo() const {return m_retrieving;}

	private:
		const std::type_info &m_stored;
		const std::type_info &m_retrieving;
	};

	
	
	
	template <class T>
	bool GetThisObject(T &object) const
	{
		return GetValue((std::string("ThisObject:")+typeid(T).name()).c_str(), object);
	}

	
	
	
	template <class T>
	bool GetThisPointer(T *&ptr) const
	{
		return GetValue((std::string("ThisPointer:")+typeid(T).name()).c_str(), ptr);
	}

	
	
	
	
	
	
	
	template <class T>
	bool GetValue(const char *name, T &value) const
	{
		return GetVoidValue(name, typeid(T), &value);
	}

	
	
	
	
	
	
	
	template <class T>
	T GetValueWithDefault(const char *name, T defaultValue) const
	{
		T value;
		bool result = GetValue(name, value);
		
		if (result) {return value;}
		return defaultValue;
	}

	
	
	
	CRYPTOPP_DLL std::string GetValueNames() const
		{std::string result; GetValue("ValueNames", result); return result;}

	
	
	
	
	
	
	
	
	CRYPTOPP_DLL bool GetIntValue(const char *name, int &value) const
		{return GetValue(name, value);}

	
	
	
	
	
	
	CRYPTOPP_DLL int GetIntValueWithDefault(const char *name, int defaultValue) const
		{return GetValueWithDefault(name, defaultValue);}

	
	
	
	
	
	
	
	
	
	CRYPTOPP_DLL static void CRYPTOPP_API ThrowIfTypeMismatch(const char *name, const std::type_info &stored, const std::type_info &retrieving)
		{if (stored != retrieving) throw ValueTypeMismatch(name, stored, retrieving);}

	
	
	
	
	
	
	
	
	
	
	template <class T>
	void GetRequiredParameter(const char *className, const char *name, T &value) const
	{
		if (!GetValue(name, value))
			throw InvalidArgument(std::string(className) + ": missing required parameter '" + name + "'");
	}

	
	
	
	
	
	
	
	
	
	CRYPTOPP_DLL void GetRequiredIntParameter(const char *className, const char *name, int &value) const
	{
		if (!GetIntValue(name, value))
			throw InvalidArgument(std::string(className) + ": missing required parameter '" + name + "'");
	}

	
	
	
	
	
	
	
	
	
	
	CRYPTOPP_DLL virtual bool GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const =0;
};

#if CRYPTOPP_DOXYGEN_PROCESSING








DOCUMENTED_NAMESPACE_BEGIN(Name)

DOCUMENTED_NAMESPACE_END














DOCUMENTED_NAMESPACE_BEGIN(Weak)

DOCUMENTED_NAMESPACE_END
#endif


extern CRYPTOPP_DLL const NameValuePairs &g_nullNameValuePairs;







class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE Clonable
{
public:
	virtual ~Clonable() {}

	
	
	
	
	
	virtual Clonable* Clone() const {throw NotImplemented("Clone() is not implemented yet.");}	
};



class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE Algorithm : public Clonable
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~Algorithm() {}
#endif

	
	
	
	
	
	
	
	
	Algorithm(bool checkSelfTestStatus = true);

	
	
	
	
	
	
	virtual std::string AlgorithmName() const {return "unknown";}
};




class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE SimpleKeyingInterface
{
public:
	virtual ~SimpleKeyingInterface() {}

	
	virtual size_t MinKeyLength() const =0;
	
	virtual size_t MaxKeyLength() const =0;
	
	virtual size_t DefaultKeyLength() const =0;

	
	
	
	virtual size_t GetValidKeyLength(size_t n) const =0;

	
	
	
	
	virtual bool IsValidKeyLength(size_t keylength) const
		{return keylength == GetValidKeyLength(keylength);}

	
	
	
	
	
	virtual void SetKey(const byte *key, size_t length, const NameValuePairs &params = g_nullNameValuePairs);

	
	
	
	
	
	
	
	
	void SetKeyWithRounds(const byte *key, size_t length, int rounds);

	
	
	
	
	
	
	
	
	void SetKeyWithIV(const byte *key, size_t length, const byte *iv, size_t ivLength);

	
	
	
	
	
	
	
	void SetKeyWithIV(const byte *key, size_t length, const byte *iv)
		{SetKeyWithIV(key, length, iv, IVSize());}

	
	
	
	
	
	enum IV_Requirement {
		
		UNIQUE_IV = 0,
		
		RANDOM_IV,
		
		UNPREDICTABLE_RANDOM_IV,
		
		INTERNALLY_GENERATED_IV,
		
		NOT_RESYNCHRONIZABLE
	};

	
	
	virtual IV_Requirement IVRequirement() const =0;

	
	
	
	
	bool IsResynchronizable() const {return IVRequirement() < NOT_RESYNCHRONIZABLE;}

	
	
	bool CanUseRandomIVs() const {return IVRequirement() <= UNPREDICTABLE_RANDOM_IV;}

	
	
	
	bool CanUsePredictableIVs() const {return IVRequirement() <= RANDOM_IV;}

	
	
	
	
	bool CanUseStructuredIVs() const {return IVRequirement() <= UNIQUE_IV;}

	
	
	
	
	virtual unsigned int IVSize() const
		{throw NotImplemented(GetAlgorithm().AlgorithmName() + ": this object doesn't support resynchronization");}

	
	
	unsigned int DefaultIVLength() const {return IVSize();}

	
	
	
	virtual unsigned int MinIVLength() const {return IVSize();}

	
	
	
	virtual unsigned int MaxIVLength() const {return IVSize();}

	
	
	
	
	
	virtual void Resynchronize(const byte *iv, int ivLength=-1) {
		CRYPTOPP_UNUSED(iv); CRYPTOPP_UNUSED(ivLength);
		throw NotImplemented(GetAlgorithm().AlgorithmName() + ": this object doesn't support resynchronization");
	}

	
	
	
	
	
	
	
	
	
	virtual void GetNextIV(RandomNumberGenerator &rng, byte *iv);

protected:
	
	
	virtual const Algorithm & GetAlgorithm() const =0;

	
	
	
	
	
	virtual void UncheckedSetKey(const byte *key, unsigned int length, const NameValuePairs &params) =0;

	
	
	
	void ThrowIfInvalidKeyLength(size_t length);

	
	
	
	
	
	void ThrowIfResynchronizable();

	
	
	
	
	
	
	
	void ThrowIfInvalidIV(const byte *iv);

	
	
	
	size_t ThrowIfInvalidIVLength(int length);

	
	
	
	
	
	const byte * GetIVAndThrowIfInvalid(const NameValuePairs &params, size_t &size);

	
	
	inline void AssertValidKeyLength(size_t length) const
		{CRYPTOPP_UNUSED(length); CRYPTOPP_ASSERT(IsValidKeyLength(length));}
};






class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE BlockTransformation : public Algorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~BlockTransformation() {}
#endif

	
	
	
	
	
	
	
	
	
	virtual void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const =0;

	
	
	
	
	
	
	
	
	void ProcessBlock(const byte *inBlock, byte *outBlock) const
		{ProcessAndXorBlock(inBlock, NULL, outBlock);}

	
	
	
	
	
	
	void ProcessBlock(byte *inoutBlock) const
		{ProcessAndXorBlock(inoutBlock, NULL, inoutBlock);}

	
	
	virtual unsigned int BlockSize() const =0;

	
	
	virtual unsigned int OptimalDataAlignment() const;

	
	virtual bool IsPermutation() const {return true;}

	
	
	
	virtual bool IsForwardTransformation() const =0;

	
	
	
	virtual unsigned int OptimalNumberOfParallelBlocks() const {return 1;}

	
	enum FlagsForAdvancedProcessBlocks {
		
		BT_InBlockIsCounter=1,
		
		BT_DontIncrementInOutPointers=2,
		
		BT_XorInput=4,
		
		BT_ReverseDirection=8,
		
		BT_AllowParallel=16};

	
	
	
	
	
	
	
	
	virtual size_t AdvancedProcessBlocks(const byte *inBlocks, const byte *xorBlocks, byte *outBlocks, size_t length, word32 flags) const;

	
	
	
	inline CipherDir GetCipherDirection() const {return IsForwardTransformation() ? ENCRYPTION : DECRYPTION;}
};




class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE StreamTransformation : public Algorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~StreamTransformation() {}
#endif

	
	
	
	StreamTransformation& Ref() {return *this;}

	
	
	virtual unsigned int MandatoryBlockSize() const {return 1;}

	
	
	
	
	
	virtual unsigned int OptimalBlockSize() const {return MandatoryBlockSize();}

	
	
	virtual unsigned int GetOptimalBlockSizeUsed() const {return 0;}

	
	
	virtual unsigned int OptimalDataAlignment() const;

	
	
	
	
	
	virtual void ProcessData(byte *outString, const byte *inString, size_t length) =0;

	
	
	
	
	
	
	virtual void ProcessLastBlock(byte *outString, const byte *inString, size_t length);

	
	
	
	
	virtual unsigned int MinLastBlockSize() const {return 0;}

	
	
	
	
	inline void ProcessString(byte *inoutString, size_t length)
		{ProcessData(inoutString, inoutString, length);}

	
	
	
	
	
	inline void ProcessString(byte *outString, const byte *inString, size_t length)
		{ProcessData(outString, inString, length);}

	
	
	
	inline byte ProcessByte(byte input)
		{ProcessData(&input, &input, 1); return input;}

	
	
	virtual bool IsRandomAccess() const =0;

	
	
	
	
	
	virtual void Seek(lword pos)
	{
		CRYPTOPP_UNUSED(pos);
		CRYPTOPP_ASSERT(!IsRandomAccess());
		throw NotImplemented("StreamTransformation: this object doesn't support random access");
	}

	
	
	
	
	virtual bool IsSelfInverting() const =0;

	
	
	
	virtual bool IsForwardTransformation() const =0;
};









class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE HashTransformation : public Algorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~HashTransformation() {}
#endif

	
	
	
	HashTransformation& Ref() {return *this;}

	
	
	
	virtual void Update(const byte *input, size_t length) =0;

	
	
	
	
	
	
	
	
	virtual byte * CreateUpdateSpace(size_t &size) {size=0; return NULL;}

	
	
	
	
	
	virtual void Final(byte *digest)
		{TruncatedFinal(digest, DigestSize());}

	
	
	virtual void Restart()
		{TruncatedFinal(NULL, 0);}

	
	
	virtual unsigned int DigestSize() const =0;

	
	
	
	unsigned int TagSize() const {return DigestSize();}

	
	
	
	
	virtual unsigned int BlockSize() const {return 0;}

	
	
	
	
	
	virtual unsigned int OptimalBlockSize() const {return 1;}

	
	
	virtual unsigned int OptimalDataAlignment() const;

	
	
	
	
	
	
	
	
	
	virtual void CalculateDigest(byte *digest, const byte *input, size_t length)
		{Update(input, length); Final(digest);}

	
	
	
	
	
	
	
	
	
	virtual bool Verify(const byte *digest)
		{return TruncatedVerify(digest, DigestSize());}

	
	
	
	
	
	
	
	
	
	
	
	
	
	virtual bool VerifyDigest(const byte *digest, const byte *input, size_t length)
		{Update(input, length); return Verify(digest);}

	
	
	
	
	
	virtual void TruncatedFinal(byte *digest, size_t digestSize) =0;

	
	
	
	
	
	
	
	
	
	
	virtual void CalculateTruncatedDigest(byte *digest, size_t digestSize, const byte *input, size_t length)
		{Update(input, length); TruncatedFinal(digest, digestSize);}

	
	
	
	
	
	
	
	
	
	
	virtual bool TruncatedVerify(const byte *digest, size_t digestLength);

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	virtual bool VerifyTruncatedDigest(const byte *digest, size_t digestLength, const byte *input, size_t length)
		{Update(input, length); return TruncatedVerify(digest, digestLength);}

protected:
	
	
	
	
	void ThrowIfInvalidTruncatedSize(size_t size) const;
};

typedef HashTransformation HashFunction;



class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE BlockCipher : public SimpleKeyingInterface, public BlockTransformation
{
protected:
	const Algorithm & GetAlgorithm() const {return *this;}
};



class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE SymmetricCipher : public SimpleKeyingInterface, public StreamTransformation
{
protected:
	const Algorithm & GetAlgorithm() const {return *this;}
};



class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE MessageAuthenticationCode : public SimpleKeyingInterface, public HashTransformation
{
protected:
	const Algorithm & GetAlgorithm() const {return *this;}
};





class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE AuthenticatedSymmetricCipher : public MessageAuthenticationCode, public StreamTransformation
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~AuthenticatedSymmetricCipher() {}
#endif

	
	
	
	class BadState : public Exception
	{
	public:
		explicit BadState(const std::string &name, const char *message) : Exception(OTHER_ERROR, name + ": " + message) {}
		explicit BadState(const std::string &name, const char *function, const char *state) : Exception(OTHER_ERROR, name + ": " + function + " was called before " + state) {}
	};

	
	
	virtual lword MaxHeaderLength() const =0;
	
	
	virtual lword MaxMessageLength() const =0;
	
	
	virtual lword MaxFooterLength() const {return 0;}
	
	
	
	
	
	virtual bool NeedsPrespecifiedDataLengths() const {return false;}
	
	
	
	void SpecifyDataLengths(lword headerLength, lword messageLength, lword footerLength=0);
	
	
	
	
	virtual void EncryptAndAuthenticate(byte *ciphertext, byte *mac, size_t macSize, const byte *iv, int ivLength, const byte *header, size_t headerLength, const byte *message, size_t messageLength);
	
	
	
	
	virtual bool DecryptAndVerify(byte *message, const byte *mac, size_t macLength, const byte *iv, int ivLength, const byte *header, size_t headerLength, const byte *ciphertext, size_t ciphertextLength);

	
	
	
	
	
	virtual std::string AlgorithmName() const =0;

protected:
	const Algorithm & GetAlgorithm() const
		{return *static_cast<const MessageAuthenticationCode *>(this);}
	virtual void UncheckedSpecifyDataLengths(lword headerLength, lword messageLength, lword footerLength)
		{CRYPTOPP_UNUSED(headerLength); CRYPTOPP_UNUSED(messageLength); CRYPTOPP_UNUSED(footerLength);}
};

#ifdef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY
typedef SymmetricCipher StreamCipher;
#endif





class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE RandomNumberGenerator : public Algorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~RandomNumberGenerator() {}
#endif

	
	
	
	
	
	
	
	
	virtual void IncorporateEntropy(const byte *input, size_t length)
	{
		CRYPTOPP_UNUSED(input); CRYPTOPP_UNUSED(length);
		throw NotImplemented("RandomNumberGenerator: IncorporateEntropy not implemented");
	}

	
	
	virtual bool CanIncorporateEntropy() const {return false;}

	
	
	
	
	
	virtual byte GenerateByte();

	
	
	
	
	
	virtual unsigned int GenerateBit();

	
	
	
	
	
	
	
	
	virtual word32 GenerateWord32(word32 min=0, word32 max=0xffffffffUL);

	
	
	
	
	
	
	
	virtual void GenerateBlock(byte *output, size_t size);

	
	
	
	
	
	
	
	
	
	
	virtual void GenerateIntoBufferedTransformation(BufferedTransformation &target, const std::string &channel, lword length);

	
	
	virtual void DiscardBytes(size_t n);

	
	
	
	
	template <class IT> void Shuffle(IT begin, IT end)
	{
		
		for (; begin != end; ++begin)
			std::iter_swap(begin, begin + GenerateWord32(0, end-begin-1));
	}

#ifdef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY
	byte GetByte() {return GenerateByte();}
	unsigned int GetBit() {return GenerateBit();}
	word32 GetLong(word32 a=0, word32 b=0xffffffffL) {return GenerateWord32(a, b);}
	word16 GetShort(word16 a=0, word16 b=0xffff) {return (word16)GenerateWord32(a, b);}
	void GetBlock(byte *output, size_t size) {GenerateBlock(output, size);}
#endif

};







CRYPTOPP_DLL RandomNumberGenerator & CRYPTOPP_API NullRNG();


class WaitObjectContainer;

class CallStack;


class CRYPTOPP_NO_VTABLE Waitable
{
public:
	virtual ~Waitable() {}

	
	
	virtual unsigned int GetMaxWaitObjectCount() const =0;

	
	
	
	
	
	
	
	
	virtual void GetWaitObjects(WaitObjectContainer &container, CallStack const& callStack) =0;

	
	
	
	
	bool Wait(unsigned long milliseconds, CallStack const& callStack);
};



extern CRYPTOPP_DLL const std::string DEFAULT_CHANNEL;



extern CRYPTOPP_DLL const std::string AAD_CHANNEL;
























class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE BufferedTransformation : public Algorithm, public Waitable
{
public:
	
	static const std::string &NULL_CHANNEL;	

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~BufferedTransformation() {}
#endif

	
	BufferedTransformation() : Algorithm(false) {}

	
	
	
	BufferedTransformation& Ref() {return *this;}

	
	

		
		
		
		
		
		size_t Put(byte inByte, bool blocking=true)
			{return Put(&inByte, 1, blocking);}

		
		
		
		
		
		
		size_t Put(const byte *inString, size_t length, bool blocking=true)
			{return Put2(inString, length, 0, blocking);}

		
		
		
		
		
		size_t PutWord16(word16 value, ByteOrder order=BIG_ENDIAN_ORDER, bool blocking=true);

		
		
		
		
		
		size_t PutWord32(word32 value, ByteOrder order=BIG_ENDIAN_ORDER, bool blocking=true);

		
		
		
		
		
		
		
		
		
		virtual byte * CreatePutSpace(size_t &size)
			{size=0; return NULL;}

		
		
		
		virtual bool CanModifyInput() const
			{return false;}

		
		
		
		
		
		
		size_t PutModifiable(byte *inString, size_t length, bool blocking=true)
			{return PutModifiable2(inString, length, 0, blocking);}

		
		
		
		
		
		bool MessageEnd(int propagation=-1, bool blocking=true)
			{return !!Put2(NULL, 0, propagation < 0 ? -1 : propagation+1, blocking);}

		
		
		
		
		
		
		
		
		
		
		size_t PutMessageEnd(const byte *inString, size_t length, int propagation=-1, bool blocking=true)
			{return Put2(inString, length, propagation < 0 ? -1 : propagation+1, blocking);}

		
		
		
		
		
		
		virtual size_t Put2(const byte *inString, size_t length, int messageEnd, bool blocking) =0;

		
		
		
		
		
		
		virtual size_t PutModifiable2(byte *inString, size_t length, int messageEnd, bool blocking)
			{return Put2(inString, length, messageEnd, blocking);}

		
		
		
		struct BlockingInputOnly : public NotImplemented
			{BlockingInputOnly(const std::string &s) : NotImplemented(s + ": Nonblocking input is not implemented by this object.") {}};
	

	
	
		
		unsigned int GetMaxWaitObjectCount() const;

		
		
		
		
		
		
		
		
		void GetWaitObjects(WaitObjectContainer &container, CallStack const& callStack);
	

	
	

		
		
		
		
		
		
		
		
		
		
		virtual void IsolatedInitialize(const NameValuePairs &parameters) {
			CRYPTOPP_UNUSED(parameters);
			throw NotImplemented("BufferedTransformation: this object can't be reinitialized");
		}

		
		
		
		
		virtual bool IsolatedFlush(bool hardFlush, bool blocking) =0;

		
		
		
		virtual bool IsolatedMessageSeriesEnd(bool blocking)
			{CRYPTOPP_UNUSED(blocking); return false;}

		
		
		
		
		
		
		
		
		virtual void Initialize(const NameValuePairs &parameters=g_nullNameValuePairs, int propagation=-1);

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		virtual bool Flush(bool hardFlush, int propagation=-1, bool blocking=true);

		
		
		
		
		
		
		
		
		virtual bool MessageSeriesEnd(int propagation=-1, bool blocking=true);

		
		
		
		
		virtual void SetAutoSignalPropagation(int propagation)
			{CRYPTOPP_UNUSED(propagation);}

		
		
		
		virtual int GetAutoSignalPropagation() const {return 0;}
public:

#ifdef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY
		void Close() {MessageEnd();}
#endif
	

	
	

		
		
		
		
		virtual lword MaxRetrievable() const;

		
		
		virtual bool AnyRetrievable() const;

		
		
		
		
		virtual size_t Get(byte &outByte);

		
		
		
		
		
		virtual size_t Get(byte *outString, size_t getMax);

		
		
		
		
		
		virtual size_t Peek(byte &outByte) const;

		
		
		
		
		
		
		virtual size_t Peek(byte *outString, size_t peekMax) const;

		
		
		
		
		
		size_t GetWord16(word16 &value, ByteOrder order=BIG_ENDIAN_ORDER);

		
		
		
		
		
		size_t GetWord32(word32 &value, ByteOrder order=BIG_ENDIAN_ORDER);

		
		
		
		
		
		
		size_t PeekWord16(word16 &value, ByteOrder order=BIG_ENDIAN_ORDER) const;

		
		
		
		
		
		
		size_t PeekWord32(word32 &value, ByteOrder order=BIG_ENDIAN_ORDER) const;

		

		
		
		
		
		
		
		
		lword TransferTo(BufferedTransformation &target, lword transferMax=LWORD_MAX, const std::string &channel=DEFAULT_CHANNEL)
			{TransferTo2(target, transferMax, channel); return transferMax;}

		
		
		
		
		
		
		
		
		
		
		virtual lword Skip(lword skipMax=LWORD_MAX);

		

		
		
		
		
		
		
		
		lword CopyTo(BufferedTransformation &target, lword copyMax=LWORD_MAX, const std::string &channel=DEFAULT_CHANNEL) const
			{return CopyRangeTo(target, 0, copyMax, channel);}

		
		
		
		
		
		
		
		
		
		
		lword CopyRangeTo(BufferedTransformation &target, lword position, lword copyMax=LWORD_MAX, const std::string &channel=DEFAULT_CHANNEL) const
			{lword i = position; CopyRangeTo2(target, i, i+copyMax, channel); return i-position;}

#ifdef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY
		unsigned long MaxRetrieveable() const {return MaxRetrievable();}
#endif
	

	
	

		
		
		virtual lword TotalBytesRetrievable() const;

		
		
		
		
		virtual unsigned int NumberOfMessages() const;

		
		
		
		virtual bool AnyMessages() const;

		
		
		
		
		virtual bool GetNextMessage();

		
		
		
		
		
		virtual unsigned int SkipMessages(unsigned int count=UINT_MAX);

		
		
		
		
		
		
		
		
		
		unsigned int TransferMessagesTo(BufferedTransformation &target, unsigned int count=UINT_MAX, const std::string &channel=DEFAULT_CHANNEL)
			{TransferMessagesTo2(target, count, channel); return count;}

		
		
		
		
		
		
		
		
		
		unsigned int CopyMessagesTo(BufferedTransformation &target, unsigned int count=UINT_MAX, const std::string &channel=DEFAULT_CHANNEL) const;

		
		virtual void SkipAll();

		
		
		
		
		
		
		void TransferAllTo(BufferedTransformation &target, const std::string &channel=DEFAULT_CHANNEL)
			{TransferAllTo2(target, channel);}

		
		
		
		
		void CopyAllTo(BufferedTransformation &target, const std::string &channel=DEFAULT_CHANNEL) const;

		
		
		
		virtual bool GetNextMessageSeries() {return false;}
		
		
		virtual unsigned int NumberOfMessagesInThisSeries() const {return NumberOfMessages();}
		
		
		virtual unsigned int NumberOfMessageSeries() const {return 0;}
	

	
	

		
		

		
		
		
		
		
		
		
		
		
		
		
		
		virtual size_t TransferTo2(BufferedTransformation &target, lword &byteCount, const std::string &channel=DEFAULT_CHANNEL, bool blocking=true) =0;

		
		

		
		
		
		
		
		
		
		
		
		
		
		
		
		
		virtual size_t CopyRangeTo2(BufferedTransformation &target, lword &begin, lword end=LWORD_MAX, const std::string &channel=DEFAULT_CHANNEL, bool blocking=true) const =0;

		
		

		
		
		
		
		
		
		
		
		
		
		size_t TransferMessagesTo2(BufferedTransformation &target, unsigned int &messageCount, const std::string &channel=DEFAULT_CHANNEL, bool blocking=true);

		

		
		
		
		
		
		
		size_t TransferAllTo2(BufferedTransformation &target, const std::string &channel=DEFAULT_CHANNEL, bool blocking=true);
	

	
	
		
		struct NoChannelSupport : public NotImplemented
			{NoChannelSupport(const std::string &name) : NotImplemented(name + ": this object doesn't support multiple channels") {}};
		
		struct InvalidChannelName : public InvalidArgument
			{InvalidChannelName(const std::string &name, const std::string &channel) : InvalidArgument(name + ": unexpected channel name \"" + channel + "\"") {}};

		
		
		
		
		
		
		size_t ChannelPut(const std::string &channel, byte inByte, bool blocking=true)
			{return ChannelPut(channel, &inByte, 1, blocking);}

		
		
		
		
		
		
		
		size_t ChannelPut(const std::string &channel, const byte *inString, size_t length, bool blocking=true)
			{return ChannelPut2(channel, inString, length, 0, blocking);}

		
		
		
		
		
		
		
		size_t ChannelPutModifiable(const std::string &channel, byte *inString, size_t length, bool blocking=true)
			{return ChannelPutModifiable2(channel, inString, length, 0, blocking);}

		
		
		
		
		
		
		
		size_t ChannelPutWord16(const std::string &channel, word16 value, ByteOrder order=BIG_ENDIAN_ORDER, bool blocking=true);

		
		
		
		
		
		
		
		size_t ChannelPutWord32(const std::string &channel, word32 value, ByteOrder order=BIG_ENDIAN_ORDER, bool blocking=true);

		
		
		
		
		
		
		
		
		bool ChannelMessageEnd(const std::string &channel, int propagation=-1, bool blocking=true)
			{return !!ChannelPut2(channel, NULL, 0, propagation < 0 ? -1 : propagation+1, blocking);}

		
		
		
		
		
		
		
		
		
		size_t ChannelPutMessageEnd(const std::string &channel, const byte *inString, size_t length, int propagation=-1, bool blocking=true)
			{return ChannelPut2(channel, inString, length, propagation < 0 ? -1 : propagation+1, blocking);}

		
		
		
		
		
		
		
		
		
		
		
		virtual byte * ChannelCreatePutSpace(const std::string &channel, size_t &size);

		
		
		
		
		
		
		
		virtual size_t ChannelPut2(const std::string &channel, const byte *inString, size_t length, int messageEnd, bool blocking);

		
		
		
		
		
		
		
		virtual size_t ChannelPutModifiable2(const std::string &channel, byte *inString, size_t length, int messageEnd, bool blocking);

		
		
		
		
		
		
		
		
		virtual bool ChannelFlush(const std::string &channel, bool hardFlush, int propagation=-1, bool blocking=true);

		
		
		
		
		
		
		
		
		
		virtual bool ChannelMessageSeriesEnd(const std::string &channel, int propagation=-1, bool blocking=true);

		
		
		
		virtual void SetRetrievalChannel(const std::string &channel);
	

	
	
	
	

	
		
		
		
		virtual bool Attachable() {return false;}

		
		
		
		
		virtual BufferedTransformation *AttachedTransformation() {CRYPTOPP_ASSERT(!Attachable()); return 0;}

		
		
		
		
		virtual const BufferedTransformation *AttachedTransformation() const
			{return const_cast<BufferedTransformation *>(this)->AttachedTransformation();}

		
		
		
		
		
		
		virtual void Detach(BufferedTransformation *newAttachment = 0) {
			CRYPTOPP_UNUSED(newAttachment); CRYPTOPP_ASSERT(!Attachable());
			throw NotImplemented("BufferedTransformation: this object is not attachable");
		}

		
		
		virtual void Attach(BufferedTransformation *newAttachment);
	

protected:
	
	
	static int DecrementPropagation(int propagation)
		{return propagation != 0 ? propagation - 1 : 0;}

private:
	byte m_buf[4];	
};



CRYPTOPP_DLL BufferedTransformation & TheBitBucket();



class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE CryptoMaterial : public NameValuePairs
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~CryptoMaterial() {}
#endif

	
	class CRYPTOPP_DLL InvalidMaterial : public InvalidDataFormat
	{
	public:
		explicit InvalidMaterial(const std::string &s) : InvalidDataFormat(s) {}
	};

	
	
	virtual void AssignFrom(const NameValuePairs &source) =0;

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	virtual bool Validate(RandomNumberGenerator &rng, unsigned int level) const =0;

	
	
	
	
	
	
	virtual void ThrowIfInvalid(RandomNumberGenerator &rng, unsigned int level) const
		{if (!Validate(rng, level)) throw InvalidMaterial("CryptoMaterial: this object contains invalid values");}

	
	
	
	
	
	
	
	
	
	virtual void Save(BufferedTransformation &bt) const
		{CRYPTOPP_UNUSED(bt); throw NotImplemented("CryptoMaterial: this object does not support saving");}

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	virtual void Load(BufferedTransformation &bt)
		{CRYPTOPP_UNUSED(bt); throw NotImplemented("CryptoMaterial: this object does not support loading");}

	
	
	
	virtual bool SupportsPrecomputation() const {return false;}

	
	
	
	
	
	
	
	
	virtual void Precompute(unsigned int precomputationStorage) {
		CRYPTOPP_UNUSED(precomputationStorage); CRYPTOPP_ASSERT(!SupportsPrecomputation());
		throw NotImplemented("CryptoMaterial: this object does not support precomputation");
	}

	
	
	
	
	virtual void LoadPrecomputation(BufferedTransformation &storedPrecomputation)
		{CRYPTOPP_UNUSED(storedPrecomputation); CRYPTOPP_ASSERT(!SupportsPrecomputation()); throw NotImplemented("CryptoMaterial: this object does not support precomputation");}
	
	
	
	
	virtual void SavePrecomputation(BufferedTransformation &storedPrecomputation) const
		{CRYPTOPP_UNUSED(storedPrecomputation); CRYPTOPP_ASSERT(!SupportsPrecomputation()); throw NotImplemented("CryptoMaterial: this object does not support precomputation");}

	
	
	void DoQuickSanityCheck() const	{ThrowIfInvalid(NullRNG(), 0);}

#if (defined(__SUNPRO_CC) && __SUNPRO_CC < 0x590)
	
	char m_sunCCworkaround;
#endif
};



class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE GeneratableCryptoMaterial : virtual public CryptoMaterial
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~GeneratableCryptoMaterial() {}
#endif

	
	
	
	
	
	
	virtual void GenerateRandom(RandomNumberGenerator &rng, const NameValuePairs &params = g_nullNameValuePairs) {
		CRYPTOPP_UNUSED(rng); CRYPTOPP_UNUSED(params);
		throw NotImplemented("GeneratableCryptoMaterial: this object does not support key/parameter generation");
	}

	
	
	
	
	
	
	void GenerateRandomWithKeySize(RandomNumberGenerator &rng, unsigned int keySize);
};


class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE PublicKey : virtual public CryptoMaterial
{
};


class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE PrivateKey : public GeneratableCryptoMaterial
{
};


class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE CryptoParameters : public GeneratableCryptoMaterial
{
};


class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE AsymmetricAlgorithm : public Algorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~AsymmetricAlgorithm() {}
#endif

	
	
	virtual CryptoMaterial & AccessMaterial() =0;

	
	
	virtual const CryptoMaterial & GetMaterial() const =0;

	
	
	
	void BERDecode(BufferedTransformation &bt)
		{AccessMaterial().Load(bt);}

	
	
	
	void DEREncode(BufferedTransformation &bt) const
		{GetMaterial().Save(bt);}
};


class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE PublicKeyAlgorithm : public AsymmetricAlgorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~PublicKeyAlgorithm() {}
#endif

	

	
	
	CryptoMaterial & AccessMaterial()
		{return AccessPublicKey();}
	
	
	const CryptoMaterial & GetMaterial() const
		{return GetPublicKey();}

	
	
	virtual PublicKey & AccessPublicKey() =0;
	
	
	virtual const PublicKey & GetPublicKey() const
		{return const_cast<PublicKeyAlgorithm *>(this)->AccessPublicKey();}
};


class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE PrivateKeyAlgorithm : public AsymmetricAlgorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~PrivateKeyAlgorithm() {}
#endif

	
	
	CryptoMaterial & AccessMaterial() {return AccessPrivateKey();}
	
	
	const CryptoMaterial & GetMaterial() const {return GetPrivateKey();}

	
	
	virtual PrivateKey & AccessPrivateKey() =0;
	
	
	virtual const PrivateKey & GetPrivateKey() const {return const_cast<PrivateKeyAlgorithm *>(this)->AccessPrivateKey();}
};


class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE KeyAgreementAlgorithm : public AsymmetricAlgorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~KeyAgreementAlgorithm() {}
#endif

	
	
	CryptoMaterial & AccessMaterial() {return AccessCryptoParameters();}
	
	
	const CryptoMaterial & GetMaterial() const {return GetCryptoParameters();}

	
	
	virtual CryptoParameters & AccessCryptoParameters() =0;
	
	
	virtual const CryptoParameters & GetCryptoParameters() const {return const_cast<KeyAgreementAlgorithm *>(this)->AccessCryptoParameters();}
};




class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE PK_CryptoSystem
{
public:
	virtual ~PK_CryptoSystem() {}

	
	
	
	virtual size_t MaxPlaintextLength(size_t ciphertextLength) const =0;

	
	
	
	virtual size_t CiphertextLength(size_t plaintextLength) const =0;

	
	
	
	
	
	virtual bool ParameterSupported(const char *name) const =0;

	
	
	
	
	virtual size_t FixedCiphertextLength() const {return 0;}

	
	
	
	
	
	virtual size_t FixedMaxPlaintextLength() const {return 0;}

#ifdef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY
	size_t MaxPlainTextLength(size_t cipherTextLength) const {return MaxPlaintextLength(cipherTextLength);}
	size_t CipherTextLength(size_t plainTextLength) const {return CiphertextLength(plainTextLength);}
#endif
};



class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE PK_Encryptor : public PK_CryptoSystem, public PublicKeyAlgorithm
{
public:
	
	class CRYPTOPP_DLL InvalidPlaintextLength : public Exception
	{
	public:
		InvalidPlaintextLength() : Exception(OTHER_ERROR, "PK_Encryptor: invalid plaintext length") {}
	};

	
	
	
	
	
	
	
	
	
	
	virtual void Encrypt(RandomNumberGenerator &rng,
		const byte *plaintext, size_t plaintextLength,
		byte *ciphertext, const NameValuePairs &parameters = g_nullNameValuePairs) const =0;

	
	
	
	
	
	
	virtual BufferedTransformation * CreateEncryptionFilter(RandomNumberGenerator &rng,
		BufferedTransformation *attachment=NULL, const NameValuePairs &parameters = g_nullNameValuePairs) const;
};



class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE PK_Decryptor : public PK_CryptoSystem, public PrivateKeyAlgorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~PK_Decryptor() {}
#endif

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	virtual DecodingResult Decrypt(RandomNumberGenerator &rng,
		const byte *ciphertext, size_t ciphertextLength,
		byte *plaintext, const NameValuePairs &parameters = g_nullNameValuePairs) const =0;

	
	
	
	
	
	
	virtual BufferedTransformation * CreateDecryptionFilter(RandomNumberGenerator &rng,
		BufferedTransformation *attachment=NULL, const NameValuePairs &parameters = g_nullNameValuePairs) const;

	
	
	
	
	
	
	
	
	
	
	
	
	
	DecodingResult FixedLengthDecrypt(RandomNumberGenerator &rng, const byte *ciphertext, byte *plaintext, const NameValuePairs &parameters = g_nullNameValuePairs) const
		{return Decrypt(rng, ciphertext, FixedCiphertextLength(), plaintext, parameters);}
};

#ifdef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY
typedef PK_CryptoSystem PK_FixedLengthCryptoSystem;
typedef PK_Encryptor PK_FixedLengthEncryptor;
typedef PK_Decryptor PK_FixedLengthDecryptor;
#endif




class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE PK_SignatureScheme
{
public:
	
	
	
	
	class CRYPTOPP_DLL InvalidKeyLength : public Exception
	{
	public:
		InvalidKeyLength(const std::string &message) : Exception(OTHER_ERROR, message) {}
	};

	
	
	
	
	class CRYPTOPP_DLL KeyTooShort : public InvalidKeyLength
	{
	public:
		KeyTooShort() : InvalidKeyLength("PK_Signer: key too short for this signature scheme") {}
	};

	virtual ~PK_SignatureScheme() {}

	
	
	
	virtual size_t SignatureLength() const =0;

	
	
	
	
	
	virtual size_t MaxSignatureLength(size_t recoverablePartLength = 0) const
	{CRYPTOPP_UNUSED(recoverablePartLength); return SignatureLength();}

	
	
	
	
	virtual size_t MaxRecoverableLength() const =0;

	
	
	
	
	
	
	virtual size_t MaxRecoverableLengthFromSignatureLength(size_t signatureLength) const =0;

	
	
	
	
	virtual bool IsProbabilistic() const =0;

	
	
	virtual bool AllowNonrecoverablePart() const =0;

	
	
	
	
	virtual bool SignatureUpfront() const {return false;}

	
	
	
	
	virtual bool RecoverablePartFirst() const =0;
};





class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE PK_MessageAccumulator : public HashTransformation
{
public:
	
	unsigned int DigestSize() const
		{throw NotImplemented("PK_MessageAccumulator: DigestSize() should not be called");}

	
	void TruncatedFinal(byte *digest, size_t digestSize)
	{
		CRYPTOPP_UNUSED(digest); CRYPTOPP_UNUSED(digestSize);
		throw NotImplemented("PK_MessageAccumulator: TruncatedFinal() should not be called");
	}
};



class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE PK_Signer : public PK_SignatureScheme, public PrivateKeyAlgorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~PK_Signer() {}
#endif

	
	
	
	
	
	virtual PK_MessageAccumulator * NewSignatureAccumulator(RandomNumberGenerator &rng) const =0;

	
	
	
	
	virtual void InputRecoverableMessage(PK_MessageAccumulator &messageAccumulator, const byte *recoverableMessage, size_t recoverableMessageLength) const =0;

	
	
	
	
	
	
	
	virtual size_t Sign(RandomNumberGenerator &rng, PK_MessageAccumulator *messageAccumulator, byte *signature) const;

	
	
	
	
	
	
	
	virtual size_t SignAndRestart(RandomNumberGenerator &rng, PK_MessageAccumulator &messageAccumulator, byte *signature, bool restart=true) const =0;

	
	
	
	
	
	
	
	virtual size_t SignMessage(RandomNumberGenerator &rng, const byte *message, size_t messageLen, byte *signature) const;

	
	
	
	
	
	
	
	
	
	virtual size_t SignMessageWithRecovery(RandomNumberGenerator &rng, const byte *recoverableMessage, size_t recoverableMessageLength,
		const byte *nonrecoverableMessage, size_t nonrecoverableMessageLength, byte *signature) const;
};








class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE PK_Verifier : public PK_SignatureScheme, public PublicKeyAlgorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~PK_Verifier() {}
#endif

	
	
	
	
	virtual PK_MessageAccumulator * NewVerificationAccumulator() const =0;

	
	
	
	
	virtual void InputSignature(PK_MessageAccumulator &messageAccumulator, const byte *signature, size_t signatureLength) const =0;

	
	
	
	
	virtual bool Verify(PK_MessageAccumulator *messageAccumulator) const;

	
	
	
	
	virtual bool VerifyAndRestart(PK_MessageAccumulator &messageAccumulator) const =0;

	
	
	
	
	
	
	virtual bool VerifyMessage(const byte *message, size_t messageLen,
		const byte *signature, size_t signatureLen) const;

	
	
	
	
	
	
	virtual DecodingResult Recover(byte *recoveredMessage, PK_MessageAccumulator *messageAccumulator) const;

	
	
	
	
	
	
	virtual DecodingResult RecoverAndRestart(byte *recoveredMessage, PK_MessageAccumulator &messageAccumulator) const =0;

	
	
	
	
	
	
	
	
	virtual DecodingResult RecoverMessage(byte *recoveredMessage,
		const byte *nonrecoverableMessage, size_t nonrecoverableMessageLength,
		const byte *signature, size_t signatureLength) const;
};






class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE SimpleKeyAgreementDomain : public KeyAgreementAlgorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~SimpleKeyAgreementDomain() {}
#endif

	
	
	virtual unsigned int AgreedValueLength() const =0;

	
	
	virtual unsigned int PrivateKeyLength() const =0;

	
	
	virtual unsigned int PublicKeyLength() const =0;

	
	
	
	
	virtual void GeneratePrivateKey(RandomNumberGenerator &rng, byte *privateKey) const =0;

	
	
	
	
	
	virtual void GeneratePublicKey(RandomNumberGenerator &rng, const byte *privateKey, byte *publicKey) const =0;

	
	
	
	
	
	
	
	virtual void GenerateKeyPair(RandomNumberGenerator &rng, byte *privateKey, byte *publicKey) const;

	
	
	
	
	
	
	
	
	
	
	
	
	virtual bool Agree(byte *agreedValue, const byte *privateKey, const byte *otherPublicKey, bool validateOtherPublicKey=true) const =0;

#ifdef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY
	bool ValidateDomainParameters(RandomNumberGenerator &rng) const
		{return GetCryptoParameters().Validate(rng, 2);}
#endif
};





class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE AuthenticatedKeyAgreementDomain : public KeyAgreementAlgorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~AuthenticatedKeyAgreementDomain() {}
#endif

	
	
	virtual unsigned int AgreedValueLength() const =0;

	
	
	virtual unsigned int StaticPrivateKeyLength() const =0;

	
	
	virtual unsigned int StaticPublicKeyLength() const =0;

	
	
	
	
	virtual void GenerateStaticPrivateKey(RandomNumberGenerator &rng, byte *privateKey) const =0;

	
	
	
	
	
	virtual void GenerateStaticPublicKey(RandomNumberGenerator &rng, const byte *privateKey, byte *publicKey) const =0;

	
	
	
	
	
	
	
	virtual void GenerateStaticKeyPair(RandomNumberGenerator &rng, byte *privateKey, byte *publicKey) const;

	
	
	virtual unsigned int EphemeralPrivateKeyLength() const =0;

	
	
	virtual unsigned int EphemeralPublicKeyLength() const =0;

	
	
	
	
	virtual void GenerateEphemeralPrivateKey(RandomNumberGenerator &rng, byte *privateKey) const =0;

	
	
	
	
	
	virtual void GenerateEphemeralPublicKey(RandomNumberGenerator &rng, const byte *privateKey, byte *publicKey) const =0;

	
	
	
	
	
	virtual void GenerateEphemeralKeyPair(RandomNumberGenerator &rng, byte *privateKey, byte *publicKey) const;

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	virtual bool Agree(byte *agreedValue,
		const byte *staticPrivateKey, const byte *ephemeralPrivateKey,
		const byte *staticOtherPublicKey, const byte *ephemeralOtherPublicKey,
		bool validateStaticOtherPublicKey=true) const =0;

#ifdef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY
	bool ValidateDomainParameters(RandomNumberGenerator &rng) const
		{return GetCryptoParameters().Validate(rng, 2);}
#endif
};


#if 0

/*! The methods should be called in the following order:

	InitializeSession(rng, parameters);	
	while (true)
	{
		if (OutgoingMessageAvailable())
		{
			length = GetOutgoingMessageLength();
			GetOutgoingMessage(message);
			; 
		}

		if (LastMessageProcessed())
			break;

		; 
		ProcessIncomingMessage(message);
	}
	; 
*/
class ProtocolSession
{
public:
	
	class ProtocolError : public Exception
	{
	public:
		ProtocolError(ErrorType errorType, const std::string &s) : Exception(errorType, s) {}
	};

	
	
	class UnexpectedMethodCall : public Exception
	{
	public:
		UnexpectedMethodCall(const std::string &s) : Exception(OTHER_ERROR, s) {}
	};

	ProtocolSession() : m_rng(NULL), m_throwOnProtocolError(true), m_validState(false) {}
	virtual ~ProtocolSession() {}

	virtual void InitializeSession(RandomNumberGenerator &rng, const NameValuePairs &parameters) =0;

	bool GetThrowOnProtocolError() const {return m_throwOnProtocolError;}
	void SetThrowOnProtocolError(bool throwOnProtocolError) {m_throwOnProtocolError = throwOnProtocolError;}

	bool HasValidState() const {return m_validState;}

	virtual bool OutgoingMessageAvailable() const =0;
	virtual unsigned int GetOutgoingMessageLength() const =0;
	virtual void GetOutgoingMessage(byte *message) =0;

	virtual bool LastMessageProcessed() const =0;
	virtual void ProcessIncomingMessage(const byte *message, unsigned int messageLength) =0;

protected:
	void HandleProtocolError(Exception::ErrorType errorType, const std::string &s) const;
	void CheckAndHandleInvalidState() const;
	void SetValidState(bool valid) {m_validState = valid;}

	RandomNumberGenerator *m_rng;

private:
	bool m_throwOnProtocolError, m_validState;
};

class KeyAgreementSession : public ProtocolSession
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~KeyAgreementSession() {}
#endif

	virtual unsigned int GetAgreedValueLength() const =0;
	virtual void GetAgreedValue(byte *agreedValue) const =0;
};

class PasswordAuthenticatedKeyAgreementSession : public KeyAgreementSession
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~PasswordAuthenticatedKeyAgreementSession() {}
#endif

	void InitializePasswordAuthenticatedKeyAgreementSession(RandomNumberGenerator &rng,
		const byte *myId, unsigned int myIdLength,
		const byte *counterPartyId, unsigned int counterPartyIdLength,
		const byte *passwordOrVerifier, unsigned int passwordOrVerifierLength);
};

class PasswordAuthenticatedKeyAgreementDomain : public KeyAgreementAlgorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~PasswordAuthenticatedKeyAgreementDomain() {}
#endif

	
	virtual bool ValidateDomainParameters(RandomNumberGenerator &rng) const
		{return GetCryptoParameters().Validate(rng, 2);}

	virtual unsigned int GetPasswordVerifierLength(const byte *password, unsigned int passwordLength) const =0;
	virtual void GeneratePasswordVerifier(RandomNumberGenerator &rng, const byte *userId, unsigned int userIdLength, const byte *password, unsigned int passwordLength, byte *verifier) const =0;

	enum RoleFlags {CLIENT=1, SERVER=2, INITIATOR=4, RESPONDER=8};

	virtual bool IsValidRole(unsigned int role) =0;
	virtual PasswordAuthenticatedKeyAgreementSession * CreateProtocolSession(unsigned int role) const =0;
};
#endif


class CRYPTOPP_DLL BERDecodeErr : public InvalidArgument
{
public:
	BERDecodeErr() : InvalidArgument("BER decode error") {}
	BERDecodeErr(const std::string &s) : InvalidArgument(s) {}
};





class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE ASN1Object
{
public:
	virtual ~ASN1Object() {}

	
	
	
	virtual void BERDecode(BufferedTransformation &bt) =0;

	
	
	
	virtual void DEREncode(BufferedTransformation &bt) const =0;

	
	
	
	
	virtual void BEREncode(BufferedTransformation &bt) const {DEREncode(bt);}
};

#ifdef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY
typedef PK_SignatureScheme PK_SignatureSystem;
typedef SimpleKeyAgreementDomain PK_SimpleKeyAgreementDomain;
typedef AuthenticatedKeyAgreementDomain PK_AuthenticatedKeyAgreementDomain;
#endif

NAMESPACE_END

#if CRYPTOPP_MSC_VERSION
# pragma warning(pop)
#endif

#endif
