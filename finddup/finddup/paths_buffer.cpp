#include "stdafx.h"
#include "paths_buffer.h"


paths_buffer::~paths_buffer()
{
}

void paths_buffer::push(const boost::filesystem::path & path)
{
    {
        boost::lock_guard<boost::mutex> guard(_mtx);
        _list.push_back(path);
    }
    _cond.notify_one();
}

bool paths_buffer::pop(boost::filesystem::path & path)
{
    boost::unique_lock<boost::mutex> lock(_mtx);
    while (_list.empty() && !_finished)
    {
        _cond.wait(lock);
    }
    if (!_list.empty())
    {
        path = _list.back();
        _list.pop_back();
        return true;
    }
    return false;
}
