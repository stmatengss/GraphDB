#if !defined(GRAPHDB_ALGORITHM_HPP_)
#define GRAPHDB_ALGORITHM_HPP_

#include <vector>
#include <string>
#include <set>

#include "core/common.h"
#include "core/storage/graph_schema.hpp"

namespace graphdb 
{

    ret_status k_way_merge_sort(std::vector<std::pair<std::string, std::string>> &explore_edges, std::vector<int> &explore_edges_len, std::vector<ide_t> &sorted_explore_edges) {

        std::set<ide_t> dst_v_set;

        for (auto &explore_edge: explore_edges) {
            std::string key_str = explore_edge.first;
            // vertex_table_item_key *vertex_item_key = reinterpret_cast<vertex_table_item_key *>(key_str.c_str());
            ide_t dst_v = *((ide_t *)(key_str.c_str() + dst_v_id_offset));
            dst_v_set.insert(dst_v);
        }
        // TODO: K-way Merge Sort
        sorted_explore_edges = std::vector<ide_t>(dst_v_set.begin(), dst_v_set.end()); 
        return ret_status::succeed;
    }
}

#endif // GRAPHDB_ALGORITHM_
