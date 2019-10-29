#include <gtest/gtest.h>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "rocksdb/options.h"
#include "rocksdb/table.h"
#include "rocksdb/cache.h"
using namespace rocksdb;
using namespace std;
string randString(int len){
    random_device rd;
    mt19937 generate(rd());
    string str;
    for(int i=0;i<len;i++){
        if(generate()&1)
            str.append(1,'0'+generate()%10);
        else
            str.append(1,'A'+generate()%26);
    }
    return str;
}
TEST(DbCache, LRUCache) {
    DB* db;
    Options options;
    WriteOptions wo;
    ReadOptions op;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    options.statistics = rocksdb::CreateDBStatistics();
    options.create_if_missing=true;

    
    shared_ptr<Cache> cache = NewLRUCache(8 << 20);  // 8M
    BlockBasedTableOptions table_options;
    table_options.block_cache = cache;
    options.table_factory.reset(NewBlockBasedTableFactory(table_options));
    vector<uint64_t> out;
}
