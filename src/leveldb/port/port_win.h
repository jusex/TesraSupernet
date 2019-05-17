





























#ifndef STORAGE_LEVELDB_PORT_PORT_WIN_H_
#define STORAGE_LEVELDB_PORT_PORT_WIN_H_

#ifdef _MSC_VER
#define snprintf _snprintf
#define close _close
#define fread_unlocked _fread_nolock
#endif

#include <string>
#include <stdint.h>
#ifdef SNAPPY
#include <snappy.h>
#endif

namespace leveldb {
namespace port {


static const bool kLittleEndian = true;

class CondVar;

class Mutex {
 public:
  Mutex();
  ~Mutex();

  void Lock();
  void Unlock();
  void AssertHeld();

 private:
  friend class CondVar;
  
  
  
  void * cs_;

  
  Mutex(const Mutex&);
  void operator=(const Mutex&);
};





class CondVar {
 public:
  explicit CondVar(Mutex* mu);
  ~CondVar();
  void Wait();
  void Signal();
  void SignalAll();
 private:
  Mutex* mu_;
  
  Mutex wait_mtx_;
  long waiting_;
  
  void * sem1_;
  void * sem2_;
  
  
};

class OnceType {
public:

    OnceType(const OnceType &once) : init_(once.init_) {}
    OnceType(bool f) : init_(f) {}
    void InitOnce(void (*initializer)()) {
        mutex_.Lock();
        if (!init_) {
            init_ = true;
            initializer();
        }
        mutex_.Unlock();
    }

private:
    bool init_;
    Mutex mutex_;
};

#define LEVELDB_ONCE_INIT false
extern void InitOnce(port::OnceType*, void (*initializer)());


class AtomicPointer {
 private:
  void * rep_;
 public:
  AtomicPointer() : rep_(NULL) { }
  explicit AtomicPointer(void* v); 
  void* Acquire_Load() const;

  void Release_Store(void* v);

  void* NoBarrier_Load() const;

  void NoBarrier_Store(void* v);
};

inline bool Snappy_Compress(const char* input, size_t length,
                            ::std::string* output) {
#ifdef SNAPPY
  output->resize(snappy::MaxCompressedLength(length));
  size_t outlen;
  snappy::RawCompress(input, length, &(*output)[0], &outlen);
  output->resize(outlen);
  return true;
#endif

  return false;
}

inline bool Snappy_GetUncompressedLength(const char* input, size_t length,
                                         size_t* result) {
#ifdef SNAPPY
  return snappy::GetUncompressedLength(input, length, result);
#else
  return false;
#endif
}

inline bool Snappy_Uncompress(const char* input, size_t length,
                              char* output) {
#ifdef SNAPPY
  return snappy::RawUncompress(input, length, output);
#else
  return false;
#endif
}

inline bool GetHeapProfile(void (*func)(void*, const char*, int), void* arg) {
  return false;
}

}
}

#endif  
