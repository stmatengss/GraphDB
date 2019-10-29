#if !defined(ROCKSDB_WRAPPER_)
#define ROCKSDB_WRAPPER_

#include <vector>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <iostream>

#include "utils.h"
#include "rocksdb_api.h"

using namespace rocksdb;
using db_status = Status;

#define USE_BENCH

class rocksdb_wrapper
{
private:
    DB* db;
    std::string db_path;

private:
    template<typename T>
    void multi_seek_t(std::vector<T> &src_v_vec, std::vector<Iterator *> &begin_pos_vec) {
        ReadOptions option = ReadOptions();  

        for(T &src_v: src_v_vec) {

            Iterator *it = db->NewIterator(option);
            it->Seek(std::string(reinterpret_cast<char *>(&src_v)));
            begin_pos_vec.emplace_back(it);
        }
    }

    void multi_seek(std::vector<std::string> &src_v_vec_str, std::vector<Iterator *> &begin_pos_vec) {
        // Fake MultiSeek
        ReadOptions option = ReadOptions();  

        for(auto src_v_str: src_v_vec_str) {
            Iterator *it = db->NewIterator(option);

            it->Seek(src_v_str);
            begin_pos_vec.emplace_back(it);
        } 
    }

public:
    rocksdb_wrapper(std::string db_path_): db_path(db_path_) { 
        create_db_if_not_exist(db_path_);
    }

    rocksdb_wrapper(std::string db_path_,Options& options): db_path(db_path_) { 
        open_db(db_path_,options);
    }
    ~rocksdb_wrapper() {
        close_db();
    }

    void create_db_if_not_exist(std::string db_path) {
        // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
        Options options;
        options.IncreaseParallelism();
        options.OptimizeLevelStyleCompaction();
        // create the DB if it's not already present
        options.create_if_missing = true;
        BlockBasedTableOptions table_options;
        table_options.no_block_cache = true;
        options.table_factory.reset(NewBlockBasedTableFactory(table_options));

        // open DB
        open_db(db_path,options);
    }

    void open_db(std::string db_path,Options& options) {
        Status s = DB::Open(options, db_path, &db);
        if(!s.ok()){
            fprintf(stderr,"%s %s\n",db_path.c_str(),s.getState());//debug
        }
        assert(s.ok());
    }

    void close_db() {
        delete db;
    }

    db_status Put(std::string key, std::string value) {

        WriteOptions wo = WriteOptions();

        return db->Put(wo, key, value);
    }

    Iterator *Seek(std::string key) {

        ReadOptions option = ReadOptions();  
        Iterator *it = db->NewIterator(option);
        it->Seek(key);
        return it;
    }

    void explore_impl(std::vector<std::string> &v_vec_str, std::vector<std::pair<std::string, std::string>> &explore_edges, std::vector<int> explore_edges_len) {
        int counter;
        std::vector<Iterator *> seek_res;
        multi_seek(v_vec_str, seek_res);
        
        int i = 0;
        for (auto &it_res: seek_res) {
            
            if (!it_res->Valid()) {
                delete it_res;
                continue;
            }

            for(; it_res->Valid(); it_res->Next()) {
                std::string key_str = it_res->key().ToString();
                std::string val_str = it_res->value().ToString();
                if (key_str.compare(0, v_vec_str[i].length(), v_vec_str[i]) == 0) {
                    explore_edges.emplace_back(make_pair(key_str, val_str));
                    counter ++;
                } else {
                    explore_edges_len.emplace_back(counter);
                    counter = 0;
                    break;
                }
            }
            i ++;
            delete it_res;
        }
        explore_edges_len.emplace_back(counter);
    }

    void explore_impl(std::vector<std::string> v_vec_str, std::vector<std::pair<std::string, std::string>> &explore_edges) {

        std::vector<Iterator *> seek_res;

#ifdef USE_BENCH
        auto begin = std::chrono::system_clock::now();
#endif
        multi_seek(v_vec_str, seek_res);
#ifdef USE_BENCH
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = end - begin;
        std::cout << "Multi Seek Time:" << diff.count() << std::endl;
#endif   

        int i = 0;
        for (auto &it_res: seek_res) {       

            if (!it_res->Valid()) {
                delete it_res; 
                continue;
            }

            for(; it_res->Valid(); it_res->Next()) {
                std::string key_str = it_res->key().ToString();
                std::string val_str = it_res->value().ToString();

                if (key_str.compare(0, v_vec_str[i].length(), v_vec_str[i]) == 0) {
                    explore_edges.emplace_back(make_pair(key_str, val_str));
                } else {
                    break;
                }
            }
            i ++;
            delete it_res; //must be released
        }
    }

};

#endif // ROCKSDB_WRAPPER_
