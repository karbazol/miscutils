#pragma once
class paths_buffer
{
private:
    volatile bool _finished;
    boost::mutex _mtx;
    std::list<boost::filesystem::path> _list;
    boost::condition_variable _cond;
public:
    paths_buffer() : _finished(false), _mtx(), _list(), _cond() {}
    ~paths_buffer();
    void push(const boost::filesystem::path& path);
    bool pop(boost::filesystem::path& path);
    void finish() { _finished = true; _cond.notify_all(); }
};

