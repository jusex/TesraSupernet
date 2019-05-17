




#ifndef CRYPTOPP_BASE64_H
#define CRYPTOPP_BASE64_H

#include "cryptlib.h"
#include "basecode.h"

NAMESPACE_BEGIN(CryptoPP)




class Base64Encoder : public SimpleProxyFilter
{
public:
	
	
	
	
	
	
	
	Base64Encoder(BufferedTransformation *attachment = NULL, bool insertLineBreaks = true, int maxLineLength = 72)
		: SimpleProxyFilter(new BaseN_Encoder(new Grouper), attachment)
	{
		IsolatedInitialize(MakeParameters(Name::InsertLineBreaks(), insertLineBreaks)(Name::MaxLineLength(), maxLineLength));
	}

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	void IsolatedInitialize(const NameValuePairs &parameters);
};




class Base64Decoder : public BaseN_Decoder
{
public:
	
	
	
	Base64Decoder(BufferedTransformation *attachment = NULL)
		: BaseN_Decoder(GetDecodingLookupArray(), 6, attachment) {}

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	void IsolatedInitialize(const NameValuePairs &parameters);

private:
	
	
	static const int * CRYPTOPP_API GetDecodingLookupArray();
};





class Base64URLEncoder : public SimpleProxyFilter
{
public:
	
	
	
	
	
	
	
	
	
	
	
	
	Base64URLEncoder(BufferedTransformation *attachment = NULL, bool insertLineBreaks = false, int maxLineLength = -1)
		: SimpleProxyFilter(new BaseN_Encoder(new Grouper), attachment)
	{
		CRYPTOPP_UNUSED(insertLineBreaks), CRYPTOPP_UNUSED(maxLineLength);
		IsolatedInitialize(MakeParameters(Name::InsertLineBreaks(), false)(Name::MaxLineLength(), -1)(Name::Pad(),false));
	}

	
	
	
	
	
	
	
	
	
	void IsolatedInitialize(const NameValuePairs &parameters);
};





class Base64URLDecoder : public BaseN_Decoder
{
public:
	
	
	
	
	Base64URLDecoder(BufferedTransformation *attachment = NULL)
		: BaseN_Decoder(GetDecodingLookupArray(), 6, attachment) {}

	
	
	
	
	
	
	
	void IsolatedInitialize(const NameValuePairs &parameters);

private:
	
	
	static const int * CRYPTOPP_API GetDecodingLookupArray();
};

NAMESPACE_END

#endif
