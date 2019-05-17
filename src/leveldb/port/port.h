



#ifndef STORAGE_LEVELDB_PORT_PORT_H_
#define STORAGE_LEVELDB_PORT_PORT_H_

#include <string.h>




#if defined(LEVELDB_PLATFORM_POSIX)
#  include "port/port_posix.h"
#elif defined(LEVELDB_PLATFORM_CHROMIUM)
#  include "port/port_chromium.h"
#elif defined(LEVELDB_PLATFORM_WINDOWS)
#  include "port/port_win.h"
#endif

#endif  
