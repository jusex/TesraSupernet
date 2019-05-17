
































#ifndef CRYPTOPP_PUBKEY_H
#define CRYPTOPP_PUBKEY_H

#include "config.h"

#if CRYPTOPP_MSC_VERSION
# pragma warning(push)
# pragma warning(disable: 4702)
#endif

#include "cryptlib.h"
#include "integer.h"
#include "algebra.h"
#include "modarith.h"
#include "filters.h"
#include "eprecomp.h"
#include "fips140.h"
#include "argnames.h"
#include "smartptr.h"
#include "stdcpp.h"


#undef INTERFACE

#if defined(__SUNPRO_CC)
# define MAYBE_RETURN(x) return x
#else
# define MAYBE_RETURN(x) CRYPTOPP_UNUSED(x)
#endif

NAMESPACE_BEGIN(CryptoPP)











class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE TrapdoorFunctionBounds
{
public:
	virtual ~TrapdoorFunctionBounds() {}

	
	
	
	virtual Integer PreimageBound() const =0;
	
	
	
	virtual Integer ImageBound() const =0;
	
	
	
	virtual Integer MaxPreimage() const {return --PreimageBound();}
	
	
	
	virtual Integer MaxImage() const {return --ImageBound();}
};







class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE RandomizedTrapdoorFunction : public TrapdoorFunctionBounds
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~RandomizedTrapdoorFunction() { }
#endif

	
	
	
	
	
	
	
	virtual Integer ApplyRandomizedFunction(RandomNumberGenerator &rng, const Integer &x) const =0;

	
	
	
	virtual bool IsRandomized() const {return true;}
};







class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE TrapdoorFunction : public RandomizedTrapdoorFunction
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~TrapdoorFunction() { }
#endif

	
	
	
	
	
	
	
	Integer ApplyRandomizedFunction(RandomNumberGenerator &rng, const Integer &x) const
		{CRYPTOPP_UNUSED(rng); return ApplyFunction(x);}
	bool IsRandomized() const {return false;}

	
	
	
	
	
	virtual Integer ApplyFunction(const Integer &x) const =0;
};







class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE RandomizedTrapdoorFunctionInverse
{
public:
	virtual ~RandomizedTrapdoorFunctionInverse() {}

	
	
	
	
	
	
	virtual Integer CalculateRandomizedInverse(RandomNumberGenerator &rng, const Integer &x) const =0;

	
	
	
	virtual bool IsRandomized() const {return true;}
};







class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE TrapdoorFunctionInverse : public RandomizedTrapdoorFunctionInverse
{
public:
	virtual ~TrapdoorFunctionInverse() {}

	
	
	
	
	
	
	
	Integer CalculateRandomizedInverse(RandomNumberGenerator &rng, const Integer &x) const
		{return CalculateInverse(rng, x);}

	
	
	
	bool IsRandomized() const {return false;}

	
	
	
	
	virtual Integer CalculateInverse(RandomNumberGenerator &rng, const Integer &x) const =0;
};





class CRYPTOPP_NO_VTABLE PK_EncryptionMessageEncodingMethod
{
public:
	virtual ~PK_EncryptionMessageEncodingMethod() {}

	virtual bool ParameterSupported(const char *name) const
		{CRYPTOPP_UNUSED(name); return false;}

	
	virtual size_t MaxUnpaddedLength(size_t paddedLength) const =0;

	virtual void Pad(RandomNumberGenerator &rng, const byte *raw, size_t inputLength, byte *padded, size_t paddedBitLength, const NameValuePairs &parameters) const =0;

	virtual DecodingResult Unpad(const byte *padded, size_t paddedBitLength, byte *raw, const NameValuePairs &parameters) const =0;
};







template <class TFI, class MEI>
class CRYPTOPP_NO_VTABLE TF_Base
{
protected:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~TF_Base() { }
#endif

	virtual const TrapdoorFunctionBounds & GetTrapdoorFunctionBounds() const =0;

	typedef TFI TrapdoorFunctionInterface;
	virtual const TrapdoorFunctionInterface & GetTrapdoorFunctionInterface() const =0;

	typedef MEI MessageEncodingInterface;
	virtual const MessageEncodingInterface & GetMessageEncodingInterface() const =0;
};






template <class BASE>
class CRYPTOPP_NO_VTABLE PK_FixedLengthCryptoSystemImpl : public BASE
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~PK_FixedLengthCryptoSystemImpl() { }
#endif

	size_t MaxPlaintextLength(size_t ciphertextLength) const
		{return ciphertextLength == FixedCiphertextLength() ? FixedMaxPlaintextLength() : 0;}
	size_t CiphertextLength(size_t plaintextLength) const
		{return plaintextLength <= FixedMaxPlaintextLength() ? FixedCiphertextLength() : 0;}

	virtual size_t FixedMaxPlaintextLength() const =0;
	virtual size_t FixedCiphertextLength() const =0;
};





template <class INTERFACE, class BASE>
class CRYPTOPP_NO_VTABLE TF_CryptoSystemBase : public PK_FixedLengthCryptoSystemImpl<INTERFACE>, protected BASE
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~TF_CryptoSystemBase() { }
#endif

	bool ParameterSupported(const char *name) const {return this->GetMessageEncodingInterface().ParameterSupported(name);}
	size_t FixedMaxPlaintextLength() const {return this->GetMessageEncodingInterface().MaxUnpaddedLength(PaddedBlockBitLength());}
	size_t FixedCiphertextLength() const {return this->GetTrapdoorFunctionBounds().MaxImage().ByteCount();}

protected:
	size_t PaddedBlockByteLength() const {return BitsToBytes(PaddedBlockBitLength());}
	
	size_t PaddedBlockBitLength() const {return SaturatingSubtract(this->GetTrapdoorFunctionBounds().PreimageBound().BitCount(),1U);}
};



class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE TF_DecryptorBase : public TF_CryptoSystemBase<PK_Decryptor, TF_Base<TrapdoorFunctionInverse, PK_EncryptionMessageEncodingMethod> >
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~TF_DecryptorBase() { }
#endif

	DecodingResult Decrypt(RandomNumberGenerator &rng, const byte *ciphertext, size_t ciphertextLength, byte *plaintext, const NameValuePairs &parameters = g_nullNameValuePairs) const;
};



class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE TF_EncryptorBase : public TF_CryptoSystemBase<PK_Encryptor, TF_Base<RandomizedTrapdoorFunction, PK_EncryptionMessageEncodingMethod> >
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~TF_EncryptorBase() { }
#endif

	void Encrypt(RandomNumberGenerator &rng, const byte *plaintext, size_t plaintextLength, byte *ciphertext, const NameValuePairs &parameters = g_nullNameValuePairs) const;
};




typedef std::pair<const byte *, unsigned int> HashIdentifier;







class CRYPTOPP_NO_VTABLE PK_SignatureMessageEncodingMethod
{
public:
	virtual ~PK_SignatureMessageEncodingMethod() {}

	virtual size_t MinRepresentativeBitLength(size_t hashIdentifierLength, size_t digestLength) const
		{CRYPTOPP_UNUSED(hashIdentifierLength); CRYPTOPP_UNUSED(digestLength); return 0;}
	virtual size_t MaxRecoverableLength(size_t representativeBitLength, size_t hashIdentifierLength, size_t digestLength) const
		{CRYPTOPP_UNUSED(representativeBitLength); CRYPTOPP_UNUSED(representativeBitLength); CRYPTOPP_UNUSED(hashIdentifierLength); CRYPTOPP_UNUSED(digestLength); return 0;}

	bool IsProbabilistic() const
		{return true;}
	bool AllowNonrecoverablePart() const
		{throw NotImplemented("PK_MessageEncodingMethod: this signature scheme does not support message recovery");}
	virtual bool RecoverablePartFirst() const
		{throw NotImplemented("PK_MessageEncodingMethod: this signature scheme does not support message recovery");}

	
	virtual void ProcessSemisignature(HashTransformation &hash, const byte *semisignature, size_t semisignatureLength) const
		{CRYPTOPP_UNUSED(hash); CRYPTOPP_UNUSED(semisignature); CRYPTOPP_UNUSED(semisignatureLength);}

	
	virtual void ProcessRecoverableMessage(HashTransformation &hash,
		const byte *recoverableMessage, size_t recoverableMessageLength,
		const byte *presignature, size_t presignatureLength,
		SecByteBlock &semisignature) const
	{
		CRYPTOPP_UNUSED(hash);CRYPTOPP_UNUSED(recoverableMessage); CRYPTOPP_UNUSED(recoverableMessageLength);
		CRYPTOPP_UNUSED(presignature); CRYPTOPP_UNUSED(presignatureLength); CRYPTOPP_UNUSED(semisignature);
		if (RecoverablePartFirst())
			CRYPTOPP_ASSERT(!"ProcessRecoverableMessage() not implemented");
	}

	virtual void ComputeMessageRepresentative(RandomNumberGenerator &rng,
		const byte *recoverableMessage, size_t recoverableMessageLength,
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength) const =0;

	virtual bool VerifyMessageRepresentative(
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength) const =0;

	virtual DecodingResult RecoverMessageFromRepresentative(	
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength,
		byte *recoveredMessage) const
		{CRYPTOPP_UNUSED(hash);CRYPTOPP_UNUSED(hashIdentifier); CRYPTOPP_UNUSED(messageEmpty);
		CRYPTOPP_UNUSED(representative); CRYPTOPP_UNUSED(representativeBitLength); CRYPTOPP_UNUSED(recoveredMessage);
		throw NotImplemented("PK_MessageEncodingMethod: this signature scheme does not support message recovery");}

	virtual DecodingResult RecoverMessageFromSemisignature(		
		HashTransformation &hash, HashIdentifier hashIdentifier,
		const byte *presignature, size_t presignatureLength,
		const byte *semisignature, size_t semisignatureLength,
		byte *recoveredMessage) const
		{CRYPTOPP_UNUSED(hash);CRYPTOPP_UNUSED(hashIdentifier); CRYPTOPP_UNUSED(presignature); CRYPTOPP_UNUSED(presignatureLength);
		CRYPTOPP_UNUSED(semisignature); CRYPTOPP_UNUSED(semisignatureLength); CRYPTOPP_UNUSED(recoveredMessage);
		throw NotImplemented("PK_MessageEncodingMethod: this signature scheme does not support message recovery");}

	
	struct HashIdentifierLookup
	{
		template <class H> struct HashIdentifierLookup2
		{
			static HashIdentifier CRYPTOPP_API Lookup()
			{
				return HashIdentifier((const byte *)NULL, 0);
			}
		};
	};
};





class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE PK_DeterministicSignatureMessageEncodingMethod : public PK_SignatureMessageEncodingMethod
{
public:
	bool VerifyMessageRepresentative(
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength) const;
};





class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE PK_RecoverableSignatureMessageEncodingMethod : public PK_SignatureMessageEncodingMethod
{
public:
	bool VerifyMessageRepresentative(
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength) const;
};





class CRYPTOPP_DLL DL_SignatureMessageEncodingMethod_DSA : public PK_DeterministicSignatureMessageEncodingMethod
{
public:
	void ComputeMessageRepresentative(RandomNumberGenerator &rng,
		const byte *recoverableMessage, size_t recoverableMessageLength,
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength) const;
};





class CRYPTOPP_DLL DL_SignatureMessageEncodingMethod_NR : public PK_DeterministicSignatureMessageEncodingMethod
{
public:
	void ComputeMessageRepresentative(RandomNumberGenerator &rng,
		const byte *recoverableMessage, size_t recoverableMessageLength,
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength) const;
};





class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE PK_MessageAccumulatorBase : public PK_MessageAccumulator
{
public:
	PK_MessageAccumulatorBase() : m_empty(true) {}

	virtual HashTransformation & AccessHash() =0;

	void Update(const byte *input, size_t length)
	{
		AccessHash().Update(input, length);
		m_empty = m_empty && length == 0;
	}

	SecByteBlock m_recoverableMessage, m_representative, m_presignature, m_semisignature;
	Integer m_k, m_s;
	bool m_empty;
};





template <class HASH_ALGORITHM>
class PK_MessageAccumulatorImpl : public PK_MessageAccumulatorBase, protected ObjectHolder<HASH_ALGORITHM>
{
public:
	HashTransformation & AccessHash() {return this->m_object;}
};


template <class INTERFACE, class BASE>
class CRYPTOPP_NO_VTABLE TF_SignatureSchemeBase : public INTERFACE, protected BASE
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~TF_SignatureSchemeBase() { }
#endif

	size_t SignatureLength() const
		{return this->GetTrapdoorFunctionBounds().MaxPreimage().ByteCount();}
	size_t MaxRecoverableLength() const
		{return this->GetMessageEncodingInterface().MaxRecoverableLength(MessageRepresentativeBitLength(), GetHashIdentifier().second, GetDigestSize());}
	size_t MaxRecoverableLengthFromSignatureLength(size_t signatureLength) const
		{CRYPTOPP_UNUSED(signatureLength); return this->MaxRecoverableLength();}

	bool IsProbabilistic() const
		{return this->GetTrapdoorFunctionInterface().IsRandomized() || this->GetMessageEncodingInterface().IsProbabilistic();}
	bool AllowNonrecoverablePart() const
		{return this->GetMessageEncodingInterface().AllowNonrecoverablePart();}
	bool RecoverablePartFirst() const
		{return this->GetMessageEncodingInterface().RecoverablePartFirst();}

protected:
	size_t MessageRepresentativeLength() const {return BitsToBytes(MessageRepresentativeBitLength());}
	
	size_t MessageRepresentativeBitLength() const {return SaturatingSubtract(this->GetTrapdoorFunctionBounds().ImageBound().BitCount(),1U);}
	virtual HashIdentifier GetHashIdentifier() const =0;
	virtual size_t GetDigestSize() const =0;
};


class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE TF_SignerBase : public TF_SignatureSchemeBase<PK_Signer, TF_Base<RandomizedTrapdoorFunctionInverse, PK_SignatureMessageEncodingMethod> >
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~TF_SignerBase() { }
#endif

	void InputRecoverableMessage(PK_MessageAccumulator &messageAccumulator, const byte *recoverableMessage, size_t recoverableMessageLength) const;
	size_t SignAndRestart(RandomNumberGenerator &rng, PK_MessageAccumulator &messageAccumulator, byte *signature, bool restart=true) const;
};


class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE TF_VerifierBase : public TF_SignatureSchemeBase<PK_Verifier, TF_Base<TrapdoorFunction, PK_SignatureMessageEncodingMethod> >
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~TF_VerifierBase() { }
#endif

	void InputSignature(PK_MessageAccumulator &messageAccumulator, const byte *signature, size_t signatureLength) const;
	bool VerifyAndRestart(PK_MessageAccumulator &messageAccumulator) const;
	DecodingResult RecoverAndRestart(byte *recoveredMessage, PK_MessageAccumulator &recoveryAccumulator) const;
};




template <class T1, class T2, class T3>
struct TF_CryptoSchemeOptions
{
	typedef T1 AlgorithmInfo;
	typedef T2 Keys;
	typedef typename Keys::PrivateKey PrivateKey;
	typedef typename Keys::PublicKey PublicKey;
	typedef T3 MessageEncodingMethod;
};


template <class T1, class T2, class T3, class T4>
struct TF_SignatureSchemeOptions : public TF_CryptoSchemeOptions<T1, T2, T3>
{
	typedef T4 HashFunction;
};


template <class BASE, class SCHEME_OPTIONS, class KEY_CLASS>
class CRYPTOPP_NO_VTABLE TF_ObjectImplBase : public AlgorithmImpl<BASE, typename SCHEME_OPTIONS::AlgorithmInfo>
{
public:
	typedef SCHEME_OPTIONS SchemeOptions;
	typedef KEY_CLASS KeyClass;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~TF_ObjectImplBase() { }
#endif

	PublicKey & AccessPublicKey() {return AccessKey();}
	const PublicKey & GetPublicKey() const {return GetKey();}

	PrivateKey & AccessPrivateKey() {return AccessKey();}
	const PrivateKey & GetPrivateKey() const {return GetKey();}

	virtual const KeyClass & GetKey() const =0;
	virtual KeyClass & AccessKey() =0;

	const KeyClass & GetTrapdoorFunction() const {return GetKey();}

	PK_MessageAccumulator * NewSignatureAccumulator(RandomNumberGenerator &rng) const
	{
		CRYPTOPP_UNUSED(rng);
		return new PK_MessageAccumulatorImpl<CPP_TYPENAME SCHEME_OPTIONS::HashFunction>;
	}
	PK_MessageAccumulator * NewVerificationAccumulator() const
	{
		return new PK_MessageAccumulatorImpl<CPP_TYPENAME SCHEME_OPTIONS::HashFunction>;
	}

protected:
	const typename BASE::MessageEncodingInterface & GetMessageEncodingInterface() const
		{return Singleton<CPP_TYPENAME SCHEME_OPTIONS::MessageEncodingMethod>().Ref();}
	const TrapdoorFunctionBounds & GetTrapdoorFunctionBounds() const
		{return GetKey();}
	const typename BASE::TrapdoorFunctionInterface & GetTrapdoorFunctionInterface() const
		{return GetKey();}

	
	HashIdentifier GetHashIdentifier() const
	{
        typedef CPP_TYPENAME SchemeOptions::MessageEncodingMethod::HashIdentifierLookup::template HashIdentifierLookup2<CPP_TYPENAME SchemeOptions::HashFunction> L;
        return L::Lookup();
	}
	size_t GetDigestSize() const
	{
		typedef CPP_TYPENAME SchemeOptions::HashFunction H;
		return H::DIGESTSIZE;
	}
};


template <class BASE, class SCHEME_OPTIONS, class KEY>
class TF_ObjectImplExtRef : public TF_ObjectImplBase<BASE, SCHEME_OPTIONS, KEY>
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~TF_ObjectImplExtRef() { }
#endif

	TF_ObjectImplExtRef(const KEY *pKey = NULL) : m_pKey(pKey) {}
	void SetKeyPtr(const KEY *pKey) {m_pKey = pKey;}

	const KEY & GetKey() const {return *m_pKey;}
	KEY & AccessKey() {throw NotImplemented("TF_ObjectImplExtRef: cannot modify refererenced key");}

private:
	const KEY * m_pKey;
};


template <class BASE, class SCHEME_OPTIONS, class KEY_CLASS>
class CRYPTOPP_NO_VTABLE TF_ObjectImpl : public TF_ObjectImplBase<BASE, SCHEME_OPTIONS, KEY_CLASS>
{
public:
	typedef KEY_CLASS KeyClass;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~TF_ObjectImpl() { }
#endif

	const KeyClass & GetKey() const {return m_trapdoorFunction;}
	KeyClass & AccessKey() {return m_trapdoorFunction;}

private:
	KeyClass m_trapdoorFunction;
};


template <class SCHEME_OPTIONS>
class TF_DecryptorImpl : public TF_ObjectImpl<TF_DecryptorBase, SCHEME_OPTIONS, typename SCHEME_OPTIONS::PrivateKey>
{
};


template <class SCHEME_OPTIONS>
class TF_EncryptorImpl : public TF_ObjectImpl<TF_EncryptorBase, SCHEME_OPTIONS, typename SCHEME_OPTIONS::PublicKey>
{
};


template <class SCHEME_OPTIONS>
class TF_SignerImpl : public TF_ObjectImpl<TF_SignerBase, SCHEME_OPTIONS, typename SCHEME_OPTIONS::PrivateKey>
{
};


template <class SCHEME_OPTIONS>
class TF_VerifierImpl : public TF_ObjectImpl<TF_VerifierBase, SCHEME_OPTIONS, typename SCHEME_OPTIONS::PublicKey>
{
};





class CRYPTOPP_NO_VTABLE MaskGeneratingFunction
{
public:
	virtual ~MaskGeneratingFunction() {}

	
	
	
	
	
	
	
	virtual void GenerateAndMask(HashTransformation &hash, byte *output, size_t outputLength, const byte *input, size_t inputLength, bool mask = true) const =0;
};












CRYPTOPP_DLL void CRYPTOPP_API P1363_MGF1KDF2_Common(HashTransformation &hash, byte *output, size_t outputLength, const byte *input, size_t inputLength, const byte *derivationParams, size_t derivationParamsLength, bool mask, unsigned int counterStart);



class P1363_MGF1 : public MaskGeneratingFunction
{
public:
	CRYPTOPP_CONSTEXPR static const char * CRYPTOPP_API StaticAlgorithmName() {return "MGF1";}
	void GenerateAndMask(HashTransformation &hash, byte *output, size_t outputLength, const byte *input, size_t inputLength, bool mask = true) const
	{
		P1363_MGF1KDF2_Common(hash, output, outputLength, input, inputLength, NULL, 0, mask, 0);
	}
};






template <class H>
class P1363_KDF2
{
public:
	static void CRYPTOPP_API DeriveKey(byte *output, size_t outputLength, const byte *input, size_t inputLength, const byte *derivationParams, size_t derivationParamsLength)
	{
		H h;
		P1363_MGF1KDF2_Common(h, output, outputLength, input, inputLength, derivationParams, derivationParamsLength, false, 1);
	}
};





class DL_BadElement : public InvalidDataFormat
{
public:
	DL_BadElement() : InvalidDataFormat("CryptoPP: invalid group element") {}
};




template <class T>
class CRYPTOPP_NO_VTABLE DL_GroupParameters : public CryptoParameters
{
	typedef DL_GroupParameters<T> ThisClass;

public:
	typedef T Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_GroupParameters() { }
#endif

	DL_GroupParameters() : m_validationLevel(0) {}

	
	bool Validate(RandomNumberGenerator &rng, unsigned int level) const
	{
		if (!GetBasePrecomputation().IsInitialized())
			return false;

		if (m_validationLevel > level)
			return true;

		bool pass = ValidateGroup(rng, level);
		pass = pass && ValidateElement(level, GetSubgroupGenerator(), &GetBasePrecomputation());

		m_validationLevel = pass ? level+1 : 0;

		return pass;
	}

	bool GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const
	{
		return GetValueHelper(this, name, valueType, pValue)
			CRYPTOPP_GET_FUNCTION_ENTRY(SubgroupOrder)
			CRYPTOPP_GET_FUNCTION_ENTRY(SubgroupGenerator)
			;
	}

	bool SupportsPrecomputation() const {return true;}

	void Precompute(unsigned int precomputationStorage=16)
	{
		AccessBasePrecomputation().Precompute(GetGroupPrecomputation(), GetSubgroupOrder().BitCount(), precomputationStorage);
	}

	void LoadPrecomputation(BufferedTransformation &storedPrecomputation)
	{
		AccessBasePrecomputation().Load(GetGroupPrecomputation(), storedPrecomputation);
		m_validationLevel = 0;
	}

	void SavePrecomputation(BufferedTransformation &storedPrecomputation) const
	{
		GetBasePrecomputation().Save(GetGroupPrecomputation(), storedPrecomputation);
	}

	
	
	
	virtual const Element & GetSubgroupGenerator() const {return GetBasePrecomputation().GetBase(GetGroupPrecomputation());}

	
	
	
	virtual void SetSubgroupGenerator(const Element &base) {AccessBasePrecomputation().SetBase(GetGroupPrecomputation(), base);}

	
	
	
	virtual Element ExponentiateBase(const Integer &exponent) const
	{
		return GetBasePrecomputation().Exponentiate(GetGroupPrecomputation(), exponent);
	}

	
	
	
	
	
	virtual Element ExponentiateElement(const Element &base, const Integer &exponent) const
	{
		Element result;
		SimultaneousExponentiate(&result, base, &exponent, 1);
		return result;
	}

	
	
	virtual const DL_GroupPrecomputation<Element> & GetGroupPrecomputation() const =0;

	
	
	virtual const DL_FixedBasePrecomputation<Element> & GetBasePrecomputation() const =0;

	
	
	virtual DL_FixedBasePrecomputation<Element> & AccessBasePrecomputation() =0;

	
	
	virtual const Integer & GetSubgroupOrder() const =0;

	
	
	virtual Integer GetMaxExponent() const =0;

	
	
	
	virtual Integer GetGroupOrder() const {return GetSubgroupOrder()*GetCofactor();}

	
	
	
	virtual Integer GetCofactor() const {return GetGroupOrder()/GetSubgroupOrder();}

	
	
	
	
	
	
	virtual unsigned int GetEncodedElementSize(bool reversible) const =0;

	
	
	
	
	
	
	virtual void EncodeElement(bool reversible, const Element &element, byte *encoded) const =0;

	
	
	
	
	
	
	virtual Element DecodeElement(const byte *encoded, bool checkForGroupMembership) const =0;

	
	
	
	
	virtual Integer ConvertElementToInteger(const Element &element) const =0;

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	virtual bool ValidateGroup(RandomNumberGenerator &rng, unsigned int level) const =0;

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	virtual bool ValidateElement(unsigned int level, const Element &element, const DL_FixedBasePrecomputation<Element> *precomp) const =0;

	virtual bool FastSubgroupCheckAvailable() const =0;

	
	
	
	
	
	
	virtual bool IsIdentity(const Element &element) const =0;

	
	
	
	
	
	
	
	
	
	
	virtual void SimultaneousExponentiate(Element *results, const Element &base, const Integer *exponents, unsigned int exponentsCount) const =0;

protected:
	void ParametersChanged() {m_validationLevel = 0;}

private:
	mutable unsigned int m_validationLevel;
};





template <class GROUP_PRECOMP, class BASE_PRECOMP = DL_FixedBasePrecomputationImpl<CPP_TYPENAME GROUP_PRECOMP::Element>, class BASE = DL_GroupParameters<CPP_TYPENAME GROUP_PRECOMP::Element> >
class DL_GroupParametersImpl : public BASE
{
public:
	typedef GROUP_PRECOMP GroupPrecomputation;
	typedef typename GROUP_PRECOMP::Element Element;
	typedef BASE_PRECOMP BasePrecomputation;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_GroupParametersImpl() { }
#endif

	
	
	const DL_GroupPrecomputation<Element> & GetGroupPrecomputation() const {return m_groupPrecomputation;}

	
	
	const DL_FixedBasePrecomputation<Element> & GetBasePrecomputation() const {return m_gpc;}

	
	
	DL_FixedBasePrecomputation<Element> & AccessBasePrecomputation() {return m_gpc;}

protected:
	GROUP_PRECOMP m_groupPrecomputation;
	BASE_PRECOMP m_gpc;
};




template <class T>
class CRYPTOPP_NO_VTABLE DL_Key
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_Key() { }
#endif

	
	
	virtual const DL_GroupParameters<T> & GetAbstractGroupParameters() const =0;
	
	
	virtual DL_GroupParameters<T> & AccessAbstractGroupParameters() =0;
};


template <class T>
class CRYPTOPP_NO_VTABLE DL_PublicKey : public DL_Key<T>
{
	typedef DL_PublicKey<T> ThisClass;

public:
	typedef T Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_PublicKey() { }
#endif

	bool GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const
	{
		return GetValueHelper(this, name, valueType, pValue, &this->GetAbstractGroupParameters())
				CRYPTOPP_GET_FUNCTION_ENTRY(PublicElement);
	}

	void AssignFrom(const NameValuePairs &source);

	
	virtual const Element & GetPublicElement() const {return GetPublicPrecomputation().GetBase(this->GetAbstractGroupParameters().GetGroupPrecomputation());}
	virtual void SetPublicElement(const Element &y) {AccessPublicPrecomputation().SetBase(this->GetAbstractGroupParameters().GetGroupPrecomputation(), y);}
	virtual Element ExponentiatePublicElement(const Integer &exponent) const
	{
		const DL_GroupParameters<T> &params = this->GetAbstractGroupParameters();
		return GetPublicPrecomputation().Exponentiate(params.GetGroupPrecomputation(), exponent);
	}
	virtual Element CascadeExponentiateBaseAndPublicElement(const Integer &baseExp, const Integer &publicExp) const
	{
		const DL_GroupParameters<T> &params = this->GetAbstractGroupParameters();
		return params.GetBasePrecomputation().CascadeExponentiate(params.GetGroupPrecomputation(), baseExp, GetPublicPrecomputation(), publicExp);
	}

	virtual const DL_FixedBasePrecomputation<T> & GetPublicPrecomputation() const =0;
	virtual DL_FixedBasePrecomputation<T> & AccessPublicPrecomputation() =0;
};


template <class T>
class CRYPTOPP_NO_VTABLE DL_PrivateKey : public DL_Key<T>
{
	typedef DL_PrivateKey<T> ThisClass;

public:
	typedef T Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_PrivateKey() { }
#endif

	void MakePublicKey(DL_PublicKey<T> &pub) const
	{
		pub.AccessAbstractGroupParameters().AssignFrom(this->GetAbstractGroupParameters());
		pub.SetPublicElement(this->GetAbstractGroupParameters().ExponentiateBase(GetPrivateExponent()));
	}

	bool GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const
	{
		return GetValueHelper(this, name, valueType, pValue, &this->GetAbstractGroupParameters())
				CRYPTOPP_GET_FUNCTION_ENTRY(PrivateExponent);
	}

	void AssignFrom(const NameValuePairs &source)
	{
		this->AccessAbstractGroupParameters().AssignFrom(source);
		AssignFromHelper(this, source)
			CRYPTOPP_SET_FUNCTION_ENTRY(PrivateExponent);
	}

	virtual const Integer & GetPrivateExponent() const =0;
	virtual void SetPrivateExponent(const Integer &x) =0;
};

template <class T>
void DL_PublicKey<T>::AssignFrom(const NameValuePairs &source)
{
	DL_PrivateKey<T> *pPrivateKey = NULL;
	if (source.GetThisPointer(pPrivateKey))
		pPrivateKey->MakePublicKey(*this);
	else
	{
		this->AccessAbstractGroupParameters().AssignFrom(source);
		AssignFromHelper(this, source)
			CRYPTOPP_SET_FUNCTION_ENTRY(PublicElement);
	}
}

class OID;


template <class PK, class GP, class O = OID>
class DL_KeyImpl : public PK
{
public:
	typedef GP GroupParameters;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_KeyImpl() { }
#endif

	O GetAlgorithmID() const {return GetGroupParameters().GetAlgorithmID();}




	bool BERDecodeAlgorithmParameters(BufferedTransformation &bt)
		{AccessGroupParameters().BERDecode(bt); return true;}
	bool DEREncodeAlgorithmParameters(BufferedTransformation &bt) const
		{GetGroupParameters().DEREncode(bt); return true;}

	const GP & GetGroupParameters() const {return m_groupParameters;}
	GP & AccessGroupParameters() {return m_groupParameters;}

private:
	GP m_groupParameters;
};

class X509PublicKey;
class PKCS8PrivateKey;


template <class GP>
class DL_PrivateKeyImpl : public DL_PrivateKey<CPP_TYPENAME GP::Element>, public DL_KeyImpl<PKCS8PrivateKey, GP>
{
public:
	typedef typename GP::Element Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_PrivateKeyImpl() { }
#endif

	
	bool Validate(RandomNumberGenerator &rng, unsigned int level) const
	{
		bool pass = GetAbstractGroupParameters().Validate(rng, level);

		const Integer &q = GetAbstractGroupParameters().GetSubgroupOrder();
		const Integer &x = GetPrivateExponent();

		pass = pass && x.IsPositive() && x < q;
		if (level >= 1)
			pass = pass && Integer::Gcd(x, q) == Integer::One();
		return pass;
	}

	bool GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const
	{
		return GetValueHelper<DL_PrivateKey<Element> >(this, name, valueType, pValue).Assignable();
	}

	void AssignFrom(const NameValuePairs &source)
	{
		AssignFromHelper<DL_PrivateKey<Element> >(this, source);
	}

	void GenerateRandom(RandomNumberGenerator &rng, const NameValuePairs &params)
	{
		if (!params.GetThisObject(this->AccessGroupParameters()))
			this->AccessGroupParameters().GenerateRandom(rng, params);

		Integer x(rng, Integer::One(), GetAbstractGroupParameters().GetMaxExponent());


		SetPrivateExponent(x);
	}

	bool SupportsPrecomputation() const {return true;}

	void Precompute(unsigned int precomputationStorage=16)
		{AccessAbstractGroupParameters().Precompute(precomputationStorage);}

	void LoadPrecomputation(BufferedTransformation &storedPrecomputation)
		{AccessAbstractGroupParameters().LoadPrecomputation(storedPrecomputation);}

	void SavePrecomputation(BufferedTransformation &storedPrecomputation) const
		{GetAbstractGroupParameters().SavePrecomputation(storedPrecomputation);}

	
	const DL_GroupParameters<Element> & GetAbstractGroupParameters() const {return this->GetGroupParameters();}
	DL_GroupParameters<Element> & AccessAbstractGroupParameters() {return this->AccessGroupParameters();}

	
	const Integer & GetPrivateExponent() const {return m_x;}
	void SetPrivateExponent(const Integer &x) {m_x = x;}

	
	void BERDecodePrivateKey(BufferedTransformation &bt, bool, size_t)
		{m_x.BERDecode(bt);}
	void DEREncodePrivateKey(BufferedTransformation &bt) const
		{m_x.DEREncode(bt);}

private:
	Integer m_x;
};


template <class BASE, class SIGNATURE_SCHEME>
class DL_PrivateKey_WithSignaturePairwiseConsistencyTest : public BASE
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_PrivateKey_WithSignaturePairwiseConsistencyTest() { }
#endif

	void GenerateRandom(RandomNumberGenerator &rng, const NameValuePairs &params)
	{
		BASE::GenerateRandom(rng, params);

		if (FIPS_140_2_ComplianceEnabled())
		{
			typename SIGNATURE_SCHEME::Signer signer(*this);
			typename SIGNATURE_SCHEME::Verifier verifier(signer);
			SignaturePairwiseConsistencyTest_FIPS_140_Only(signer, verifier);
		}
	}
};


template <class GP>
class DL_PublicKeyImpl : public DL_PublicKey<typename GP::Element>, public DL_KeyImpl<X509PublicKey, GP>
{
public:
	typedef typename GP::Element Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_PublicKeyImpl() { }
#endif

	
	bool Validate(RandomNumberGenerator &rng, unsigned int level) const
	{
		bool pass = GetAbstractGroupParameters().Validate(rng, level);
		pass = pass && GetAbstractGroupParameters().ValidateElement(level, this->GetPublicElement(), &GetPublicPrecomputation());
		return pass;
	}

	bool GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const
	{
		return GetValueHelper<DL_PublicKey<Element> >(this, name, valueType, pValue).Assignable();
	}

	void AssignFrom(const NameValuePairs &source)
	{
		AssignFromHelper<DL_PublicKey<Element> >(this, source);
	}

	bool SupportsPrecomputation() const {return true;}

	void Precompute(unsigned int precomputationStorage=16)
	{
		AccessAbstractGroupParameters().Precompute(precomputationStorage);
		AccessPublicPrecomputation().Precompute(GetAbstractGroupParameters().GetGroupPrecomputation(), GetAbstractGroupParameters().GetSubgroupOrder().BitCount(), precomputationStorage);
	}

	void LoadPrecomputation(BufferedTransformation &storedPrecomputation)
	{
		AccessAbstractGroupParameters().LoadPrecomputation(storedPrecomputation);
		AccessPublicPrecomputation().Load(GetAbstractGroupParameters().GetGroupPrecomputation(), storedPrecomputation);
	}

	void SavePrecomputation(BufferedTransformation &storedPrecomputation) const
	{
		GetAbstractGroupParameters().SavePrecomputation(storedPrecomputation);
		GetPublicPrecomputation().Save(GetAbstractGroupParameters().GetGroupPrecomputation(), storedPrecomputation);
	}

	
	const DL_GroupParameters<Element> & GetAbstractGroupParameters() const {return this->GetGroupParameters();}
	DL_GroupParameters<Element> & AccessAbstractGroupParameters() {return this->AccessGroupParameters();}

	
	const DL_FixedBasePrecomputation<Element> & GetPublicPrecomputation() const {return m_ypc;}
	DL_FixedBasePrecomputation<Element> & AccessPublicPrecomputation() {return m_ypc;}

	
	bool operator==(const DL_PublicKeyImpl<GP> &rhs) const
		{return this->GetGroupParameters() == rhs.GetGroupParameters() && this->GetPublicElement() == rhs.GetPublicElement();}

private:
	typename GP::BasePrecomputation m_ypc;
};


template <class T>
class CRYPTOPP_NO_VTABLE DL_ElgamalLikeSignatureAlgorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_ElgamalLikeSignatureAlgorithm() { }
#endif

	virtual void Sign(const DL_GroupParameters<T> &params, const Integer &privateKey, const Integer &k, const Integer &e, Integer &r, Integer &s) const =0;
	virtual bool Verify(const DL_GroupParameters<T> &params, const DL_PublicKey<T> &publicKey, const Integer &e, const Integer &r, const Integer &s) const =0;
	virtual Integer RecoverPresignature(const DL_GroupParameters<T> &params, const DL_PublicKey<T> &publicKey, const Integer &r, const Integer &s) const
	{
		CRYPTOPP_UNUSED(params); CRYPTOPP_UNUSED(publicKey); CRYPTOPP_UNUSED(r); CRYPTOPP_UNUSED(s);
		throw NotImplemented("DL_ElgamalLikeSignatureAlgorithm: this signature scheme does not support message recovery");
		MAYBE_RETURN(Integer::Zero());
	}
	virtual size_t RLen(const DL_GroupParameters<T> &params) const
		{return params.GetSubgroupOrder().ByteCount();}
	virtual size_t SLen(const DL_GroupParameters<T> &params) const
		{return params.GetSubgroupOrder().ByteCount();}
};


template <class T>
class CRYPTOPP_NO_VTABLE DL_KeyAgreementAlgorithm
{
public:
	typedef T Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_KeyAgreementAlgorithm() { }
#endif

	virtual Element AgreeWithEphemeralPrivateKey(const DL_GroupParameters<Element> &params, const DL_FixedBasePrecomputation<Element> &publicPrecomputation, const Integer &privateExponent) const =0;
	virtual Element AgreeWithStaticPrivateKey(const DL_GroupParameters<Element> &params, const Element &publicElement, bool validateOtherPublicKey, const Integer &privateExponent) const =0;
};


template <class T>
class CRYPTOPP_NO_VTABLE DL_KeyDerivationAlgorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_KeyDerivationAlgorithm() { }
#endif

	virtual bool ParameterSupported(const char *name) const
		{CRYPTOPP_UNUSED(name); return false;}
	virtual void Derive(const DL_GroupParameters<T> &groupParams, byte *derivedKey, size_t derivedLength, const T &agreedElement, const T &ephemeralPublicKey, const NameValuePairs &derivationParams) const =0;
};


class CRYPTOPP_NO_VTABLE DL_SymmetricEncryptionAlgorithm
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_SymmetricEncryptionAlgorithm() { }
#endif

	virtual bool ParameterSupported(const char *name) const
		{CRYPTOPP_UNUSED(name); return false;}
	virtual size_t GetSymmetricKeyLength(size_t plaintextLength) const =0;
	virtual size_t GetSymmetricCiphertextLength(size_t plaintextLength) const =0;
	virtual size_t GetMaxSymmetricPlaintextLength(size_t ciphertextLength) const =0;
	virtual void SymmetricEncrypt(RandomNumberGenerator &rng, const byte *key, const byte *plaintext, size_t plaintextLength, byte *ciphertext, const NameValuePairs &parameters) const =0;
	virtual DecodingResult SymmetricDecrypt(const byte *key, const byte *ciphertext, size_t ciphertextLength, byte *plaintext, const NameValuePairs &parameters) const =0;
};



template <class KI>
class CRYPTOPP_NO_VTABLE DL_Base
{
protected:
	typedef KI KeyInterface;
	typedef typename KI::Element Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_Base() { }
#endif

	const DL_GroupParameters<Element> & GetAbstractGroupParameters() const {return GetKeyInterface().GetAbstractGroupParameters();}
	DL_GroupParameters<Element> & AccessAbstractGroupParameters() {return AccessKeyInterface().AccessAbstractGroupParameters();}

	virtual KeyInterface & AccessKeyInterface() =0;
	virtual const KeyInterface & GetKeyInterface() const =0;
};






template <class INTERFACE, class KEY_INTERFACE>
class CRYPTOPP_NO_VTABLE DL_SignatureSchemeBase : public INTERFACE, public DL_Base<KEY_INTERFACE>
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_SignatureSchemeBase() { }
#endif

	
	
	
	size_t SignatureLength() const
	{
		return GetSignatureAlgorithm().RLen(this->GetAbstractGroupParameters())
			+ GetSignatureAlgorithm().SLen(this->GetAbstractGroupParameters());
	}

	
	
	size_t MaxRecoverableLength() const
		{return GetMessageEncodingInterface().MaxRecoverableLength(0, GetHashIdentifier().second, GetDigestSize());}

	
	
	
	
	size_t MaxRecoverableLengthFromSignatureLength(size_t signatureLength) const
		{CRYPTOPP_UNUSED(signatureLength); CRYPTOPP_ASSERT(false); return 0;}	

	
	
	bool IsProbabilistic() const
		{return true;}

	
	
	bool AllowNonrecoverablePart() const
		{return GetMessageEncodingInterface().AllowNonrecoverablePart();}

	
	
	bool RecoverablePartFirst() const
		{return GetMessageEncodingInterface().RecoverablePartFirst();}

protected:
	size_t MessageRepresentativeLength() const {return BitsToBytes(MessageRepresentativeBitLength());}
	size_t MessageRepresentativeBitLength() const {return this->GetAbstractGroupParameters().GetSubgroupOrder().BitCount();}

	virtual const DL_ElgamalLikeSignatureAlgorithm<CPP_TYPENAME KEY_INTERFACE::Element> & GetSignatureAlgorithm() const =0;
	virtual const PK_SignatureMessageEncodingMethod & GetMessageEncodingInterface() const =0;
	virtual HashIdentifier GetHashIdentifier() const =0;
	virtual size_t GetDigestSize() const =0;
};



template <class T>
class CRYPTOPP_NO_VTABLE DL_SignerBase : public DL_SignatureSchemeBase<PK_Signer, DL_PrivateKey<T> >
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_SignerBase() { }
#endif

	
	
	
	
	
	void RawSign(const Integer &k, const Integer &e, Integer &r, Integer &s) const
	{
		const DL_ElgamalLikeSignatureAlgorithm<T> &alg = this->GetSignatureAlgorithm();
		const DL_GroupParameters<T> &params = this->GetAbstractGroupParameters();
		const DL_PrivateKey<T> &key = this->GetKeyInterface();

		r = params.ConvertElementToInteger(params.ExponentiateBase(k));
		alg.Sign(params, key.GetPrivateExponent(), k, e, r, s);
	}

	void InputRecoverableMessage(PK_MessageAccumulator &messageAccumulator, const byte *recoverableMessage, size_t recoverableMessageLength) const
	{
		PK_MessageAccumulatorBase &ma = static_cast<PK_MessageAccumulatorBase &>(messageAccumulator);
		ma.m_recoverableMessage.Assign(recoverableMessage, recoverableMessageLength);
		this->GetMessageEncodingInterface().ProcessRecoverableMessage(ma.AccessHash(),
			recoverableMessage, recoverableMessageLength,
			ma.m_presignature, ma.m_presignature.size(),
			ma.m_semisignature);
	}

	size_t SignAndRestart(RandomNumberGenerator &rng, PK_MessageAccumulator &messageAccumulator, byte *signature, bool restart) const
	{
		this->GetMaterial().DoQuickSanityCheck();

		PK_MessageAccumulatorBase &ma = static_cast<PK_MessageAccumulatorBase &>(messageAccumulator);
		const DL_ElgamalLikeSignatureAlgorithm<T> &alg = this->GetSignatureAlgorithm();
		const DL_GroupParameters<T> &params = this->GetAbstractGroupParameters();
		const DL_PrivateKey<T> &key = this->GetKeyInterface();

		SecByteBlock representative(this->MessageRepresentativeLength());
		this->GetMessageEncodingInterface().ComputeMessageRepresentative(
			rng,
			ma.m_recoverableMessage, ma.m_recoverableMessage.size(),
			ma.AccessHash(), this->GetHashIdentifier(), ma.m_empty,
			representative, this->MessageRepresentativeBitLength());
		ma.m_empty = true;
		Integer e(representative, representative.size());

		
		
		if (rng.CanIncorporateEntropy())
			rng.IncorporateEntropy(representative, representative.size());
		Integer k(rng, 1, params.GetSubgroupOrder()-1);
		Integer r, s;
		r = params.ConvertElementToInteger(params.ExponentiateBase(k));
		alg.Sign(params, key.GetPrivateExponent(), k, e, r, s);

		/*
		Integer r, s;
		if (this->MaxRecoverableLength() > 0)
			r.Decode(ma.m_semisignature, ma.m_semisignature.size());
		else
			r.Decode(ma.m_presignature, ma.m_presignature.size());
		alg.Sign(params, key.GetPrivateExponent(), ma.m_k, e, r, s);
		*/

		size_t rLen = alg.RLen(params);
		r.Encode(signature, rLen);
		s.Encode(signature+rLen, alg.SLen(params));

		if (restart)
			RestartMessageAccumulator(rng, ma);

		return this->SignatureLength();
	}

protected:
	void RestartMessageAccumulator(RandomNumberGenerator &rng, PK_MessageAccumulatorBase &ma) const
	{
		
		
		
		
		/*
		const DL_ElgamalLikeSignatureAlgorithm<T> &alg = this->GetSignatureAlgorithm();
		const DL_GroupParameters<T> &params = this->GetAbstractGroupParameters();
		ma.m_k.Randomize(rng, 1, params.GetSubgroupOrder()-1);
		ma.m_presignature.New(params.GetEncodedElementSize(false));
		params.ConvertElementToInteger(params.ExponentiateBase(ma.m_k)).Encode(ma.m_presignature, ma.m_presignature.size());
		*/
		CRYPTOPP_UNUSED(rng); CRYPTOPP_UNUSED(ma);
	}
};


template <class T>
class CRYPTOPP_NO_VTABLE DL_VerifierBase : public DL_SignatureSchemeBase<PK_Verifier, DL_PublicKey<T> >
{
public:
#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_VerifierBase() { }
#endif

	void InputSignature(PK_MessageAccumulator &messageAccumulator, const byte *signature, size_t signatureLength) const
	{
		CRYPTOPP_UNUSED(signature); CRYPTOPP_UNUSED(signatureLength);
		PK_MessageAccumulatorBase &ma = static_cast<PK_MessageAccumulatorBase &>(messageAccumulator);
		const DL_ElgamalLikeSignatureAlgorithm<T> &alg = this->GetSignatureAlgorithm();
		const DL_GroupParameters<T> &params = this->GetAbstractGroupParameters();

		size_t rLen = alg.RLen(params);
		ma.m_semisignature.Assign(signature, rLen);
		ma.m_s.Decode(signature+rLen, alg.SLen(params));

		this->GetMessageEncodingInterface().ProcessSemisignature(ma.AccessHash(), ma.m_semisignature, ma.m_semisignature.size());
	}

	bool VerifyAndRestart(PK_MessageAccumulator &messageAccumulator) const
	{
		this->GetMaterial().DoQuickSanityCheck();

		PK_MessageAccumulatorBase &ma = static_cast<PK_MessageAccumulatorBase &>(messageAccumulator);
		const DL_ElgamalLikeSignatureAlgorithm<T> &alg = this->GetSignatureAlgorithm();
		const DL_GroupParameters<T> &params = this->GetAbstractGroupParameters();
		const DL_PublicKey<T> &key = this->GetKeyInterface();

		SecByteBlock representative(this->MessageRepresentativeLength());
		this->GetMessageEncodingInterface().ComputeMessageRepresentative(NullRNG(), ma.m_recoverableMessage, ma.m_recoverableMessage.size(),
			ma.AccessHash(), this->GetHashIdentifier(), ma.m_empty,
			representative, this->MessageRepresentativeBitLength());
		ma.m_empty = true;
		Integer e(representative, representative.size());

		Integer r(ma.m_semisignature, ma.m_semisignature.size());
		return alg.Verify(params, key, e, r, ma.m_s);
	}

	DecodingResult RecoverAndRestart(byte *recoveredMessage, PK_MessageAccumulator &messageAccumulator) const
	{
		this->GetMaterial().DoQuickSanityCheck();

		PK_MessageAccumulatorBase &ma = static_cast<PK_MessageAccumulatorBase &>(messageAccumulator);
		const DL_ElgamalLikeSignatureAlgorithm<T> &alg = this->GetSignatureAlgorithm();
		const DL_GroupParameters<T> &params = this->GetAbstractGroupParameters();
		const DL_PublicKey<T> &key = this->GetKeyInterface();

		SecByteBlock representative(this->MessageRepresentativeLength());
		this->GetMessageEncodingInterface().ComputeMessageRepresentative(
			NullRNG(),
			ma.m_recoverableMessage, ma.m_recoverableMessage.size(),
			ma.AccessHash(), this->GetHashIdentifier(), ma.m_empty,
			representative, this->MessageRepresentativeBitLength());
		ma.m_empty = true;
		Integer e(representative, representative.size());

		ma.m_presignature.New(params.GetEncodedElementSize(false));
		Integer r(ma.m_semisignature, ma.m_semisignature.size());
		alg.RecoverPresignature(params, key, r, ma.m_s).Encode(ma.m_presignature, ma.m_presignature.size());

		return this->GetMessageEncodingInterface().RecoverMessageFromSemisignature(
			ma.AccessHash(), this->GetHashIdentifier(),
			ma.m_presignature, ma.m_presignature.size(),
			ma.m_semisignature, ma.m_semisignature.size(),
			recoveredMessage);
	}
};




template <class PK, class KI>
class CRYPTOPP_NO_VTABLE DL_CryptoSystemBase : public PK, public DL_Base<KI>
{
public:
	typedef typename DL_Base<KI>::Element Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_CryptoSystemBase() { }
#endif

	size_t MaxPlaintextLength(size_t ciphertextLength) const
	{
		unsigned int minLen = this->GetAbstractGroupParameters().GetEncodedElementSize(true);
		return ciphertextLength < minLen ? 0 : GetSymmetricEncryptionAlgorithm().GetMaxSymmetricPlaintextLength(ciphertextLength - minLen);
	}

	size_t CiphertextLength(size_t plaintextLength) const
	{
		size_t len = GetSymmetricEncryptionAlgorithm().GetSymmetricCiphertextLength(plaintextLength);
		return len == 0 ? 0 : this->GetAbstractGroupParameters().GetEncodedElementSize(true) + len;
	}

	bool ParameterSupported(const char *name) const
		{return GetKeyDerivationAlgorithm().ParameterSupported(name) || GetSymmetricEncryptionAlgorithm().ParameterSupported(name);}

protected:
	virtual const DL_KeyAgreementAlgorithm<Element> & GetKeyAgreementAlgorithm() const =0;
	virtual const DL_KeyDerivationAlgorithm<Element> & GetKeyDerivationAlgorithm() const =0;
	virtual const DL_SymmetricEncryptionAlgorithm & GetSymmetricEncryptionAlgorithm() const =0;
};



template <class T>
class CRYPTOPP_NO_VTABLE DL_DecryptorBase : public DL_CryptoSystemBase<PK_Decryptor, DL_PrivateKey<T> >
{
public:
	typedef T Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_DecryptorBase() { }
#endif

	DecodingResult Decrypt(RandomNumberGenerator &rng, const byte *ciphertext, size_t ciphertextLength, byte *plaintext, const NameValuePairs &parameters = g_nullNameValuePairs) const
	{
		try
		{
			CRYPTOPP_UNUSED(rng);
			const DL_KeyAgreementAlgorithm<T> &agreeAlg = this->GetKeyAgreementAlgorithm();
			const DL_KeyDerivationAlgorithm<T> &derivAlg = this->GetKeyDerivationAlgorithm();
			const DL_SymmetricEncryptionAlgorithm &encAlg = this->GetSymmetricEncryptionAlgorithm();
			const DL_GroupParameters<T> &params = this->GetAbstractGroupParameters();
			const DL_PrivateKey<T> &key = this->GetKeyInterface();

			Element q = params.DecodeElement(ciphertext, true);
			size_t elementSize = params.GetEncodedElementSize(true);
			ciphertext += elementSize;
			ciphertextLength -= elementSize;

			Element z = agreeAlg.AgreeWithStaticPrivateKey(params, q, true, key.GetPrivateExponent());

			SecByteBlock derivedKey(encAlg.GetSymmetricKeyLength(encAlg.GetMaxSymmetricPlaintextLength(ciphertextLength)));
			derivAlg.Derive(params, derivedKey, derivedKey.size(), z, q, parameters);

			return encAlg.SymmetricDecrypt(derivedKey, ciphertext, ciphertextLength, plaintext, parameters);
		}
		catch (DL_BadElement &)
		{
			return DecodingResult();
		}
	}
};



template <class T>
class CRYPTOPP_NO_VTABLE DL_EncryptorBase : public DL_CryptoSystemBase<PK_Encryptor, DL_PublicKey<T> >
{
public:
	typedef T Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_EncryptorBase() { }
#endif

	void Encrypt(RandomNumberGenerator &rng, const byte *plaintext, size_t plaintextLength, byte *ciphertext, const NameValuePairs &parameters = g_nullNameValuePairs) const
	{
		const DL_KeyAgreementAlgorithm<T> &agreeAlg = this->GetKeyAgreementAlgorithm();
		const DL_KeyDerivationAlgorithm<T> &derivAlg = this->GetKeyDerivationAlgorithm();
		const DL_SymmetricEncryptionAlgorithm &encAlg = this->GetSymmetricEncryptionAlgorithm();
		const DL_GroupParameters<T> &params = this->GetAbstractGroupParameters();
		const DL_PublicKey<T> &key = this->GetKeyInterface();

		Integer x(rng, Integer::One(), params.GetMaxExponent());
		Element q = params.ExponentiateBase(x);
		params.EncodeElement(true, q, ciphertext);
		unsigned int elementSize = params.GetEncodedElementSize(true);
		ciphertext += elementSize;

		Element z = agreeAlg.AgreeWithEphemeralPrivateKey(params, key.GetPublicPrecomputation(), x);

		SecByteBlock derivedKey(encAlg.GetSymmetricKeyLength(plaintextLength));
		derivAlg.Derive(params, derivedKey, derivedKey.size(), z, q, parameters);

		encAlg.SymmetricEncrypt(rng, derivedKey, plaintext, plaintextLength, ciphertext, parameters);
	}
};




template <class T1, class T2>
struct DL_SchemeOptionsBase
{
	typedef T1 AlgorithmInfo;
	typedef T2 GroupParameters;
	typedef typename GroupParameters::Element Element;
};




template <class T1, class T2>
struct DL_KeyedSchemeOptions : public DL_SchemeOptionsBase<T1, typename T2::PublicKey::GroupParameters>
{
	typedef T2 Keys;
	typedef typename Keys::PrivateKey PrivateKey;
	typedef typename Keys::PublicKey PublicKey;
};







template <class T1, class T2, class T3, class T4, class T5>
struct DL_SignatureSchemeOptions : public DL_KeyedSchemeOptions<T1, T2>
{
	typedef T3 SignatureAlgorithm;
	typedef T4 MessageEncodingMethod;
	typedef T5 HashFunction;
};







template <class T1, class T2, class T3, class T4, class T5>
struct DL_CryptoSchemeOptions : public DL_KeyedSchemeOptions<T1, T2>
{
	typedef T3 KeyAgreementAlgorithm;
	typedef T4 KeyDerivationAlgorithm;
	typedef T5 SymmetricEncryptionAlgorithm;
};





template <class BASE, class SCHEME_OPTIONS, class KEY>
class CRYPTOPP_NO_VTABLE DL_ObjectImplBase : public AlgorithmImpl<BASE, typename SCHEME_OPTIONS::AlgorithmInfo>
{
public:
	typedef SCHEME_OPTIONS SchemeOptions;
	typedef typename KEY::Element Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_ObjectImplBase() { }
#endif

	PrivateKey & AccessPrivateKey() {return m_key;}
	PublicKey & AccessPublicKey() {return m_key;}

	
	const KEY & GetKey() const {return m_key;}
	KEY & AccessKey() {return m_key;}

protected:
	typename BASE::KeyInterface & AccessKeyInterface() {return m_key;}
	const typename BASE::KeyInterface & GetKeyInterface() const {return m_key;}

	
	HashIdentifier GetHashIdentifier() const
	{
		typedef typename SchemeOptions::MessageEncodingMethod::HashIdentifierLookup HashLookup;
		return HashLookup::template HashIdentifierLookup2<CPP_TYPENAME SchemeOptions::HashFunction>::Lookup();
	}
	size_t GetDigestSize() const
	{
		typedef CPP_TYPENAME SchemeOptions::HashFunction H;
		return H::DIGESTSIZE;
	}

private:
	KEY m_key;
};





template <class BASE, class SCHEME_OPTIONS, class KEY>
class CRYPTOPP_NO_VTABLE DL_ObjectImpl : public DL_ObjectImplBase<BASE, SCHEME_OPTIONS, KEY>
{
public:
	typedef typename KEY::Element Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_ObjectImpl() { }
#endif

protected:
	const DL_ElgamalLikeSignatureAlgorithm<Element> & GetSignatureAlgorithm() const
		{return Singleton<CPP_TYPENAME SCHEME_OPTIONS::SignatureAlgorithm>().Ref();}
	const DL_KeyAgreementAlgorithm<Element> & GetKeyAgreementAlgorithm() const
		{return Singleton<CPP_TYPENAME SCHEME_OPTIONS::KeyAgreementAlgorithm>().Ref();}
	const DL_KeyDerivationAlgorithm<Element> & GetKeyDerivationAlgorithm() const
		{return Singleton<CPP_TYPENAME SCHEME_OPTIONS::KeyDerivationAlgorithm>().Ref();}
	const DL_SymmetricEncryptionAlgorithm & GetSymmetricEncryptionAlgorithm() const
		{return Singleton<CPP_TYPENAME SCHEME_OPTIONS::SymmetricEncryptionAlgorithm>().Ref();}
	HashIdentifier GetHashIdentifier() const
		{return HashIdentifier();}
	const PK_SignatureMessageEncodingMethod & GetMessageEncodingInterface() const
		{return Singleton<CPP_TYPENAME SCHEME_OPTIONS::MessageEncodingMethod>().Ref();}
};



template <class SCHEME_OPTIONS>
class DL_SignerImpl : public DL_ObjectImpl<DL_SignerBase<typename SCHEME_OPTIONS::Element>, SCHEME_OPTIONS, typename SCHEME_OPTIONS::PrivateKey>
{
public:
	PK_MessageAccumulator * NewSignatureAccumulator(RandomNumberGenerator &rng) const
	{
		member_ptr<PK_MessageAccumulatorBase> p(new PK_MessageAccumulatorImpl<CPP_TYPENAME SCHEME_OPTIONS::HashFunction>);
		this->RestartMessageAccumulator(rng, *p);
		return p.release();
	}
};



template <class SCHEME_OPTIONS>
class DL_VerifierImpl : public DL_ObjectImpl<DL_VerifierBase<typename SCHEME_OPTIONS::Element>, SCHEME_OPTIONS, typename SCHEME_OPTIONS::PublicKey>
{
public:
	PK_MessageAccumulator * NewVerificationAccumulator() const
	{
		return new PK_MessageAccumulatorImpl<CPP_TYPENAME SCHEME_OPTIONS::HashFunction>;
	}
};



template <class SCHEME_OPTIONS>
class DL_EncryptorImpl : public DL_ObjectImpl<DL_EncryptorBase<typename SCHEME_OPTIONS::Element>, SCHEME_OPTIONS, typename SCHEME_OPTIONS::PublicKey>
{
};



template <class SCHEME_OPTIONS>
class DL_DecryptorImpl : public DL_ObjectImpl<DL_DecryptorBase<typename SCHEME_OPTIONS::Element>, SCHEME_OPTIONS, typename SCHEME_OPTIONS::PrivateKey>
{
};





template <class T>
class CRYPTOPP_NO_VTABLE DL_SimpleKeyAgreementDomainBase : public SimpleKeyAgreementDomain
{
public:
	typedef T Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_SimpleKeyAgreementDomainBase() { }
#endif

	CryptoParameters & AccessCryptoParameters() {return AccessAbstractGroupParameters();}
	unsigned int AgreedValueLength() const {return GetAbstractGroupParameters().GetEncodedElementSize(false);}
	unsigned int PrivateKeyLength() const {return GetAbstractGroupParameters().GetSubgroupOrder().ByteCount();}
	unsigned int PublicKeyLength() const {return GetAbstractGroupParameters().GetEncodedElementSize(true);}

	void GeneratePrivateKey(RandomNumberGenerator &rng, byte *privateKey) const
	{
		Integer x(rng, Integer::One(), GetAbstractGroupParameters().GetMaxExponent());
		x.Encode(privateKey, PrivateKeyLength());
	}

	void GeneratePublicKey(RandomNumberGenerator &rng, const byte *privateKey, byte *publicKey) const
	{
		CRYPTOPP_UNUSED(rng);
		const DL_GroupParameters<T> &params = GetAbstractGroupParameters();
		Integer x(privateKey, PrivateKeyLength());
		Element y = params.ExponentiateBase(x);
		params.EncodeElement(true, y, publicKey);
	}

	bool Agree(byte *agreedValue, const byte *privateKey, const byte *otherPublicKey, bool validateOtherPublicKey=true) const
	{
		try
		{
			const DL_GroupParameters<T> &params = GetAbstractGroupParameters();
			Integer x(privateKey, PrivateKeyLength());
			Element w = params.DecodeElement(otherPublicKey, validateOtherPublicKey);

			Element z = GetKeyAgreementAlgorithm().AgreeWithStaticPrivateKey(
				GetAbstractGroupParameters(), w, validateOtherPublicKey, x);
			params.EncodeElement(false, z, agreedValue);
		}
		catch (DL_BadElement &)
		{
			return false;
		}
		return true;
	}

	
	
	const Element &GetGenerator() const {return GetAbstractGroupParameters().GetSubgroupGenerator();}

protected:
	virtual const DL_KeyAgreementAlgorithm<Element> & GetKeyAgreementAlgorithm() const =0;
	virtual DL_GroupParameters<Element> & AccessAbstractGroupParameters() =0;
	const DL_GroupParameters<Element> & GetAbstractGroupParameters() const {return const_cast<DL_SimpleKeyAgreementDomainBase<Element> *>(this)->AccessAbstractGroupParameters();}
};





enum CofactorMultiplicationOption {
	
	NO_COFACTOR_MULTIPLICTION,
	
	
	
	COMPATIBLE_COFACTOR_MULTIPLICTION,
	
	
	
	INCOMPATIBLE_COFACTOR_MULTIPLICTION};

typedef EnumToType<CofactorMultiplicationOption, NO_COFACTOR_MULTIPLICTION> NoCofactorMultiplication;
typedef EnumToType<CofactorMultiplicationOption, COMPATIBLE_COFACTOR_MULTIPLICTION> CompatibleCofactorMultiplication;
typedef EnumToType<CofactorMultiplicationOption, INCOMPATIBLE_COFACTOR_MULTIPLICTION> IncompatibleCofactorMultiplication;


template <class ELEMENT, class COFACTOR_OPTION>
class DL_KeyAgreementAlgorithm_DH : public DL_KeyAgreementAlgorithm<ELEMENT>
{
public:
	typedef ELEMENT Element;

#ifndef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY_562
	virtual ~DL_KeyAgreementAlgorithm_DH() {}
#endif

	CRYPTOPP_CONSTEXPR static const char * CRYPTOPP_API StaticAlgorithmName()
		{return COFACTOR_OPTION::ToEnum() == INCOMPATIBLE_COFACTOR_MULTIPLICTION ? "DHC" : "DH";}

	Element AgreeWithEphemeralPrivateKey(const DL_GroupParameters<Element> &params, const DL_FixedBasePrecomputation<Element> &publicPrecomputation, const Integer &privateExponent) const
	{
		return publicPrecomputation.Exponentiate(params.GetGroupPrecomputation(),
			COFACTOR_OPTION::ToEnum() == INCOMPATIBLE_COFACTOR_MULTIPLICTION ? privateExponent*params.GetCofactor() : privateExponent);
	}

	Element AgreeWithStaticPrivateKey(const DL_GroupParameters<Element> &params, const Element &publicElement, bool validateOtherPublicKey, const Integer &privateExponent) const
	{
		if (COFACTOR_OPTION::ToEnum() == COMPATIBLE_COFACTOR_MULTIPLICTION)
		{
			const Integer &k = params.GetCofactor();
			return params.ExponentiateElement(publicElement,
				ModularArithmetic(params.GetSubgroupOrder()).Divide(privateExponent, k)*k);
		}
		else if (COFACTOR_OPTION::ToEnum() == INCOMPATIBLE_COFACTOR_MULTIPLICTION)
			return params.ExponentiateElement(publicElement, privateExponent*params.GetCofactor());
		else
		{
			CRYPTOPP_ASSERT(COFACTOR_OPTION::ToEnum() == NO_COFACTOR_MULTIPLICTION);

			if (!validateOtherPublicKey)
				return params.ExponentiateElement(publicElement, privateExponent);

			if (params.FastSubgroupCheckAvailable())
			{
				if (!params.ValidateElement(2, publicElement, NULL))
					throw DL_BadElement();
				return params.ExponentiateElement(publicElement, privateExponent);
			}
			else
			{
				const Integer e[2] = {params.GetSubgroupOrder(), privateExponent};
				Element r[2];
				params.SimultaneousExponentiate(r, publicElement, e, 2);
				if (!params.IsIdentity(r[0]))
					throw DL_BadElement();
				return r[1];
			}
		}
	}
};




template <class BASE>
class CRYPTOPP_NO_VTABLE PK_FinalTemplate : public BASE
{
public:
	PK_FinalTemplate() {}

	PK_FinalTemplate(const CryptoMaterial &key)
		{this->AccessKey().AssignFrom(key);}

	PK_FinalTemplate(BufferedTransformation &bt)
		{this->AccessKey().BERDecode(bt);}

	PK_FinalTemplate(const AsymmetricAlgorithm &algorithm)
		{this->AccessKey().AssignFrom(algorithm.GetMaterial());}

	PK_FinalTemplate(const Integer &v1)
		{this->AccessKey().Initialize(v1);}

#if (defined(_MSC_VER) && _MSC_VER < 1300)

	template <class T1, class T2>
	PK_FinalTemplate(T1 &v1, T2 &v2)
		{this->AccessKey().Initialize(v1, v2);}

	template <class T1, class T2, class T3>
	PK_FinalTemplate(T1 &v1, T2 &v2, T3 &v3)
		{this->AccessKey().Initialize(v1, v2, v3);}

	template <class T1, class T2, class T3, class T4>
	PK_FinalTemplate(T1 &v1, T2 &v2, T3 &v3, T4 &v4)
		{this->AccessKey().Initialize(v1, v2, v3, v4);}

	template <class T1, class T2, class T3, class T4, class T5>
	PK_FinalTemplate(T1 &v1, T2 &v2, T3 &v3, T4 &v4, T5 &v5)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5);}

	template <class T1, class T2, class T3, class T4, class T5, class T6>
	PK_FinalTemplate(T1 &v1, T2 &v2, T3 &v3, T4 &v4, T5 &v5, T6 &v6)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6);}

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
	PK_FinalTemplate(T1 &v1, T2 &v2, T3 &v3, T4 &v4, T5 &v5, T6 &v6, T7 &v7)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6, v7);}

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
	PK_FinalTemplate(T1 &v1, T2 &v2, T3 &v3, T4 &v4, T5 &v5, T6 &v6, T7 &v7, T8 &v8)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6, v7, v8);}

#else

	template <class T1, class T2>
	PK_FinalTemplate(const T1 &v1, const T2 &v2)
		{this->AccessKey().Initialize(v1, v2);}

	template <class T1, class T2, class T3>
	PK_FinalTemplate(const T1 &v1, const T2 &v2, const T3 &v3)
		{this->AccessKey().Initialize(v1, v2, v3);}

	template <class T1, class T2, class T3, class T4>
	PK_FinalTemplate(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4)
		{this->AccessKey().Initialize(v1, v2, v3, v4);}

	template <class T1, class T2, class T3, class T4, class T5>
	PK_FinalTemplate(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5);}

	template <class T1, class T2, class T3, class T4, class T5, class T6>
	PK_FinalTemplate(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6);}

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
	PK_FinalTemplate(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6, v7);}

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
	PK_FinalTemplate(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7, const T8 &v8)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6, v7, v8);}

	template <class T1, class T2>
	PK_FinalTemplate(T1 &v1, const T2 &v2)
		{this->AccessKey().Initialize(v1, v2);}

	template <class T1, class T2, class T3>
	PK_FinalTemplate(T1 &v1, const T2 &v2, const T3 &v3)
		{this->AccessKey().Initialize(v1, v2, v3);}

	template <class T1, class T2, class T3, class T4>
	PK_FinalTemplate(T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4)
		{this->AccessKey().Initialize(v1, v2, v3, v4);}

	template <class T1, class T2, class T3, class T4, class T5>
	PK_FinalTemplate(T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5);}

	template <class T1, class T2, class T3, class T4, class T5, class T6>
	PK_FinalTemplate(T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6);}

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7>
	PK_FinalTemplate(T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6, v7);}

	template <class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
	PK_FinalTemplate(T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4, const T5 &v5, const T6 &v6, const T7 &v7, const T8 &v8)
		{this->AccessKey().Initialize(v1, v2, v3, v4, v5, v6, v7, v8);}

#endif
};




struct EncryptionStandard {};




struct SignatureStandard {};





template <class STANDARD, class KEYS, class ALG_INFO>
class TF_ES;

template <class STANDARD, class KEYS, class ALG_INFO = TF_ES<STANDARD, KEYS, int> >
class TF_ES : public KEYS
{
	typedef typename STANDARD::EncryptionMessageEncodingMethod MessageEncodingMethod;

public:
	
	typedef STANDARD Standard;
	typedef TF_CryptoSchemeOptions<ALG_INFO, KEYS, MessageEncodingMethod> SchemeOptions;

	static std::string CRYPTOPP_API StaticAlgorithmName() {return std::string(KEYS::StaticAlgorithmName()) + "/" + MessageEncodingMethod::StaticAlgorithmName();}

	
	typedef PK_FinalTemplate<TF_DecryptorImpl<SchemeOptions> > Decryptor;
	
	typedef PK_FinalTemplate<TF_EncryptorImpl<SchemeOptions> > Encryptor;
};







template <class STANDARD, class H, class KEYS, class ALG_INFO>	
class TF_SS;

template <class STANDARD, class H, class KEYS, class ALG_INFO = TF_SS<STANDARD, H, KEYS, int> >	
class TF_SS : public KEYS
{
public:
	
	typedef STANDARD Standard;
	typedef typename Standard::SignatureMessageEncodingMethod MessageEncodingMethod;
	typedef TF_SignatureSchemeOptions<ALG_INFO, KEYS, MessageEncodingMethod, H> SchemeOptions;

	static std::string CRYPTOPP_API StaticAlgorithmName() {return std::string(KEYS::StaticAlgorithmName()) + "/" + MessageEncodingMethod::StaticAlgorithmName() + "(" + H::StaticAlgorithmName() + ")";}

	
	typedef PK_FinalTemplate<TF_SignerImpl<SchemeOptions> > Signer;
	
	typedef PK_FinalTemplate<TF_VerifierImpl<SchemeOptions> > Verifier;
};








template <class KEYS, class SA, class MEM, class H, class ALG_INFO>
class DL_SS;

template <class KEYS, class SA, class MEM, class H, class ALG_INFO = DL_SS<KEYS, SA, MEM, H, int> >
class DL_SS : public KEYS
{
	typedef DL_SignatureSchemeOptions<ALG_INFO, KEYS, SA, MEM, H> SchemeOptions;

public:
	static std::string StaticAlgorithmName() {return SA::StaticAlgorithmName() + std::string("/EMSA1(") + H::StaticAlgorithmName() + ")";}

	
	typedef PK_FinalTemplate<DL_SignerImpl<SchemeOptions> > Signer;
	
	typedef PK_FinalTemplate<DL_VerifierImpl<SchemeOptions> > Verifier;
};







template <class KEYS, class AA, class DA, class EA, class ALG_INFO>
class DL_ES : public KEYS
{
	typedef DL_CryptoSchemeOptions<ALG_INFO, KEYS, AA, DA, EA> SchemeOptions;

public:
	
	typedef PK_FinalTemplate<DL_DecryptorImpl<SchemeOptions> > Decryptor;
	
	typedef PK_FinalTemplate<DL_EncryptorImpl<SchemeOptions> > Encryptor;
};

NAMESPACE_END

#if CRYPTOPP_MSC_VERSION
# pragma warning(pop)
#endif

#endif
