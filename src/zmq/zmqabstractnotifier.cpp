



#include "zmqabstractnotifier.h"
#include "util.h"


CZMQAbstractNotifier::~CZMQAbstractNotifier()
{
    assert(!psocket);
}

bool CZMQAbstractNotifier::NotifyBlock(const CBlockIndex * )
{
    return true;
}

bool CZMQAbstractNotifier::NotifyTransaction(const CTransaction &)
{
    return true;
}

bool CZMQAbstractNotifier::NotifyTransactionLock(const CTransaction &)
{
    return true;
}
