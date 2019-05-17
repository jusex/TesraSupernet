



#ifndef STORAGE_LEVELDB_TABLE_MERGER_H_
#define STORAGE_LEVELDB_TABLE_MERGER_H_

namespace leveldb {

class Comparator;
class Iterator;









extern Iterator* NewMergingIterator(
    const Comparator* comparator, Iterator** children, int n);

}  

#endif  
