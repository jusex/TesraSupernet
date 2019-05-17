






#ifndef STORAGE_LEVELDB_UTIL_LOGGING_H_
#define STORAGE_LEVELDB_UTIL_LOGGING_H_

#include <stdio.h>
#include <stdint.h>
#include <string>
#include "port/port.h"

namespace leveldb {

class Slice;
class WritableFile;


extern void AppendNumberTo(std::string* str, uint64_t num);



extern void AppendEscapedStringTo(std::string* str, const Slice& value);


extern std::string NumberToString(uint64_t num);



extern std::string EscapeString(const Slice& value);





extern bool ConsumeDecimalNumber(Slice* in, uint64_t* val);

}  

#endif  
