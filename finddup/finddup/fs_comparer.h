#pragma once
#include "fs_duplicates.h"
#include "paths_buffer.h"

class fs_comparer
{
private:
    paths_buffer & _paths;
    fs_duplicates& _duplicates;
public:
    fs_comparer(paths_buffer& paths, fs_duplicates& duplicates) : 
        _paths(paths), _duplicates(duplicates){}
    ~fs_comparer();
    void operator()();
};

