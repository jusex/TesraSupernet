


#ifndef TESRA_ACCUMULATORMAP_H
#define TESRA_ACCUMULATORMAP_H

#include "libzerocoin/Accumulator.h"
#include "libzerocoin/Coin.h"


class AccumulatorMap
{
private:
    std::map<libzerocoin::CoinDenomination, std::unique_ptr<libzerocoin::Accumulator> > mapAccumulators;
public:
    AccumulatorMap();
    bool Load(uint256 nCheckpoint);
    bool Accumulate(libzerocoin::PublicCoin pubCoin, bool fSkipValidation = false);
    CBigNum GetValue(libzerocoin::CoinDenomination denom);
    uint256 GetCheckpoint();
    void Reset();
};
#endif 
