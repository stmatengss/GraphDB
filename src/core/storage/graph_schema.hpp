#if !defined(GRAPH_SCHEMA_HPP_)
#define GRAPH_SCHEMA_HPP_

#include <cstdint>
#include <string>
#include "core/utils.h"

/*
Data Model
 Type Table: [Vertex/Edge/Property Type Identifier] -> [Type ID]
 Each directing edge type will generate two type ID for both direction
 Assure that the ID for property type must be smaller than edge type
Vertex Table: 
 Design Choice
  store the value and relation in the same table with property smaller always at the head of edges
 Reason
  we can directly obtain the needed property value without the need of a join (i.e., a separate PropertyScan and a PropertyAttach operator), this is very expensive when there are many kinds of properties needs to be attached and it can not be achieved in only one path of the data
  we can push down more filter (the local filter condition) to VertexScan
Data
 [Source Vertex Type ID][Source Vertex ID][Property Type ID] -> 
 [Source Vertex Type ID][Source Vertex ID][Edge Type ID][Destination Vertex Type ID][Destination Vertex ID][Edge ID] -> [Edge Property Values in JSON]
Edge Table: 
 [Edge Type ID][Edge ID] -> [Source Vertex Type ID][Source Vertex ID][Destination Vertex Type ID][Destination Vertex ID]
*/

/*
    Type: 16 bits
    ID: 64 bits
*/

namespace graphdb
{
    
typedef uint64_t ide_t;
typedef uint16_t type_t;

const type_t MIN_P_TYPE = 1 << 10;
const type_t MAX_P_TYPE = 2 << 10;

const type_t MIN_V_TYPE = 2 << 10;
const type_t MAX_V_TYPE = 3 << 10;

const type_t MIN_E_TYPE = 3 << 10;
const type_t MAX_E_TYPE = 4 << 10;

struct rocksdb_key 
{

};

struct rocksdb_value
{

};

/* Vertex Table */
// [Source Vertex Type ID][Source Vertex ID][Property Type ID] -> 

struct packed vertex_table_item_key: rocksdb_key
{
    type_t src_v_type;
    ide_t src_v_id;
    type_t p_type;
    vertex_table_item_key(type_t src_v_type_, ide_t src_v_id_, 
        type_t p_type_): src_v_type(src_v_type_), src_v_id(src_v_id_), p_type(p_type_){}
};

struct vertex_table_item_value: rocksdb_value
{
    std::string json_str;
    vertex_table_item_value(std::string json_str_): json_str(json_str_) {}
};

struct vertex_table_item
{
    vertex_table_item_key key;
    vertex_table_item_value value;
    vertex_table_item(vertex_table_item_key key_,
        vertex_table_item_value value_): key(key_), value(value_) {}
};

/* Conn Table */
// [Source Vertex Type ID][Source Vertex ID][Edge Type ID][Destination Vertex Type ID][Destination Vertex ID][Edge ID] -> [Edge Property Values in JSON]

struct packed conn_table_item_key: rocksdb_key
{
    type_t src_v_type;
    ide_t src_v_id;
    type_t e_type;
    type_t dst_v_type;
    ide_t dst_v_id;
    ide_t e_id;
    conn_table_item_key(type_t src_v_type_,
    ide_t src_v_id_, type_t e_type_, type_t dst_v_type_, ide_t dst_v_id_,  ide_t e_id_): src_v_type(src_v_type_), src_v_id(src_v_id_), e_type(e_type_), dst_v_type(dst_v_type_), dst_v_id(dst_v_id_), e_id(e_id_) {}
};

struct conn_table_item_value: rocksdb_value
{
    std::string json_str;
    conn_table_item_value(std::string json_str_): json_str(json_str_) {}
};

struct conn_table_item
{
    conn_table_item_key key;
    conn_table_item_value value;
    conn_table_item(conn_table_item_key key_,
        conn_table_item_value value_): key(key_), value(value_) {}
};

/* Edge Table */
// [Edge Type ID][Edge ID] -> [Source Vertex Type ID][Source Vertex ID][Destination Vertex Type ID][Destination Vertex ID]

struct packed edge_table_item_key: rocksdb_key
{
    type_t e_type;
    ide_t e_id;
};

struct packed edge_table_item_value: rocksdb_value
{
    type_t src_v_type;
    ide_t src_v_id;
    type_t dst_v_type;
    ide_t dst_v_id;
};

struct edge_table_item
{
    edge_table_item_key key;
    edge_table_item_value value;
};

template<typename T>
char *struct_to_char(T *st) {
    return reinterpret_cast<char*>(st);
}

template<typename T>
std::string struct_to_string(T *st) {
    return std::string(reinterpret_cast<char*>(st));
}

} // graphdb

#endif // GRAPH_SCHEMA_HPP_
