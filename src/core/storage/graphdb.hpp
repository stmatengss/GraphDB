#if !defined(GRAPHDB_HPP_)
#define GRAPHDB_HPP_

#include <vector>
#include <functional>

#include "rocksdb_api.h"
#include "core/storage/graph_schema.hpp"

namespace graphdb 
{

    class graphdb 
    {
    private:
        rocksdb::DB* db;
        rocksdb::Options options;

        // meta-data
        std::string db_path;

    public:
        graphdb(std::string db_path_): db_path(db_path_) { 
            create_db_if_not_exist(db_path);
        }
        ~graphdb() { 
            close_db();
        }
        
        /*  */

        void create_db_if_not_exist(std::string db_path) {
            // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
            options.IncreaseParallelism();
            options.OptimizeLevelStyleCompaction();
            // create the DB if it's not already present
            options.create_if_missing = true;
              // open DB
            rocksdb::Status s = rocksdb::DB::Open(options, db_path, &db);
            assert(s.ok());
        }

        void close_db() {
            delete db;
        }

        /*
            
        */

        /*
        ExploreScan()
        Input:
        1:Required: an ordered list of source vertex list
        2:Optional: the filter condition for source vertex
        3:Optional: the filter condition for out-going edge
        Output:
        1:Required: extracted readed source vertex
        2:Optional: an ordered list of the reached vertex
        Implementation:
        scroll the kv store for only one pass and gap-skip
        record the reached vertex in an in-memory heap and flush to the disk if it reach a certain limitation
        the result will be multiple ordered list of reached vertex and can be merged in one pass
        */
        bool explore_scan(std::vector<ide_t> v_vec_sorted, 
                        std::function<bool(vertex_table_item)> src_v_filter,
                        std::function<bool(conn_table_item)> edge_filter,
                        std::vector<ide_t> &src_v_vec_out,
                        bool ouput_dst_vec,
                        std::vector<ide_t> &dst_v_vec_out) {
            if (src_v_filter != nullptr) {

            }
        
            if (edge_filter != nullptr) {

            }

            return true;
        }

        /*
        BFS()
        Input:
        1:Required: an ordered list of source vertex list
        2:Optional: a description of the filter conditions
        3:Optional: visit the vertex for only once or multiple times
        Output:
        1:Required: extracted readed source vertex
        Implementaion:
        based on ExploreScan()
        can be down in only a few (the same number as the depth of BFS) passes of the data
        */

        bool bfs(std::vector<ide_t> &vec_sorted,
                std::function<bool(vertex_table_item)> src_v_filter,
                std::function<bool(conn_table_item)> edge_filter,
                int bfs_total_times,
                std::vector<ide_t> &v_out) {
            
            

            return true;
        }
    };
}

#endif // GRAPHDB_HPP_
