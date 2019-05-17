




#ifndef CRYPTOPP_ARGNAMES_H
#define CRYPTOPP_ARGNAMES_H

#include "cryptlib.h"

NAMESPACE_BEGIN(CryptoPP)

DOCUMENTED_NAMESPACE_BEGIN(Name)

#define CRYPTOPP_DEFINE_NAME_STRING(name)	inline const char *name() {return #name;}

CRYPTOPP_DEFINE_NAME_STRING(ValueNames)			
CRYPTOPP_DEFINE_NAME_STRING(Version)			
CRYPTOPP_DEFINE_NAME_STRING(Seed)				
CRYPTOPP_DEFINE_NAME_STRING(Key)				
CRYPTOPP_DEFINE_NAME_STRING(IV)					
CRYPTOPP_DEFINE_NAME_STRING(StolenIV)			
CRYPTOPP_DEFINE_NAME_STRING(Rounds)				
CRYPTOPP_DEFINE_NAME_STRING(FeedbackSize)		
CRYPTOPP_DEFINE_NAME_STRING(WordSize)			
CRYPTOPP_DEFINE_NAME_STRING(BlockSize)			
CRYPTOPP_DEFINE_NAME_STRING(EffectiveKeyLength)	
CRYPTOPP_DEFINE_NAME_STRING(KeySize)			
CRYPTOPP_DEFINE_NAME_STRING(ModulusSize)		
CRYPTOPP_DEFINE_NAME_STRING(SubgroupOrderSize)	
CRYPTOPP_DEFINE_NAME_STRING(PrivateExponentSize)
CRYPTOPP_DEFINE_NAME_STRING(Modulus)			
CRYPTOPP_DEFINE_NAME_STRING(PublicExponent)		
CRYPTOPP_DEFINE_NAME_STRING(PrivateExponent)	
CRYPTOPP_DEFINE_NAME_STRING(PublicElement)		
CRYPTOPP_DEFINE_NAME_STRING(SubgroupOrder)		
CRYPTOPP_DEFINE_NAME_STRING(Cofactor)			
CRYPTOPP_DEFINE_NAME_STRING(SubgroupGenerator)	
CRYPTOPP_DEFINE_NAME_STRING(Curve)				
CRYPTOPP_DEFINE_NAME_STRING(GroupOID)			
CRYPTOPP_DEFINE_NAME_STRING(PointerToPrimeSelector)		
CRYPTOPP_DEFINE_NAME_STRING(Prime1)				
CRYPTOPP_DEFINE_NAME_STRING(Prime2)				
CRYPTOPP_DEFINE_NAME_STRING(ModPrime1PrivateExponent)	
CRYPTOPP_DEFINE_NAME_STRING(ModPrime2PrivateExponent)	
CRYPTOPP_DEFINE_NAME_STRING(MultiplicativeInverseOfPrime2ModPrime1)	
CRYPTOPP_DEFINE_NAME_STRING(QuadraticResidueModPrime1)	
CRYPTOPP_DEFINE_NAME_STRING(QuadraticResidueModPrime2)	
CRYPTOPP_DEFINE_NAME_STRING(PutMessage)			
CRYPTOPP_DEFINE_NAME_STRING(TruncatedDigestSize)	
CRYPTOPP_DEFINE_NAME_STRING(BlockPaddingScheme)	
CRYPTOPP_DEFINE_NAME_STRING(HashVerificationFilterFlags)		
CRYPTOPP_DEFINE_NAME_STRING(AuthenticatedDecryptionFilterFlags)	
CRYPTOPP_DEFINE_NAME_STRING(SignatureVerificationFilterFlags)	
CRYPTOPP_DEFINE_NAME_STRING(InputBuffer)		
CRYPTOPP_DEFINE_NAME_STRING(OutputBuffer)		
CRYPTOPP_DEFINE_NAME_STRING(InputFileName)		
CRYPTOPP_DEFINE_NAME_STRING(InputFileNameWide)	
CRYPTOPP_DEFINE_NAME_STRING(InputStreamPointer)	
CRYPTOPP_DEFINE_NAME_STRING(InputBinaryMode)	
CRYPTOPP_DEFINE_NAME_STRING(OutputFileName)		
CRYPTOPP_DEFINE_NAME_STRING(OutputFileNameWide)	
CRYPTOPP_DEFINE_NAME_STRING(OutputStreamPointer)	
CRYPTOPP_DEFINE_NAME_STRING(OutputBinaryMode)	
CRYPTOPP_DEFINE_NAME_STRING(EncodingParameters)	
CRYPTOPP_DEFINE_NAME_STRING(KeyDerivationParameters)	
CRYPTOPP_DEFINE_NAME_STRING(Separator)			
CRYPTOPP_DEFINE_NAME_STRING(Terminator)			
CRYPTOPP_DEFINE_NAME_STRING(Uppercase)			
CRYPTOPP_DEFINE_NAME_STRING(GroupSize)			
CRYPTOPP_DEFINE_NAME_STRING(Pad)				
CRYPTOPP_DEFINE_NAME_STRING(PaddingByte)		
CRYPTOPP_DEFINE_NAME_STRING(Log2Base)			
CRYPTOPP_DEFINE_NAME_STRING(EncodingLookupArray)	
CRYPTOPP_DEFINE_NAME_STRING(DecodingLookupArray)	
CRYPTOPP_DEFINE_NAME_STRING(InsertLineBreaks)	
CRYPTOPP_DEFINE_NAME_STRING(MaxLineLength)		
CRYPTOPP_DEFINE_NAME_STRING(DigestSize)			
CRYPTOPP_DEFINE_NAME_STRING(L1KeyLength)		
CRYPTOPP_DEFINE_NAME_STRING(TableSize)			
CRYPTOPP_DEFINE_NAME_STRING(Blinding)			
CRYPTOPP_DEFINE_NAME_STRING(DerivedKey)			
CRYPTOPP_DEFINE_NAME_STRING(DerivedKeyLength)	
CRYPTOPP_DEFINE_NAME_STRING(Personalization)	
CRYPTOPP_DEFINE_NAME_STRING(PersonalizationSize)	
CRYPTOPP_DEFINE_NAME_STRING(Salt)				
CRYPTOPP_DEFINE_NAME_STRING(Tweak)				
CRYPTOPP_DEFINE_NAME_STRING(SaltSize)			
CRYPTOPP_DEFINE_NAME_STRING(TreeMode)			
DOCUMENTED_NAMESPACE_END

NAMESPACE_END

#endif