



#ifndef STORAGE_LEVELDB_UTIL_CRC32C_H_
#define STORAGE_LEVELDB_UTIL_CRC32C_H_

#include <stddef.h>
#include <stdint.h>

namespace leveldb {
namespace crc32c {




extern uint32_t Extend(uint32_t init_crc, const char* data, size_t n);


inline uint32_t Value(const char* data, size_t n) {
  return Extend(0, data, n);
}

static const uint32_t kMaskDelta = 0xa282ead8ul;






inline uint32_t Mask(uint32_t crc) {
  
  return ((crc >> 15) | (crc << 17)) + kMaskDelta;
}


inline uint32_t Unmask(uint32_t masked_crc) {
  uint32_t rot = masked_crc - kMaskDelta;
  return ((rot >> 17) | (rot << 15));
}

}  
}  

#endif  
