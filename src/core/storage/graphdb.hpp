#if !defined(GRAPHDB_HPP_)
#define GRAPHDB_HPP_

#include <vector>
#include <set>
#include <chrono>
#include <functional>
#include <climits>

#include "core/common.h"
#include "core/rocksdb_wrapper.hpp"
#include "core/storage/graph_schema.hpp"
#include "core/storage/graphdb_algorithm.hpp"
#include "core/storage/hashmap.hpp"

namespace graphdb 
{

class graphdb 
{
private:
    rocksdb_wrapper *db_ins;

    hash_map *hm_ins;
    // meta-data

public:
    graphdb(std::string db_path) { 
        db_ins = new rocksdb_wrapper(db_path);
        hm_ins = new hash_map();
    }
    graphdb(std::string db_path,Options& options) { 
        db_ins = new rocksdb_wrapper(db_path,options);
        hm_ins = new hash_map();
    }
    ~graphdb() { 
        delete db_ins;
    }

    /*
    FilterActive()
    Input:
    1. Required: Source Vertex ID Vector
    Output:
    1. Requir=
    */
    ret_status filter_active(std::vector<ide_t> &vec_sorted, std::vector<ide_t> &vec_sorted_out) {

        vec_sorted_out.clear();

        for_each(vec_sorted.begin(), vec_sorted.end(), [&](ide_t &v_t){

            if (!hm_ins->get_bit(v_t)) {

                hm_ins->set_bit(v_t, true);
                vec_sorted_out.emplace_back(v_t);
            }
        });

        return ret_status::succeed;
    }

    ret_status filter_active(std::vector<vertex_t> &vec_sorted, std::vector<vertex_t> &vec_sorted_out) {

        vec_sorted_out.clear();

        for_each(vec_sorted.begin(), vec_sorted.end(), [&](vertex_t &v_t){

            ide_t v_id = v_t.src_id;
            if (!hm_ins->get_bit(v_id)) {

                hm_ins->set_bit(v_id, true);
                vec_sorted_out.emplace_back(v_t);
            }
        });

        return ret_status::succeed;
    }
    
    /*
    Add Vertex (Wrap InsertNewVertex)
    */
    ret_status add_vertex(type_t src_v_type, ide_t src_v_id, type_t p_type, std::string json_str) {
        return insert_new_vertex(src_v_type, src_v_id, p_type, json_str);
    }

    /*
    InsertNewVertex()
    Input:
    1:Required: Source Vertex Type ID
    2.Required: Source Vertex ID
    3.Required: Property Type ID
    4.Required: Json String
    Output:
    1:succeed or failed
    Implementation:
    Refer to the Set API in rocksdb
    */
    ret_status insert_new_vertex(type_t src_v_type, ide_t src_v_id, type_t p_type, std::string json_str) {
        assert(db_ins != nullptr);
        
        vertex_table_item *v_item = new vertex_table_item(new vertex_table_item_key(src_v_type, src_v_id, p_type), new vertex_table_item_value(json_str));
        insert_new_vertex(v_item);

        return ret_status::succeed;
    }
    
    /*
    InsertNewVertex()
    Input:
    1:Required: vertex 
    Output:
    1:succeed or failed
    Implementation:
    Refer to the Set API in rocksdb
    */
    ret_status insert_new_vertex(vertex_table_item *v_item) {
        assert(db_ins != nullptr);
        // rocksdb::WriteOptions wo();
        
        db_status s = db_ins->Put(struct_to_string<vertex_table_item_key>(v_item->key), 
                                struct_to_string<vertex_table_item_value>(v_item->value));
        assert(s.ok());
        return ret_status::succeed;
    } 

    /*
    Add Edge (Wrap InsertNewVertex)
    */
    ret_status add_edge(type_t src_v_type,
    ide_t src_v_id, type_t e_type, type_t dst_v_type, ide_t dst_v_id,  ide_t e_id, std::string json_str) {

        ret_status s = insert_new_conn(src_v_type, src_v_id, e_type, dst_v_type, dst_v_id, e_id, json_str);
        if (s != ret_status::succeed) {
            return s;
        }

        return insert_new_edge(e_type, e_id, src_v_type, src_v_id, dst_v_type, dst_v_id);
    }

    /*
    InsertNewConn()
    Input:
    1:Required: Source Vertex Type ID
    2.Required: Source Vertex ID
    3.Required: Property Type ID
    4.Required: Json String
    Output:
    1:succeed or failed
    Implementation:
    Refer to the Set API in rocksdb
    */
    ret_status insert_new_conn(type_t src_v_type,
    ide_t src_v_id, type_t e_type, type_t dst_v_type, ide_t dst_v_id,  ide_t e_id, std::string json_str) {
        assert(db_ins != nullptr);
        
        conn_table_item *conn_item = new conn_table_item(new conn_table_item_key(src_v_type, src_v_id, e_type, dst_v_type, dst_v_id, e_id), new conn_table_item_value(json_str));
        insert_new_conn(conn_item);
        delete conn_item;
        return ret_status::succeed;
    }
    
    /*
    InsertNewConn()
    Input:
    1:Required: vertex 
    Output:
    1:succeed or failed
    Implementation:
    Refer to the Set API in rocksdb
    */
    ret_status insert_new_conn(conn_table_item *conn_item) {
        assert(db_ins != nullptr);
        // rocksdb::WriteOptions wo();
        
        db_status s = db_ins->Put(struct_to_string<conn_table_item_key>(conn_item->key), 
                                struct_to_string<conn_table_item_value>(conn_item->value));
        assert(s.ok());
        return ret_status::succeed;
    }

    /*
    InsertNewEdge()
    Input:    
    1:Required: Edge Type ID
    2.Required: Edge ID  
    3:Required: Source Vertex Type ID
    4.Required: Source Vertex ID
    5.Required: Destination Vertex Type ID
    6.Required: Destination Vertex ID
    Output:
    1:succeed or failed
    Implementation:
    Refer to the Set API in rocksdb
    */
    ret_status insert_new_edge(type_t e_type, ide_t e_id, type_t src_v_type,
    ide_t src_v_id, type_t dst_v_type, ide_t dst_v_id) {
        assert(db_ins != nullptr);
        
        edge_table_item *edge_item = new edge_table_item(new edge_table_item_key(e_type, e_id), new edge_table_item_value(src_v_type, src_v_id, dst_v_type, dst_v_id));
        insert_new_edge(edge_item);
        delete edge_item;
        return ret_status::succeed;
    }
    
    /*
    InsertNewEdge()
    Input:
    1:Required: vertex 
    Output:
    1:succeed or failed
    Implementation:
    Refer to the Set API in rocksdb
    */
    ret_status insert_new_edge(edge_table_item *edge_item) {
        assert(db_ins != nullptr);
        // rocksdb::WriteOptions wo();
        
        db_status s = db_ins->Put(struct_to_string<edge_table_item_key>(edge_item->key), 
                                struct_to_string<edge_table_item_value>(edge_item->value));
        assert(s.ok());
        return ret_status::succeed;
    }

    /*
    GetMaxVertex()
    Input:
    Ouput:
    1. Max Vertex ID
    */
    vertex_t get_max_vertex() {

        assert(db_ins != nullptr);

        std::string seek_str;
        seek_str.append(reinterpret_cast<const char *>(&MIN_E_TYPE), sizeof(type_t));

        std::string res_str = db_ins->seek_to_last_key(seek_str);

        vertex_t res;
        std::memcpy(reinterpret_cast<char *>(&res), res_str.c_str(), sizeof(vertex_t));

        return res;
    }

    ide_t get_max_vertex_id() {
        return get_max_vertex().src_id;
    }

    /*
    GetMaxEdge()
    Input:
    Ouput:
    1. Max Vertex ID
    */
    edge_t get_max_edge() {

        assert(db_ins != nullptr);

        std::string res_str = db_ins->seek_to_final_last_key();

        edge_t res;
        std::memcpy(reinterpret_cast<char *>(&res), res_str.c_str(), sizeof(edge_t));

        return res;
    }

    ide_t get_max_edge_id() {
        return get_max_edge().edge_id;
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

    ret_status explore_scan(std::vector<ide_t> &v_vec_sorted, 
                    std::vector<ide_t> &dst_v_vec_out) {

#ifdef USE_BENCH
        auto begin_all = std::chrono::system_clock::now();
#endif
        std::vector<std::pair<std::string, std::string>> explore_edges;
        std::vector<int> explore_edges_len;

        std::vector<std::string> v_vec_str;
        for (uint64_t &src_v: v_vec_sorted) {
            std::string seek_str;
            seek_str.append(reinterpret_cast<char *>(&V_TYPE), sizeof(uint16_t));
            seek_str.append(reinterpret_cast<char *>(&src_v), sizeof(uint64_t));
            seek_str.append(reinterpret_cast<char *>(&E_TYPE), sizeof(uint16_t));
            v_vec_str.emplace_back(seek_str);
        } 
#ifdef USE_BENCH
        auto begin = std::chrono::system_clock::now();
#endif
        db_ins->explore_impl(v_vec_str, explore_edges, explore_edges_len);
#ifdef USE_BENCH
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = end - begin;
        std::cout << "DB Imple Time:" << diff.count() << std::endl;
#endif

        k_way_merge_sort(explore_edges, explore_edges_len, dst_v_vec_out);

#ifdef USE_BENCH
        auto end_all = std::chrono::system_clock::now();
        std::chrono::duration<double> diff_all = end_all - begin_all;
        std::cout << "Explore Time:" << diff_all.count() << std::endl;
#endif   
        return ret_status::succeed;
    }

    ret_status explore_scan(std::vector<vertex_t> v_vec_sorted, 
                    std::function<bool(vertex_table_item)> src_v_filter,
                    std::function<bool(conn_table_item)> edge_filter,
                    bool ouput_src_vec,
                    std::vector<vertex_t> &src_v_vec_out,
                    bool ouput_dst_vec,
                    std::vector<vertex_t> &dst_v_vec_out) {

        // TODO

        if (src_v_filter != nullptr || edge_filter != nullptr) {
            return ret_status::error;
        }
        return ret_status::succeed;
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
    ret_status bfs(std::vector<ide_t> vec_sorted,
            int bfs_total_times,
            std::vector<ide_t> &v_out, long long &total_counter) {
        
        hm_ins->clear();
        total_counter = 0;
        std::vector<ide_t> res_vec_sorted;

        int tmp_bfs_total_times = (bfs_total_times == -1 ? INT_MAX : bfs_total_times);

        for (int i = 0; i < tmp_bfs_total_times; i ++ ) {

            // for_each(vec_sorted.begin(), vec_sorted.end(), [](ide_t v){
            //     printf("||%d\n", (long)v);
            // });
            total_counter += vec_sorted.size();

            explore_scan(vec_sorted, res_vec_sorted);    

            // for_each(res_vec_sorted.begin(), res_vec_sorted.end(), [](ide_t v){
            //     printf("<>%d\n", (long)v);
            // });

            // vec_sorted.clear();
            filter_active(res_vec_sorted, vec_sorted);

            if (bfs_total_times == -1 && vec_sorted.size() == 0) {

                return ret_status::succeed;                
            }
        }

        v_out.swap(vec_sorted);
            
        return ret_status::succeed;
    }

    ret_status bfs(vertex_t v_in,
            int bfs_total_times,
            std::vector<vertex_t> &v_out) {

        return ret_status::succeed;
    }

    ret_status bfs(std::vector<vertex_t> &vec_sorted,
            int bfs_total_times,
            std::vector<vertex_t> &v_out) {
        
        return ret_status::succeed;
    }

    ret_status bfs(std::vector<vertex_t> &vec_sorted,
            std::function<bool(vertex_table_item)> src_v_filter,
            std::function<bool(conn_table_item)> edge_filter,
            int bfs_total_times,
            std::vector<vertex_t> &v_out) {
        
        if (src_v_filter != nullptr || edge_filter != nullptr) {
            return ret_status::error;
        }    

        return ret_status::succeed;
    }
};

}

#endif // GRAPHDB_HPP_
