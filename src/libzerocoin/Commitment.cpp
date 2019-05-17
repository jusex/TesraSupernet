/**
 * @file       Commitment.cpp
 *
 * @brief      Commitment and CommitmentProof classes for the Zerocoin library.
 *
 * @author     Ian Miers, Christina Garman and Matthew Green
 * @date       June 2013
 *
 * @copyright  Copyright 2013 Ian Miers, Christina Garman and Matthew Green
 * @license    This project is released under the MIT license.
 **/


#include <stdlib.h>
#include "Commitment.h"
#include "hash.h"

namespace libzerocoin {


Commitment::Commitment(const IntegerGroupParams* p,
                                   const CBigNum& value): params(p), contents(value) {
	this->randomness = CBigNum::randBignum(params->groupOrder);
	this->commitmentValue = (params->g.pow_mod(this->contents, params->modulus).mul_mod(
	                         params->h.pow_mod(this->randomness, params->modulus), params->modulus));
}

const CBigNum& Commitment::getCommitmentValue() const {
	return this->commitmentValue;
}

const CBigNum& Commitment::getRandomness() const {
	return this->randomness;
}

const CBigNum& Commitment::getContents() const {
	return this->contents;
}


CommitmentProofOfKnowledge::CommitmentProofOfKnowledge(const IntegerGroupParams* ap, const IntegerGroupParams* bp): ap(ap), bp(bp) {}


CommitmentProofOfKnowledge::CommitmentProofOfKnowledge(const IntegerGroupParams* aParams,
        const IntegerGroupParams* bParams, const Commitment& a, const Commitment& b):
	ap(aParams),bp(bParams)
{
	CBigNum r1, r2, r3;

	
	
	if (a.getContents() != b.getContents()) {
		throw std::runtime_error("Both commitments must contain the same value");
	}

	
	
	
	
	
	uint32_t randomSize = COMMITMENT_EQUALITY_CHALLENGE_SIZE + COMMITMENT_EQUALITY_SECMARGIN +
	                      std::max(std::max(this->ap->modulus.bitSize(), this->bp->modulus.bitSize()),
	                               std::max(this->ap->groupOrder.bitSize(), this->bp->groupOrder.bitSize()));
	CBigNum maxRange = (CBigNum(2).pow(randomSize) - CBigNum(1));

	r1 = CBigNum::randBignum(maxRange);
	r2 = CBigNum::randBignum(maxRange);
	r3 = CBigNum::randBignum(maxRange);

	
	
	
	
	
	
	CBigNum T1 = this->ap->g.pow_mod(r1, this->ap->modulus).mul_mod((this->ap->h.pow_mod(r2, this->ap->modulus)), this->ap->modulus);
	CBigNum T2 = this->bp->g.pow_mod(r1, this->bp->modulus).mul_mod((this->bp->h.pow_mod(r3, this->bp->modulus)), this->bp->modulus);

	
	
	this->challenge = calculateChallenge(a.getCommitmentValue(), b.getCommitmentValue(), T1, T2);

	
	
	
	
	
	
	
	
	
	
	this->S1 = r1 + (a.getContents() * this->challenge);
	this->S2 = r2 + (a.getRandomness() * this->challenge);
	this->S3 = r3 + (b.getRandomness() * this->challenge);

	
	
}

bool CommitmentProofOfKnowledge::Verify(const CBigNum& A, const CBigNum& B) const
{
	
	
	uint32_t maxSize = 64 * (COMMITMENT_EQUALITY_CHALLENGE_SIZE + COMMITMENT_EQUALITY_SECMARGIN +
	                         std::max(std::max(this->ap->modulus.bitSize(), this->bp->modulus.bitSize()),
	                                  std::max(this->ap->groupOrder.bitSize(), this->bp->groupOrder.bitSize())));

	if ((uint32_t)this->S1.bitSize() > maxSize ||
	        (uint32_t)this->S2.bitSize() > maxSize ||
	        (uint32_t)this->S3.bitSize() > maxSize ||
	        this->S1 < CBigNum(0) ||
	        this->S2 < CBigNum(0) ||
	        this->S3 < CBigNum(0) ||
	        this->challenge < CBigNum(0) ||
	        this->challenge > (CBigNum(2).pow(COMMITMENT_EQUALITY_CHALLENGE_SIZE) - CBigNum(1))) {
		
		return false;
	}

	
	CBigNum T1 = A.pow_mod(this->challenge, ap->modulus).inverse(ap->modulus).mul_mod(
	                (ap->g.pow_mod(S1, ap->modulus).mul_mod(ap->h.pow_mod(S2, ap->modulus), ap->modulus)),
	                ap->modulus);

	
	CBigNum T2 = B.pow_mod(this->challenge, bp->modulus).inverse(bp->modulus).mul_mod(
	                (bp->g.pow_mod(S1, bp->modulus).mul_mod(bp->h.pow_mod(S3, bp->modulus), bp->modulus)),
	                bp->modulus);

	
	CBigNum computedChallenge = calculateChallenge(A, B, T1, T2);

	
	return computedChallenge == this->challenge;
}

const CBigNum CommitmentProofOfKnowledge::calculateChallenge(const CBigNum& a, const CBigNum& b, const CBigNum &commitOne, const CBigNum &commitTwo) const {
	CHashWriter hasher(0,0);

	
	
	
	
	
	
	
	

	hasher << std::string(ZEROCOIN_COMMITMENT_EQUALITY_PROOF);
	hasher << commitOne;
	hasher << std::string("||");
	hasher << commitTwo;
	hasher << std::string("||");
	hasher << a;
	hasher << std::string("||");
	hasher << b;
	hasher << std::string("||");
	hasher << *(this->ap);
	hasher << std::string("||");
	hasher << *(this->bp);

	
	
	
	return CBigNum(hasher.GetHash());
}

} 
