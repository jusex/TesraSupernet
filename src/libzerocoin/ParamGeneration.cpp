










#include "ParamGeneration.h"
#include <string>
#include <cmath>
#include "hash.h"
#include "uint256.h"

using namespace std;

namespace libzerocoin {



















void
CalculateParams(ZerocoinParams &params, CBigNum N, string aux, uint32_t securityLevel)
{
	params.initialized = false;
	params.accumulatorParams.initialized = false;

	
	uint32_t NLen = N.bitSize();
	if (NLen < 1023) {
		throw std::runtime_error("Modulus must be at least 1023 bits");
	}

	
	if (securityLevel < 80) {
		throw std::runtime_error("Security level must be at least 80 bits.");
	}

	
	params.accumulatorParams.accumulatorModulus = N;

	
	
	
	
	uint32_t pLen = 0;
	uint32_t qLen = 0;
	calculateGroupParamLengths(NLen - 2, securityLevel, &pLen, &qLen);

	
	
	
	params.coinCommitmentGroup = deriveIntegerGroupParams(calculateSeed(N, aux, securityLevel, STRING_COMMIT_GROUP),
	                             pLen, qLen);

	
	
	
	
	params.serialNumberSoKCommitmentGroup = deriveIntegerGroupFromOrder(params.coinCommitmentGroup.modulus);

	
	
	params.accumulatorParams.accumulatorPoKCommitmentGroup = deriveIntegerGroupParams(calculateSeed(N, aux, securityLevel, STRING_AIC_GROUP),
	        qLen + 300, qLen + 1);

	
	
	uint32_t resultCtr;
	params.accumulatorParams.accumulatorQRNCommitmentGroup.g = generateIntegerFromSeed(NLen - 1,
	        calculateSeed(N, aux, securityLevel, STRING_QRNCOMMIT_GROUPG),
											 &resultCtr).pow_mod(CBigNum(2),N);
	params.accumulatorParams.accumulatorQRNCommitmentGroup.h = generateIntegerFromSeed(NLen - 1,
	        calculateSeed(N, aux, securityLevel, STRING_QRNCOMMIT_GROUPH),
											 &resultCtr).pow_mod(CBigNum(2), N);

	
	
	
	CBigNum constant(ACCUMULATOR_BASE_CONSTANT);
	params.accumulatorParams.accumulatorBase = CBigNum(1);
	for (uint32_t count = 0; count < MAX_ACCUMGEN_ATTEMPTS && params.accumulatorParams.accumulatorBase.isOne(); count++) {
		params.accumulatorParams.accumulatorBase = constant.pow_mod(CBigNum(2), params.accumulatorParams.accumulatorModulus);
	}

	
	
	
	params.accumulatorParams.maxCoinValue = params.coinCommitmentGroup.modulus;
	params.accumulatorParams.minCoinValue = CBigNum(2).pow((params.coinCommitmentGroup.modulus.bitSize() / 2) + 3);

	
	params.accumulatorParams.initialized = true;

	
	params.initialized = true;
}










uint256
calculateGeneratorSeed(uint256 seed, uint256 pSeed, uint256 qSeed, string label, uint32_t index, uint32_t count)
{
	CHashWriter hasher(0,0);
	uint256     hash;

	
	
	hasher << seed;
	hasher << string("||");
	hasher << pSeed;
	hasher << string("||");
	hasher << qSeed;
	hasher << string("||");
	hasher << label;
	hasher << string("||");
	hasher << index;
	hasher << string("||");
	hasher << count;

	return hasher.GetHash();
}










uint256
calculateSeed(CBigNum modulus, string auxString, uint32_t securityLevel, string groupName)
{
	CHashWriter hasher(0,0);
	uint256     hash;

	
	
	hasher << modulus;
	hasher << string("||");
	hasher << securityLevel;
	hasher << string("||");
	hasher << auxString;
	hasher << string("||");
	hasher << groupName;

	return hasher.GetHash();
}

uint256
calculateHash(uint256 input)
{
	CHashWriter hasher(0,0);

	
	hasher << input;

	return hasher.GetHash();
}




















void
calculateGroupParamLengths(uint32_t maxPLen, uint32_t securityLevel,
                           uint32_t *pLen, uint32_t *qLen)
{
	*pLen = *qLen = 0;

	if (securityLevel < 80) {
		throw std::runtime_error("Security level must be at least 80 bits.");
	} else if (securityLevel == 80) {
		*qLen = 256;
		*pLen = 1024;
	} else if (securityLevel <= 112) {
		*qLen = 256;
		*pLen = 2048;
	} else if (securityLevel <= 128) {
		*qLen = 320;
		*pLen = 3072;
	} else {
		throw std::runtime_error("Security level not supported.");
	}

	if (*pLen > maxPLen) {
		throw std::runtime_error("Modulus size is too small for this security level.");
	}
}













IntegerGroupParams
deriveIntegerGroupParams(uint256 seed, uint32_t pLen, uint32_t qLen)
{
	IntegerGroupParams result;
	CBigNum p;
	CBigNum q;
	uint256 pSeed, qSeed;

	
	
	
	calculateGroupModulusAndOrder(seed, pLen, qLen, &(result.modulus),
	                              &(result.groupOrder), &pSeed, &qSeed);

	
	
	
	
	result.g = calculateGroupGenerator(seed, pSeed, qSeed, result.modulus, result.groupOrder, 1);
	result.h = calculateGroupGenerator(seed, pSeed, qSeed, result.modulus, result.groupOrder, 2);

	
	if ((uint32_t)(result.modulus.bitSize()) < pLen ||          
	        (uint32_t)(result.groupOrder.bitSize()) < qLen ||       
	        !(result.modulus.isPrime()) ||                          
	        !(result.groupOrder.isPrime()) ||                       
	        !((result.g.pow_mod(result.groupOrder, result.modulus)).isOne()) || 
	        !((result.h.pow_mod(result.groupOrder, result.modulus)).isOne()) || 
	        ((result.g.pow_mod(CBigNum(100), result.modulus)).isOne()) ||        
	        ((result.h.pow_mod(CBigNum(100), result.modulus)).isOne()) ||        
	        result.g == result.h ||                                 
	        result.g.isOne()) {                                     
		
		throw std::runtime_error("Group parameters are not valid");
	}

	return result;
}








IntegerGroupParams
deriveIntegerGroupFromOrder(CBigNum &groupOrder)
{
	IntegerGroupParams result;

	
	result.groupOrder = groupOrder;

	
	
	for (uint32_t i = 1; i < NUM_SCHNORRGEN_ATTEMPTS; i++) {
		
		result.modulus = (result.groupOrder * CBigNum(i*2)) + CBigNum(1);

		
		
		if (result.modulus.isPrime(256)) {

			
			
			
			
			
			
			uint256 seed = calculateSeed(groupOrder, "", 128, "");
			uint256 pSeed = calculateHash(seed);
			uint256 qSeed = calculateHash(pSeed);
			result.g = calculateGroupGenerator(seed, pSeed, qSeed, result.modulus, result.groupOrder, 1);
			result.h = calculateGroupGenerator(seed, pSeed, qSeed, result.modulus, result.groupOrder, 2);

			
			if (!(result.modulus.isPrime()) ||                          
			        !(result.groupOrder.isPrime()) ||                       
			        !((result.g.pow_mod(result.groupOrder, result.modulus)).isOne()) || 
			        !((result.h.pow_mod(result.groupOrder, result.modulus)).isOne()) || 
			        ((result.g.pow_mod(CBigNum(100), result.modulus)).isOne()) ||        
			        ((result.h.pow_mod(CBigNum(100), result.modulus)).isOne()) ||        
			        result.g == result.h ||                                 
			        result.g.isOne()) {                                     
				
				throw std::runtime_error("Group parameters are not valid");
			}

			return result;
		}
	}

	
	throw std::runtime_error("Too many attempts to generate Schnorr group.");
}














void
calculateGroupModulusAndOrder(uint256 seed, uint32_t pLen, uint32_t qLen,
                              CBigNum *resultModulus, CBigNum *resultGroupOrder,
                              uint256 *resultPseed, uint256 *resultQseed)
{
	
	if (qLen > (sizeof(seed)) * 8) {
		
		
	}

#ifdef ZEROCOIN_DEBUG
	cout << "calculateGroupModulusAndOrder: pLen = " << pLen << endl;
#endif

	
	
	
	uint256     qseed;
	uint32_t    qgen_counter;
	*resultGroupOrder = generateRandomPrime(qLen, seed, &qseed, &qgen_counter);

	
	
	uint32_t    p0len = ceil((pLen / 2.0) + 1);
	uint256     pseed;
	uint32_t    pgen_counter;
	CBigNum p0 = generateRandomPrime(p0len, qseed, &pseed, &pgen_counter);

	
	uint32_t    old_counter = pgen_counter;

	
	uint32_t iterations;
	CBigNum x = generateIntegerFromSeed(pLen, pseed, &iterations);
	pseed += (iterations + 1);

	
	CBigNum powerOfTwo = CBigNum(2).pow(pLen-1);
	x = powerOfTwo + (x % powerOfTwo);

	
	
	CBigNum t = x / (CBigNum(2) * (*resultGroupOrder) * p0);

	
	
	for ( ; pgen_counter <= ((4*pLen) + old_counter) ; pgen_counter++) {
		
		
		powerOfTwo = CBigNum(2).pow(pLen);
		CBigNum prod = (CBigNum(2) * t * (*resultGroupOrder) * p0) + CBigNum(1);
		if (prod > powerOfTwo) {
			
			t = CBigNum(2).pow(pLen-1) / (CBigNum(2) * (*resultGroupOrder) * p0);
		}

		
		*resultModulus = (CBigNum(2) * t * (*resultGroupOrder) * p0) + CBigNum(1);

		
		CBigNum a = generateIntegerFromSeed(pLen, pseed, &iterations);
		pseed += iterations + 1;

		
		a = CBigNum(2) + (a % ((*resultModulus) - CBigNum(3)));

		
		CBigNum z = a.pow_mod(CBigNum(2) * t * (*resultGroupOrder), (*resultModulus));

		
		
		if ((resultModulus->gcd(z - CBigNum(1))).isOne() &&
		        (z.pow_mod(p0, (*resultModulus))).isOne()) {
			
			*resultPseed = pseed;
			*resultQseed = qseed;
			return;
		}

		
		t = t + CBigNum(1);
	} 

	
	
	throw std::runtime_error("Unable to generate a prime modulus for the group");
}














CBigNum
calculateGroupGenerator(uint256 seed, uint256 pSeed, uint256 qSeed, CBigNum modulus, CBigNum groupOrder, uint32_t index)
{
	CBigNum result;

	
	if (index > 255) {
		throw std::runtime_error("Invalid index for group generation");
	}

	
	CBigNum e = (modulus - CBigNum(1)) / groupOrder;

	
	for (uint32_t count = 1; count < MAX_GENERATOR_ATTEMPTS; count++) {
		
		uint256 hash = calculateGeneratorSeed(seed, pSeed, qSeed, "ggen", index, count);
		CBigNum W(hash);

		
		result = W.pow_mod(e, modulus);

		
		if (result > 1) {
			return result;
		}
	}

	
	throw std::runtime_error("Unable to find a generator, too many attempts");
}













CBigNum
generateRandomPrime(uint32_t primeBitLen, uint256 in_seed, uint256 *out_seed,
                    uint32_t *prime_gen_counter)
{
	
	if (primeBitLen < 2) {
		throw std::runtime_error("Prime length is too short");
	}

	
	if (primeBitLen < 33) {
		CBigNum result(0);

		
		uint256     prime_seed = in_seed;
		(*prime_gen_counter) = 0;

		
		while ((*prime_gen_counter) < (4 * primeBitLen)) {

			
			uint32_t iteration_count;
			CBigNum c = generateIntegerFromSeed(primeBitLen, prime_seed, &iteration_count);
#ifdef ZEROCOIN_DEBUG
			cout << "generateRandomPrime: primeBitLen = " << primeBitLen << endl;
			cout << "Generated c = " << c << endl;
#endif

			prime_seed += (iteration_count + 1);
			(*prime_gen_counter)++;

			
			uint32_t intc = c.getulong();
			intc = (2 * floor(intc / 2.0)) + 1;
#ifdef ZEROCOIN_DEBUG
			cout << "Should be odd. c = " << intc << endl;
			cout << "The big num is: c = " << c << endl;
#endif

			
			
			if (primalityTestByTrialDivision(intc)) {
				
				
				result = intc;
				*out_seed = prime_seed;

				
				return result;
			}
		} 

		
		
		throw std::runtime_error("Unable to find prime in Shawe-Taylor algorithm");

		
	}
	
	else {
		
		uint32_t newLength = ceil((double)primeBitLen / 2.0) + 1;
		CBigNum c0 = generateRandomPrime(newLength, in_seed, out_seed, prime_gen_counter);

		
		
		uint32_t numIterations;
		CBigNum x = generateIntegerFromSeed(primeBitLen, *out_seed, &numIterations);
		(*out_seed) += numIterations + 1;

		
		
		CBigNum t = x / (CBigNum(2) * c0);

		
		for (uint32_t testNum = 0; testNum < MAX_PRIMEGEN_ATTEMPTS; testNum++) {

			
			
			if ((CBigNum(2) * t * c0) > (CBigNum(2).pow(CBigNum(primeBitLen)))) {
				t = ((CBigNum(2).pow(CBigNum(primeBitLen))) - CBigNum(1)) / (CBigNum(2) * c0);
			}

			
			CBigNum c = (CBigNum(2) * t * c0) + CBigNum(1);

			
			(*prime_gen_counter)++;

			
			
			CBigNum a = generateIntegerFromSeed(c.bitSize(), (*out_seed), &numIterations);
			a = CBigNum(2) + (a % (c - CBigNum(3)));
			(*out_seed) += (numIterations + 1);

			
			CBigNum z = a.pow_mod(CBigNum(2) * t, c);

			
			
			
			if (c.gcd(z - CBigNum(1)).isOne() && z.pow_mod(c0, c).isOne()) {
				
				
				return c;
			}

			
			t = t + CBigNum(1);
		} 
	}

	
	
	throw std::runtime_error("Unable to generate random prime (too many tests)");
}

CBigNum
generateIntegerFromSeed(uint32_t numBits, uint256 seed, uint32_t *numIterations)
{
	CBigNum      result(0);
	uint32_t    iterations = ceil((double)numBits / (double)HASH_OUTPUT_BITS);

#ifdef ZEROCOIN_DEBUG
	cout << "numBits = " << numBits << endl;
	cout << "iterations = " << iterations << endl;
#endif

	
	for (uint32_t count = 0; count < iterations; count++) {
		
		result += CBigNum(calculateHash(seed + count)) * CBigNum(2).pow(count * HASH_OUTPUT_BITS);
	}

	result = CBigNum(2).pow(numBits - 1) + (result % (CBigNum(2).pow(numBits - 1)));

	
	*numIterations = iterations;
	return result;
}







bool
primalityTestByTrialDivision(uint32_t candidate)
{
	
	CBigNum canBignum(candidate);

	return canBignum.isPrime();
}

} 
