#include <cstdio>
#include <string>
#include <iostream>

#include "rocksdb_api.h"
#include "tool/strawman_join.hpp"

using namespace rocksdb;

std::string kDBPath = "/tmp/rocksdb_simple_example";

int main() {
  DB* db;
  Options options;
  // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  // create the DB if it's not already present
  options.create_if_missing = true;

  // open DB
  Status s = DB::Open(options, kDBPath, &db);
  assert(s.ok());

  // Put key-value
  auto wo = WriteOptions();
  auto k = "key1";
  auto v = "value";
  db->Put(wo, k, v);
  //assert(s.ok());
  std::string value;
  // get value
  s = db->Get(ReadOptions(), "key1", &value);
  assert(s.ok());
  assert(value == "value");

  std::string const_key("key0");

  for(int i = 0; i < 10; i ++ ) {
      
	  const_key[3] = '0' + i;
	  s = db->Put(WriteOptions(), const_key, "value");
  }

  Iterator *it = db->NewIterator(ReadOptions());

  printf("OK\n");
  

  std::string begin_k("key0");
  it->Seek(begin_k);
  for (; it->Valid(); 
		  it->Next()) {
	  std::cout << it->key().ToString() << ": " << it->value().ToString() << std::endl;
	  //printf("%s: %s\n", it->key(), it->value());
  }

  delete it;
  
  delete db;

  return 0;
}

