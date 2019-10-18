#if !defined(DATA_LOADER_HPP_)
#define DATA_LOADER_HPP_

#include <string>
#include <iostream>
#include <fstream>
#include <functional>

#include "storage/graphdb.hpp"
namespace graphdb {
    
class data_loader
{
public:
    std::string file_path;
public:
    data_loader(std::string file_path_): file_path(file_path_) { }
    ~data_loader() { }
};

class simple_data_loader: public data_loader
{
private:
    /* data */
public:
    simple_data_loader(std::string file_path_): data_loader(file_path_) { }
    ~simple_data_loader() {
        
    }

    // void fill_db_wo_weight(std::function<void(ide_t, ide_t, ide_t)> &db_func) {
    template <class T>
    void fill_db_wo_weight(T &out_ins) {

        std::ifstream input_file(file_path.c_str());

        if(!input_file){
            printf("Input file not found!\n");
            exit(-1);
        } 

        char buf[256];
        long count = 0;
        while(!input_file.eof()) {
            if (!input_file.getline(buf, 256) || buf[0] == '#')
                continue;

            int a, b, c;
            sscanf(buf, "%d %d", &a, &b);
            
            out_ins.insert_edge_wo_proper(a, b, count);
            // db_func(a, b, count);
            // std::bind(db_func, a, b, count);

            count++;
        }

        input_file.close();
        std::cout << "edge:"<< count << std::endl;
    }
};

}

#endif // DATA_LOADER_HPP_
