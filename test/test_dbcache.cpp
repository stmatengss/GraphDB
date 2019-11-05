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
#include "rocksdb/db.h"
using namespace rocksdb;
using namespace std;
string randString(int len){
    static random_device rd;
    static mt19937 generate(rd());
    string str;
    for(int i=0;i<len;i++){
        if(generate()&1)
            str.append(1,'0'+generate()%10);
        else
            str.append(1,'A'+generate()%26);
    }
    return str;
}
void genData(int num,vector<string>* &keys,vector<string>* &values){
    keys=new vector<string>(num);
    values=new vector<string>(num);
    for(int i=0;i<num;i++){
        string key=randString(10);
        string value=randString(20);
        (*keys)[i]=key;
        (*values)[i]=value;
        if(i%100000==0){
            printf("gendata:%.2lf%%\n",100.0*i/num);
        }
    }
}
void batchPut(DB* db,vector<string>* keys,vector<string>* values,WriteOptions &wo) {
	auto begin = std::chrono::system_clock::now();
    int num=keys->size();
    for(int i=0;i<num;i++){
        db->Put(wo,(*keys)[i],(*values)[i]);
        if(i%100000==0){
            printf("put:%.2lf%%\n",100.0*i/num);
        }
    }
	auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> diff = end - begin;
	std::cout << "Time:" << diff.count() << ", Counter:" << num << ", IOPS:" << num / diff.count() << std::endl;
}
void batchPutByWB(DB* db,vector<string>* keys,vector<string>* values,int batchsize,WriteOptions &wo) {
	auto begin = std::chrono::system_clock::now();
    int num=keys->size();
    WriteBatch wb;
    for(int i=0;i<num;i++){
        wb.Put((*keys)[i],(*values)[i]);
        if(i%100000==0){
            printf("put:%.2lf%%\n",100.0*i/num);
        }
        if(i%batchsize==batchsize-1||i==num-1){
            db->Write(wo,&wb);
            wb.Clear();
        }
    }
	auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> diff = end - begin;
	std::cout << "Time:" << diff.count() << ", Counter:" << num << ", IOPS:" << num / diff.count() << std::endl;
}
vector<string>* batchGet(DB* db,vector<string>* keys,ReadOptions &ro) {
    vector<string>* values=new vector<string>;
	auto begin = std::chrono::system_clock::now();
    int num=keys->size();
    for(int i=0;i<num;i++){
        string value;
        db->Get(ro,(*keys)[i],&value);
        values->push_back(value);
        if(i%100000==0){
            printf("get:%.2lf%%\n",100.0*i/num);
        }
    }
	auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - begin;
	std::cout << "Time:" << diff.count() << ", Counter:" << num << ", IOPS:" << num / diff.count() << std::endl;
    return values;
}
// vector<string>* batchGetByMG(DB* db,vector<string>* keys,int batchsize,ReadOptions &ro) {
//     vector<string>* values=new vector<string>;
// 	auto begin = std::chrono::system_clock::now();
//     int num=keys->size();
//     for(int i=0;i<num;i++){
//         string value;
//         db->Get(ro,(*keys)[i],&value);
//         values->push_back(value);
//         if(i%100000==0){
//             printf("get:%.2lf%%\n",100.0*i/num);
//         }
//     }
// 	auto end = std::chrono::system_clock::now();
//     std::chrono::duration<double> diff = end - begin;
// 	std::cout << "Time:" << diff.count() << ", Counter:" << num << ", IOPS:" << num / diff.count() << std::endl;
//     return values;
// }
TEST(Db, ReadWrite) {
    DB* db;
    Options options;
    WriteOptions wo;
    ReadOptions ro;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    options.statistics = rocksdb::CreateDBStatistics();
    options.create_if_missing=true;
    options.error_if_exists=true;
    string dbpath="/tmp/DbCache"+randString(10);

    shared_ptr<Cache> cache = NewLRUCache(8 << 20);  // 8M
    BlockBasedTableOptions table_options;
    table_options.block_cache = cache;
    options.table_factory.reset(NewBlockBasedTableFactory(table_options));

    // options.memtable_factory.reset(new VectorRepFactory(1024));
    // options.allow_concurrent_memtable_write = false;

    // options.enable_pipelined_write=true;

    Status s = DB::Open(options, dbpath, &db);
    vector<string> *keys,*values;
    genData(2000000,keys,values);
    // wo.disableWAL=true;
    // wo.memtable_insert_hint_per_batch=true;
    batchPut(db,keys,values,wo);
    sort(keys->begin(),keys->end());
    // batchPutByWB(db,keys,values,10000,wo);
    FlushOptions fo;
    db->Flush(fo);
    vector<string> *gvalues=batchGet(db,keys,ro);
    // EXPECT_EQ(*values,*gvalues);
    cout<<options.statistics->ToString();
    db->Close();
    // DestroyDB(dbpath,options);
    delete values;
    delete gvalues;
    delete keys;
    delete db;
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
