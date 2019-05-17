/**
 * @file       Coin.cpp
 *
 * @brief      PublicCoin and PrivateCoin classes for the Zerocoin library.
 *
 * @author     Ian Miers, Christina Garman and Matthew Green
 * @date       June 2013
 *
 * @copyright  Copyright 2013 Ian Miers, Christina Garman and Matthew Green
 * @license    This project is released under the MIT license.
 **/


#include <stdexcept>
#include <iostream>
#include "Coin.h"
#include "Commitment.h"
#include "Denominations.h"

namespace libzerocoin {


PublicCoin::PublicCoin(const ZerocoinParams* p):
	params(p) {
	if (this->params->initialized == false) {
		throw std::runtime_error("Params are not initialized");
	}
    
    denomination = ZQ_ERROR;
};

PublicCoin::PublicCoin(const ZerocoinParams* p, const CBigNum& coin, const CoinDenomination d):
	params(p), value(coin) {
	if (this->params->initialized == false) {
		throw std::runtime_error("Params are not initialized");
	}

	denomination = d;
	for(const CoinDenomination denom : zerocoinDenomList) {
		if(denom == d)
			denomination = d;
	}
    if(denomination == 0){
		std::cout << "denom does not exist\n";
		throw std::runtime_error("Denomination does not exist");
	}
};


PrivateCoin::PrivateCoin(const ZerocoinParams* p, const CoinDenomination denomination): params(p), publicCoin(p) {
	
	if(this->params->initialized == false) {
		throw std::runtime_error("Params are not initialized");
	}

#ifdef ZEROCOIN_FAST_MINT
	
	
	
	this->mintCoinFast(denomination);
#else
	
	this->mintCoin(denomination);
#endif
	
}

void PrivateCoin::mintCoin(const CoinDenomination denomination) {
	
	
	for(uint32_t attempt = 0; attempt < MAX_COINMINT_ATTEMPTS; attempt++) {

		
		
		CBigNum s = CBigNum::randBignum(this->params->coinCommitmentGroup.groupOrder);

		
		Commitment coin(&params->coinCommitmentGroup, s);

		
		
		
		if (coin.getCommitmentValue().isPrime(ZEROCOIN_MINT_PRIME_PARAM) &&
		        coin.getCommitmentValue() >= params->accumulatorParams.minCoinValue &&
		        coin.getCommitmentValue() <= params->accumulatorParams.maxCoinValue) {
			
			this->serialNumber = s;
			this->randomness = coin.getRandomness();
			this->publicCoin = PublicCoin(params,coin.getCommitmentValue(), denomination);

			
			return;
		}
	}

	
	
	throw std::runtime_error("Unable to mint a new Zerocoin (too many attempts)");
}

void PrivateCoin::mintCoinFast(const CoinDenomination denomination) {
	
	
	
	CBigNum s = CBigNum::randBignum(this->params->coinCommitmentGroup.groupOrder);
	
	
	CBigNum r = CBigNum::randBignum(this->params->coinCommitmentGroup.groupOrder);
	
	
	
	CBigNum commitmentValue = this->params->coinCommitmentGroup.g.pow_mod(s, this->params->coinCommitmentGroup.modulus).mul_mod(this->params->coinCommitmentGroup.h.pow_mod(r, this->params->coinCommitmentGroup.modulus), this->params->coinCommitmentGroup.modulus);
	
	
	
	for (uint32_t attempt = 0; attempt < MAX_COINMINT_ATTEMPTS; attempt++) {
		
		
		
		if (commitmentValue.isPrime(ZEROCOIN_MINT_PRIME_PARAM) &&
			commitmentValue >= params->accumulatorParams.minCoinValue &&
			commitmentValue <= params->accumulatorParams.maxCoinValue) {
			
			this->serialNumber = s;
			this->randomness = r;
			this->publicCoin = PublicCoin(params, commitmentValue, denomination);
				
			
			return;
		}
		
		
		CBigNum r_delta = CBigNum::randBignum(this->params->coinCommitmentGroup.groupOrder);

		
		
		
		r = (r + r_delta) % this->params->coinCommitmentGroup.groupOrder;
		commitmentValue = commitmentValue.mul_mod(this->params->coinCommitmentGroup.h.pow_mod(r_delta, this->params->coinCommitmentGroup.modulus), this->params->coinCommitmentGroup.modulus);
	}
		
	
	
	throw std::runtime_error("Unable to mint a new Zerocoin (too many attempts)");
}
	
} 
