#if !defined(ROCKSDB_WRAPPER_)
#define ROCKSDB_WRAPPER_

#include <vector>
#include <cstdint>

#include "rocksdb_api.h"

using namespace rocksdb;
using db_status = Status;

class rocksdb_wrapper
{
private:
    DB* db;
    Options options;

    std::string db_path;

private:
    std::vector<Iterator *> MultiSeek(std::vector<uint64_t> src_v_vec) {
        // Fake MultiSeek
        std::vector<Iterator *> begin_pos_vec;
        ReadOptions option = ReadOptions();  

        for(uint64_t &src_v: src_v_vec) {
            Iterator *it = db->NewIterator(option);
            it->Seek(std::string(reinterpret_cast<char *>(&src_v)));
            begin_pos_vec.push_back(it);
        }    
        return begin_pos_vec;
    }

public:
    rocksdb_wrapper(std::string db_path_): db_path(db_path_) { 
        create_db_if_not_exist(db_path_);
    }

    ~rocksdb_wrapper() { }

    void create_db_if_not_exist(std::string db_path) {
        // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        // create the DB if it's not already present
        options.create_if_missing = true;
            // open DB
        Status s = DB::Open(options, db_path, &db);
        assert(s.ok());
    }

    void close_db() {
        delete db;
    }

    db_status Put(std::string key, std::string value) {
        WriteOptions wo = WriteOptions();
        return db->Put(wo, key, value);
    }

};

#endif // ROCKSDB_WRAPPER_
