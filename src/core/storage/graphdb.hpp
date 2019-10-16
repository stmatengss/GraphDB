#if !defined(GRAPHDB_HPP_)
#define GRAPHDB_HPP_

#include <vector>
#include <set>
#include <functional>

#include "core/common.h"
#include "core/rocksdb_wrapper.hpp"
#include "core/storage/graph_schema.hpp"

namespace graphdb 
{

class graphdb 
{
private:
    rocksdb_wrapper *db_ins;

    // meta-data

public:
    graphdb(std::string db_path) { 
        db_ins = new rocksdb_wrapper(db_path);
    }
    ~graphdb() { 
        delete db_ins;
    }
    
    /*  */

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
        
        conn_table_item *v_item = new conn_table_item(new conn_table_item_key(src_v_type, src_v_id, e_type, dst_v_type, dst_v_id, e_id), new conn_table_item_value(json_str));
        insert_new_conn(v_item);

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
    ide_t src_v_id, type_t dst_v_type, ide_t dst_v_id,  std::string json_str) {
        assert(db_ins != nullptr);
        
        edge_table_item *v_item = new edge_table_item(new edge_table_item_key(e_type, e_id), new edge_table_item_value(src_v_type, src_v_id, dst_v_type, dst_v_id));
        insert_new_edge(v_item);

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
        
        std::vector<std::pair<std::string, std::string>> explore_edges;
        std::set<ide_t> dst_v_set;

        std::vector<std::string> v_vec_str;
        for (uint64_t &src_v: v_vec_sorted) {
            std::string seek_str;
            seek_str.append(reinterpret_cast<char *>(&V_TYPE), sizeof(uint16_t));
            seek_str.append(reinterpret_cast<char *>(&src_v), sizeof(uint64_t));
            seek_str.append(reinterpret_cast<char *>(&E_TYPE), sizeof(uint16_t));
            v_vec_str.emplace_back(seek_str);
        } 
        db_ins->explore_impl(v_vec_str, explore_edges);

        for (auto &explore_edge: explore_edges) {
            std::string key_str = explore_edge.first;
            // vertex_table_item_key *vertex_item_key = reinterpret_cast<vertex_table_item_key *>(key_str.c_str());
            ide_t dst_v = *((ide_t *)(key_str.c_str() + dst_v_id_offset));
            dst_v_set.insert(dst_v);
        }
        // TODO: K-way Merge Sort
        dst_v_vec_out = std::vector<ide_t>(dst_v_set.begin(), dst_v_set.end()); 

        return ret_status::succeed;
    }

    ret_status explore_scan(std::vector<vertex_t> v_vec_sorted, 
                    std::function<bool(vertex_table_item)> src_v_filter,
                    std::function<bool(conn_table_item)> edge_filter,
                    bool ouput_src_vec,
                    std::vector<vertex_t> &src_v_vec_out,
                    bool ouput_dst_vec,
                    std::vector<vertex_t> &dst_v_vec_out) {

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
    ret_status bfs(ide_t v_in,
            int bfs_total_times,
            std::vector<ide_t> &v_out) {
            
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
