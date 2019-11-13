#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <gtest/gtest.h>
// #include <gmock/gmock.h>
#include "rocksdb/table.h"
#include "rocksdb/cache.h"
#include "storage/graphdb.hpp"
#include "tool/filesystem.hpp"
#include "workload/data_loader.hpp"

namespace graphdb
{

class graphdb_test: public graphdb { 

public:
    graphdb_test(std::string db_path): graphdb(db_path) { 
    }
    graphdb_test(std::string db_path,Options& options): graphdb(db_path,options) { 
    }
    ~graphdb_test() { 
    }

    void insert_conn_table_wo_proper(uint64_t src, ide_t dst, ide_t edge) {
        insert_new_conn(V_TYPE, src, E_TYPE, V_TYPE, dst, edge, "");
    }

    void insert_edge_table_wo_proper(ide_t src, ide_t dst, ide_t edge) {
        insert_new_edge(E_TYPE, edge, V_TYPE, src, V_TYPE, dst);
    }

    void insert_edge_wo_proper(ide_t src, ide_t dst, ide_t edge) {
        insert_conn_table_wo_proper(src, dst, edge);
        insert_edge_table_wo_proper(src, dst, edge);
    }

    void insert_vertex_wo_proper(ide_t src) {
        insert_new_vertex(V_TYPE, src, P_TYPE, "");
    }
};

}
std::string randString(int len){
    std::random_device rd;
    std::mt19937 generate(rd());
    std::string str;
    for(int i=0;i<len;i++){
        if(generate()&1)
            str.append(1,'0'+generate()%10);
        else
            str.append(1,'A'+generate()%26);
    }
    return str;
}


void test_simple() {

    std::string db_path("/tmp/db_data");
    auto db=new graphdb::graphdb_test (db_path);

    db->insert_edge_wo_proper(1, 2, 1);
    db->insert_edge_wo_proper(2, 3, 2);
    db->insert_edge_wo_proper(1, 4, 3);
    db->insert_edge_wo_proper(1, 5, 4);
    db->insert_edge_wo_proper(2, 6, 5);
    db->insert_edge_wo_proper(3, 4, 6);
    db->insert_edge_wo_proper(1, 6, 7);

    db->insert_vertex_wo_proper(1);
    db->insert_vertex_wo_proper(2);
    db->insert_vertex_wo_proper(3);
    db->insert_vertex_wo_proper(4);
    db->insert_vertex_wo_proper(5);
    db->insert_vertex_wo_proper(6);
    db->insert_vertex_wo_proper(7);


    std::vector<uint64_t> in;
    std::vector<uint64_t> out;

    in.emplace_back(1);

    db->explore_scan(in, out);

    // printf("Len: %lld\n", (long long)out.size());
    EXPECT_EQ(out.size(), 4);

    int result1[] = {2, 4, 5, 6};
    int counter = 0;
    for (auto i: out) {
        // printf("Key: %lld\n", (long long)i);
        EXPECT_EQ(i, result1[counter++]);
    }

    out.clear();
    in.emplace_back(2);
    db->explore_scan(in, out);

    // printf("Len: %lld\n", (long long)out.size());
    EXPECT_EQ(out.size(), 5);

    int result2[] = {2, 3, 4, 5, 6};
    counter = 0;
    for (auto i: out) {
        // printf("Key: %lld\n", (long long)i);
        EXPECT_EQ(i, result2[counter++]);
    }

    EXPECT_EQ(reinterpret_cast<uint64_t>(db->get_max_edge().edge_id), 7ULL);
    EXPECT_EQ(reinterpret_cast<uint16_t>(db->get_max_edge().edge_type), E_TYPE);

    EXPECT_EQ(reinterpret_cast<uint64_t>(db->get_max_vertex().src_id), 7ULL);
    EXPECT_EQ(reinterpret_cast<uint16_t>(db->get_max_vertex().src_type), V_TYPE);

    db->insert_edge_wo_proper(6, 7, 8);
    db->insert_edge_wo_proper(2, 9, 9);
    db->insert_edge_wo_proper(3, 8, 10);
    db->insert_edge_wo_proper(8, 9, 11);
    db->insert_edge_wo_proper(8, 10, 12);

    std::vector<uint64_t> res;
    std::vector<uint64_t> true_res{3, 7, 9};
    long long total_counter;
    db->bfs(std::vector<uint64_t>{1}, 2, res, total_counter);

    // EXPECT_THAT(res, ::testing::ContainerEq(true_res)); //FIXME
    counter = 0;
    for_each(res.begin(), res.end(), [&](uint64_t &x){
        // printf("%ld\n", (long)x);
        EXPECT_EQ(true_res[counter ++], x);
    });

    delete db;
    remove_directory(db_path);
}

void test_wiki_vote() {

    std::string db_path("/tmp/db_data_wiki_vote");
    std::string file_path("/home/mateng/third_party/dataset/wiki-Vote-shuf.txt");

    graphdb::graphdb_test db(db_path);
    graphdb::simple_data_loader dl(file_path);

    dl.fill_db_wo_weight<graphdb::graphdb_test>(db);

    std::vector<uint64_t> in;
    std::vector<uint64_t> out;
    in.emplace_back(7727);

	auto begin = std::chrono::system_clock::now();
    db.explore_scan(in, out);
	auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> diff = end - begin;
    std::cout << "Time:" << diff.count() << std::endl;

    EXPECT_EQ(out.size(), 30);

    std::vector<uint64_t> in2{
        7727, 3196, 1166, 1594, 3802, 879 , 1700, 255 , 3227, 311 , 2369, 3798, 5591, 5002, 5822, 762 , 3660, 172 , 2474, 3032, 3642, 722 , 1305, 3439, 6946, 4255, 5185, 1210, 68  , 1151, 2972, 4151, 2504, 3586, 2535, 2966, 765 , 4233, 6855, 0   
    };

    begin = std::chrono::system_clock::now();
    db.explore_scan(in2, out);
    end = std::chrono::system_clock::now();

    diff = end - begin;
    std::cout << "Time:" << diff.count() << std::endl;

    std::sort(in2.begin(), in2.end());

    begin = std::chrono::system_clock::now();
    db.explore_scan(in2, out);
    end = std::chrono::system_clock::now();

    diff = end - begin;
    std::cout << "Time:" << diff.count() << std::endl;
    
    std::vector<uint64_t> in3{8010};
    long long total_counter;

    begin = std::chrono::system_clock::now();
    db.bfs(in3, -1, out, total_counter);
    end = std::chrono::system_clock::now();

    diff = end - begin;
    std::cout << "Total Counter:" << total_counter << std::endl;
    std::cout << "Time:" << diff.count() << std::endl;

    remove_directory(db_path);
}

void test_web_NotreDame() {

    std::string db_path("/tmp/db_web-NotreDame");
    std::string file_path("/home/mateng/third_party/dataset/web-NotreDame-shuf.txt");

    graphdb::graphdb_test db(db_path);
    graphdb::simple_data_loader dl(file_path);

    dl.fill_db_wo_weight<graphdb::graphdb_test>(db);

    std::vector<uint64_t> in;
    std::vector<uint64_t> out;
    in.emplace_back(260697);

	auto begin = std::chrono::system_clock::now();
    db.explore_scan(in, out);
	auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> diff = end - begin;
    std::cout << "Time:" << diff.count() << std::endl;

    EXPECT_EQ(out.size(), 156);

    std::vector<uint64_t> in2{
        291399 ,32559  ,233731 ,88529  ,25135  ,1698   ,42501  ,62830  ,260565 ,319339 ,260101 ,128090 ,228286 ,229641 ,231400 ,233574 ,234393 ,286118 ,306916 ,306678 ,260710 ,54520  ,258381 ,98780 
    };

    begin = std::chrono::system_clock::now();
    db.explore_scan(in2, out);
    end = std::chrono::system_clock::now();

    diff = end - begin;
    std::cout << "Time:" << diff.count() << std::endl;

    std::sort(in2.begin(), in2.end());

    begin = std::chrono::system_clock::now();
    db.explore_scan(in2, out);
    end = std::chrono::system_clock::now();

    diff = end - begin;
    std::cout << "Time:" << diff.count() << std::endl;

    // EXPECT_EQ(reinterpret_cast<uint64_t>(db.get_max_vertex().src_id), 325728ULL);
    // EXPECT_EQ(reinterpret_cast<uint64_t>(db.get_max_edge().edge_id), 1497133ULL);

    remove_directory(db_path);
}

/**
 * batch put vertices and edges randomly
 * scale : num of vertices
 * ratio : ratio of  num of edges to num of vertices
 **/
void batchRandPut(graphdb::graphdb_test* db,int scale,int ratio) {
    std::random_device rd;
	std::mt19937 generate(rd());
    // srand(time(NULL));
    // for (int i = 0; i <= iter_num; i ++) 
    int i=0;
    int iter_num = scale * ratio;

	auto begin = std::chrono::system_clock::now();
    while (i < iter_num) {
        int src = generate() % scale;
        int dst = generate() % scale;
        if (src != dst) {
            db->insert_edge_wo_proper(src, dst, i);
            i++;
        }
        if(i%100000==0){
            printf("loaded:%.2lf%%\n",100.0*i/iter_num);
        }
    }
	auto end = std::chrono::system_clock::now();

    std::chrono::duration<double> diff = end - begin;
	std::cout << "Time:" << diff.count() << ", Counter:" << iter_num << ", IOPS:" << iter_num / diff.count() << std::endl;
}

TEST(test_simple, test_simple) {
    test_simple();
}

TEST(test_wiki_vote, test_wiki_vote) {
    test_wiki_vote();
}

TEST(test_web_NotreDame, test_web_NotreDame) {
    test_web_NotreDame();
}
TEST(RandPut, Tiny) {
    // const int iter_num = 1000000;
    std::string db_path("/tmp/db-put-throughput"+randString(10));
    auto db=new graphdb::graphdb_test(db_path);
    printf("%s\n",db_path.c_str());
    // std::map<std::string, int> graph_scale_map = {
    //     {"tiny", 1000},
    //     {"small", 100000},
    //     {"middle", 10000000}
    // };
    // int graph_scale = graph_scale_map["small"];
    batchRandPut(db,1000,10);
    delete db;
    remove_directory(db_path);
}

TEST(RandPut, Small) {
    std::string db_path("/tmp/db-put-throughput"+randString(10));
    auto db=new graphdb::graphdb_test(db_path);
    printf("%s\n",db_path.c_str());
    batchRandPut(db,100000,10);
    delete db;
    remove_directory(db_path);
}

class CacheTest: public ::testing::Test
{
public:
protected:
    graphdb::graphdb_test* db;
    std::string db_path;
    std::vector<uint64_t> vertexs;
    void SetUp() {
        fprintf(stderr,"initDb\n");
        db_path="/tmp/dbCacheTest"+randString(10);
        db=new graphdb::graphdb_test(db_path);
        fprintf(stderr,"%s\n",db_path.c_str());
        int scale=50000;
        batchRandPut(db,scale,10);
        for(int i=0;i<scale;i++){
            vertexs.push_back(i);
        }
    }
    void TearDown() {
        delete db;
        remove_directory(db_path);
    }
    void ReOpen(Options& options){
        delete db;
        db=new graphdb::graphdb_test(db_path,options);
    }
private:
};

// TEST_F(CacheTest, ClockCache) {
// }
TEST_F(CacheTest, LRUCache) {
    Options options;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    std::shared_ptr<Cache> cache = NewLRUCache(8<<20);//8M
    BlockBasedTableOptions table_options;
    table_options.block_cache = cache;
    options.table_factory.reset(NewBlockBasedTableFactory(table_options));
    ReOpen(options);
    std::vector<uint64_t> out;
    db->explore_scan(vertexs, out);
    printf("%d\n", (int)out.size());
}

TEST_F(CacheTest, NoCache) {
    Options options;
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    BlockBasedTableOptions table_options;
    table_options.no_block_cache = true;
    options.table_factory.reset(NewBlockBasedTableFactory(table_options));
    ReOpen(options);
    std::vector<uint64_t> out;
    db->explore_scan(vertexs, out);
    printf("%d\n", (int)out.size());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
