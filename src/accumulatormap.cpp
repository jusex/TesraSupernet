



#include "accumulatormap.h"
#include "accumulators.h"
#include "main.h"
#include "txdb.h"
#include "libzerocoin/Denominations.h"

using namespace libzerocoin;
using namespace std;


AccumulatorMap::AccumulatorMap()
{
    for (auto& denom : zerocoinDenomList) {
        unique_ptr<Accumulator> uptr(new Accumulator(Params().Zerocoin_Params(), denom));
        mapAccumulators.insert(make_pair(denom, std::move(uptr)));
    }
}


void AccumulatorMap::Reset()
{
    mapAccumulators.clear();
    for (auto& denom : zerocoinDenomList) {
        unique_ptr<Accumulator> uptr(new Accumulator(Params().Zerocoin_Params(), denom));
        mapAccumulators.insert(make_pair(denom, std::move(uptr)));
    }
}


bool AccumulatorMap::Load(uint256 nCheckpoint)
{
    for (auto& denom : zerocoinDenomList) {
        uint32_t nChecksum = ParseChecksum(nCheckpoint, denom);

        CBigNum bnValue;
        if (!zerocoinDB->ReadAccumulatorValue(nChecksum, bnValue)) {
            LogPrintf("%s : cannot find checksum %d\n", __func__, nChecksum);
            return false;
        }

        mapAccumulators.at(denom)->setValue(bnValue);
    }
    return true;
}


bool AccumulatorMap::Accumulate(PublicCoin pubCoin, bool fSkipValidation)
{
    CoinDenomination denom = pubCoin.getDenomination();
    if (denom == CoinDenomination::ZQ_ERROR)
        return false;

    if (fSkipValidation)
        mapAccumulators.at(denom)->increment(pubCoin.getValue());
    else
        mapAccumulators.at(denom)->accumulate(pubCoin);
    return true;
}


CBigNum AccumulatorMap::GetValue(CoinDenomination denom)
{
    if (denom == CoinDenomination::ZQ_ERROR)
        return CBigNum(0);
    return mapAccumulators.at(denom)->getValue();
}


uint256 AccumulatorMap::GetCheckpoint()
{
    uint256 nCheckpoint;

    
    assert(zerocoinDenomList.size() == 8);
    for (auto& denom : zerocoinDenomList) {
        CBigNum bnValue = mapAccumulators.at(denom)->getValue();
        uint32_t nCheckSum = GetChecksum(bnValue);
        nCheckpoint = nCheckpoint << 32 | nCheckSum;
    }

    return nCheckpoint;
}


