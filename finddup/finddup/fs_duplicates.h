#pragma once
class fs_duplicates
{
public:
    typedef std::list<boost::filesystem::path> path_list_t;
    typedef std::map<boost::filesystem::path, path_list_t> duplicates_map_t;
private:
    class md5_hash
    {
    private:
        boost::uuids::detail::md5::digest_type _value;
    public:
        md5_hash() { memset(&_value, 0, sizeof(_value)); }
        md5_hash(const boost::uuids::detail::md5::digest_type& value)
        {
            memmove(&_value, &value, sizeof(value));
        }
        md5_hash(const md5_hash& right)
        {
            memmove(&_value, &right._value, sizeof(_value));
        }
        bool operator < (const md5_hash& right) const
        {
            for (size_t i = 0; i < sizeof(_value)/sizeof(_value[0]); i++)
            {
                if (_value[i] < right._value[i])
                {
                    return true;
                }
                else if (_value[i] > right._value[i])
                {
                    return false;
                }
            }
            return false;
        }
        boost::uuids::detail::md5::digest_type& value() { return _value; }
        void calc_hash(const boost::filesystem::path& path, uintmax_t size);
    };
    
    typedef std::list<path_list_t> duplicates_list_t;
    typedef std::map<md5_hash, path_list_t> md5_map_t;
    typedef std::unordered_map < uintmax_t, md5_map_t> size_map_t;
private:
    volatile uintmax_t _processed;
    size_map_t _size_map;
    duplicates_map_t _duplicates_map;
public:
    fs_duplicates() : _processed(0), _size_map(), _duplicates_map() {}
    ~fs_duplicates();
    void add(const boost::filesystem::path& path);
    const duplicates_map_t& duplicates_map() const { return _duplicates_map; }
    uintmax_t processed() const { return _processed; }
};

