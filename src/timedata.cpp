



#include "timedata.h"

#include "netbase.h"
#include "sync.h"
#include "ui_interface.h"
#include "util.h"
#include "utilstrencodings.h"

#include <boost/foreach.hpp>

using namespace std;

static CCriticalSection cs_nTimeOffset;
static int64_t nTimeOffset = 0;

/**
 * "Never go to sea with two chronometers; take one or three."
 * Our three time sources are:
 *  - System clock
 *  - Median of other nodes clocks
 *  - The user (asking the user to fix the system clock if the first two disagree)
 */
int64_t GetTimeOffset()
{
    LOCK(cs_nTimeOffset);
    return nTimeOffset;
}

int64_t GetAdjustedTime()
{
    return GetTime() + GetTimeOffset();
}

static int64_t abs64(int64_t n)
{
    return (n >= 0 ? n : -n);
}

void AddTimeData(const CNetAddr& ip, int64_t nTime)
{
    int64_t nOffsetSample = nTime - GetTime();

    LOCK(cs_nTimeOffset);
    
    static set<CNetAddr> setKnown;
    if (!setKnown.insert(ip).second)
        return;

    
    static CMedianFilter<int64_t> vTimeOffsets(200, 0);
    vTimeOffsets.input(nOffsetSample);
    LogPrintf("Added time data, samples %d, offset %+d (%+d minutes)\n", vTimeOffsets.size(), nOffsetSample, nOffsetSample / 60);

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    if (vTimeOffsets.size() >= 5 && vTimeOffsets.size() % 2 == 1) {
        int64_t nMedian = vTimeOffsets.median();
        std::vector<int64_t> vSorted = vTimeOffsets.sorted();
        
        if (abs64(nMedian) < 70 * 60) {
            nTimeOffset = nMedian;
        } else {
            nTimeOffset = 0;

            static bool fDone;
            if (!fDone) {
                
                bool fMatch = false;
                BOOST_FOREACH (int64_t nOffset, vSorted)
                    if (nOffset != 0 && abs64(nOffset) < 5 * 60)
                        fMatch = true;

                if (!fMatch) {
                    fDone = true;
                    string strMessage = _("Warning: Please check that your computer's date and time are correct! If your clock is wrong TESRA Core will not work properly.");
                    strMiscWarning = strMessage;
                    LogPrintf("*** %s\n", strMessage);
                    uiInterface.ThreadSafeMessageBox(strMessage, "", CClientUIInterface::MSG_WARNING);
                }
            }
        }
        if (fDebug) {
            BOOST_FOREACH (int64_t n, vSorted)
                LogPrintf("%+d  ", n);
            LogPrintf("|  ");
        }
        LogPrintf("nTimeOffset = %+d  (%+d minutes)\n", nTimeOffset, nTimeOffset / 60);
    }
}
