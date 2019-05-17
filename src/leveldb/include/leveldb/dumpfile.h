



#ifndef STORAGE_LEVELDB_INCLUDE_DUMPFILE_H_
#define STORAGE_LEVELDB_INCLUDE_DUMPFILE_H_

#include <string>
#include "leveldb/env.h"
#include "leveldb/status.h"

namespace leveldb {








Status DumpFile(Env* env, const std::string& fname, WritableFile* dst);

}  

#endif  
