#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <gtest/gtest.h>

// #include "storage/graph_schema.hpp"
#include "storage/graphdb.hpp"

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
        insert_new_edge(E_TYPE, edge, V_TYPE, src, V_TYPE, dst, "");
    }

    void insert_edge_wo_proper(ide_t src, ide_t dst, ide_t edge) {
        insert_conn_table_wo_proper(src, dst, edge);
        insert_edge_table_wo_proper(src, dst, edge);
    }
};

}

void test_simple() {

    graphdb::graphdb_test db("/tmp/db_data");

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
}

TEST(test_simple, test_simple) {
    test_simple();
}

int main(int argc, char **argv)
{

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    return 0;
}
