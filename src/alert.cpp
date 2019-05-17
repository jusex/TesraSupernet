




#include "alert.h"

#include "chainparams.h"
#include "clientversion.h"
#include "net.h"
#include "pubkey.h"
#include "timedata.h"
#include "ui_interface.h"
#include "util.h"

#include <algorithm>
#include <map>
#include <stdint.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/foreach.hpp>
#include <boost/thread.hpp>

using namespace std;

map<uint256, CAlert> mapAlerts;
CCriticalSection cs_mapAlerts;

void CUnsignedAlert::SetNull()
{
    nVersion = 1;
    nRelayUntil = 0;
    nExpiration = 0;
    nID = 0;
    nCancel = 0;
    setCancel.clear();
    nMinVer = 0;
    nMaxVer = 0;
    setSubVer.clear();
    nPriority = 0;

    strComment.clear();
    strStatusBar.clear();
    strReserved.clear();
}

std::string CUnsignedAlert::ToString() const
{
    std::string strSetCancel;
    for (auto& n: setCancel)
        strSetCancel += strprintf("%d ", n);
    std::string strSetSubVer;
    BOOST_FOREACH (std::string str, setSubVer)
        strSetSubVer += "\"" + str + "\" ";
    return strprintf(
        "CAlert(\n"
        "    nVersion     = %d\n"
        "    nRelayUntil  = %d\n"
        "    nExpiration  = %d\n"
        "    nID          = %d\n"
        "    nCancel      = %d\n"
        "    setCancel    = %s\n"
        "    nMinVer      = %d\n"
        "    nMaxVer      = %d\n"
        "    setSubVer    = %s\n"
        "    nPriority    = %d\n"
        "    strComment   = \"%s\"\n"
        "    strStatusBar = \"%s\"\n"
        ")\n",
        nVersion,
        nRelayUntil,
        nExpiration,
        nID,
        nCancel,
        strSetCancel,
        nMinVer,
        nMaxVer,
        strSetSubVer,
        nPriority,
        strComment,
        strStatusBar);
}

void CAlert::SetNull()
{
    CUnsignedAlert::SetNull();
    vchMsg.clear();
    vchSig.clear();
}

bool CAlert::IsNull() const
{
    return (nExpiration == 0);
}

uint256 CAlert::GetHash() const
{
    return Hash(this->vchMsg.begin(), this->vchMsg.end());
}

bool CAlert::IsInEffect() const
{
    return (GetAdjustedTime() < nExpiration);
}

bool CAlert::Cancels(const CAlert& alert) const
{
    if (!IsInEffect())
        return false; 
    return (alert.nID <= nCancel || setCancel.count(alert.nID));
}

bool CAlert::AppliesTo(int nVersion, std::string strSubVerIn) const
{
    
    return (IsInEffect() &&
            nMinVer <= nVersion && nVersion <= nMaxVer &&
            (setSubVer.empty() || setSubVer.count(strSubVerIn)));
}

bool CAlert::AppliesToMe() const
{
    return AppliesTo(PROTOCOL_VERSION, FormatSubVersion(CLIENT_NAME, CLIENT_VERSION, std::vector<std::string>()));
}

bool CAlert::RelayTo(CNode* pnode) const
{
    if (!IsInEffect())
        return false;
    
    if (pnode->nVersion == 0)
        return false;
    
    if (pnode->setKnown.insert(GetHash()).second) {
        if (AppliesTo(pnode->nVersion, pnode->strSubVer) ||
            AppliesToMe() ||
            GetAdjustedTime() < nRelayUntil) {
            pnode->PushMessage("alert", *this);
            return true;
        }
    }
    return false;
}

bool CAlert::CheckSignature() const
{
    CPubKey key(Params().AlertKey());
    if (!key.Verify(Hash(vchMsg.begin(), vchMsg.end()), vchSig))
        return error("CAlert::CheckSignature() : verify signature failed");

    
    CDataStream sMsg(vchMsg, SER_NETWORK, PROTOCOL_VERSION);
    sMsg >> *(CUnsignedAlert*)this;
    return true;
}

CAlert CAlert::getAlertByHash(const uint256& hash)
{
    CAlert retval;
    {
        LOCK(cs_mapAlerts);
        map<uint256, CAlert>::iterator mi = mapAlerts.find(hash);
        if (mi != mapAlerts.end())
            retval = mi->second;
    }
    return retval;
}

bool CAlert::ProcessAlert(bool fThread)
{
    if (!CheckSignature())
        return false;
    if (!IsInEffect())
        return false;

    
    
    
    
    
    
    
    int maxInt = std::numeric_limits<int>::max();
    if (nID == maxInt) {
        if (!(
                nExpiration == maxInt &&
                nCancel == (maxInt - 1) &&
                nMinVer == 0 &&
                nMaxVer == maxInt &&
                setSubVer.empty() &&
                nPriority == maxInt &&
                strStatusBar == "URGENT: Alert key compromised, upgrade required"))
            return false;
    }

    {
        LOCK(cs_mapAlerts);
        
        for (map<uint256, CAlert>::iterator mi = mapAlerts.begin(); mi != mapAlerts.end();) {
            const CAlert& alert = (*mi).second;
            if (Cancels(alert)) {
                LogPrint("alert", "cancelling alert %d\n", alert.nID);
                uiInterface.NotifyAlertChanged((*mi).first, CT_DELETED);
                mapAlerts.erase(mi++);
            } else if (!alert.IsInEffect()) {
                LogPrint("alert", "expiring alert %d\n", alert.nID);
                uiInterface.NotifyAlertChanged((*mi).first, CT_DELETED);
                mapAlerts.erase(mi++);
            } else
                mi++;
        }

        
        BOOST_FOREACH (PAIRTYPE(const uint256, CAlert) & item, mapAlerts) {
            const CAlert& alert = item.second;
            if (alert.Cancels(*this)) {
                LogPrint("alert", "alert already cancelled by %d\n", alert.nID);
                return false;
            }
        }

        
        mapAlerts.insert(make_pair(GetHash(), *this));
        
        if (AppliesToMe()) {
            uiInterface.NotifyAlertChanged(GetHash(), CT_NEW);
            Notify(strStatusBar, fThread);
        }
    }

    LogPrint("alert", "accepted alert %d, AppliesToMe()=%d\n", nID, AppliesToMe());
    return true;
}

void CAlert::Notify(const std::string& strMessage, bool fThread)
{
    std::string strCmd = GetArg("-alertnotify", "");
    if (strCmd.empty()) return;

    
    
    
    std::string singleQuote("'");
    std::string safeStatus = SanitizeString(strMessage);
    safeStatus = singleQuote + safeStatus + singleQuote;
    boost::replace_all(strCmd, "%s", safeStatus);

    if (fThread)
        boost::thread t(runCommand, strCmd); 
    else
        runCommand(strCmd);
}
