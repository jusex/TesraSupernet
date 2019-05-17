



#include "merkleblock.h"
#include "serialize.h"
#include "streams.h"
#include "uint256.h"
#include "version.h"

#include <vector>

#include <boost/test/unit_test.hpp>

using namespace std;

class CPartialMerkleTreeTester : public CPartialMerkleTree
{
public:
    
    void Damage() {
        unsigned int n = rand() % vHash.size();
        int bit = rand() % 256;
        uint256 &hash = vHash[n];
        hash ^= ((uint256)1 << bit);
    }
};

BOOST_AUTO_TEST_SUITE(pmt_tests)

BOOST_AUTO_TEST_CASE(pmt_test1)
{
    static const unsigned int nTxCounts[] = {1, 4, 7, 17, 56, 100, 127, 256, 312, 513, 1000, 4095};

    for (int n = 0; n < 12; n++) {
        unsigned int nTx = nTxCounts[n];

        
        CBlock block;
        for (unsigned int j=0; j<nTx; j++) {
            CMutableTransaction tx;
            tx.nLockTime = rand(); 
            block.vtx.push_back(CTransaction(tx));
        }

        
        uint256 merkleRoot1 = block.BuildMerkleTree();
        std::vector<uint256> vTxid(nTx, 0);
        for (unsigned int j=0; j<nTx; j++)
            vTxid[j] = block.vtx[j].GetHash();
        int nHeight = 1, nTx_ = nTx;
        while (nTx_ > 1) {
            nTx_ = (nTx_+1)/2;
            nHeight++;
        }

        
        for (int att = 1; att < 15; att++) {
            
            std::vector<bool> vMatch(nTx, false);
            std::vector<uint256> vMatchTxid1;
            for (unsigned int j=0; j<nTx; j++) {
                bool fInclude = (rand() & ((1 << (att/2)) - 1)) == 0;
                vMatch[j] = fInclude;
                if (fInclude)
                    vMatchTxid1.push_back(vTxid[j]);
            }

            
            CPartialMerkleTree pmt1(vTxid, vMatch);

            
            CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
            ss << pmt1;

            
            unsigned int n = std::min<unsigned int>(nTx, 1 + vMatchTxid1.size()*nHeight);
            BOOST_CHECK(ss.size() <= 10 + (258*n+7)/8);

            
            CPartialMerkleTreeTester pmt2;
            ss >> pmt2;

            
            std::vector<uint256> vMatchTxid2;
            uint256 merkleRoot2 = pmt2.ExtractMatches(vMatchTxid2);

            
            BOOST_CHECK(merkleRoot1 == merkleRoot2);
            BOOST_CHECK(merkleRoot2 != 0);

            
            BOOST_CHECK(vMatchTxid1 == vMatchTxid2);

            
            for (int j=0; j<4; j++) {
                CPartialMerkleTreeTester pmt3(pmt2);
                pmt3.Damage();
                std::vector<uint256> vMatchTxid3;
                uint256 merkleRoot3 = pmt3.ExtractMatches(vMatchTxid3);
                BOOST_CHECK(merkleRoot3 != merkleRoot1);
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
