#pragma once

#include <shared_mutex>
#include <iostream>

class forward_shared_lock_guard
{
public:
    forward_shared_lock_guard(std::shared_mutex& mutex)
        : mutex_(&mutex)
    {
        mutex_->lock_shared();
    }

    ~forward_shared_lock_guard()
    {
        mutex_->unlock_shared();
    }

    void forward(std::shared_mutex& next)
    {
        next.lock_shared();
        mutex_->unlock_shared();
        mutex_ = &next;
    }

private:
    std::shared_mutex* mutex_;
};