




#ifndef CRYPTOPP_BASE32_H
#define CRYPTOPP_BASE32_H

#include "cryptlib.h"
#include "basecode.h"

NAMESPACE_BEGIN(CryptoPP)




class Base32Encoder : public SimpleProxyFilter
{
public:
	
	
	
	
	
	
	
	
	
	Base32Encoder(BufferedTransformation *attachment = NULL, bool uppercase = true, int groupSize = 0, const std::string &separator = ":", const std::string &terminator = "")
		: SimpleProxyFilter(new BaseN_Encoder(new Grouper), attachment)
	{
		IsolatedInitialize(MakeParameters(Name::Uppercase(), uppercase)(Name::GroupSize(), groupSize)(Name::Separator(), ConstByteArrayParameter(separator))(Name::Terminator(), ConstByteArrayParameter(terminator)));
	}

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	void IsolatedInitialize(const NameValuePairs &parameters);
};




class Base32Decoder : public BaseN_Decoder
{
public:
	
	
	
	Base32Decoder(BufferedTransformation *attachment = NULL)
		: BaseN_Decoder(GetDefaultDecodingLookupArray(), 5, attachment) {}

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	void IsolatedInitialize(const NameValuePairs &parameters);

private:
	
	
	static const int * CRYPTOPP_API GetDefaultDecodingLookupArray();
};

NAMESPACE_END

#endif
