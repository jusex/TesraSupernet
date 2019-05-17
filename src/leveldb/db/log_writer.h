



#ifndef STORAGE_LEVELDB_DB_LOG_WRITER_H_
#define STORAGE_LEVELDB_DB_LOG_WRITER_H_

#include <stdint.h>
#include "db/log_format.h"
#include "leveldb/slice.h"
#include "leveldb/status.h"

namespace leveldb {

class WritableFile;

namespace log {

class Writer {
 public:
  
  
  
  explicit Writer(WritableFile* dest);
  ~Writer();

  Status AddRecord(const Slice& slice);

 private:
  WritableFile* dest_;
  int block_offset_;       

  
  
  
  uint32_t type_crc_[kMaxRecordType + 1];

  Status EmitPhysicalRecord(RecordType type, const char* ptr, size_t length);

  
  Writer(const Writer&);
  void operator=(const Writer&);
};

}  
}  

#endif  
