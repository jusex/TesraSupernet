

#include <stddef.h>
#define b64_encode_len(A) ((A+2)/3 * 4 + 1)
#define b64_decode_len(A) (A / 4 * 3 + 2)

int	libscrypt_b64_encode(unsigned char const *src, size_t srclength, 
         char *target, size_t targetsize);
int	libscrypt_b64_decode(char const *src,  unsigned char *target, 
        size_t targetsize);
