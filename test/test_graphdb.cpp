#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <gtest/gtest.h>

// #include "storage/graph_schema.hpp"
#include "storage/graphdb.hpp"
#include "tool/filesystem.hpp"
#include "workload/data_loader.hpp"

namespace graphdb
{

class graphdb_test: public graphdb { 

public:
    graphdb_test(std::string db_path): graphdb(db_path) { 
    }
    ~graphdb_test() { 
    }

    void insert_conn_table_wo_proper(ide_t src, ide_t dst, ide_t edge) {
        insert_new_conn(V_TYPE, src, E_TYPE, V_TYPE, dst, edge, "");
    }

    void insert_edge_table_wo_proper(ide_t src, ide_t dst, ide_t edge) {
        insert_new_edge(E_TYPE, edge, V_TYPE, src, V_TYPE, dst);
    }

    void insert_edge_wo_proper(ide_t src, ide_t dst, ide_t edge) {
        insert_conn_table_wo_proper(src, dst, edge);
        insert_edge_table_wo_proper(src, dst, edge);
    }
};

}

void test_simple() {

    std::string db_path("/tmp/db_data");
    graphdb::graphdb_test db(db_path);

    db.insert_edge_wo_proper(1, 2, 1);
    db.insert_edge_wo_proper(2, 3, 2);
    db.insert_edge_wo_proper(1, 4, 3);
    db.insert_edge_wo_proper(1, 5, 4);
    db.insert_edge_wo_proper(2, 6, 5);
    db.insert_edge_wo_proper(3, 4, 6);
    db.insert_edge_wo_proper(1, 6, 7);

    std::vector<uint64_t> in;
    std::vector<uint64_t> out;

    in.emplace_back(1);

    db.explore_scan(in, out);

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
    db.explore_scan(in, out);

    // printf("Len: %lld\n", (long long)out.size());
    EXPECT_EQ(out.size(), 5);

    int result2[] = {2, 3, 4, 5, 6};
    counter = 0;
    for (auto i: out) {
        // printf("Key: %lld\n", (long long)i);
        EXPECT_EQ(i, result2[counter++]);
    }

    remove_directory(db_path);
}

void test_wiki_vote() {

    std::string db_path("/tmp/db_data_wiki_vote");
    std::string file_path("/home/lanvent/data/wiki-Vote.txt");

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

    remove_directory(db_path);
}

void test_web_NotreDame() {

    std::string db_path("/tmp/db_web-NotreDame");
    std::string file_path("/home/lanvent/data/web-NotreDame.txt");

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

    remove_directory(db_path);
}

void test_put_throughput() {
    
    // const int iter_num = 1000000;
    const int ratio = 10;

    std::string db_path("/tmp/db-put-throughput");
    graphdb::graphdb_test db(db_path);

    std::map<std::string, int> graph_scale_map = {
        {"tiny", 1000},
        {"small", 100000},
        {"middle", 10000000}
    };

    int graph_scale = graph_scale_map["small"];
    std::random_device rd;
	std::mt19937 generate(rd());
    // srand(time(NULL));

    // for (int i = 0; i <= iter_num; i ++) 
    int i;
    int iter_num = graph_scale * ratio;

	auto begin = std::chrono::system_clock::now();
    while (i < iter_num) {
        int src = generate() % graph_scale;
        int dst = generate() % graph_scale;
        if (src != dst) {
            db.insert_edge_wo_proper(src, dst, i);
            i++;
        }
    }
	auto end = std::chrono::system_clock::now();

    auto diff = end - begin;
	std::cout << "Time:" << diff.count() << ", Counter:" << iter_num << ", IOPS:" << iter_num * 1000.0 / diff.count() << std::endl;
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

TEST(test_put_throughput, test_put_throughput) {
    test_put_throughput();
}


int main(int argc, char **argv)
{

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    return 0;
}
