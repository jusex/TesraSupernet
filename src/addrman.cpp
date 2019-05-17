



#include "addrman.h"

#include "hash.h"
#include "serialize.h"
#include "streams.h"

using namespace std;

int CAddrInfo::GetTriedBucket(const uint256& nKey) const
{
    uint64_t hash1 = (CHashWriter(SER_GETHASH, 0) << nKey << GetKey()).GetHash().GetLow64();
    uint64_t hash2 = (CHashWriter(SER_GETHASH, 0) << nKey << GetGroup() << (hash1 % ADDRMAN_TRIED_BUCKETS_PER_GROUP)).GetHash().GetLow64();
    return hash2 % ADDRMAN_TRIED_BUCKET_COUNT;
}

int CAddrInfo::GetNewBucket(const uint256& nKey, const CNetAddr& src) const
{
    std::vector<unsigned char> vchSourceGroupKey = src.GetGroup();
    uint64_t hash1 = (CHashWriter(SER_GETHASH, 0) << nKey << GetGroup() << vchSourceGroupKey).GetHash().GetLow64();
    uint64_t hash2 = (CHashWriter(SER_GETHASH, 0) << nKey << vchSourceGroupKey << (hash1 % ADDRMAN_NEW_BUCKETS_PER_SOURCE_GROUP)).GetHash().GetLow64();
    return hash2 % ADDRMAN_NEW_BUCKET_COUNT;
}

int CAddrInfo::GetBucketPosition(const uint256& nKey, bool fNew, int nBucket) const
{
    uint64_t hash1 = (CHashWriter(SER_GETHASH, 0) << nKey << (fNew ? 'N' : 'K') << nBucket << GetKey()).GetHash().GetLow64();
    return hash1 % ADDRMAN_BUCKET_SIZE;
}

bool CAddrInfo::IsTerrible(int64_t nNow) const
{
    if (nLastTry && nLastTry >= nNow - 60) 
        return false;

    if (nTime > nNow + 10 * 60) 
        return true;

    if (nTime == 0 || nNow - nTime > ADDRMAN_HORIZON_DAYS * 24 * 60 * 60) 
        return true;

    if (nLastSuccess == 0 && nAttempts >= ADDRMAN_RETRIES) 
        return true;

    if (nNow - nLastSuccess > ADDRMAN_MIN_FAIL_DAYS * 24 * 60 * 60 && nAttempts >= ADDRMAN_MAX_FAILURES) 
        return true;

    return false;
}

double CAddrInfo::GetChance(int64_t nNow) const
{
    double fChance = 1.0;

    int64_t nSinceLastSeen = nNow - nTime;
    int64_t nSinceLastTry = nNow - nLastTry;

    if (nSinceLastSeen < 0)
        nSinceLastSeen = 0;
    if (nSinceLastTry < 0)
        nSinceLastTry = 0;

    
    if (nSinceLastTry < 60 * 10)
        fChance *= 0.01;

    
    fChance *= pow(0.66, min(nAttempts, 8));

    return fChance;
}

CAddrInfo* CAddrMan::Find(const CNetAddr& addr, int* pnId)
{
    std::map<CNetAddr, int>::iterator it = mapAddr.find(addr);
    if (it == mapAddr.end())
        return NULL;
    if (pnId)
        *pnId = (*it).second;
    std::map<int, CAddrInfo>::iterator it2 = mapInfo.find((*it).second);
    if (it2 != mapInfo.end())
        return &(*it2).second;
    return NULL;
}

CAddrInfo* CAddrMan::Create(const CAddress& addr, const CNetAddr& addrSource, int* pnId)
{
    int nId = nIdCount++;
    mapInfo[nId] = CAddrInfo(addr, addrSource);
    mapAddr[addr] = nId;
    mapInfo[nId].nRandomPos = vRandom.size();
    vRandom.push_back(nId);
    if (pnId)
        *pnId = nId;
    return &mapInfo[nId];
}

void CAddrMan::SwapRandom(unsigned int nRndPos1, unsigned int nRndPos2)
{
    if (nRndPos1 == nRndPos2)
        return;

    assert(nRndPos1 < vRandom.size() && nRndPos2 < vRandom.size());

    int nId1 = vRandom[nRndPos1];
    int nId2 = vRandom[nRndPos2];

    assert(mapInfo.count(nId1) == 1);
    assert(mapInfo.count(nId2) == 1);

    mapInfo[nId1].nRandomPos = nRndPos2;
    mapInfo[nId2].nRandomPos = nRndPos1;

    vRandom[nRndPos1] = nId2;
    vRandom[nRndPos2] = nId1;
}

void CAddrMan::Delete(int nId)
{
    assert(mapInfo.count(nId) != 0);
    CAddrInfo& info = mapInfo[nId];
    assert(!info.fInTried);
    assert(info.nRefCount == 0);

    SwapRandom(info.nRandomPos, vRandom.size() - 1);
    vRandom.pop_back();
    mapAddr.erase(info);
    mapInfo.erase(nId);
    nNew--;
}

void CAddrMan::ClearNew(int nUBucket, int nUBucketPos)
{
    
    if (vvNew[nUBucket][nUBucketPos] != -1) {
        int nIdDelete = vvNew[nUBucket][nUBucketPos];
        CAddrInfo& infoDelete = mapInfo[nIdDelete];
        assert(infoDelete.nRefCount > 0);
        infoDelete.nRefCount--;
        vvNew[nUBucket][nUBucketPos] = -1;
        if (infoDelete.nRefCount == 0) {
            Delete(nIdDelete);
        }
    }
}

void CAddrMan::MakeTried(CAddrInfo& info, int nId)
{
    
    for (int bucket = 0; bucket < ADDRMAN_NEW_BUCKET_COUNT; bucket++) {
        int pos = info.GetBucketPosition(nKey, true, bucket);
        if (vvNew[bucket][pos] == nId) {
            vvNew[bucket][pos] = -1;
            info.nRefCount--;
        }
    }
    nNew--;

    assert(info.nRefCount == 0);

    
    int nKBucket = info.GetTriedBucket(nKey);
    int nKBucketPos = info.GetBucketPosition(nKey, false, nKBucket);

    
    if (vvTried[nKBucket][nKBucketPos] != -1) {
        
        int nIdEvict = vvTried[nKBucket][nKBucketPos];
        assert(mapInfo.count(nIdEvict) == 1);
        CAddrInfo& infoOld = mapInfo[nIdEvict];

        
        infoOld.fInTried = false;
        vvTried[nKBucket][nKBucketPos] = -1;
        nTried--;

        
        int nUBucket = infoOld.GetNewBucket(nKey);
        int nUBucketPos = infoOld.GetBucketPosition(nKey, true, nUBucket);
        ClearNew(nUBucket, nUBucketPos);
        assert(vvNew[nUBucket][nUBucketPos] == -1);

        
        infoOld.nRefCount = 1;
        vvNew[nUBucket][nUBucketPos] = nIdEvict;
        nNew++;
    }
    assert(vvTried[nKBucket][nKBucketPos] == -1);

    vvTried[nKBucket][nKBucketPos] = nId;
    nTried++;
    info.fInTried = true;
}

void CAddrMan::Good_(const CService& addr, int64_t nTime)
{
    int nId;
    CAddrInfo* pinfo = Find(addr, &nId);

    
    if (!pinfo)
        return;

    CAddrInfo& info = *pinfo;

    
    if (info != addr)
        return;

    
    info.nLastSuccess = nTime;
    info.nLastTry = nTime;
    info.nAttempts = 0;
    
    

    
    if (info.fInTried)
        return;

    
    int nRnd = GetRandInt(ADDRMAN_NEW_BUCKET_COUNT);
    int nUBucket = -1;
    for (unsigned int n = 0; n < ADDRMAN_NEW_BUCKET_COUNT; n++) {
        int nB = (n + nRnd) % ADDRMAN_NEW_BUCKET_COUNT;
        int nBpos = info.GetBucketPosition(nKey, true, nB);
        if (vvNew[nB][nBpos] == nId) {
            nUBucket = nB;
            break;
        }
    }

    
    
    if (nUBucket == -1)
        return;

    LogPrint("addrman", "Moving %s to tried\n", addr.ToString());

    
    MakeTried(info, nId);
}

bool CAddrMan::Add_(const CAddress& addr, const CNetAddr& source, int64_t nTimePenalty)
{
    if (!addr.IsRoutable())
        return false;

    bool fNew = false;
    int nId;
    CAddrInfo* pinfo = Find(addr, &nId);

    if (pinfo) {
        
        bool fCurrentlyOnline = (GetAdjustedTime() - addr.nTime < 24 * 60 * 60);
        int64_t nUpdateInterval = (fCurrentlyOnline ? 60 * 60 : 24 * 60 * 60);
        if (addr.nTime && (!pinfo->nTime || pinfo->nTime < addr.nTime - nUpdateInterval - nTimePenalty))
            pinfo->nTime = max((int64_t)0, addr.nTime - nTimePenalty);

        
        pinfo->nServices |= addr.nServices;

        
        if (!addr.nTime || (pinfo->nTime && addr.nTime <= pinfo->nTime))
            return false;

        
        if (pinfo->fInTried)
            return false;

        
        if (pinfo->nRefCount == ADDRMAN_NEW_BUCKETS_PER_ADDRESS)
            return false;

        
        int nFactor = 1;
        for (int n = 0; n < pinfo->nRefCount; n++)
            nFactor *= 2;
        if (nFactor > 1 && (GetRandInt(nFactor) != 0))
            return false;
    } else {
        pinfo = Create(addr, source, &nId);
        pinfo->nTime = max((int64_t)0, (int64_t)pinfo->nTime - nTimePenalty);
        nNew++;
        fNew = true;
    }

    int nUBucket = pinfo->GetNewBucket(nKey, source);
    int nUBucketPos = pinfo->GetBucketPosition(nKey, true, nUBucket);
    if (vvNew[nUBucket][nUBucketPos] != nId) {
        bool fInsert = vvNew[nUBucket][nUBucketPos] == -1;
        if (!fInsert) {
            CAddrInfo& infoExisting = mapInfo[vvNew[nUBucket][nUBucketPos]];
            if (infoExisting.IsTerrible() || (infoExisting.nRefCount > 1 && pinfo->nRefCount == 0)) {
                
                fInsert = true;
            }
        }
        if (fInsert) {
            ClearNew(nUBucket, nUBucketPos);
            pinfo->nRefCount++;
            vvNew[nUBucket][nUBucketPos] = nId;
        } else {
            if (pinfo->nRefCount == 0) {
                Delete(nId);
            }
        }
    }
    return fNew;
}

void CAddrMan::Attempt_(const CService& addr, int64_t nTime)
{
    CAddrInfo* pinfo = Find(addr);

    
    if (!pinfo)
        return;

    CAddrInfo& info = *pinfo;

    
    if (info != addr)
        return;

    
    info.nLastTry = nTime;
    info.nAttempts++;
}

CAddress CAddrMan::Select_()
{
    if (size() == 0)
        return CAddress();

    
    if (nTried > 0 && (nNew == 0 || GetRandInt(2) == 0)) {
        
        double fChanceFactor = 1.0;
        while (1) {
            int nKBucket = GetRandInt(ADDRMAN_TRIED_BUCKET_COUNT);
            int nKBucketPos = GetRandInt(ADDRMAN_BUCKET_SIZE);
            if (vvTried[nKBucket][nKBucketPos] == -1)
                continue;
            int nId = vvTried[nKBucket][nKBucketPos];
            assert(mapInfo.count(nId) == 1);
            CAddrInfo& info = mapInfo[nId];
            if (GetRandInt(1 << 30) < fChanceFactor * info.GetChance() * (1 << 30))
                return info;
            fChanceFactor *= 1.2;
        }
    } else {
        
        double fChanceFactor = 1.0;
        while (1) {
            int nUBucket = GetRandInt(ADDRMAN_NEW_BUCKET_COUNT);
            int nUBucketPos = GetRandInt(ADDRMAN_BUCKET_SIZE);
            if (vvNew[nUBucket][nUBucketPos] == -1)
                continue;
            int nId = vvNew[nUBucket][nUBucketPos];
            assert(mapInfo.count(nId) == 1);
            CAddrInfo& info = mapInfo[nId];
            if (GetRandInt(1 << 30) < fChanceFactor * info.GetChance() * (1 << 30))
                return info;
            fChanceFactor *= 1.2;
        }
    }
}

#ifdef DEBUG_ADDRMAN
int CAddrMan::Check_()
{
    std::set<int> setTried;
    std::map<int, int> mapNew;

    if (vRandom.size() != nTried + nNew)
        return -7;

    for (std::map<int, CAddrInfo>::iterator it = mapInfo.begin(); it != mapInfo.end(); it++) {
        int n = (*it).first;
        CAddrInfo& info = (*it).second;
        if (info.fInTried) {
            if (!info.nLastSuccess)
                return -1;
            if (info.nRefCount)
                return -2;
            setTried.insert(n);
        } else {
            if (info.nRefCount < 0 || info.nRefCount > ADDRMAN_NEW_BUCKETS_PER_ADDRESS)
                return -3;
            if (!info.nRefCount)
                return -4;
            mapNew[n] = info.nRefCount;
        }
        if (mapAddr[info] != n)
            return -5;
        if (info.nRandomPos < 0 || info.nRandomPos >= vRandom.size() || vRandom[info.nRandomPos] != n)
            return -14;
        if (info.nLastTry < 0)
            return -6;
        if (info.nLastSuccess < 0)
            return -8;
    }

    if (setTried.size() != nTried)
        return -9;
    if (mapNew.size() != nNew)
        return -10;

    for (int n = 0; n < ADDRMAN_TRIED_BUCKET_COUNT; n++) {
        for (int i = 0; i < ADDRMAN_BUCKET_SIZE; i++) {
            if (vvTried[n][i] != -1) {
                if (!setTried.count(vvTried[n][i]))
                    return -11;
                if (mapInfo[vvTried[n][i]].GetTriedBucket(nKey) != n)
                    return -17;
                if (mapInfo[vvTried[n][i]].GetBucketPosition(nKey, false, n) != i)
                    return -18;
                setTried.erase(vvTried[n][i]);
            }
        }
    }

    for (int n = 0; n < ADDRMAN_NEW_BUCKET_COUNT; n++) {
        for (int i = 0; i < ADDRMAN_BUCKET_SIZE; i++) {
            if (vvNew[n][i] != -1) {
                if (!mapNew.count(vvNew[n][i]))
                    return -12;
                if (mapInfo[vvNew[n][i]].GetBucketPosition(nKey, true, n) != i)
                    return -19;
                if (--mapNew[vvNew[n][i]] == 0)
                    mapNew.erase(vvNew[n][i]);
            }
        }
    }

    if (setTried.size())
        return -13;
    if (mapNew.size())
        return -15;
    if (nKey.IsNull())
        return -16;

    return 0;
}
#endif

void CAddrMan::GetAddr_(std::vector<CAddress>& vAddr)
{
    unsigned int nNodes = ADDRMAN_GETADDR_MAX_PCT * vRandom.size() / 100;
    if (nNodes > ADDRMAN_GETADDR_MAX)
        nNodes = ADDRMAN_GETADDR_MAX;

    
    for (unsigned int n = 0; n < vRandom.size(); n++) {
        if (vAddr.size() >= nNodes)
            break;

        int nRndPos = GetRandInt(vRandom.size() - n) + n;
        SwapRandom(n, nRndPos);
        assert(mapInfo.count(vRandom[n]) == 1);

        const CAddrInfo& ai = mapInfo[vRandom[n]];
        if (!ai.IsTerrible())
            vAddr.push_back(ai);
    }
}

void CAddrMan::Connected_(const CService& addr, int64_t nTime)
{
    CAddrInfo* pinfo = Find(addr);

    
    if (!pinfo)
        return;

    CAddrInfo& info = *pinfo;

    
    if (info != addr)
        return;

    
    int64_t nUpdateInterval = 20 * 60;
    if (nTime - info.nTime > nUpdateInterval)
        info.nTime = nTime;
}
