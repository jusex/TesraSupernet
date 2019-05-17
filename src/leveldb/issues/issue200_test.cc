







#include "leveldb/db.h"
#include "util/testharness.h"

namespace leveldb {

class Issue200 { };

TEST(Issue200, Test) {
  
  std::string dbpath = test::TmpDir() + "/leveldb_issue200_test";
  DestroyDB(dbpath, Options());

  DB *db;
  Options options;
  options.create_if_missing = true;
  ASSERT_OK(DB::Open(options, dbpath, &db));

  WriteOptions write_options;
  ASSERT_OK(db->Put(write_options, "1", "b"));
  ASSERT_OK(db->Put(write_options, "2", "c"));
  ASSERT_OK(db->Put(write_options, "3", "d"));
  ASSERT_OK(db->Put(write_options, "4", "e"));
  ASSERT_OK(db->Put(write_options, "5", "f"));

  ReadOptions read_options;
  Iterator *iter = db->NewIterator(read_options);

  
  ASSERT_OK(db->Put(write_options, "25", "cd"));

  iter->Seek("5");
  ASSERT_EQ(iter->key().ToString(), "5");
  iter->Prev();
  ASSERT_EQ(iter->key().ToString(), "4");
  iter->Prev();
  ASSERT_EQ(iter->key().ToString(), "3");
  iter->Next();
  ASSERT_EQ(iter->key().ToString(), "4");
  iter->Next();
  ASSERT_EQ(iter->key().ToString(), "5");

  delete iter;
  delete db;
  DestroyDB(dbpath, options);
}

}  

int main(int argc, char** argv) {
  return leveldb::test::RunAllTests();
}
