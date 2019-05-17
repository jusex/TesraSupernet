






#include "chainparamsbase.h"
#include "clientversion.h"
#include "rpcclient.h"
#include "rpcprotocol.h"
#include "util.h"
#include "utilstrencodings.h"

#include <boost/filesystem/operations.hpp>

#include <univalue.h>

#define _(x) std::string(x) 

using namespace std;
using namespace boost;
using namespace boost::asio;

std::string HelpMessageCli()
{
    string strUsage;
    strUsage += HelpMessageGroup(_("Options:"));
    strUsage += HelpMessageOpt("-?", _("This help message"));
    strUsage += HelpMessageOpt("-conf=<file>", strprintf(_("Specify configuration file (default: %s)"), "tesra.conf"));
    strUsage += HelpMessageOpt("-datadir=<dir>", _("Specify data directory"));
    strUsage += HelpMessageOpt("-testnet", _("Use the test network"));
    strUsage += HelpMessageOpt("-regtest", _("Enter regression test mode, which uses a special chain in which blocks can be "
                                             "solved instantly. This is intended for regression testing tools and app development."));
    strUsage += HelpMessageOpt("-rpcconnect=<ip>", strprintf(_("Send commands to node running on <ip> (default: %s)"), "127.0.0.1"));
    strUsage += HelpMessageOpt("-rpcport=<port>", strprintf(_("Connect to JSON-RPC on <port> (default: %u or testnet: %u)"), 58803, 58805));
    strUsage += HelpMessageOpt("-rpcwait", _("Wait for RPC server to start"));
    strUsage += HelpMessageOpt("-rpcuser=<user>", _("Username for JSON-RPC connections"));
    strUsage += HelpMessageOpt("-rpcpassword=<pw>", _("Password for JSON-RPC connections"));

    strUsage += HelpMessageGroup(_("SSL options: (see the Bitcoin Wiki for SSL setup instructions)"));
    strUsage += HelpMessageOpt("-rpcssl", _("Use OpenSSL (https) for JSON-RPC connections"));

    return strUsage;
}










class CConnectionFailed : public std::runtime_error
{
public:
    explicit inline CConnectionFailed(const std::string& msg) : std::runtime_error(msg)
    {
    }
};

static bool AppInitRPC(int argc, char* argv[])
{
    
    
    
    ParseParameters(argc, argv);
    if (argc < 2 || mapArgs.count("-?") || mapArgs.count("-help") || mapArgs.count("-version")) {
        std::string strUsage = _("Tesra Core RPC client version") + " " + FormatFullVersion() + "\n";
        if (!mapArgs.count("-version")) {
            strUsage += "\n" + _("Usage:") + "\n" +
                        "  tesra-cli [options] <command> [params]  " + _("Send command to Tesra Core") + "\n" +
                        "  tesra-cli [options] help                " + _("List commands") + "\n" +
                        "  tesra-cli [options] help <command>      " + _("Get help for a command") + "\n";

            strUsage += "\n" + HelpMessageCli();
        }

        fprintf(stdout, "%s", strUsage.c_str());
        return false;
    }
    if (!boost::filesystem::is_directory(GetDataDir(false))) {
        fprintf(stderr, "Error: Specified data directory \"%s\" does not exist.\n", mapArgs["-datadir"].c_str());
        return false;
    }
    try {
        ReadConfigFile(mapArgs, mapMultiArgs);
    } catch (std::exception& e) {
        fprintf(stderr, "Error reading configuration file: %s\n", e.what());
        return false;
    }
    
    if (!SelectBaseParamsFromCommandLine()) {
        fprintf(stderr, "Error: Invalid combination of -regtest and -testnet.\n");
        return false;
    }
    return true;
}

UniValue CallRPC(const string& strMethod, const UniValue& params)
{
    
    bool fUseSSL = GetBoolArg("-rpcssl", false);
    asio::io_service io_service;
    ssl::context context(io_service, ssl::context::sslv23);
    context.set_options(ssl::context::no_sslv2 | ssl::context::no_sslv3);
    asio::ssl::stream<asio::ip::tcp::socket> sslStream(io_service, context);
    SSLIOStreamDevice<asio::ip::tcp> d(sslStream, fUseSSL);
    iostreams::stream<SSLIOStreamDevice<asio::ip::tcp> > stream(d);

    const bool fConnected = d.connect(GetArg("-rpcconnect", "127.0.0.1"), GetArg("-rpcport", itostr(BaseParams().RPCPort())));
    if (!fConnected)
        throw CConnectionFailed("couldn't connect to server");

    
    std::string strRPCUserColonPass;
    if (mapArgs["-rpcpassword"] == "") {
        
        if (!GetAuthCookie(&strRPCUserColonPass)) {
            throw runtime_error(strprintf(
                _("You must set rpcpassword=<password> in the configuration file:\n%s\n"
                  "If the file does not exist, create it with owner-readable-only file permissions."),
                    GetConfigFile().string().c_str()));

        }
    } else {
        strRPCUserColonPass = mapArgs["-rpcuser"] + ":" + mapArgs["-rpcpassword"];
    }

    
    map<string, string> mapRequestHeaders;
    mapRequestHeaders["Authorization"] = string("Basic ") + EncodeBase64(strRPCUserColonPass);

    
    string strRequest = JSONRPCRequest(strMethod, params, 1);
    string strPost = HTTPPost(strRequest, mapRequestHeaders);
    stream << strPost << std::flush;

    
    int nProto = 0;
    int nStatus = ReadHTTPStatus(stream, nProto);

    
    map<string, string> mapHeaders;
    string strReply;
    ReadHTTPMessage(stream, mapHeaders, strReply, nProto, std::numeric_limits<size_t>::max());

    if (nStatus == HTTP_UNAUTHORIZED)
        throw runtime_error("incorrect rpcuser or rpcpassword (authorization failed)");
    else if (nStatus >= 400 && nStatus != HTTP_BAD_REQUEST && nStatus != HTTP_NOT_FOUND && nStatus != HTTP_INTERNAL_SERVER_ERROR)
        throw runtime_error(strprintf("server returned HTTP error %d", nStatus));
    else if (strReply.empty())
        throw runtime_error("no response from server");

    
    UniValue valReply(UniValue::VSTR);
    if (!valReply.read(strReply))
        throw runtime_error("couldn't parse reply from server");
    const UniValue& reply = valReply.get_obj();
    if (reply.empty())
        throw runtime_error("expected reply to have result, error and id properties");

    return reply;
}

int CommandLineRPC(int argc, char* argv[])
{
    string strPrint;
    int nRet = 0;
    try {
        
        while (argc > 1 && IsSwitchChar(argv[1][0])) {
            argc--;
            argv++;
        }

        
        if (argc < 2)
            throw runtime_error("too few parameters");
        string strMethod = argv[1];

        
        std::vector<std::string> strParams(&argv[2], &argv[argc]);
        UniValue params = RPCConvertValues(strMethod, strParams);

        
        const bool fWait = GetBoolArg("-rpcwait", false);
        do {
            try {
                const UniValue reply = CallRPC(strMethod, params);

                
                const UniValue& result = find_value(reply, "result");
                const UniValue& error = find_value(reply, "error");

                if (!error.isNull()) {
                    
                    int code = error["code"].get_int();
                    if (fWait && code == RPC_IN_WARMUP)
                        throw CConnectionFailed("server in warmup");
                    strPrint = "error: " + error.write();
                    nRet = abs(code);
                } else {
                    
                    if (result.isNull())
                        strPrint = "";
                    else if (result.isStr())
                        strPrint = result.get_str();
                    else
                        strPrint = result.write(2);
                }
                
                break;
            } catch (const CConnectionFailed& e) {
                if (fWait)
                    MilliSleep(1000);
                else
                    throw;
            }
        } while (fWait);
    } catch (boost::thread_interrupted) {
        throw;
    } catch (std::exception& e) {
        strPrint = string("error: ") + e.what();
        nRet = EXIT_FAILURE;
    } catch (...) {
        PrintExceptionContinue(NULL, "CommandLineRPC()");
        throw;
    }

    if (strPrint != "") {
        fprintf((nRet == 0 ? stdout : stderr), "%s\n", strPrint.c_str());
    }
    return nRet;
}

int main(int argc, char* argv[])
{
    SetupEnvironment();

    try {
        if (!AppInitRPC(argc, argv))
            return EXIT_FAILURE;
    } catch (std::exception& e) {
        PrintExceptionContinue(&e, "AppInitRPC()");
        return EXIT_FAILURE;
    } catch (...) {
        PrintExceptionContinue(NULL, "AppInitRPC()");
        return EXIT_FAILURE;
    }

    int ret = EXIT_FAILURE;
    try {
        ret = CommandLineRPC(argc, argv);
    } catch (std::exception& e) {
        PrintExceptionContinue(&e, "CommandLineRPC()");
    } catch (...) {
        PrintExceptionContinue(NULL, "CommandLineRPC()");
    }
    return ret;
}
