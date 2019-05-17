



#ifndef STORAGE_LEVELDB_TABLE_TWO_LEVEL_ITERATOR_H_
#define STORAGE_LEVELDB_TABLE_TWO_LEVEL_ITERATOR_H_

#include "leveldb/iterator.h"

namespace leveldb {

struct ReadOptions;










extern Iterator* NewTwoLevelIterator(
    Iterator* index_iter,
    Iterator* (*block_function)(
        void* arg,
        const ReadOptions& options,
        const Slice& index_value),
    void* arg,
    const ReadOptions& options);

}  

#endif  
