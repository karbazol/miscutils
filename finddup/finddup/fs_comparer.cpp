#include "stdafx.h"
#include "fs_comparer.h"


fs_comparer::~fs_comparer()
{
}

void fs_comparer::operator()()
{
    boost::filesystem::path path;
    while (_paths.pop(path))
    {
        _duplicates.add(path);
    }
}
