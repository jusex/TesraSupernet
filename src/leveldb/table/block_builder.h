



#ifndef STORAGE_LEVELDB_TABLE_BLOCK_BUILDER_H_
#define STORAGE_LEVELDB_TABLE_BLOCK_BUILDER_H_

#include <vector>

#include <stdint.h>
#include "leveldb/slice.h"

namespace leveldb {

struct Options;

class BlockBuilder {
 public:
  explicit BlockBuilder(const Options* options);

  
  void Reset();

  
  
  void Add(const Slice& key, const Slice& value);

  
  
  
  Slice Finish();

  
  
  size_t CurrentSizeEstimate() const;

  
  bool empty() const {
    return buffer_.empty();
  }

 private:
  const Options*        options_;
  std::string           buffer_;      
  std::vector<uint32_t> restarts_;    
  int                   counter_;     
  bool                  finished_;    
  std::string           last_key_;

  
  BlockBuilder(const BlockBuilder&);
  void operator=(const BlockBuilder&);
};

}  

#endif  
