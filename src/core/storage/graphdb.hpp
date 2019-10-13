#if !defined(GRAPHDB_HPP_)
#define GRAPHDB_HPP_

#include "rocksdb_api.h"

namespace graphdb 
{

    class graphdb 
    {
    private:
        rocksdb::DB* db;
        rocksdb::Options options;
        /* data */

    public:
        graphdb(std::string db_path) { 
            // create_if_not_exist(db_path);
        }
        ~graphdb() { 

        }
        
        void create_if_not_exist(std::string db_path) {
            // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
            options.IncreaseParallelism();
            options.OptimizeLevelStyleCompaction();
            // create the DB if it's not already present
            options.create_if_missing = true;
              // open DB
            rocksdb::Status s = rocksdb::DB::Open(options, db_path, &db);
            assert(s.ok());
        }

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
       void explore_scan() {

       }  
    };
}

#endif // GRAPHDB_HPP_
