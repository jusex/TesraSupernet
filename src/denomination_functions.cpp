/**
 * @file       denominations_functions.cpp
 *
 * @brief      Denomination functions for the Zerocoin library.
 *
 * @copyright  Copyright 2017 PIVX Developers
 * @license    This project is released under the MIT license.
 **/


#include "denomination_functions.h"

using namespace libzerocoin;




int getNumberOfCoinsUsed(
    const std::map<CoinDenomination, CAmount>& mapChange)
{
    int nChangeCount = 0;
    for (const auto& denom : zerocoinDenomList) {
        nChangeCount += mapChange.at(denom);
    }
    return nChangeCount;
}




CoinDenomination getMaxDenomHeld(
    const std::map<CoinDenomination, CAmount>& mapCoinsHeld)
{
    CoinDenomination maxDenom = ZQ_ERROR;
    for (auto& coin : reverse_iterate(zerocoinDenomList)) {
        if (mapCoinsHeld.at(coin)) {
            maxDenom = coin;
            break;
        }
    }
    return maxDenom;
}



std::map<CoinDenomination, CAmount> getSpendCoins(const CAmount nValueTarget,
    const std::map<CoinDenomination, CAmount> mapOfDenomsHeld)

{
    std::map<CoinDenomination, CAmount> mapUsed;
    CAmount nRemainingValue = nValueTarget;
    
    for (const auto& denom : zerocoinDenomList)
        mapUsed.insert(std::pair<CoinDenomination, CAmount>(denom, 0));

    
    
    for (auto& coin : reverse_iterate(zerocoinDenomList)) {
        CAmount nValue = ZerocoinDenominationToAmount(coin);
        do {
            if ((nRemainingValue >= nValue) && (mapUsed.at(coin) < mapOfDenomsHeld.at(coin))) {
                mapUsed.at(coin)++;
                nRemainingValue -= nValue;
            }
        } while ((nRemainingValue >= nValue) && (mapUsed.at(coin) < mapOfDenomsHeld.at(coin)));
    }
    return mapUsed;
}




std::map<CoinDenomination, CAmount> getChange(const CAmount nValueTarget)
{
    std::map<CoinDenomination, CAmount> mapChange;
    CAmount nRemainingValue = nValueTarget;
    
    for (const auto& denom : zerocoinDenomList)
        mapChange.insert(std::pair<CoinDenomination, CAmount>(denom, 0));

    
    
    for (auto& coin : reverse_iterate(zerocoinDenomList)) {
        CAmount nValue = ZerocoinDenominationToAmount(coin);
        do {
            if (nRemainingValue >= nValue) {
                mapChange.at(coin)++;
                nRemainingValue -= nValue;
            }
        } while (nRemainingValue >= nValue);
    }
    return mapChange;
}





bool getIdealSpends(
    const CAmount nValueTarget,
    const std::list<CZerocoinMint>& listMints,
    const std::map<CoinDenomination, CAmount> mapOfDenomsHeld,
    std::map<CoinDenomination, CAmount>& mapOfDenomsUsed)
{
    CAmount nRemainingValue = nValueTarget;
    
    for (const auto& denom : zerocoinDenomList)
        mapOfDenomsUsed.insert(std::pair<CoinDenomination, CAmount>(denom, 0));

    
    
    for (auto& coin : reverse_iterate(zerocoinDenomList)) {
        for (const CZerocoinMint mint : listMints) {
            if (mint.IsUsed()) continue;
            if (nRemainingValue >= ZerocoinDenominationToAmount(coin) && coin == mint.GetDenomination()) {
                mapOfDenomsUsed.at(coin)++;
                nRemainingValue -= mint.GetDenominationAsAmount();
            }
            if (nRemainingValue < ZerocoinDenominationToAmount(coin)) break;
        }
    }
    return (nRemainingValue == 0);
}




std::vector<CZerocoinMint> getSpends(
    const std::list<CZerocoinMint>& listMints,
    std::map<CoinDenomination, CAmount>& mapOfDenomsUsed,
    CAmount& nCoinsSpentValue)
{
    std::vector<CZerocoinMint> vSelectedMints;
    nCoinsSpentValue = 0;
    for (auto& coin : reverse_iterate(zerocoinDenomList)) {
        do {
            for (const CZerocoinMint mint : listMints) {
                if (mint.IsUsed()) continue;
                if (coin == mint.GetDenomination() && mapOfDenomsUsed.at(coin)) {
                    vSelectedMints.push_back(mint);
                    nCoinsSpentValue += ZerocoinDenominationToAmount(coin);
                    mapOfDenomsUsed.at(coin)--;
                }
            }
        } while (mapOfDenomsUsed.at(coin));
    }
    return vSelectedMints;
}



void listSpends(const std::vector<CZerocoinMint>& vSelectedMints)
{
    std::map<libzerocoin::CoinDenomination, int64_t> mapZerocoinSupply;
    for (auto& denom : libzerocoin::zerocoinDenomList)
        mapZerocoinSupply.insert(std::make_pair(denom, 0));

    for (const CZerocoinMint mint : vSelectedMints) {
        libzerocoin::CoinDenomination denom = mint.GetDenomination();
        mapZerocoinSupply.at(denom)++;
    }

    CAmount nTotal = 0;
    for (auto& denom : libzerocoin::zerocoinDenomList) {
        LogPrint("zero", "%s %d coins for denomination %d used\n", __func__, mapZerocoinSupply.at(denom), denom);
        nTotal += libzerocoin::ZerocoinDenominationToAmount(denom);
    }
    LogPrint("zero", "Total value of coins %d\n", nTotal);
}




CoinDenomination getDenomWithMostCoins(
    const std::map<CoinDenomination, CAmount>& mapOfDenomsUsed)
{
    CoinDenomination maxCoins = ZQ_ERROR;
    CAmount nMaxNumber = 0;
    for (const auto& denom : zerocoinDenomList) {
        CAmount amount = mapOfDenomsUsed.at(denom);
        if (amount > nMaxNumber) {
            nMaxNumber = amount;
            maxCoins = denom;
        }
    }
    return maxCoins;
}



CoinDenomination getNextHighestDenom(const CoinDenomination& this_denom)
{
    CoinDenomination nextValue = ZQ_ERROR;
    for (const auto& denom : zerocoinDenomList) {
        if (ZerocoinDenominationToAmount(denom) > ZerocoinDenominationToAmount(this_denom)) {
            nextValue = denom;
            break;
        }
    }
    return nextValue;
}




CoinDenomination getNextLowerDenomHeld(const CoinDenomination& this_denom,
    const std::map<CoinDenomination, CAmount>& mapCoinsHeld)
{
    CoinDenomination nextValue = ZQ_ERROR;
    for (auto& denom : reverse_iterate(zerocoinDenomList)) {
        if ((denom < this_denom) && (mapCoinsHeld.at(denom) != 0)) {
            nextValue = denom;
            break;
        }
    }
    return nextValue;
}

int minimizeChange(
    int nMaxNumberOfSpends,
    int nChangeCount,
    const CoinDenomination nextToMaxDenom,
    const CAmount nValueTarget,
    const std::map<CoinDenomination, CAmount>& mapOfDenomsHeld,
    std::map<CoinDenomination, CAmount>& mapOfDenomsUsed)
{
    
    
    CAmount nRemainingValue = nValueTarget;
    CAmount AmountUsed = 0;
    int nCoinCount = 0;

    
    std::map<CoinDenomination, CAmount> savedMapOfDenomsUsed = mapOfDenomsUsed;
    for (const auto& denom : zerocoinDenomList)
        mapOfDenomsUsed.at(denom) = 0;

    
    
    for (const auto& denom : reverse_iterate(zerocoinDenomList)) {
        if (denom <= nextToMaxDenom) {
            CAmount nValue = ZerocoinDenominationToAmount(denom);
            do {
                if ((nRemainingValue > nValue) && (mapOfDenomsUsed.at(denom) < mapOfDenomsHeld.at(denom))) {
                    mapOfDenomsUsed.at(denom)++;
                    nRemainingValue -= nValue;
                    AmountUsed += nValue;
                    nCoinCount++;
                }
            } while ((nRemainingValue > nValue) && (mapOfDenomsUsed.at(denom) < mapOfDenomsHeld.at(denom)));
        }
    }

    
    
    
    for (const auto& denom : zerocoinDenomList) {
        CAmount nValue = ZerocoinDenominationToAmount(denom);
        if ((nValue > nRemainingValue) && (mapOfDenomsUsed.at(denom) < mapOfDenomsHeld.at(denom))) {
            mapOfDenomsUsed.at(denom)++;
            nRemainingValue -= nValue;
            AmountUsed += nValue;
            nCoinCount++;
        }
        if (nRemainingValue < 0) break;
    }

    
    
    
    
    

    CAmount nAltChangeAmount = AmountUsed - nValueTarget;
    std::map<CoinDenomination, CAmount> mapAltChange = getChange(nAltChangeAmount);

    
    
    for (const auto& denom : zerocoinDenomList) {
        do {
            if (mapAltChange.at(denom) && mapOfDenomsUsed.at(denom)) {
                mapOfDenomsUsed.at(denom)--;
                mapAltChange.at(denom)--;
                nCoinCount--;
                CAmount nValue = ZerocoinDenominationToAmount(denom);
                AmountUsed -= nValue;
            }
        } while (mapAltChange.at(denom) && mapOfDenomsUsed.at(denom));
    }

    
    mapOfDenomsUsed = getSpendCoins(AmountUsed, mapOfDenomsHeld);
    nCoinCount = getNumberOfCoinsUsed(mapOfDenomsUsed);

    
    nAltChangeAmount = AmountUsed - nValueTarget;
    mapAltChange = getChange(nAltChangeAmount);
    int AltChangeCount = getNumberOfCoinsUsed(mapAltChange);

    
    if ((AltChangeCount < nChangeCount) && (nCoinCount <= nMaxNumberOfSpends)) {
        return AltChangeCount;
    } else {
        
        mapOfDenomsUsed = savedMapOfDenomsUsed;
        return nChangeCount;
    }
}







int calculateChange(
    int nMaxNumberOfSpends,
    bool fMinimizeChange,
    const CAmount nValueTarget,
    const std::map<CoinDenomination, CAmount>& mapOfDenomsHeld,
    std::map<CoinDenomination, CAmount>& mapOfDenomsUsed)
{
    CoinDenomination minDenomOverTarget = ZQ_ERROR;
    
    mapOfDenomsUsed.clear();
    for (const auto& denom : zerocoinDenomList)
        mapOfDenomsUsed.insert(std::pair<CoinDenomination, CAmount>(denom, 0));

    for (const auto& denom : zerocoinDenomList) {
        if (nValueTarget < ZerocoinDenominationToAmount(denom) && mapOfDenomsHeld.at(denom)) {
            minDenomOverTarget = denom;
            break;
        }
    }
    
    if (minDenomOverTarget != ZQ_ERROR) {
        mapOfDenomsUsed.at(minDenomOverTarget) = 1;

        
        CAmount nChangeAmount = ZerocoinDenominationToAmount(minDenomOverTarget) - nValueTarget;
        std::map<CoinDenomination, CAmount> mapChange = getChange(nChangeAmount);
        int nChangeCount = getNumberOfCoinsUsed(mapChange);

        if (fMinimizeChange) {
            CoinDenomination nextToMaxDenom = getNextLowerDenomHeld(minDenomOverTarget, mapOfDenomsHeld);
            int newChangeCount = minimizeChange(nMaxNumberOfSpends, nChangeCount,
                                                nextToMaxDenom, nValueTarget,
                                                mapOfDenomsHeld, mapOfDenomsUsed);

            
            if (newChangeCount < nChangeCount) return newChangeCount;

            
            for (const auto& denom : zerocoinDenomList)
                mapOfDenomsUsed.at(denom) = 0;
            
            mapOfDenomsUsed.at(minDenomOverTarget) = 1;
        }

        return nChangeCount;

    } else {
        
        for (const auto& denom : zerocoinDenomList)
            mapOfDenomsUsed.at(denom) = 0;
        CAmount nRemainingValue = nValueTarget;
        int nCoinCount = 0;
        CAmount AmountUsed = 0;
        for (const auto& denom : reverse_iterate(zerocoinDenomList)) {
            CAmount nValue = ZerocoinDenominationToAmount(denom);
            do {
                if (mapOfDenomsHeld.at(denom) && nRemainingValue > 0) {
                    mapOfDenomsUsed.at(denom)++;
                    AmountUsed += nValue;
                    nRemainingValue -= nValue;
                    nCoinCount++;
                }
            } while ((nRemainingValue > 0) && (mapOfDenomsUsed.at(denom) < mapOfDenomsHeld.at(denom)));
            if (nRemainingValue < 0) break;
        }

        CAmount nChangeAmount = AmountUsed - nValueTarget;
        std::map<CoinDenomination, CAmount> mapChange = getChange(nChangeAmount);
        int nMaxChangeCount = getNumberOfCoinsUsed(mapChange);

        
        CoinDenomination maxDenomHeld = getMaxDenomHeld(mapOfDenomsHeld);

        
        std::map<CoinDenomination, CAmount> mapOfMinDenomsUsed = mapOfDenomsUsed;

        int nChangeCount = minimizeChange(nMaxNumberOfSpends, nMaxChangeCount,
                                          maxDenomHeld, nValueTarget,
                                          mapOfDenomsHeld, mapOfMinDenomsUsed);

        int nNumSpends = getNumberOfCoinsUsed(mapOfMinDenomsUsed);

        if (!fMinimizeChange && (nCoinCount < nNumSpends)) {
            return nMaxChangeCount;
        }

        mapOfDenomsUsed = mapOfMinDenomsUsed;
        return nChangeCount;
    }
}





std::vector<CZerocoinMint> SelectMintsFromList(const CAmount nValueTarget, CAmount& nSelectedValue, int nMaxNumberOfSpends, bool fMinimizeChange,
                                               int& nCoinsReturned, const std::list<CZerocoinMint>& listMints, 
                                               const std::map<CoinDenomination, CAmount> mapOfDenomsHeld, int& nNeededSpends)
{
    std::vector<CZerocoinMint> vSelectedMints;
    std::map<CoinDenomination, CAmount> mapOfDenomsUsed;

    nNeededSpends = 0;
    bool fCanMeetExactly = getIdealSpends(nValueTarget, listMints, mapOfDenomsHeld, mapOfDenomsUsed);
    if (fCanMeetExactly) {
        nCoinsReturned = 0;
        nSelectedValue = nValueTarget;
        vSelectedMints = getSpends(listMints, mapOfDenomsUsed, nSelectedValue);
        
        if (vSelectedMints.size() <= (size_t)nMaxNumberOfSpends) {
            return vSelectedMints;
        }
        else {
            nNeededSpends = vSelectedMints.size();
        }
    }
    
    
    nCoinsReturned = calculateChange(nMaxNumberOfSpends, fMinimizeChange, nValueTarget, mapOfDenomsHeld, mapOfDenomsUsed);
    if (nCoinsReturned == 0) {
        LogPrint("zero", "%s: Problem getting change (TBD) or Too many spends %d\n", __func__, nValueTarget);
        vSelectedMints.clear();
    } else {
        vSelectedMints = getSpends(listMints, mapOfDenomsUsed, nSelectedValue);
        LogPrint("zero", "%s: %d coins in change for %d\n", __func__, nCoinsReturned, nValueTarget);
    }
    return vSelectedMints;
}
