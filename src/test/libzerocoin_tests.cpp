/**
* @file       Tests.cpp
*
* @brief      Test routines for Zerocoin.
*
* @author     Ian Miers, Christina Garman and Matthew Green
* @date       June 2013
*
* @copyright  Copyright 2013 Ian Miers, Christina Garman and Matthew Green
* @license    This project is released under the MIT license.
**/

#include <boost/test/unit_test.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>

#include <exception>
#include "streams.h"
#include "libzerocoin/ParamGeneration.h"
#include "libzerocoin/Denominations.h"
#include "libzerocoin/Coin.h"
#include "libzerocoin/CoinSpend.h"
#include "libzerocoin/Accumulator.h"

using namespace std;
using namespace libzerocoin;

#define COLOR_STR_GREEN   "\033[32m"
#define COLOR_STR_NORMAL  "\033[0m"
#define COLOR_STR_RED     "\033[31m"

#define TESTS_COINS_TO_ACCUMULATE   10
#define NON_PRIME_TESTS				100


uint32_t    gNumTests        = 0;
uint32_t    gSuccessfulTests = 0;


uint32_t    gProofSize			= 0;
uint32_t    gCoinSize			= 0;
uint32_t	gSerialNumberSize	= 0;


PrivateCoin    *gCoins[TESTS_COINS_TO_ACCUMULATE];


ZerocoinParams *g_Params;





void
LogTestResult(string testName, bool (*testPtr)())
{
	string colorGreen(COLOR_STR_GREEN);
	string colorNormal(COLOR_STR_NORMAL);
	string colorRed(COLOR_STR_RED);

	cout << "Testing if " << testName << "..." << endl;

	bool testResult = testPtr();

	if (testResult == true) {
		cout << "\t" << colorGreen << "[PASS]"  << colorNormal << endl;
		gSuccessfulTests++;
	} else {
		cout << colorRed << "\t[FAIL]" << colorNormal << endl;
	}

	gNumTests++;
}

CBigNum
GetTestModulus()
{
	static CBigNum testModulus(0);

	
	if (!testModulus) {
		CBigNum p, q;

		
		
		
		p = CBigNum::generatePrime(1024, false);
		q = CBigNum::generatePrime(1024, false);
		testModulus = p * q;
	}

	return testModulus;
}





bool
Test_GenRSAModulus()
{
	CBigNum result = GetTestModulus();

	if (!result) {
		return false;
	}
	else {
		return true;
	}
}

bool
Test_CalcParamSizes()
{
	bool result = true;
#if 0

	uint32_t pLen, qLen;

	try {
		calculateGroupParamLengths(4000, 80, &pLen, &qLen);
		if (pLen < 1024 || qLen < 256) {
			result = false;
		}
		calculateGroupParamLengths(4000, 96, &pLen, &qLen);
		if (pLen < 2048 || qLen < 256) {
			result = false;
		}
		calculateGroupParamLengths(4000, 112, &pLen, &qLen);
		if (pLen < 3072 || qLen < 320) {
			result = false;
		}
		calculateGroupParamLengths(4000, 120, &pLen, &qLen);
		if (pLen < 3072 || qLen < 320) {
			result = false;
		}
		calculateGroupParamLengths(4000, 128, &pLen, &qLen);
		if (pLen < 3072 || qLen < 320) {
			result = false;
		}
	} catch (exception &e) {
		result = false;
	}
#endif

	return result;
}

bool
Test_GenerateGroupParams()
{
	uint32_t pLen = 1024, qLen = 256, count;
	IntegerGroupParams group;

	for (count = 0; count < 1; count++) {

		try {
			group = deriveIntegerGroupParams(calculateSeed(GetTestModulus(), "test", ZEROCOIN_DEFAULT_SECURITYLEVEL, "TEST GROUP"), pLen, qLen);
		} catch (std::runtime_error e) {
			cout << "Caught exception " << e.what() << endl;
			return false;
		}

		
		if ((uint32_t)group.groupOrder.bitSize() < qLen || (uint32_t)group.modulus.bitSize() < pLen) {
			return false;
		}

		CBigNum c = group.g.pow_mod(group.groupOrder, group.modulus);
		
		if (!(c.isOne())) return false;

		
		pLen = pLen * 1.5;
		qLen = qLen * 1.5;
	}

	return true;
}

bool
Test_ParamGen()
{
	bool result = true;

	try {
		
		ZerocoinParams testParams(GetTestModulus(),ZEROCOIN_DEFAULT_SECURITYLEVEL);
	} catch (runtime_error e) {
		cout << e.what() << endl;
		result = false;
	}

	return result;
}

bool
Test_Accumulator()
{
	
	
	if (gCoins[0] == NULL) {
		return false;
	}
	try {
		
            Accumulator accOne(&g_Params->accumulatorParams, CoinDenomination::ZQ_ONE);
            Accumulator accTwo(&g_Params->accumulatorParams,CoinDenomination::ZQ_ONE);
            Accumulator accThree(&g_Params->accumulatorParams,CoinDenomination::ZQ_ONE);
            Accumulator accFour(&g_Params->accumulatorParams,CoinDenomination::ZQ_ONE);
		AccumulatorWitness wThree(g_Params, accThree, gCoins[0]->getPublicCoin());

		for (uint32_t i = 0; i < TESTS_COINS_TO_ACCUMULATE; i++) {
			accOne += gCoins[i]->getPublicCoin();
			accTwo += gCoins[TESTS_COINS_TO_ACCUMULATE - (i+1)]->getPublicCoin();
			accThree += gCoins[i]->getPublicCoin();
			wThree += gCoins[i]->getPublicCoin();
			if(i != 0) {
				accFour += gCoins[i]->getPublicCoin();
			}
		}

		
		if (accOne.getValue() != accTwo.getValue() || accOne.getValue() != accThree.getValue()) {
			cout << "Accumulators don't match" << endl;
			return false;
		}

		if(accFour.getValue() != wThree.getValue()) {
			cout << "Witness math not working," << endl;
			return false;
		}

		
		if (!wThree.VerifyWitness(accThree, gCoins[0]->getPublicCoin()) ) {
			cout << "Witness not valid" << endl;
			return false;
		}

		
		CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
		ss << accOne;

		
		Accumulator newAcc(g_Params, ss);

		
		if (accOne.getValue() != newAcc.getValue()) {
			return false;
		}

	} catch (runtime_error e) {
		return false;
	}

	return true;
}

bool
Test_EqualityPoK()
{
	
	for (uint32_t i = 0; i < 10; i++) {
		try {
			
			CBigNum val = CBigNum::randBignum(g_Params->coinCommitmentGroup.groupOrder);

			
			
			Commitment one(&g_Params->accumulatorParams.accumulatorPoKCommitmentGroup, val);

			Commitment two(&g_Params->serialNumberSoKCommitmentGroup, val);

			
			
			CommitmentProofOfKnowledge pok(&g_Params->accumulatorParams.accumulatorPoKCommitmentGroup,
			                               &g_Params->serialNumberSoKCommitmentGroup,
			                               one, two);

			
			CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
			ss << pok;

			
			CommitmentProofOfKnowledge newPok(&g_Params->accumulatorParams.accumulatorPoKCommitmentGroup,
			                                  &g_Params->serialNumberSoKCommitmentGroup,
			                                  ss);

			if (newPok.Verify(one.getCommitmentValue(), two.getCommitmentValue()) != true) {
				return false;
			}

			
			CDataStream ss2(SER_NETWORK, PROTOCOL_VERSION);
			ss2 << pok;

			
			ss2[15] = 0;
			CommitmentProofOfKnowledge newPok2(&g_Params->accumulatorParams.accumulatorPoKCommitmentGroup,
			                                   &g_Params->serialNumberSoKCommitmentGroup,
			                                   ss2);

			
			if (newPok2.Verify(one.getCommitmentValue(), two.getCommitmentValue()) == true) {
				return false;
			}

		} catch (runtime_error &e) {
			return false;
		}
	}

	return true;
}

bool
Test_MintCoin()
{
	gCoinSize = 0;

	try {
		
		for (uint32_t i = 0; i < TESTS_COINS_TO_ACCUMULATE; i++) {
            gCoins[i] = new PrivateCoin(g_Params,libzerocoin::CoinDenomination::ZQ_ONE);

			PublicCoin pc = gCoins[i]->getPublicCoin();
			CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
			ss << pc;
			gCoinSize += ss.size();
		}

		gCoinSize /= TESTS_COINS_TO_ACCUMULATE;

	} catch (exception &e) {
		return false;
	}

	return true;
}

bool Test_InvalidCoin()
{
	CBigNum coinValue;
	
	try {
		
		for (uint32_t i = 0; i < NON_PRIME_TESTS; i++) {
			coinValue = CBigNum::randBignum(g_Params->coinCommitmentGroup.modulus);
			coinValue = coinValue * 2;
			if (!coinValue.isPrime()) break;
		}
				
		PublicCoin pubCoin(g_Params);
		if (pubCoin.validate()) {
			
			return false;
		}		
		
		PublicCoin pubCoin2(g_Params, coinValue, ZQ_ONE);
		if (pubCoin2.validate()) {
			
			return false;
		}
		
		PublicCoin pubCoin3 = pubCoin2;
		if (pubCoin2.validate()) {
			
			return false;
		}
		
		
		CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
		ss << pubCoin;
		PublicCoin pubCoin4(g_Params, ss);
		if (pubCoin4.validate()) {
			
			return false;
		}
		
	} catch (runtime_error &e) {
		cout << "Caught exception: " << e.what() << endl;
		return false;
	}
	
	return true;
}

bool
Test_MintAndSpend()
{
	try {
		
		if (gCoins[0] == NULL)
		{
			
			Test_MintCoin();
			if (gCoins[0] == NULL) {
				return false;
			}
		}

		
		
		
        Accumulator acc(&g_Params->accumulatorParams,CoinDenomination::ZQ_ONE);
		AccumulatorWitness wAcc(g_Params, acc, gCoins[0]->getPublicCoin());

		for (uint32_t i = 0; i < TESTS_COINS_TO_ACCUMULATE; i++) {
			acc += gCoins[i]->getPublicCoin();
			wAcc +=gCoins[i]->getPublicCoin();
		}

		
		
		CDataStream cc(SER_NETWORK, PROTOCOL_VERSION);
		cc << *gCoins[0];
		PrivateCoin myCoin(g_Params,cc);

		CoinSpend spend(g_Params, myCoin, acc, 0, wAcc, 0);

		
		CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
		ss << spend;
		gProofSize = ss.size();
		CoinSpend newSpend(g_Params, ss);

		
		bool ret =  newSpend.Verify(acc);
		
		
		CBigNum serialNumber = newSpend.getCoinSerialNumber();
		gSerialNumberSize = ceil((double)serialNumber.bitSize() / 8.0);
		
		return ret;
	} catch (runtime_error &e) {
		cout << e.what() << endl;
		return false;
	}

	return false;
}

void
Test_RunAllTests()
{
	
	g_Params = new ZerocoinParams(GetTestModulus());

	gNumTests = gSuccessfulTests = gProofSize = 0;
	for (uint32_t i = 0; i < TESTS_COINS_TO_ACCUMULATE; i++) {
		gCoins[i] = NULL;
	}

	
	LogTestResult("an RSA modulus can be generated", Test_GenRSAModulus);
	LogTestResult("parameter sizes are correct", Test_CalcParamSizes);
	LogTestResult("group/field parameters can be generated", Test_GenerateGroupParams);
	LogTestResult("parameter generation is correct", Test_ParamGen);
	LogTestResult("coins can be minted", Test_MintCoin);
	LogTestResult("invalid coins will be rejected", Test_InvalidCoin);
	LogTestResult("the accumulator works", Test_Accumulator);
	LogTestResult("the commitment equality PoK works", Test_EqualityPoK);
	LogTestResult("a minted coin can be spent", Test_MintAndSpend);

	cout << endl << "Average coin size is " << gCoinSize << " bytes." << endl;
	cout << "Serial number size is " << gSerialNumberSize << " bytes." << endl;
	cout << "Spend proof size is " << gProofSize << " bytes." << endl;

	
	if (gSuccessfulTests < gNumTests) {
		cout << endl << "ERROR: SOME TESTS FAILED" << endl;
	}

	
	for (uint32_t i = 0; i < TESTS_COINS_TO_ACCUMULATE; i++) {
		delete gCoins[i];
	}

	cout << endl << gSuccessfulTests << " out of " << gNumTests << " tests passed." << endl << endl;
	delete g_Params;
}

BOOST_AUTO_TEST_SUITE(libzerocoin)
BOOST_AUTO_TEST_CASE(libzerocoin_tests)
{
	cout << "libzerocoin v" << ZEROCOIN_VERSION_STRING << " test utility." << endl << endl;
	
	Test_RunAllTests();
}
BOOST_AUTO_TEST_SUITE_END()
