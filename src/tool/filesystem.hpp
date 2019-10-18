#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <string>

inline bool file_exists(std::string filename) {
	struct stat st;
	return stat(filename.c_str(), &st)==0;
}

inline long file_size(std::string filename){
	struct stat st;
	assert(stat(filename.c_str(), &st)==0);
	return st.st_size;
}

inline void create_directory(std::string path) {
	assert(mkdir(path.c_str(), 0764) == 0 || errno==EEXIST);
}

inline pid_t remove_directory(std::string path) {
	char command[1024];
	sprintf(command, "rm -rf %s", path.c_str());
	return system(command);
}
#endif