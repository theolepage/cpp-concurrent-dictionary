#pragma once

#include <tbb/tbb.h>
#include <vector>
#include <iostream>

class Thread_Pool
{
public:
    Thread_Pool(/*int pool_size*/) : /*pools_(pool_size),*/ counter_(0) {}

    ~Thread_Pool()
    {
        pools_.wait();
    }

    ~Thread_Pool()
    {
        for (size_t i = 0; i < pools_.size(); i++)
            pools_[i].wait();
    }

    template <typename T>
    void push(T&& t)
    {
        pools_/*[counter_++ % pools_.size()]*/.run(t);
    }

private:
    /*std::vector<*/tbb::task_group/*>*/ pools_;
    int counter_;
};