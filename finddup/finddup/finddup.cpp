// finddup.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "fs_duplicates.h"
#include "fs_comparer.h"
#include "fs_reader.h"
#include "paths_buffer.h"

int main()
{
    paths_buffer paths;
    fs_duplicates duplicates;
    fs_comparer comparer(paths, duplicates);
    fs_reader reader(".", paths);
    boost::thread reader_thread(reader);
    boost::thread comparer_thread(comparer);

    boost::chrono::duration<double> sec(1);
    if (!comparer_thread.try_join_for(sec))
    {
        std::ofstream fout("con");
        do
        {
            fout << "\r" << "Processed " << duplicates.processed();
            fout.flush();
        } while (!comparer_thread.try_join_for(sec));
    }
    reader_thread.join();

    if (duplicates.duplicates_map().empty())
    {
        std::cout << "No duplicates have been found" << std::endl;
    }
    else
    {
        std::cout << "Found duplicates:" << std::endl;
        int i = 0;
        for (const fs_duplicates::duplicates_map_t::value_type& v : duplicates.duplicates_map())
        {
            std::cout << ++i << std::endl;
            std::cout << v.first << std::endl;
            for (const boost::filesystem::path& p : v.second)
            {
                std::cout << p << std::endl;
            }
            std::cout << std::endl;
        }
    }
    return 0;
}

