




#ifndef BITCOIN_RPCPROTOCOL_H
#define BITCOIN_RPCPROTOCOL_H

#include <list>
#include <map>
#include <stdint.h>
#include <string>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/filesystem.hpp>

#include <univalue.h>


enum HTTPStatusCode {
    HTTP_OK = 200,
    HTTP_BAD_REQUEST = 400,
    HTTP_UNAUTHORIZED = 401,
    HTTP_FORBIDDEN = 403,
    HTTP_NOT_FOUND = 404,
    HTTP_INTERNAL_SERVER_ERROR = 500,
    HTTP_SERVICE_UNAVAILABLE = 503,
};


enum RPCErrorCode {
    
    RPC_INVALID_REQUEST = -32600,
    RPC_METHOD_NOT_FOUND = -32601,
    RPC_INVALID_PARAMS = -32602,
    RPC_INTERNAL_ERROR = -32603,
    RPC_PARSE_ERROR = -32700,

    
    RPC_MISC_ERROR = -1,               
    RPC_FORBIDDEN_BY_SAFE_MODE = -2,   
    RPC_TYPE_ERROR = -3,               
    RPC_INVALID_ADDRESS_OR_KEY = -5,   
    RPC_OUT_OF_MEMORY = -7,            
    RPC_INVALID_PARAMETER = -8,        
    RPC_DATABASE_ERROR = -20,          
    RPC_DESERIALIZATION_ERROR = -22,   
    RPC_VERIFY_ERROR = -25,            
    RPC_VERIFY_REJECTED = -26,         
    RPC_VERIFY_ALREADY_IN_CHAIN = -27, 
    RPC_IN_WARMUP = -28,               

    
    RPC_TRANSACTION_ERROR = RPC_VERIFY_ERROR,
    RPC_TRANSACTION_REJECTED = RPC_VERIFY_REJECTED,
    RPC_TRANSACTION_ALREADY_IN_CHAIN = RPC_VERIFY_ALREADY_IN_CHAIN,

    
    RPC_CLIENT_NOT_CONNECTED = -9,        
    RPC_CLIENT_IN_INITIAL_DOWNLOAD = -10, 
    RPC_CLIENT_NODE_ALREADY_ADDED = -23,  
    RPC_CLIENT_NODE_NOT_ADDED = -24,      
    RPC_CLIENT_NODE_NOT_CONNECTED   = -29, 
    RPC_CLIENT_INVALID_IP_OR_SUBNET = -30, 

    
    RPC_WALLET_ERROR = -4,                 
    RPC_WALLET_INSUFFICIENT_FUNDS = -6,    
    RPC_WALLET_INVALID_ACCOUNT_NAME = -11, 
    RPC_WALLET_KEYPOOL_RAN_OUT = -12,      
    RPC_WALLET_UNLOCK_NEEDED = -13,        
    RPC_WALLET_PASSPHRASE_INCORRECT = -14, 
    RPC_WALLET_WRONG_ENC_STATE = -15,      
    RPC_WALLET_ENCRYPTION_FAILED = -16,    
    RPC_WALLET_ALREADY_UNLOCKED = -17,     
};

/**
 * IOStream device that speaks SSL but can also speak non-SSL
 */
template <typename Protocol>
class SSLIOStreamDevice : public boost::iostreams::device<boost::iostreams::bidirectional>
{
public:
    SSLIOStreamDevice(boost::asio::ssl::stream<typename Protocol::socket>& streamIn, bool fUseSSLIn) : stream(streamIn)
    {
        fUseSSL = fUseSSLIn;
        fNeedHandshake = fUseSSLIn;
    }

    void handshake(boost::asio::ssl::stream_base::handshake_type role)
    {
        if (!fNeedHandshake) return;
        fNeedHandshake = false;
        stream.handshake(role);
    }
    std::streamsize read(char* s, std::streamsize n)
    {
        handshake(boost::asio::ssl::stream_base::server); 
        if (fUseSSL) return stream.read_some(boost::asio::buffer(s, n));
        return stream.next_layer().read_some(boost::asio::buffer(s, n));
    }
    std::streamsize write(const char* s, std::streamsize n)
    {
        handshake(boost::asio::ssl::stream_base::client); 
        if (fUseSSL) return boost::asio::write(stream, boost::asio::buffer(s, n));
        return boost::asio::write(stream.next_layer(), boost::asio::buffer(s, n));
    }
    bool connect(const std::string& server, const std::string& port)
    {
        using namespace boost::asio::ip;
        tcp::resolver resolver(stream.get_io_service());
        tcp::resolver::iterator endpoint_iterator;
#if BOOST_VERSION >= 104300
        try {
#endif
            
            
            
            tcp::resolver::query query(server.c_str(), port.c_str());
            endpoint_iterator = resolver.resolve(query);
#if BOOST_VERSION >= 104300
        } catch (boost::system::system_error& e) {
            
            tcp::resolver::query query(server.c_str(), port.c_str(), resolver_query_base::flags());
            endpoint_iterator = resolver.resolve(query);
        }
#endif
        boost::system::error_code error = boost::asio::error::host_not_found;
        tcp::resolver::iterator end;
        while (error && endpoint_iterator != end) {
            stream.lowest_layer().close();
            stream.lowest_layer().connect(*endpoint_iterator++, error);
        }
        if (error)
            return false;
        return true;
    }

private:
    bool fNeedHandshake;
    bool fUseSSL;
    boost::asio::ssl::stream<typename Protocol::socket>& stream;
};

std::string HTTPPost(const std::string& strMsg, const std::map<std::string, std::string>& mapRequestHeaders);
std::string HTTPError(int nStatus, bool keepalive, bool headerOnly = false);
std::string HTTPReplyHeader(int nStatus, bool keepalive, size_t contentLength, const char* contentType = "application/json");
std::string HTTPReply(int nStatus, const std::string& strMsg, bool keepalive, bool headerOnly = false, const char* contentType = "application/json");
bool ReadHTTPRequestLine(std::basic_istream<char>& stream, int& proto, std::string& http_method, std::string& http_uri);
int ReadHTTPStatus(std::basic_istream<char>& stream, int& proto);
int ReadHTTPHeaders(std::basic_istream<char>& stream, std::map<std::string, std::string>& mapHeadersRet);
int ReadHTTPMessage(std::basic_istream<char>& stream, std::map<std::string, std::string>& mapHeadersRet, std::string& strMessageRet, int nProto, size_t max_size);
std::string JSONRPCRequest(const std::string& strMethod, const UniValue& params, const UniValue& id);
UniValue JSONRPCReplyObj(const UniValue& result, const UniValue& error, const UniValue& id);
std::string JSONRPCReply(const UniValue& result, const UniValue& error, const UniValue& id);
UniValue JSONRPCError(int code, const std::string& message);


boost::filesystem::path GetAuthCookieFile();

bool GenerateAuthCookie(std::string *cookie_out);

bool GetAuthCookie(std::string *cookie_out);

void DeleteAuthCookie();

#endif 
