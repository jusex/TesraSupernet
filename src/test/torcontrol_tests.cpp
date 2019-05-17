



#include "torcontrol.cpp"

#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE(torcontrol_tests)

void CheckSplitTorReplyLine(std::string input, std::string command, std::string args)
{
    BOOST_TEST_MESSAGE(std::string("CheckSplitTorReplyLine(") + input + ")");
    auto ret = SplitTorReplyLine(input);
    BOOST_CHECK_EQUAL(ret.first, command);
    BOOST_CHECK_EQUAL(ret.second, args);
}

BOOST_AUTO_TEST_CASE(util_SplitTorReplyLine)
{
    
    CheckSplitTorReplyLine(
        "PROTOCOLINFO ULOERSION",
        "PROTOCOLINFO", "ULOERSION");
    CheckSplitTorReplyLine(
        "AUTH METHODS=COOKIE,SAFECOOKIE COOKIEFILE=\"/home/x/.tor/control_auth_cookie\"",
        "AUTH", "METHODS=COOKIE,SAFECOOKIE COOKIEFILE=\"/home/x/.tor/control_auth_cookie\"");
    CheckSplitTorReplyLine(
        "AUTH METHODS=NULL",
        "AUTH", "METHODS=NULL");
    CheckSplitTorReplyLine(
        "AUTH METHODS=HASHEDPASSWORD",
        "AUTH", "METHODS=HASHEDPASSWORD");
    CheckSplitTorReplyLine(
        "VERSION Tor=\"0.2.9.8 (git-a0df013ea241b026)\"",
        "VERSION", "Tor=\"0.2.9.8 (git-a0df013ea241b026)\"");
    CheckSplitTorReplyLine(
        "AUTHCHALLENGE SERVERHASH=aaaa SERVERNONCE=bbbb",
        "AUTHCHALLENGE", "SERVERHASH=aaaa SERVERNONCE=bbbb");

    
    CheckSplitTorReplyLine("COMMAND", "COMMAND", "");
    CheckSplitTorReplyLine("COMMAND SOME  ARGS", "COMMAND", "SOME  ARGS");

    
    
    
    CheckSplitTorReplyLine("COMMAND  ARGS", "COMMAND", " ARGS");
    CheckSplitTorReplyLine("COMMAND   EVEN+more  ARGS", "COMMAND", "  EVEN+more  ARGS");
}

void CheckParseTorReplyMapping(std::string input, std::map<std::string,std::string> expected)
{
    BOOST_TEST_MESSAGE(std::string("CheckParseTorReplyMapping(") + input + ")");
    auto ret = ParseTorReplyMapping(input);
    BOOST_CHECK_EQUAL(ret.size(), expected.size());
    auto r_it = ret.begin();
    auto e_it = expected.begin();
    while (r_it != ret.end() && e_it != expected.end()) {
        BOOST_CHECK_EQUAL(r_it->first, e_it->first);
        BOOST_CHECK_EQUAL(r_it->second, e_it->second);
        r_it++;
        e_it++;
    }
}

BOOST_AUTO_TEST_CASE(util_ParseTorReplyMapping)
{
    
    CheckParseTorReplyMapping(
        "METHODS=COOKIE,SAFECOOKIE COOKIEFILE=\"/home/x/.tor/control_auth_cookie\"", {
            {"METHODS", "COOKIE,SAFECOOKIE"},
            {"COOKIEFILE", "/home/x/.tor/control_auth_cookie"},
        });
    CheckParseTorReplyMapping(
        "METHODS=NULL", {
            {"METHODS", "NULL"},
        });
    CheckParseTorReplyMapping(
        "METHODS=HASHEDPASSWORD", {
            {"METHODS", "HASHEDPASSWORD"},
        });
    CheckParseTorReplyMapping(
        "Tor=\"0.2.9.8 (git-a0df013ea241b026)\"", {
            {"Tor", "0.2.9.8 (git-a0df013ea241b026)"},
        });
    CheckParseTorReplyMapping(
        "SERVERHASH=aaaa SERVERNONCE=bbbb", {
            {"SERVERHASH", "aaaa"},
            {"SERVERNONCE", "bbbb"},
        });
    CheckParseTorReplyMapping(
        "ServiceID=exampleonion1234", {
            {"ServiceID", "exampleonion1234"},
        });
    CheckParseTorReplyMapping(
        "PrivateKey=RSA1024:BLOB", {
            {"PrivateKey", "RSA1024:BLOB"},
        });
    CheckParseTorReplyMapping(
        "ClientAuth=bob:BLOB", {
            {"ClientAuth", "bob:BLOB"},
        });

    
    CheckParseTorReplyMapping(
        "Foo=Bar=Baz Spam=Eggs", {
            {"Foo", "Bar=Baz"},
            {"Spam", "Eggs"},
        });
    CheckParseTorReplyMapping(
        "Foo=\"Bar=Baz\"", {
            {"Foo", "Bar=Baz"},
        });
    CheckParseTorReplyMapping(
        "Foo=\"Bar Baz\"", {
            {"Foo", "Bar Baz"},
        });

    
    CheckParseTorReplyMapping(
        "Foo=\"Bar\\ Baz\"", {
            {"Foo", "Bar Baz"},
        });
    CheckParseTorReplyMapping(
        "Foo=\"Bar\\Baz\"", {
            {"Foo", "BarBaz"},
        });
    CheckParseTorReplyMapping(
        "Foo=\"Bar\\@Baz\"", {
            {"Foo", "Bar@Baz"},
        });
    CheckParseTorReplyMapping(
        "Foo=\"Bar\\\"Baz\" Spam=\"\\\"Eggs\\\"\"", {
            {"Foo", "Bar\"Baz"},
            {"Spam", "\"Eggs\""},
        });
    CheckParseTorReplyMapping(
        "Foo=\"Bar\\\\Baz\"", {
            {"Foo", "Bar\\Baz"},
        });

    
    CheckParseTorReplyMapping(
        "Foo=\"Bar\\nBaz\\t\" Spam=\"\\rEggs\" Octals=\"\\1a\\11\\17\\18\\81\\377\\378\\400\\2222\" Final=Check", {
            {"Foo", "Bar\nBaz\t"},
            {"Spam", "\rEggs"},
            {"Octals", "\1a\11\17\1" "881\377\37" "8\40" "0\222" "2"},
            {"Final", "Check"},
        });
    CheckParseTorReplyMapping(
        "Valid=Mapping Escaped=\"Escape\\\\\"", {
            {"Valid", "Mapping"},
            {"Escaped", "Escape\\"},
        });
    CheckParseTorReplyMapping(
        "Valid=Mapping Bare=\"Escape\\\"", {});
    CheckParseTorReplyMapping(
        "OneOctal=\"OneEnd\\1\" TwoOctal=\"TwoEnd\\11\"", {
            {"OneOctal", "OneEnd\1"},
            {"TwoOctal", "TwoEnd\11"},
        });

    
    
    BOOST_TEST_MESSAGE(std::string("CheckParseTorReplyMapping(Null=\"\\0\")"));
    auto ret = ParseTorReplyMapping("Null=\"\\0\"");
    BOOST_CHECK_EQUAL(ret.size(), 1);
    auto r_it = ret.begin();
    BOOST_CHECK_EQUAL(r_it->first, "Null");
    BOOST_CHECK_EQUAL(r_it->second.size(), 1);
    BOOST_CHECK_EQUAL(r_it->second[0], '\0');

    
    
    
    
    CheckParseTorReplyMapping(
        "SOME=args,here MORE optional=arguments  here", {
            {"SOME", "args,here"},
        });

    
    
    
    
    
    
    
    CheckParseTorReplyMapping("ARGS", {});
    CheckParseTorReplyMapping("MORE ARGS", {});
    CheckParseTorReplyMapping("MORE  ARGS", {});
    CheckParseTorReplyMapping("EVEN more=ARGS", {});
    CheckParseTorReplyMapping("EVEN+more ARGS", {});
}

BOOST_AUTO_TEST_SUITE_END()
