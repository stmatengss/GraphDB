#include <cstdio>
#include <string>
#include <iostream>
#include <chrono>

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

	bool use_tailing = false;
	if (argc > 1 && argv[1][0] == '1') {
		use_tailing = true;
        cout << "[Use tailing]" << endl;
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
	if (use_tailing) {
		option.tailing = true;
	} 

	Iterator *it = db->NewIterator(ReadOptions());
	it->SeekToFirst();
	for (; it->Valid(); it->Next()) {
		counter ++;
		//if (counter ++ > 10000) break;
		//if (counter < 99)
		//	std::cout << it->key().ToString() << ": " << it->value().ToString() << std::endl;
		//std::cout << it->key().ToString().size() << endl;
		//printf("%s: %s\n", it->key(), it->value());
	}

	auto end = std::chrono::system_clock::now();
	auto diff = end - start;
	cout << "Time:" << diff.count() << ", Counter:" << counter << ", IOPS:" << counter * 1000.0 / diff.count() << endl;

	std::string value;
	std::string key;
	key.resize(16);
	for (int i = 0; i < 16; i ++) 
		if (i < 8) key[i] = char(0);
		else key[i] = '0';

	auto ss = db->Get(ReadOptions(), key, &value);

	cout << key << ":" << value << endl;    


	if (ss.ok()) {
		std::cout << "Everything goes well" << std::endl;
	}

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

