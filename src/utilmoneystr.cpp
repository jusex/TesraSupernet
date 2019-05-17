




#include "utilmoneystr.h"

#include "primitives/transaction.h"
#include "tinyformat.h"
#include "utilstrencodings.h"

using namespace std;

string FormatMoney(const CAmount& n, bool fPlus)
{
    
    
    int64_t n_abs = (n > 0 ? n : -n);
    int64_t quotient = n_abs / COIN;
    int64_t remainder = n_abs % COIN;
    string str = strprintf("%d.%08d", quotient, remainder);

    
    int nTrim = 0;
    for (int i = str.size() - 1; (str[i] == '0' && isdigit(str[i - 2])); --i)
        ++nTrim;
    if (nTrim)
        str.erase(str.size() - nTrim, nTrim);

    if (n < 0)
        str.insert((unsigned int)0, 1, '-');
    else if (fPlus && n > 0)
        str.insert((unsigned int)0, 1, '+');
    return str;
}


bool ParseMoney(const string& str, CAmount& nRet)
{
    return ParseMoney(str.c_str(), nRet);
}

bool ParseMoney(const char* pszIn, CAmount& nRet)
{
    string strWhole;
    int64_t nUnits = 0;
    const char* p = pszIn;
    while (isspace(*p))
        p++;
    for (; *p; p++) {
        if (*p == '.') {
            p++;
            int64_t nMult = CENT * 10;
            while (isdigit(*p) && (nMult > 0)) {
                nUnits += nMult * (*p++ - '0');
                nMult /= 10;
            }
            break;
        }
        if (isspace(*p))
            break;
        if (!isdigit(*p))
            return false;
        strWhole.insert(strWhole.end(), *p);
    }
    for (; *p; p++)
        if (!isspace(*p))
            return false;
    if (strWhole.size() > 10) 
        return false;
    if (nUnits < 0 || nUnits > COIN)
        return false;
    int64_t nWhole = atoi64(strWhole);
    CAmount nValue = nWhole * COIN + nUnits;

    nRet = nValue;
    return true;
}
