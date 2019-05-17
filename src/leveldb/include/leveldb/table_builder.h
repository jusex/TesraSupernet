











#ifndef STORAGE_LEVELDB_INCLUDE_TABLE_BUILDER_H_
#define STORAGE_LEVELDB_INCLUDE_TABLE_BUILDER_H_

#include <stdint.h>
#include "leveldb/options.h"
#include "leveldb/status.h"

namespace leveldb {

class BlockBuilder;
class BlockHandle;
class WritableFile;

class TableBuilder {
 public:
  
  
  
  TableBuilder(const Options& options, WritableFile* file);

  
  ~TableBuilder();

  
  
  
  
  
  
  Status ChangeOptions(const Options& options);

  
  
  
  void Add(const Slice& key, const Slice& value);

  
  
  
  
  void Flush();

  
  Status status() const;

  
  
  
  Status Finish();

  
  
  
  
  
  void Abandon();

  
  uint64_t NumEntries() const;

  
  
  uint64_t FileSize() const;

 private:
  bool ok() const { return status().ok(); }
  void WriteBlock(BlockBuilder* block, BlockHandle* handle);
  void WriteRawBlock(const Slice& data, CompressionType, BlockHandle* handle);

  struct Rep;
  Rep* rep_;

  
  TableBuilder(const TableBuilder&);
  void operator=(const TableBuilder&);
};

}  

#endif  
