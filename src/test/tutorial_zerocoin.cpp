/**
* @file       tutorial.cpp
*
* @brief      Simple tutorial program to illustrate Zerocoin usage.
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
#include "streams.h"
#include "libzerocoin/bignum.h"
#include "libzerocoin/ParamGeneration.h"
#include "libzerocoin/Denominations.h"
#include "libzerocoin/Coin.h"
#include "libzerocoin/CoinSpend.h"
#include "libzerocoin/Accumulator.h"

using namespace std;

#define COINS_TO_ACCUMULATE     5
#define DUMMY_TRANSACTION_HASH  0 
#define DUMMY_ACCUMULATOR_ID    0 




#define TUTORIAL_TEST_MODULUS   "a8852ebf7c49f01cd196e35394f3b74dd86283a07f57e0a262928e7493d4a3961d93d93c90ea3369719641d626d28b9cddc6d9307b9aabdbffc40b6d6da2e329d079b4187ff784b2893d9f53e9ab913a04ff02668114695b07d8ce877c4c8cac1b12b9beff3c51294ebe349eca41c24cd32a6d09dd1579d3947e5c4dcc30b2090b0454edb98c6336e7571db09e0fdafbd68d8f0470223836e90666a5b143b73b9cd71547c917bf24c0efc86af2eba046ed781d9acb05c80f007ef5a0a5dfca23236f37e698e8728def12554bc80f294f71c040a88eff144d130b24211016a97ce0f5fe520f477e555c9997683d762aff8bd1402ae6938dd5c994780b1bf6aa7239e9d8101630ecfeaa730d2bbc97d39beb057f016db2e28bf12fab4989c0170c2593383fd04660b5229adcd8486ba78f6cc1b558bcd92f344100dff239a8c00dbc4c2825277f24bdd04475bcc9a8c39fd895eff97c1967e434effcb9bd394e0577f4cf98c30d9e6b54cd47d6e447dcf34d67e48e4421691dbe4a7d9bd503abb9"










bool
ZerocoinTutorial()
{
	
	
	
	
	

	
	

	try {

		
		
		
		
		
		
		
		

		
		CBigNum testModulus;
		testModulus.SetHex(std::string(TUTORIAL_TEST_MODULUS));

		
		libzerocoin::ZerocoinParams* params = new libzerocoin::ZerocoinParams(testModulus);

		cout << "Successfully loaded parameters." << endl;

		
		
		
		
		
		
		
		
		
		

		
		
		
		
        libzerocoin::PrivateCoin newCoin(params, libzerocoin::CoinDenomination::ZQ_ONE);

		
		
		
		libzerocoin::PublicCoin pubCoin = newCoin.getPublicCoin();

		cout << "Successfully minted a zerocoin." << endl;

		
		CDataStream serializedCoin(SER_NETWORK, PROTOCOL_VERSION);
		serializedCoin << pubCoin;

		
		
		
		
		
		
		
		
		
		
		
		

		
		libzerocoin::PublicCoin pubCoinNew(params, serializedCoin);

		
		if (!pubCoinNew.validate()) {
			
			
			
			cout << "Error: coin is not valid!";
		}
		
		cout << "Deserialized and verified the coin." << endl;

		
		
		
		
		
		
		
		
		
		
		
		
		

		
        libzerocoin::Accumulator accumulator(params,libzerocoin::CoinDenomination::ZQ_ONE);

		
		for (uint32_t i = 0; i < COINS_TO_ACCUMULATE; i++) {
            libzerocoin::PrivateCoin testCoin(params, libzerocoin::CoinDenomination::ZQ_ONE);
			accumulator += testCoin.getPublicCoin();
		}

		
		
		
		
		
		
		
		
		
		CDataStream serializedAccumulator(SER_NETWORK, PROTOCOL_VERSION);
		serializedAccumulator << accumulator;

		
		libzerocoin::Accumulator newAccumulator(params, serializedAccumulator);

		
		
		
		newAccumulator += pubCoinNew;

		cout << "Successfully accumulated coins." << endl;

		
		
		
		
		
		
		
		
		
		
		
		
		

		
		
		
		
		
		
		
		
		libzerocoin::AccumulatorWitness witness(params, accumulator, newCoin.getPublicCoin());

		
		accumulator += newCoin.getPublicCoin();

		
		
		

		
		
        libzerocoin::CoinSpend spend(params, newCoin, accumulator, 0, witness, 0);

		
		
		if (!spend.Verify(accumulator)) {
			cout << "ERROR: Our new CoinSpend transaction did not verify!" << endl;
			return false;
		}
		
		
		CDataStream serializedCoinSpend(SER_NETWORK, PROTOCOL_VERSION);
		serializedCoinSpend << spend;
		
		cout << "Successfully generated a coin spend transaction." << endl;

		
		
		
		
		
		
		
		
		
		

		
		libzerocoin::CoinSpend newSpend(params, serializedCoinSpend);

		
		
		
		
		
		
		
		
		
		
		if (!newSpend.Verify(accumulator)) {
			cout << "ERROR: The CoinSpend transaction did not verify!" << endl;
			return false;
		}

		
		
		
		
		CBigNum serialNumber = newSpend.getCoinSerialNumber();

		cout << "Successfully verified a coin spend transaction." << endl;
		cout << endl << "Coin serial number is:" << endl << serialNumber << endl;

		
		return true;

	} catch (runtime_error &e) {
		cout << e.what() << endl;
		return false;
	}

	return false;
}

BOOST_AUTO_TEST_SUITE(tutorial_libzerocoin)
BOOST_AUTO_TEST_CASE(tutorial_libzerocoin_tests)
{
	cout << "libzerocoin v" << ZEROCOIN_VERSION_STRING << " tutorial." << endl << endl;

	ZerocoinTutorial();
}
BOOST_AUTO_TEST_SUITE_END()
