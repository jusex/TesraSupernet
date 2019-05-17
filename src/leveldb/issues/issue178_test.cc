




#include <iostream>
#include <sstream>
#include <cstdlib>

#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include "util/testharness.h"

namespace {

const int kNumKeys = 1100000;

std::string Key1(int i) {
  char buf[100];
  snprintf(buf, sizeof(buf), "my_key_%d", i);
  return buf;
}

std::string Key2(int i) {
  return Key1(i) + "_xxx";
}

class Issue178 { };

TEST(Issue178, Test) {
  
  std::string dbpath = leveldb::test::TmpDir() + "/leveldb_cbug_test";
  DestroyDB(dbpath, leveldb::Options());

  
  
  
  leveldb::DB* db;
  leveldb::Options db_options;
  db_options.create_if_missing = true;
  db_options.compression = leveldb::kNoCompression;
  ASSERT_OK(leveldb::DB::Open(db_options, dbpath, &db));

  
  leveldb::WriteBatch batch;
  for (size_t i = 0; i < kNumKeys; i++) {
    batch.Put(Key1(i), "value for range 1 key");
  }
  ASSERT_OK(db->Write(leveldb::WriteOptions(), &batch));

  
  batch.Clear();
  for (size_t i = 0; i < kNumKeys; i++) {
    batch.Put(Key2(i), "value for range 2 key");
  }
  ASSERT_OK(db->Write(leveldb::WriteOptions(), &batch));

  
  batch.Clear();
  for (size_t i = 0; i < kNumKeys; i++) {
    batch.Delete(Key2(i));
  }
  ASSERT_OK(db->Write(leveldb::WriteOptions(), &batch));

  
  std::string start_key = Key1(0);
  std::string end_key = Key1(kNumKeys - 1);
  leveldb::Slice least(start_key.data(), start_key.size());
  leveldb::Slice greatest(end_key.data(), end_key.size());

  
  db->CompactRange(&least, &greatest);

  
  leveldb::Iterator* iter = db->NewIterator(leveldb::ReadOptions());
  size_t num_keys = 0;
  for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
    num_keys++;
  }
  delete iter;
  ASSERT_EQ(kNumKeys, num_keys) << "Bad number of keys";

  
  delete db;
  DestroyDB(dbpath, leveldb::Options());
}

}  

int main(int argc, char** argv) {
  return leveldb::test::RunAllTests();
}
