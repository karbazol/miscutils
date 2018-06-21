#include "stdafx.h"
#include "fs_reader.h"


fs_reader::~fs_reader()
{
}

void fs_reader::operator()()
{
    std::list<boost::filesystem::path> dirs;
    dirs.push_back(_root_dir);

    while (!dirs.empty())
    {
        boost::filesystem::path path = dirs.front();
        dirs.pop_front();
        for (boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(path))
        {
            if (boost::filesystem::is_directory(x.path()))
            {
                dirs.push_back(x.path());
            }
            else if (boost::filesystem::is_regular_file(x.path()))
            {
                _paths.push(x.path());
            }
        }
    }
    _paths.finish();
}
