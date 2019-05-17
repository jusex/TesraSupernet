


#define BOOST_TEST_MODULE Zerocoin Test Suite
#define BOOST_TEST_MAIN

#include "libzerocoin/Denominations.h"
#include "amount.h"
#include "chainparams.h"
#include "main.h"
#include "txdb.h"

#include <boost/test/unit_test.hpp>
#include <iostream>

struct ZeroSetup {
    ZeroSetup() {
        std::cout << "global setup\n";
    }
    ~ZeroSetup()
    {
        std::cout << "global teardown\n";
    }
};

BOOST_GLOBAL_FIXTURE(ZeroSetup);

