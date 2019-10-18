#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
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

    remove_directory(db_path);
}

TEST(test_simple, test_simple) {
    test_simple();
}

TEST(test_wiki_vote, test_wiki_vote) {
    test_wiki_vote();
}

int main(int argc, char **argv)
{

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    return 0;
}
