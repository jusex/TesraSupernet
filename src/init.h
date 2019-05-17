




#ifndef BITCOIN_INIT_H
#define BITCOIN_INIT_H

#include <string>

class CWallet;

namespace boost
{
class thread_group;
} 

extern CWallet* pwalletMain;

void StartShutdown();
bool ShutdownRequested();
void Shutdown();
void PrepareShutdown();
bool AppInit2(boost::thread_group& threadGroup);


enum HelpMessageMode {
    HMM_BITCOIND,
    HMM_BITCOIN_QT
};


std::string HelpMessage(HelpMessageMode mode);

std::string LicenseInfo();

#endif 
