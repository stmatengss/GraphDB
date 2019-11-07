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
#define DATA_LENGTH 50000000
// #define PROCESS
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
        #ifdef PROCESS
        if(i%100000==0){
            printf("gendata:%.2lf%%\n",100.0*i/num);
        }
        #endif
    }
}
void batchPut(DB* db,vector<string>* keys,vector<string>* values,WriteOptions &wo) {
	auto begin = std::chrono::system_clock::now();
    int num=keys->size();
    for(int i=0;i<num;i++){
        db->Put(wo,(*keys)[i],(*values)[i]);
        #ifdef PROCESS
        if(i%100000==0){
            printf("put:%.2lf%%\n",100.0*i/num);
        }
        #endif
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
        #ifdef PROCESS
        if(i%100000==0){
            printf("put:%.2lf%%\n",100.0*i/num);
        }
        #endif
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
        #ifdef PROCESS
        if(i%100000==0){
            printf("get:%.2lf%%\n",100.0*i/num);
        }
        #endif
    }
	auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - begin;
	std::cout << "Time:" << diff.count() << ", Counter:" << num << ", IOPS:" << num / diff.count() << std::endl;
    return values;
}
vector<string>* batchGetByMG(DB* db,vector<string>* keys,int batchsize,ReadOptions &ro) {
    vector<string>* values=new vector<string>;
	auto begin = std::chrono::system_clock::now();
    int num=keys->size();
    vector<Slice> tmpk;
    vector<string> tmpv;
    for(int i=0;i<num;){
        string value;
        tmpk.clear();
        int ks=min(batchsize,num-i);
        tmpk.insert(tmpk.end(),keys->begin()+i,keys->begin()+i+ks);
        db->MultiGet(ro,tmpk,&tmpv);
        values->insert(values->end(),tmpv.begin(),tmpv.end());
        i+=ks;
        #ifdef PROCESS
        if(i%100000==0){
            printf("get:%.2lf%%\n",100.0*i/num);
        }
        #endif
    }
	auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - begin;
	std::cout << "Time:" << diff.count() << ", Counter:" << num << ", IOPS:" << num / diff.count() << std::endl;
    return values;
}
class DbTest: public ::testing::Test
{
public:
    DB* db;
    Options* options;
    WriteOptions* wo;
    ReadOptions* ro;
    string dbpath;
    static vector<string> *keys,*values;
    static void SetUpTestCase(){
        genData(DATA_LENGTH,keys,values);
    }
    void SetUp() {
        options=new Options();
        wo=new WriteOptions();
        ro=new ReadOptions();
        options->IncreaseParallelism();
        options->OptimizeLevelStyleCompaction();
        options->statistics = rocksdb::CreateDBStatistics();
        options->create_if_missing=true;
        options->error_if_exists=true;
        dbpath="/tmp/DbCache"+randString(10);
        cout<<endl;
    }
    void TearDown() {
        db->Close();
        DestroyDB(dbpath,*options);
        delete options;
        delete db;
        delete wo;
        delete ro;
    }
    static void TearDownTestCase(){
        delete keys;
        delete values;
    }
private:
};
vector<string> *DbTest::keys,*DbTest::values;
/**
 * parm
 * bool: use vecmem?
 * bool: use wal?
 * bool: sort keys?
 * int: 0:writebatch 1:put
 **/

class DbWriteTest: public DbTest,public ::testing::WithParamInterface<tuple<bool,bool,bool,int>>
{};
TEST_P(DbWriteTest, Write) {
    bool useVecMem=get<0>(GetParam());
    bool useWAL=get<1>(GetParam());
    bool sortkey=get<2>(GetParam());
    int writemethod=get<3>(GetParam());
    cout<<"VecMem: "<<useVecMem<<",WAL: "<<useWAL<<",sortkey: "<<sortkey
    <<",writemethod: "<<(writemethod==0?"writebatch":"put")<<endl;
    if(useVecMem){
        options->memtable_factory.reset(new VectorRepFactory(1024));
        options->allow_concurrent_memtable_write = false;// can only be used in skiplist memtable
    }
    // options.enable_pipelined_write=true;

    // wo.memtable_insert_hint_per_batch=true;
    Status s = DB::Open(*options, dbpath, &db);

    if(!useWAL){
        wo->disableWAL=true;
    }
    vector<string>* tmpkeys=new vector<string>;
    if(sortkey){
        tmpkeys->insert(tmpkeys->end(),keys->begin(),keys->end());
        sort(tmpkeys->begin(),tmpkeys->end());
        swap(keys,tmpkeys);
    }
    if(writemethod==0){
        batchPutByWB(db,keys,values,100000,*wo);
    }else{
        batchPut(db,keys,values,*wo);
    }
    FlushOptions fo;
    db->Flush(fo);
    if(sortkey){
        swap(keys,tmpkeys);
    }
    delete tmpkeys;
}
/**
 * parm
 * bool: sort keys?
 * int: 0:nocache 1:lrucache
 **/
class DbReadTest: public DbTest,public ::testing::WithParamInterface<tuple<bool,int>>
{};
TEST_P(DbReadTest, Read) {
    bool sortkey=get<0>(GetParam());
    int cachemethod=get<1>(GetParam());
    cout<<"sortkey: "<<sortkey<<",cache: "<<(cachemethod==0?"no cache":"lrucache")<<endl;
    if(cachemethod==0){
        BlockBasedTableOptions table_options;
        table_options.no_block_cache = true;
        options->table_factory.reset(NewBlockBasedTableFactory(table_options));
    }
    else if(cachemethod==1){
        shared_ptr<Cache> cache = NewLRUCache(8 << 20);  // 8M
        BlockBasedTableOptions table_options;
        table_options.block_cache = cache;
        options->table_factory.reset(NewBlockBasedTableFactory(table_options));
    }

    Status s = DB::Open(*options, dbpath, &db);

    vector<string>* tmpkeys=new vector<string>;
    tmpkeys->insert(tmpkeys->end(),keys->begin(),keys->end());
    sort(tmpkeys->begin(),tmpkeys->end());
    cout<<"start put"<<endl;
    batchPutByWB(db,tmpkeys,values,100000,*wo); // quick insert
    cout<<endl;
    FlushOptions fo;
    db->Flush(fo);

    if(sortkey){
        swap(keys,tmpkeys);
    }
    options->statistics->Reset();
    cout<<"do Get:"<<endl;
    vector<string> *gvalues1=batchGet(db,keys,*ro);
    cout<<"Block Cache hit:"<<options->statistics->getTickerCount(BLOCK_CACHE_HIT)
    <<", Block Cache miss:"<<options->statistics->getTickerCount(BLOCK_CACHE_MISS)<<endl;
    delete gvalues1;
    options->statistics->Reset();
    cout<<"do MultiGet:"<<endl;
    vector<string> *gvalues2=batchGetByMG(db,keys,200000,*ro);
    cout<<"Block Cache hit:"<<options->statistics->getTickerCount(BLOCK_CACHE_HIT)
    <<", Block Cache miss:"<<options->statistics->getTickerCount(BLOCK_CACHE_MISS)<<endl;
    delete gvalues2;
    if(sortkey){
        swap(keys,tmpkeys);
    }
    delete tmpkeys;
}
INSTANTIATE_TEST_CASE_P(
    DbWriteTestP,
    DbWriteTest,
    ::testing::Values(
            std::make_tuple(false, true, false,0),
            std::make_tuple(false, true, false,1),
            std::make_tuple(false, false, false,0),
            std::make_tuple(false, false, false,1),
            std::make_tuple(false, true, true,0),
            std::make_tuple(false, true, true,1),
            std::make_tuple(true, true, false,0),
            std::make_tuple(true, true, false,1),
            std::make_tuple(true, false, true,0),
            std::make_tuple(true, false, true,1)));
INSTANTIATE_TEST_CASE_P(
    DbReadTestP,
    DbReadTest,
    ::testing::Values(
            std::make_tuple(false, 0),
            std::make_tuple(false, 1),
            std::make_tuple(true, 0),
            std::make_tuple(true, 1)
            ));
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    cout<<"datanum: "<<DATA_LENGTH<<"\nkeylen:10\nvaluelen:20"<<endl;
    return RUN_ALL_TESTS();
}
