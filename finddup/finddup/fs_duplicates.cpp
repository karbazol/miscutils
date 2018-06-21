#include "stdafx.h"
#include "fs_duplicates.h"

fs_duplicates::~fs_duplicates()
{
}

void fs_duplicates::md5_hash::calc_hash(const boost::filesystem::path& path, uintmax_t size)
{
    if (size == 0)
    {
        memset(&_value, 0, sizeof(_value));
        return;
    }
    std::ifstream fin(path.native(), std::ios_base::binary);
    if (size <= sizeof(_value))
    {
        memset(&_value, 0, sizeof(_value));
        fin.read(reinterpret_cast<char*>(&_value), sizeof(_value));
    }
    else
    {
        char buff[4096];
        boost::uuids::detail::md5 md5;
        while (!fin.eof())
        {
            fin.read(buff, sizeof(buff));
            uintmax_t read = fin.gcount();
            if (!read)
            {
                break;
            }
            md5.process_bytes(buff, read);
        }
        md5.get_digest(_value);
    }
}

static bool same_content(const boost::filesystem::path& left,
    const boost::filesystem::path& right)
{
    char buff1[4096], buff2[4096];
    std::ifstream f1(left.native(), std::ios_base::binary), f2(right.native(), std::ios_base::binary);
    while (!f1.eof() && !f2.eof())
    {
        f1.read(buff1, sizeof(buff1));
        f2.read(buff2, sizeof(buff2));
        uintmax_t read1 = f1.gcount();
        uintmax_t read2 = f2.gcount();
        if (read1 != read2)
        {
            return false;
        }
        if (memcmp(buff1, buff2, read1))
        {
            return false;
        }
    }
    return f1.eof() && f2.eof();
}

void fs_duplicates::add(const boost::filesystem::path & path)
{
    _processed++;
    uintmax_t size = boost::filesystem::file_size(path);
    md5_hash hash_value;
    hash_value.calc_hash(path, size);
    size_map_t::iterator to_insert = _size_map.find(size);
    if (to_insert == _size_map.end())
    {
        path_list_t paths;
        paths.push_back(path);
        md5_map_t md5_map;
        md5_map[hash_value] = paths;
        _size_map.insert(size_map_t::value_type(size, md5_map));
        return;
    }
    else
    {
        md5_map_t& md5_map = _size_map[size];
        md5_map_t::iterator it = md5_map.find(hash_value);
        if (it == md5_map.end())
        {
            path_list_t paths;
            paths.push_back(path);
            md5_map[hash_value] = paths;
        }
        else
        {
            path_list_t& paths = it->second;
            for (boost::filesystem::path& known : paths)
            {
                if (same_content(known, path))
                {
                    duplicates_map_t::iterator p = _duplicates_map.find(known);
                    if (p == _duplicates_map.end())
                    {
                        path_list_t new_paths;
                        new_paths.push_back(path);
                        _duplicates_map[known] = new_paths;
                    }
                    else
                    {
                        p->second.push_back(path);
                    }

                    return;
                }
            }
            paths.push_back(path);
        }
    }
}
