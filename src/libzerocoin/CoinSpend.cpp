/**
 * @file       CoinSpend.cpp
 *
 * @brief      CoinSpend class for the Zerocoin library.
 *
 * @author     Ian Miers, Christina Garman and Matthew Green
 * @date       June 2013
 *
 * @copyright  Copyright 2013 Ian Miers, Christina Garman and Matthew Green
 * @license    This project is released under the MIT license.
 **/

#include "CoinSpend.h"
#include <iostream>
namespace libzerocoin
{
CoinSpend::CoinSpend(const ZerocoinParams* p, const PrivateCoin& coin, Accumulator& a, const uint32_t checksum, const AccumulatorWitness& witness, const uint256& ptxHash) : accChecksum(checksum),
                                                                                                                                                                             ptxHash(ptxHash),
                                                                                                                                                                             coinSerialNumber((coin.getSerialNumber())),
                                                                                                                                                                             accumulatorPoK(&p->accumulatorParams),
                                                                                                                                                                             serialNumberSoK(p),
                                                                                                                                                                             commitmentPoK(&p->serialNumberSoKCommitmentGroup, &p->accumulatorParams.accumulatorPoKCommitmentGroup)
{
    denomination = coin.getPublicCoin().getDenomination();
    
    
    if (!(witness.VerifyWitness(a, coin.getPublicCoin()))) {
        std::cout << "CoinSpend: Accumulator witness does not verify\n";
        throw std::runtime_error("Accumulator witness does not verify");
    }

    
    
    
    
    
    
    
    const Commitment fullCommitmentToCoinUnderSerialParams(&p->serialNumberSoKCommitmentGroup, coin.getPublicCoin().getValue());
    this->serialCommitmentToCoinValue = fullCommitmentToCoinUnderSerialParams.getCommitmentValue();

    const Commitment fullCommitmentToCoinUnderAccParams(&p->accumulatorParams.accumulatorPoKCommitmentGroup, coin.getPublicCoin().getValue());
    this->accCommitmentToCoinValue = fullCommitmentToCoinUnderAccParams.getCommitmentValue();

    
    this->commitmentPoK = CommitmentProofOfKnowledge(&p->serialNumberSoKCommitmentGroup, &p->accumulatorParams.accumulatorPoKCommitmentGroup, fullCommitmentToCoinUnderSerialParams, fullCommitmentToCoinUnderAccParams);

    
    
    this->accumulatorPoK = AccumulatorProofOfKnowledge(&p->accumulatorParams, fullCommitmentToCoinUnderAccParams, witness, a);

    
    
    this->serialNumberSoK = SerialNumberSignatureOfKnowledge(p, coin, fullCommitmentToCoinUnderSerialParams, signatureHash());
}

bool CoinSpend::Verify(const Accumulator& a) const
{
    
    return (a.getDenomination() == this->denomination) && commitmentPoK.Verify(serialCommitmentToCoinValue, accCommitmentToCoinValue) && accumulatorPoK.Verify(a, accCommitmentToCoinValue) && serialNumberSoK.Verify(coinSerialNumber, serialCommitmentToCoinValue, signatureHash());
}

const uint256 CoinSpend::signatureHash() const
{
    CHashWriter h(0, 0);
    h << serialCommitmentToCoinValue << accCommitmentToCoinValue << commitmentPoK << accumulatorPoK << ptxHash
      << coinSerialNumber << accChecksum << denomination;
    return h.GetHash();
}

bool CoinSpend::HasValidSerial(ZerocoinParams* params) const
{
    return coinSerialNumber > 0 && coinSerialNumber < params->coinCommitmentGroup.groupOrder;
}

CBigNum CoinSpend::CalculateValidSerial(ZerocoinParams* params)
{
    CBigNum bnSerial = coinSerialNumber;
    bnSerial = bnSerial.mul_mod(CBigNum(1),params->coinCommitmentGroup.groupOrder);
    return bnSerial;
}

} 
