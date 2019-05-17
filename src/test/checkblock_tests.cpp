









#include "clientversion.h"
#include "main.h"
#include "utiltime.h"

#include <cstdio>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE(CheckBlock_tests)

bool read_block(const std::string& filename, CBlock& block)
{
    namespace fs = boost::filesystem;
    fs::path testFile = fs::current_path() / "data" / filename;
#ifdef TEST_DATA_DIR
    if (!fs::exists(testFile))
    {
        testFile = fs::path(BOOST_PP_STRINGIZE(TEST_DATA_DIR)) / filename;
    }
#endif
    FILE* fp = fopen(testFile.string().c_str(), "rb");
    if (!fp) return false;

    fseek(fp, 8, SEEK_SET); 

    CAutoFile filein(fp, SER_DISK, CLIENT_VERSION);
    if (filein.IsNull()) return false;

    filein >> block;

    return true;
}

BOOST_AUTO_TEST_CASE(May15)
{
    
    
    
    
    unsigned int tMay15 = 1368576000;
    SetMockTime(tMay15); 

    CBlock forkingBlock;
    if (read_block("Mar12Fork.dat", forkingBlock))
    {
        CValidationState state;

        
        forkingBlock.nTime = tMay15; 
        BOOST_CHECK(CheckBlock(forkingBlock, state, false, false));
    }

    SetMockTime(0);
}

BOOST_AUTO_TEST_SUITE_END()
