





#ifndef STORAGE_LEVELDB_DB_FILENAME_H_
#define STORAGE_LEVELDB_DB_FILENAME_H_

#include <stdint.h>
#include <string>
#include "leveldb/slice.h"
#include "leveldb/status.h"
#include "port/port.h"

namespace leveldb {

class Env;

enum FileType {
  kLogFile,
  kDBLockFile,
  kTableFile,
  kDescriptorFile,
  kCurrentFile,
  kTempFile,
  kInfoLogFile  
};




extern std::string LogFileName(const std::string& dbname, uint64_t number);




extern std::string TableFileName(const std::string& dbname, uint64_t number);




extern std::string SSTTableFileName(const std::string& dbname, uint64_t number);




extern std::string DescriptorFileName(const std::string& dbname,
                                      uint64_t number);




extern std::string CurrentFileName(const std::string& dbname);



extern std::string LockFileName(const std::string& dbname);



extern std::string TempFileName(const std::string& dbname, uint64_t number);


extern std::string InfoLogFileName(const std::string& dbname);


extern std::string OldInfoLogFileName(const std::string& dbname);




extern bool ParseFileName(const std::string& filename,
                          uint64_t* number,
                          FileType* type);



extern Status SetCurrentFile(Env* env, const std::string& dbname,
                             uint64_t descriptor_number);


}  

#endif  
