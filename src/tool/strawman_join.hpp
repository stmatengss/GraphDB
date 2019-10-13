#ifndef FS_IO_HPP_

#define FS_IO_HPP_

#include <fstream>
#include <random>
#include <string>
#include <cassert>

#include "rocksdb_api.h"

using namespace rocksdb;
using namespace std;

void load_simple_graph_from_file(string input, DB* db) {
	std::ifstream input_file(input.c_str());

	if(!input_file){
		printf("Input file not found!\n");
		exit(-1);
	} 

	char buf[256];
    char str_item_key[256];
    char str_item_value[256];

	long count = 0;

	while(!input_file.eof()) {
		if (!input_file.getline(buf, 256) || buf[0] == '#')
			continue;

		int src, dst, weight;
		sscanf(buf, "%d %d %d", &src, &dst, &weight);

        memcpy(str_item_key, &src, sizeof(int));
        memcpy(str_item_key + sizeof(int), &dst, sizeof(int));
        memcpy(str_item_value, &weight, sizeof(int));

        Status s = db->Put(WriteOptions(), 
                string(str_item_key, str_item_key + sizeof(int) * 2), 
                string(str_item_value, str_item_value + sizeof(int)));

        assert(s.ok());

		count++;

	}

	input_file.close();
	std::cout<<"edge:"<<count<<std::endl;
}

void explore_scan() {
	
}

#endif