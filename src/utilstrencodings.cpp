





#include "utilstrencodings.h"

#include "tinyformat.h"

#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <limits>

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/crypto.h> 
#include <openssl/evp.h>


using namespace std;

string SanitizeString(const string& str)
{
    /**
     * safeChars chosen to allow simple messages/URLs/email addresses, but avoid anything
     * even possibly remotely dangerous like & or >
     */
    static string safeChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890 .,;_-/:?@()");
    string strResult;
    for (std::string::size_type i = 0; i < str.size(); i++) {
        if (safeChars.find(str[i]) != std::string::npos)
            strResult.push_back(str[i]);
    }
    return strResult;
}

const signed char p_util_hexdigit[256] =
    {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1,
        -1, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

signed char HexDigit(char c)
{
    return p_util_hexdigit[(unsigned char)c];
}

bool IsHex(const string& str)
{
    for (std::string::const_iterator it(str.begin()); it != str.end(); ++it) {
        if (HexDigit(*it) < 0)
            return false;
    }
    return (str.size() > 0) && (str.size() % 2 == 0);
}

vector<unsigned char> ParseHex(const char* psz)
{
    
    vector<unsigned char> vch;
    while (true) {
        while (isspace(*psz))
            psz++;
        signed char c = HexDigit(*psz++);
        if (c == (signed char)-1)
            break;
        unsigned char n = (c << 4);
        c = HexDigit(*psz++);
        if (c == (signed char)-1)
            break;
        n |= c;
        vch.push_back(n);
    }
    return vch;
}

vector<unsigned char> ParseHex(const string& str)
{
    return ParseHex(str.c_str());
}

string EncodeBase64(const unsigned char* pch, size_t len)
{
    static const char* pbase64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    string strRet = "";
    strRet.reserve((len + 2) / 3 * 4);

    int mode = 0, left = 0;
    const unsigned char* pchEnd = pch + len;

    while (pch < pchEnd) {
        int enc = *(pch++);
        switch (mode) {
        case 0: 
            strRet += pbase64[enc >> 2];
            left = (enc & 3) << 4;
            mode = 1;
            break;

        case 1: 
            strRet += pbase64[left | (enc >> 4)];
            left = (enc & 15) << 2;
            mode = 2;
            break;

        case 2: 
            strRet += pbase64[left | (enc >> 6)];
            strRet += pbase64[enc & 63];
            mode = 0;
            break;
        }
    }

    if (mode) {
        strRet += pbase64[left];
        strRet += '=';
        if (mode == 1)
            strRet += '=';
    }

    return strRet;
}

string EncodeBase64(const string& str)
{
    return EncodeBase64((const unsigned char*)str.c_str(), str.size());
}

vector<unsigned char> DecodeBase64(const char* p, bool* pfInvalid)
{
    static const int decode64_table[256] =
        {
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1,
            -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
            29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
            49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    if (pfInvalid)
        *pfInvalid = false;

    vector<unsigned char> vchRet;
    vchRet.reserve(strlen(p) * 3 / 4);

    int mode = 0;
    int left = 0;

    while (1) {
        int dec = decode64_table[(unsigned char)*p];
        if (dec == -1) break;
        p++;
        switch (mode) {
        case 0: 
            left = dec;
            mode = 1;
            break;

        case 1: 
            vchRet.push_back((left << 2) | (dec >> 4));
            left = dec & 15;
            mode = 2;
            break;

        case 2: 
            vchRet.push_back((left << 4) | (dec >> 2));
            left = dec & 3;
            mode = 3;
            break;

        case 3: 
            vchRet.push_back((left << 6) | dec);
            mode = 0;
            break;
        }
    }

    if (pfInvalid)
        switch (mode) {
        case 0: 
            break;

        case 1: 
            *pfInvalid = true;
            break;

        case 2: 
            if (left || p[0] != '=' || p[1] != '=' || decode64_table[(unsigned char)p[2]] != -1)
                *pfInvalid = true;
            break;

        case 3: 
            if (left || p[0] != '=' || decode64_table[(unsigned char)p[1]] != -1)
                *pfInvalid = true;
            break;
        }

    return vchRet;
}

string DecodeBase64(const string& str)
{
    vector<unsigned char> vchRet = DecodeBase64(str.c_str());
    return (vchRet.size() == 0) ? string() : string((const char*)&vchRet[0], vchRet.size());
}


SecureString DecodeBase64Secure(const SecureString& input)
{
    SecureString output;

    
    BIO *b64, *mem;
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); 
    mem = BIO_new_mem_buf((void*)&input[0], input.size());
    BIO_push(b64, mem);

    
    if (input.size() % 4 != 0) {
        throw runtime_error("Input length should be a multiple of 4");
    }
    size_t nMaxLen = input.size() / 4 * 3; 
    output.resize(nMaxLen);

    
    size_t nLen;
    nLen = BIO_read(b64, (void*)&output[0], input.size());
    output.resize(nLen);

    
    BIO_free_all(b64);
    return output;
}


SecureString EncodeBase64Secure(const SecureString& input)
{
    
    BIO *b64, *mem;
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); 
    mem = BIO_new(BIO_s_mem());
    BIO_push(b64, mem);

    
    BIO_write(b64, &input[0], input.size());
    (void)BIO_flush(b64);

    
    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64, &bptr);
    SecureString output(bptr->data, bptr->length);

    
    OPENSSL_cleanse((void*)bptr->data, bptr->length);

    
    BIO_free_all(b64);
    return output;
}

string EncodeBase32(const unsigned char* pch, size_t len)
{
    static const char* pbase32 = "abcdefghijklmnopqrstuvwxyz234567";

    string strRet = "";
    strRet.reserve((len + 4) / 5 * 8);

    int mode = 0, left = 0;
    const unsigned char* pchEnd = pch + len;

    while (pch < pchEnd) {
        int enc = *(pch++);
        switch (mode) {
        case 0: 
            strRet += pbase32[enc >> 3];
            left = (enc & 7) << 2;
            mode = 1;
            break;

        case 1: 
            strRet += pbase32[left | (enc >> 6)];
            strRet += pbase32[(enc >> 1) & 31];
            left = (enc & 1) << 4;
            mode = 2;
            break;

        case 2: 
            strRet += pbase32[left | (enc >> 4)];
            left = (enc & 15) << 1;
            mode = 3;
            break;

        case 3: 
            strRet += pbase32[left | (enc >> 7)];
            strRet += pbase32[(enc >> 2) & 31];
            left = (enc & 3) << 3;
            mode = 4;
            break;

        case 4: 
            strRet += pbase32[left | (enc >> 5)];
            strRet += pbase32[enc & 31];
            mode = 0;
        }
    }

    static const int nPadding[5] = {0, 6, 4, 3, 1};
    if (mode) {
        strRet += pbase32[left];
        for (int n = 0; n < nPadding[mode]; n++)
            strRet += '=';
    }

    return strRet;
}

string EncodeBase32(const string& str)
{
    return EncodeBase32((const unsigned char*)str.c_str(), str.size());
}

vector<unsigned char> DecodeBase32(const char* p, bool* pfInvalid)
{
    static const int decode32_table[256] =
        {
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 0, 1, 2,
            3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
            23, 24, 25, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    if (pfInvalid)
        *pfInvalid = false;

    vector<unsigned char> vchRet;
    vchRet.reserve((strlen(p)) * 5 / 8);

    int mode = 0;
    int left = 0;

    while (1) {
        int dec = decode32_table[(unsigned char)*p];
        if (dec == -1) break;
        p++;
        switch (mode) {
        case 0: 
            left = dec;
            mode = 1;
            break;

        case 1: 
            vchRet.push_back((left << 3) | (dec >> 2));
            left = dec & 3;
            mode = 2;
            break;

        case 2: 
            left = left << 5 | dec;
            mode = 3;
            break;

        case 3: 
            vchRet.push_back((left << 1) | (dec >> 4));
            left = dec & 15;
            mode = 4;
            break;

        case 4: 
            vchRet.push_back((left << 4) | (dec >> 1));
            left = dec & 1;
            mode = 5;
            break;

        case 5: 
            left = left << 5 | dec;
            mode = 6;
            break;

        case 6: 
            vchRet.push_back((left << 2) | (dec >> 3));
            left = dec & 7;
            mode = 7;
            break;

        case 7: 
            vchRet.push_back((left << 5) | dec);
            mode = 0;
            break;
        }
    }

    if (pfInvalid)
        switch (mode) {
        case 0: 
            break;

        case 1: 
        case 3: 
        case 6: 
            *pfInvalid = true;
            break;

        case 2: 
            if (left || p[0] != '=' || p[1] != '=' || p[2] != '=' || p[3] != '=' || p[4] != '=' || p[5] != '=' || decode32_table[(unsigned char)p[6]] != -1)
                *pfInvalid = true;
            break;

        case 4: 
            if (left || p[0] != '=' || p[1] != '=' || p[2] != '=' || p[3] != '=' || decode32_table[(unsigned char)p[4]] != -1)
                *pfInvalid = true;
            break;

        case 5: 
            if (left || p[0] != '=' || p[1] != '=' || p[2] != '=' || decode32_table[(unsigned char)p[3]] != -1)
                *pfInvalid = true;
            break;

        case 7: 
            if (left || p[0] != '=' || decode32_table[(unsigned char)p[1]] != -1)
                *pfInvalid = true;
            break;
        }

    return vchRet;
}

string DecodeBase32(const string& str)
{
    vector<unsigned char> vchRet = DecodeBase32(str.c_str());
    return (vchRet.size() == 0) ? string() : string((const char*)&vchRet[0], vchRet.size());
}

static bool ParsePrechecks(const std::string& str)
{
    if (str.empty()) 
        return false;
    if (str.size() >= 1 && (isspace(str[0]) || isspace(str[str.size()-1]))) 
        return false;
    if (str.size() != strlen(str.c_str())) 
        return false;
    return true;
}

bool ParseInt32(const std::string& str, int32_t *out)
{
    if (!ParsePrechecks(str))
        return false;
    char *endp = NULL;
    errno = 0; 
    long int n = strtol(str.c_str(), &endp, 10);
    if(out) *out = (int32_t)n;
    
    
    
    return endp && *endp == 0 && !errno &&
        n >= std::numeric_limits<int32_t>::min() &&
        n <= std::numeric_limits<int32_t>::max();
}

bool ParseInt64(const std::string& str, int64_t *out)
{
    if (!ParsePrechecks(str))
        return false;
    char *endp = NULL;
    errno = 0; 
    long long int n = strtoll(str.c_str(), &endp, 10);
    if(out) *out = (int64_t)n;
    
    
    return endp && *endp == 0 && !errno &&
        n >= std::numeric_limits<int64_t>::min() &&
        n <= std::numeric_limits<int64_t>::max();
}

bool ParseDouble(const std::string& str, double *out)
{
    if (!ParsePrechecks(str))
        return false;
    if (str.size() >= 2 && str[0] == '0' && str[1] == 'x') 
        return false;
    std::istringstream text(str);
    text.imbue(std::locale::classic());
    double result;
    text >> result;
    if(out) *out = result;
    return text.eof() && !text.fail();
}

std::string FormatParagraph(const std::string in, size_t width, size_t indent)
{
    std::stringstream out;
    size_t col = 0;
    size_t ptr = 0;
    while (ptr < in.size()) {
        
        ptr = in.find_first_not_of(' ', ptr);
        if (ptr == std::string::npos)
            break;
        
        size_t endword = in.find_first_of(' ', ptr);
        if (endword == std::string::npos)
            endword = in.size();
        
        if (col > 0) {
            if ((col + endword - ptr) > width) {
                out << '\n';
                for (size_t i = 0; i < indent; ++i)
                    out << ' ';
                col = 0;
            } else
                out << ' ';
        }
        
        out << in.substr(ptr, endword - ptr);
        col += endword - ptr + 1;
        ptr = endword;
    }
    return out.str();
}

std::string i64tostr(int64_t n)
{
    return strprintf("%d", n);
}

std::string itostr(int n)
{
    return strprintf("%d", n);
}

int64_t atoi64(const char* psz)
{
#ifdef _MSC_VER
    return _atoi64(psz);
#else
    return strtoll(psz, NULL, 10);
#endif
}

int64_t atoi64(const std::string& str)
{
#ifdef _MSC_VER
    return _atoi64(str.c_str());
#else
    return strtoll(str.c_str(), NULL, 10);
#endif
}

int atoi(const std::string& str)
{
    return atoi(str.c_str());
}
