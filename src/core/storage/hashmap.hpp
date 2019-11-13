#if !defined(HASHMAP_HPP_)
#define HASHMAP_HPP_

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/concurrent_hash_map.h>

namespace graphdb{

class hash_map {
public:
    hash_map() {
    }
    ~hash_map() = delete;

    tbb::concurrent_hash_map<uint64_t, bool> r_ht;
    void set_bit(uint64_t key, bool value) {

        tbb::concurrent_hash_map<uint64_t, bool>::accessor ac;
        r_ht.insert(ac, key);
        ac->second = value;
        ac.release();
    }

    bool get_bit(uint64_t key) {

        tbb::concurrent_hash_map<uint64_t, bool>::accessor ac;
        if(r_ht.find(ac, key)) {
            ac.release();
            return true;
        }
        ac.release();
        return false;
    }

    void clear() {
        r_ht.clear();
    }
};

}


#endif // HASHMAP_HPP_