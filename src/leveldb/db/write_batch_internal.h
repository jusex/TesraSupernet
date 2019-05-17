



#ifndef STORAGE_LEVELDB_DB_WRITE_BATCH_INTERNAL_H_
#define STORAGE_LEVELDB_DB_WRITE_BATCH_INTERNAL_H_

#include "leveldb/write_batch.h"

namespace leveldb {

class MemTable;



class WriteBatchInternal {
 public:
  
  static int Count(const WriteBatch* batch);

  
  static void SetCount(WriteBatch* batch, int n);

  
  static SequenceNumber Sequence(const WriteBatch* batch);

  
  
  static void SetSequence(WriteBatch* batch, SequenceNumber seq);

  static Slice Contents(const WriteBatch* batch) {
    return Slice(batch->rep_);
  }

  static size_t ByteSize(const WriteBatch* batch) {
    return batch->rep_.size();
  }

  static void SetContents(WriteBatch* batch, const Slice& contents);

  static Status InsertInto(const WriteBatch* batch, MemTable* memtable);

  static void Append(WriteBatch* dst, const WriteBatch* src);
};

}  


#endif  
