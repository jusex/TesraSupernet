



#if defined(HAVE_CONFIG_H)
#include "config/tesra-config.h"
#endif

#include <cstring>


size_t strnlen_int( const char *start, size_t max_len)
{
    const char *end = (const char *)memchr(start, '\0', max_len);

    return end ? (size_t)(end - start) : max_len;
}
