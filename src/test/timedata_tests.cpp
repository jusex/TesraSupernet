



#include "timedata.h"

#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_AUTO_TEST_SUITE(timedata_tests)

BOOST_AUTO_TEST_CASE(util_MedianFilter)
{
    CMedianFilter<int> filter(5, 15);

    BOOST_CHECK_EQUAL(filter.median(), 15);

    filter.input(20); 
    BOOST_CHECK_EQUAL(filter.median(), 17);

    filter.input(30); 
    BOOST_CHECK_EQUAL(filter.median(), 20);

    filter.input(3); 
    BOOST_CHECK_EQUAL(filter.median(), 17);

    filter.input(7); 
    BOOST_CHECK_EQUAL(filter.median(), 15);

    filter.input(18); 
    BOOST_CHECK_EQUAL(filter.median(), 18);

    filter.input(0); 
    BOOST_CHECK_EQUAL(filter.median(), 7);
}

BOOST_AUTO_TEST_SUITE_END()
