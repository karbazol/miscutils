#pragma once
#include "paths_buffer.h"

class fs_reader
{
private:
    paths_buffer& _paths;
    std::string _root_dir;
public:
    fs_reader(const std::string& root_dir, paths_buffer& names): _paths(names),
        _root_dir(root_dir) {}
    ~fs_reader();
    void operator()();
};

