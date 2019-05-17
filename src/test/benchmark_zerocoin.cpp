/**
 * @file       Benchmark.cpp
 *
 * @brief      Benchmarking tests for Zerocoin.
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

#include <exception>
#include <cstdlib>
#include <sys/time.h>
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

#define TESTS_COINS_TO_ACCUMULATE   50


uint32_t    ggNumTests        = 0;
uint32_t    ggSuccessfulTests = 0;


PrivateCoin    *ggCoins[TESTS_COINS_TO_ACCUMULATE];


ZerocoinParams *gg_Params;





class Timer
{
	timeval timer[2];

public:

	timeval start()
	{
		gettimeofday(&this->timer[0], NULL);
		return this->timer[0];
	}

	timeval stop()
	{
		gettimeofday(&this->timer[1], NULL);
		return this->timer[1];
	}

	int duration() const
	{
		int secs(this->timer[1].tv_sec - this->timer[0].tv_sec);
		int usecs(this->timer[1].tv_usec - this->timer[0].tv_usec);

		if(usecs < 0)
		{
			--secs;
			usecs += 1000000;
		}

		return static_cast<int>(secs * 1000 + usecs / 1000.0 + 0.5);
	}
};


Timer timer;

void
gLogTestResult(string testName, bool (*testPtr)())
{
	string colorGreen(COLOR_STR_GREEN);
	string colorNormal(COLOR_STR_NORMAL);
	string colorRed(COLOR_STR_RED);

	cout << "Testing if " << testName << "..." << endl;

	bool testResult = testPtr();

	if (testResult == true) {
		cout << "\t" << colorGreen << "[PASS]"  << colorNormal << endl;
		ggSuccessfulTests++;
	} else {
		cout << colorRed << "\t[FAIL]" << colorNormal << endl;
	}

	ggNumTests++;
}

CBigNum
gGetTestModulus()
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
Testb_GenRSAModulus()
{
	CBigNum result = gGetTestModulus();

	if (!result) {
		return false;
	}
	else {
		return true;
	}
}

bool
Testb_CalcParamSizes()
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
Testb_GenerateGroupParams()
{
	uint32_t pLen = 1024, qLen = 256, count;
	IntegerGroupParams group;

	for (count = 0; count < 1; count++) {

		try {
			group = deriveIntegerGroupParams(calculateSeed(gGetTestModulus(), "test", ZEROCOIN_DEFAULT_SECURITYLEVEL, "TEST GROUP"), pLen, qLen);
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
Testb_ParamGen()
{
	bool result = true;

	try {
		timer.start();
		
		ZerocoinParams testParams(gGetTestModulus(),ZEROCOIN_DEFAULT_SECURITYLEVEL);
		timer.stop();

		cout << "\tPARAMGEN ELAPSED TIME: " << timer.duration() << " ms\t" << timer.duration()*0.001 << " s" << endl;
	} catch (runtime_error e) {
		cout << e.what() << endl;
		result = false;
	}

	return result;
}

bool
Testb_Accumulator()
{
	
	
	if (ggCoins[0] == NULL) {
		return false;
	}
	try {
		
            Accumulator accOne(&gg_Params->accumulatorParams,libzerocoin::CoinDenomination::ZQ_ONE);
            Accumulator accTwo(&gg_Params->accumulatorParams,libzerocoin::CoinDenomination::ZQ_ONE);
            Accumulator accThree(&gg_Params->accumulatorParams,libzerocoin::CoinDenomination::ZQ_ONE);
            Accumulator accFour(&gg_Params->accumulatorParams,libzerocoin::CoinDenomination::ZQ_ONE);
		AccumulatorWitness wThree(gg_Params, accThree, ggCoins[0]->getPublicCoin());

		for (uint32_t i = 0; i < TESTS_COINS_TO_ACCUMULATE; i++) {
			accOne += ggCoins[i]->getPublicCoin();
			accTwo += ggCoins[TESTS_COINS_TO_ACCUMULATE - (i+1)]->getPublicCoin();
			accThree += ggCoins[i]->getPublicCoin();
			wThree += ggCoins[i]->getPublicCoin();
			if(i != 0) {
				accFour += ggCoins[i]->getPublicCoin();
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

		
		if (!wThree.VerifyWitness(accThree, ggCoins[0]->getPublicCoin()) ) {
			cout << "Witness not valid" << endl;
			return false;
		}

	} catch (runtime_error e) {
		cout << e.what() << endl;
        return false;
	}

	return true;
}

bool
Testb_MintCoin()
{
	try {
		
		timer.start();
		for (uint32_t i = 0; i < TESTS_COINS_TO_ACCUMULATE; i++) {
            ggCoins[i] = new PrivateCoin(gg_Params,CoinDenomination::ZQ_ONE);
		}
		timer.stop();
	} catch (exception &e) {
		return false;
	}

	cout << "\tMINT ELAPSED TIME:\n\t\tTotal: " << timer.duration() << " ms\t" << timer.duration()*0.001 << " s\n\t\tPer Coin: " << timer.duration()/TESTS_COINS_TO_ACCUMULATE << " ms\t" << (timer.duration()/TESTS_COINS_TO_ACCUMULATE)*0.001 << " s" << endl;

	return true;
}

bool
Testb_MintAndSpend()
{
	try {
		
		if (ggCoins[0] == NULL)
		{
			
			Testb_MintCoin();
			if (ggCoins[0] == NULL) {
				return false;
			}
		}

		
		
		
        Accumulator acc(&gg_Params->accumulatorParams,CoinDenomination::ZQ_ONE);
		AccumulatorWitness wAcc(gg_Params, acc, ggCoins[0]->getPublicCoin());

		timer.start();
		for (uint32_t i = 0; i < TESTS_COINS_TO_ACCUMULATE; i++) {
			acc += ggCoins[i]->getPublicCoin();
		}
		timer.stop();

		cout << "\tACCUMULATOR ELAPSED TIME:\n\t\tTotal: " << timer.duration() << " ms\t" << timer.duration()*0.001 << " s\n\t\tPer Element: " << timer.duration()/TESTS_COINS_TO_ACCUMULATE << " ms\t" << (timer.duration()/TESTS_COINS_TO_ACCUMULATE)*0.001 << " s" << endl;

		timer.start();
		for (uint32_t i = 0; i < TESTS_COINS_TO_ACCUMULATE; i++) {
			wAcc +=ggCoins[i]->getPublicCoin();
		}
		timer.stop();

		cout << "\tWITNESS ELAPSED TIME: \n\t\tTotal: " << timer.duration() << " ms\t" << timer.duration()*0.001 << " s\n\t\tPer Element: " << timer.duration()/TESTS_COINS_TO_ACCUMULATE << " ms\t" << (timer.duration()/TESTS_COINS_TO_ACCUMULATE)*0.001 << " s" << endl;

		
		timer.start();
		CoinSpend spend(gg_Params, *(ggCoins[0]), acc, 0, wAcc, 0); 
		timer.stop();

		cout << "\tSPEND ELAPSED TIME: " << timer.duration() << " ms\t" << timer.duration()*0.001 << " s" << endl;

		
		CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);

		timer.start();
		ss << spend;
		timer.stop();

		CoinSpend newSpend(gg_Params, ss);

		cout << "\tSERIALIZE ELAPSED TIME: " << timer.duration() << " ms\t" << timer.duration()*0.001 << " s" << endl;

		
		timer.start();
		bool ret = newSpend.Verify(acc);
		timer.stop();

		cout << "\tSPEND VERIFY ELAPSED TIME: " << timer.duration() << " ms\t" << timer.duration()*0.001 << " s" << endl;

		return ret;
	} catch (runtime_error &e) {
		cout << e.what() << endl;
		return false;
	}

	return false;
}

void
Testb_RunAllTests()
{
	
	gg_Params = new ZerocoinParams(gGetTestModulus());

	ggNumTests = ggSuccessfulTests = 0;
	for (uint32_t i = 0; i < TESTS_COINS_TO_ACCUMULATE; i++) {
		ggCoins[i] = NULL;
	}

	
	gLogTestResult("an RSA modulus can be generated", Testb_GenRSAModulus);
	gLogTestResult("parameter sizes are correct", Testb_CalcParamSizes);
	gLogTestResult("group/field parameters can be generated", Testb_GenerateGroupParams);
	gLogTestResult("parameter generation is correct", Testb_ParamGen);
	gLogTestResult("coins can be minted", Testb_MintCoin);
	gLogTestResult("the accumulator works", Testb_Accumulator);
	gLogTestResult("a minted coin can be spent", Testb_MintAndSpend);

	
	if (ggSuccessfulTests < ggNumTests) {
		cout << endl << "ERROR: SOME TESTS FAILED" << endl;
	}

	
	for (uint32_t i = 0; i < TESTS_COINS_TO_ACCUMULATE; i++) {
		delete ggCoins[i];
	}

	cout << ggSuccessfulTests << " out of " << ggNumTests << " tests passed." << endl << endl;
	delete gg_Params;
}
BOOST_AUTO_TEST_SUITE(benchmark_zerocoin)

BOOST_AUTO_TEST_CASE(benchmark_test)
{
	cout << "libzerocoin v" << ZEROCOIN_VERSION_STRING << " benchmark utility." << endl << endl;

	Testb_RunAllTests();
}
BOOST_AUTO_TEST_SUITE_END()

