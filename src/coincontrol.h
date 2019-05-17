





#ifndef BITCOIN_COINCONTROL_H
#define BITCOIN_COINCONTROL_H

#include "primitives/transaction.h"
#include "script/standard.h"


class CCoinControl
{
public:
    CTxDestination destChange;
    bool useObfuScation;
    bool useSwiftTX;
    bool fSplitBlock;
    int nSplitBlock;
    
    bool fAllowOtherInputs;
    
    bool fAllowWatchOnly;
    
    CAmount nMinimumTotalFee;

    CCoinControl()
    {
        SetNull();
    }

    void SetNull()
    {
        destChange = CNoDestination();
        setSelected.clear();
        useSwiftTX = false;
        useObfuScation = false;
        fAllowOtherInputs = false;
        fAllowWatchOnly = true;
        nMinimumTotalFee = 0;
        fSplitBlock = false;
        nSplitBlock = 1;
    }

    bool HasSelected() const
    {
        return (setSelected.size() > 0);
    }

    bool IsSelected(const uint256& hash, unsigned int n) const
    {
        COutPoint outpt(hash, n);
        return (setSelected.count(outpt) > 0);
    }

    void Select(const COutPoint& output)
    {
        setSelected.insert(output);
    }

    void UnSelect(const COutPoint& output)
    {
        setSelected.erase(output);
    }

    void UnSelectAll()
    {
        setSelected.clear();
    }

    void ListSelected(std::vector<COutPoint>& vOutpoints) const
    {
        vOutpoints.assign(setSelected.begin(), setSelected.end());
    }

    unsigned int QuantitySelected()
    {
        return setSelected.size();
    }

    void SetSelection(std::set<COutPoint> setSelected)
    {
        this->setSelected.clear();
        this->setSelected = setSelected;
    }

private:
    std::set<COutPoint> setSelected;
};

#endif 
