




#ifndef CRYPTOPP_HEX_H
#define CRYPTOPP_HEX_H

#include "cryptlib.h"
#include "basecode.h"

NAMESPACE_BEGIN(CryptoPP)



class CRYPTOPP_DLL HexEncoder : public SimpleProxyFilter
{
public:
	
	
	
	
	
	
	HexEncoder(BufferedTransformation *attachment = NULL, bool uppercase = true, int groupSize = 0, const std::string &separator = ":", const std::string &terminator = "")
		: SimpleProxyFilter(new BaseN_Encoder(new Grouper), attachment)
	{
		IsolatedInitialize(MakeParameters(Name::Uppercase(), uppercase)(Name::GroupSize(), groupSize)(Name::Separator(), ConstByteArrayParameter(separator))(Name::Terminator(), ConstByteArrayParameter(terminator)));
	}

	void IsolatedInitialize(const NameValuePairs &parameters);
};



class CRYPTOPP_DLL HexDecoder : public BaseN_Decoder
{
public:
	
	
	HexDecoder(BufferedTransformation *attachment = NULL)
		: BaseN_Decoder(GetDefaultDecodingLookupArray(), 4, attachment) {}

	void IsolatedInitialize(const NameValuePairs &parameters);

private:
	static const int * CRYPTOPP_API GetDefaultDecodingLookupArray();
};

NAMESPACE_END

#endif
