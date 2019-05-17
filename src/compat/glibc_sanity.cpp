



#if defined(HAVE_CONFIG_H)
#include "config/tesra-config.h"
#endif

#include <cstddef>

#if defined(HAVE_SYS_SELECT_H)
#include <sys/select.h>
#endif

extern "C" void* memcpy(void* a, const void* b, size_t c);
void* memcpy_int(void* a, const void* b, size_t c)
{
    return memcpy(a, b, c);
}

namespace
{





template <unsigned int T>
bool sanity_test_memcpy()
{
    unsigned int memcpy_test[T];
    unsigned int memcpy_verify[T] = {};
    for (unsigned int i = 0; i != T; ++i)
        memcpy_test[i] = i;

    memcpy_int(memcpy_verify, memcpy_test, sizeof(memcpy_test));

    for (unsigned int i = 0; i != T; ++i) {
        if (memcpy_verify[i] != i)
            return false;
    }
    return true;
}

#if defined(HAVE_SYS_SELECT_H)




bool sanity_test_fdelt()
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return FD_ISSET(0, &fds);
}
#endif

} 

bool glibc_sanity_test()
{
#if defined(HAVE_SYS_SELECT_H)
    if (!sanity_test_fdelt())
        return false;
#endif
    return sanity_test_memcpy<1025>();
}