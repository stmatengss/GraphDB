#if !defined(GRAPHDB_HPP_)
#define GRAPHDB_HPP_

#include <vector>
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
        
        vertex_table_item v_item(vertex_table_item_key(src_v_type, src_v_id, p_type), vertex_table_item_value(json_str));
        insert_new_vertex(&v_item);

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
        
        db_status s = db_ins->Put(struct_to_string<vertex_table_item_key>(&v_item->key), 
                                struct_to_string<vertex_table_item_value>(&v_item->value));
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
        
        conn_table_item v_item(conn_table_item_key(src_v_type, src_v_id, e_type, dst_v_type, dst_v_id, e_id), conn_table_item_value(json_str));
        insert_new_conn(&v_item);

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
        
        db_status s = db_ins->Put(struct_to_string<conn_table_item_key>(&conn_item->key), 
                                struct_to_string<conn_table_item_value>(&conn_item->value));
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
    ret_status explore_scan(std::vector<ide_t> v_vec_sorted, 
                    std::function<bool(vertex_table_item)> src_v_filter,
                    std::function<bool(conn_table_item)> edge_filter,
                    std::vector<ide_t> &src_v_vec_out,
                    bool ouput_dst_vec,
                    std::vector<ide_t> &dst_v_vec_out) {
        


        if (src_v_filter != nullptr) {

        }
    
        if (edge_filter != nullptr) {

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

    ret_status bfs(std::vector<ide_t> &vec_sorted,
            std::function<bool(vertex_table_item)> src_v_filter,
            std::function<bool(conn_table_item)> edge_filter,
            int bfs_total_times,
            std::vector<ide_t> &v_out) {
        
        

        return ret_status::succeed;
    }
};

}

#endif // GRAPHDB_HPP_
