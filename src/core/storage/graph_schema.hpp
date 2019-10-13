#if !defined(GRAPH_SCHEMA_HPP_)
#define GRAPH_SCHEMA_HPP_

#include <cstdint>
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

struct packed vertex_table_item_key: rocksdb_key
{
    type_t src_v_type;
    ide_t src_v_id;
    type_t p_type;
};

struct packed vertex_table_item_value: rocksdb_value
{

};

struct vertex_table_item
{
    vertex_table_item_key key;
    vertex_table_item_value value;
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
};

struct packed conn_table_item_value: rocksdb_value
{

};

struct conn_table_item
{
    conn_table_item_key key;
    conn_table_item_value value;
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
    reinterpret_cast<char*>(st);
}

#endif // GRAPH_SCHEMA_HPP_
