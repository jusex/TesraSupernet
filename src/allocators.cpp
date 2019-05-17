



#include "allocators.h"

#ifdef WIN32
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN 1
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>




#else
#include <limits.h> 
#include <sys/mman.h>
#include <unistd.h> 
#endif

LockedPageManager* LockedPageManager::_instance = NULL;
boost::once_flag LockedPageManager::init_flag = BOOST_ONCE_INIT;


static inline size_t GetSystemPageSize()
{
    size_t page_size;
#if defined(WIN32)
    SYSTEM_INFO sSysInfo;
    GetSystemInfo(&sSysInfo);
    page_size = sSysInfo.dwPageSize;
#elif defined(PAGESIZE) 
    page_size = PAGESIZE;
#else                   
    page_size = sysconf(_SC_PAGESIZE);
#endif
    return page_size;
}

bool MemoryPageLocker::Lock(const void* addr, size_t len)
{
#ifdef WIN32
    return VirtualLock(const_cast<void*>(addr), len) != 0;
#else
    return mlock(addr, len) == 0;
#endif
}

bool MemoryPageLocker::Unlock(const void* addr, size_t len)
{
#ifdef WIN32
    return VirtualUnlock(const_cast<void*>(addr), len) != 0;
#else
    return munlock(addr, len) == 0;
#endif
}

LockedPageManager::LockedPageManager() : LockedPageManagerBase<MemoryPageLocker>(GetSystemPageSize())
{
}
