#include <cstdio>
#include <string>
#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

using namespace rocksdb;
using namespace std;

std::string kDBPath = "/tmp/rocksdbtest-1041/dbbench";

/*
   void GenerateKeyFromInt(uint64_t v, int64_t num_keys, string* key) {
   if (!keys_.empty()) {
   assert(FLAGS_use_existing_keys);
   assert(keys_.size() == static_cast<size_t>(num_keys));
   assert(v < static_cast<uint64_t>(num_keys));
 *key = keys_[v];
 return;
 }
 char* start = const_cast<char*>(key->data());
 char* pos = start;
 if (keys_per_prefix_ > 0) {
 int64_t num_prefix = num_keys / keys_per_prefix_;
 int64_t prefix = v % num_prefix;
 int bytes_to_fill = std::min(prefix_size_, 8);
 if (port::kLittleEndian) {
 for (int i = 0; i < bytes_to_fill; ++i) {
 pos[i] = (prefix >> ((bytes_to_fill - i - 1) << 3)) & 0xFF;
 }
 } else {
 memcpy(pos, static_cast<void*>(&prefix), bytes_to_fill);
 }
 if (prefix_size_ > 8) {
// fill the rest with 0s
memset(pos + 8, '0', prefix_size_ - 8);
}
pos += prefix_size_;
}

int bytes_to_fill = std::min(key_size_ - static_cast<int>(pos - start), 8);
if (port::kLittleEndian) {
for (int i = 0; i < bytes_to_fill; ++i) {
pos[i] = (v >> ((bytes_to_fill - i - 1) << 3)) & 0xFF;
}
} else {
memcpy(pos, static_cast<void*>(&v), bytes_to_fill);
}
pos += bytes_to_fill;
if (key_size_ > pos - start) {
memset(pos, '0', key_size_ - (pos - start));
}
}
*/

int main(int argc, char **argv) {

	bool use_thread = false;
	bool use_multiget = false;
	int batch_size = 8;
	if (argc > 2 && argv[2][0] == '1') {
		use_thread = true;
		cout << "[Use thread]" << endl;
	}

	if (argc > 1 && argv[1][0] == '1') {
		use_multiget = true;
		cout << "[Use multiget]" << endl;
	}

	DB* db;
	Options options;
	// Optimize RocksDB. This is the easiest way to get RocksDB to perform well
	options.IncreaseParallelism();
	options.OptimizeLevelStyleCompaction();

	// open DB
	Status s = DB::Open(options, kDBPath, &db);
	assert(s.ok());

	//std::string begin_k("key0");


	auto start = std::chrono::system_clock::now();

	int counter = 0;
	auto option = ReadOptions();  

	vector<Slice> candidate_vec;
	std::random_device rd;

	Iterator *it = db->NewIterator(ReadOptions());
	it->SeekToFirst();
	for (; it->Valid(); it->Next()) {
		counter ++;
		if (rd() % 4 == 0) {
			candidate_vec.emplace_back(it->key());
		} 
		//if (counter ++ > 10000) break;
		//		if (counter < 99)
		//			std::cout << it->key().ToString() << ": " << it->value().ToString() << std::endl;
		//std::cout << it->key().ToString().size() << endl;
		//printf("%s: %s\n", it->key(), it->value());
	}

	auto end = std::chrono::system_clock::now();
	auto diff = end - start;
	cout << "Time:" << diff.count() << ", Counter:" << counter << ", IOPS:" << counter * 1000.0 / diff.count() << endl;
    
    int can_len = candidate_vec.size();
    for (int i = 0; i < counter; i ++ ) {
        candidate_vec.emplace_back(candidate_vec[rd()/can_len]);
	}
    /*
    for (int i = 0; i < counter / 4; i ++ ) {
        int num = rd() / can_len;
        string tmp(candidate_vec[num].ToString());
        tmp[0] = char((tmp[0] + 1) % 256);
        candidate_vec.emplace_back(Slice(tmp));
	}
    */

	std::mt19937 g(rd());
	shuffle(candidate_vec.begin(), candidate_vec.end(), g);

	int iter_num = candidate_vec.size();

	start = std::chrono::system_clock::now();

	if (use_thread) {
		int thread_num = 4;
		// TODO
		for (int i = 0; i < iter_num / thread_num; i ++ ) {
		}
	} else {
		if (use_multiget) {
			vector<std::string> values(batch_size);
			auto ro = ReadOptions();
			for (int i = 0; i < iter_num - batch_size; i += batch_size ) {
				db->MultiGet(ro, vector<Slice>(candidate_vec.begin() + i, candidate_vec.begin() + i + batch_size), &values);
			}
		} else {
			std::string value;
			auto ro = ReadOptions();
			for (int i = 0; i < iter_num; i += 1 ) {

				auto ss = db->Get(ro, candidate_vec[i], &value);

/*
				if (!ss.ok()) {
					std::cout << "Error Happenes" << std::endl;
					break;
				}
*/
			}
		}
	}
	end = std::chrono::system_clock::now();

	diff = end - start;
	cout << "Time:" << diff.count() << ", Counter:" << iter_num << ", IOPS:" << iter_num * 1000.0 / diff.count() << endl;

	delete it;

	// atomically apply a set of updates
	/*
	   {
	   WriteBatch batch;
	   batch.Delete("key1");
	   batch.Put("key2", value);
	   s = db->Write(WriteOptions(), &batch);
	   }

	   s = db->Get(ReadOptions(), "key1", &value);
	   assert(s.IsNotFound());

	   db->Get(ReadOptions(), "key2", &value);
	   assert(value == "value");

	   {
	   PinnableSlice pinnable_val;
	   db->Get(ReadOptions(), db->DefaultColumnFamily(), "key2", &pinnable_val);
	   assert(pinnable_val == "value");
	   }

	   {
	   std::string string_val;
	// If it cannot pin the value, it copies the value to its internal buffer.
	// The intenral buffer could be set during construction.
	PinnableSlice pinnable_val(&string_val);
	db->Get(ReadOptions(), db->DefaultColumnFamily(), "key2", &pinnable_val);
	assert(pinnable_val == "value");
	// If the value is not pinned, the internal buffer must have the value.
	assert(pinnable_val.IsPinned() || string_val == "value");
	}

	PinnableSlice pinnable_val;
	db->Get(ReadOptions(), db->DefaultColumnFamily(), "key1", &pinnable_val);
	assert(s.IsNotFound());
	// Reset PinnableSlice after each use and before each reuse
	pinnable_val.Reset();
	db->Get(ReadOptions(), db->DefaultColumnFamily(), "key2", &pinnable_val);
	assert(pinnable_val == "value");
	pinnable_val.Reset();
	// The Slice pointed by pinnable_val is not valid after this point
	*/
	delete db;

	return 0;
}

