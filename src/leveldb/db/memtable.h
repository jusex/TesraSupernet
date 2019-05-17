



#ifndef STORAGE_LEVELDB_DB_MEMTABLE_H_
#define STORAGE_LEVELDB_DB_MEMTABLE_H_

#include <string>
#include "leveldb/db.h"
#include "db/dbformat.h"
#include "db/skiplist.h"
#include "util/arena.h"

namespace leveldb {

class InternalKeyComparator;
class Mutex;
class MemTableIterator;

class MemTable {
 public:
  
  
  explicit MemTable(const InternalKeyComparator& comparator);

  
  void Ref() { ++refs_; }

  
  void Unref() {
    --refs_;
    assert(refs_ >= 0);
    if (refs_ <= 0) {
      delete this;
    }
  }

  
  
  
  
  
  size_t ApproximateMemoryUsage();

  
  
  
  
  
  
  Iterator* NewIterator();

  
  
  
  void Add(SequenceNumber seq, ValueType type,
           const Slice& key,
           const Slice& value);

  
  
  
  
  bool Get(const LookupKey& key, std::string* value, Status* s);

 private:
  ~MemTable();  

  struct KeyComparator {
    const InternalKeyComparator comparator;
    explicit KeyComparator(const InternalKeyComparator& c) : comparator(c) { }
    int operator()(const char* a, const char* b) const;
  };
  friend class MemTableIterator;
  friend class MemTableBackwardIterator;

  typedef SkipList<const char*, KeyComparator> Table;

  KeyComparator comparator_;
  int refs_;
  Arena arena_;
  Table table_;

  
  MemTable(const MemTable&);
  void operator=(const MemTable&);
};

}  

#endif  
