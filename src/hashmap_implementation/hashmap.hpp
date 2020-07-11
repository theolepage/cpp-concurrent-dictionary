#pragma once

#include <memory>
#include <shared_mutex>
#include <iostream>

template <typename K, typename V>
class hashmap_node
{
public:
    hashmap_node()
    {}

    hashmap_node(const K& key, std::shared_ptr<V> value)
        : key_(key), value_(value)
    {}

    K get_key() const
    {
        return key_;
    }

    std::shared_ptr<V> get_value() const
    {
        return value_;
    }

    std::shared_ptr<hashmap_node> get_next() const
    {
        return next_;
    }

    std::shared_mutex& get_mutex()
    {
        return mutex_;
    }

    void set_next(std::shared_ptr<hashmap_node> next)
    {
        next_ = next;
    }

private:
    K key_;
    std::shared_ptr<V> value_;
    std::shared_ptr<hashmap_node> next_;
    mutable std::shared_mutex mutex_;
};

enum class lock_type
{
    SHARED = 0,
    EXCLUSIVE = 1 // unique
};

template <typename K, typename V>
class forward_lock_guard
{
    using node_t = hashmap_node<K, V>;
    using node_ptr_t = std::shared_ptr<node_t>;

public:
    forward_lock_guard(lock_type type, node_ptr_t node, bool do_not_unlock = false)
        : type_(type)
        , mutex_(&node->get_mutex())
        , prev_mutex_(nullptr)
        , do_not_unlock_(do_not_unlock)
    {
        // Lock mutex_
        if      (type_ == lock_type::SHARED)    mutex_->lock_shared();
        else if (type_ == lock_type::EXCLUSIVE) mutex_->lock();
    }

    ~forward_lock_guard()
    {
        if (do_not_unlock_)
            return;

        if (mutex_ != nullptr)
        {
            // Unlock mutex_
            if      (type_ == lock_type::SHARED)    mutex_->unlock_shared();
            else if (type_ == lock_type::EXCLUSIVE) mutex_->unlock();
        }

        if (prev_mutex_ != nullptr)
        {
            // Unlock prev_mutex_
            if      (type_ == lock_type::SHARED)    prev_mutex_->unlock_shared();
            else if (type_ == lock_type::EXCLUSIVE) prev_mutex_->unlock();
        }
    }

    void set_do_not_unlock(bool val)
    {
        do_not_unlock_ = val;
    }

    void forward(node_ptr_t next)
    {
        auto next_mutex = &next->get_mutex();

        // Lock next
        if      (type_ == lock_type::SHARED)    next_mutex->lock_shared();
        else if (type_ == lock_type::EXCLUSIVE) next_mutex->lock();

        // Unlock mutex_
        if      (type_ == lock_type::SHARED)    mutex_->unlock_shared();
        else if (type_ == lock_type::EXCLUSIVE) mutex_->unlock();

        mutex_ = next_mutex;
    }

    void forward_remove(node_ptr_t next, const K& key)
    {
        auto next_mutex = &next->get_mutex();

        // Lock next
        if      (type_ == lock_type::SHARED)    next_mutex->lock_shared();
        else if (type_ == lock_type::EXCLUSIVE) next_mutex->lock();

        if (next->get_key() != key)
        {
            // Unlock mutex_
            if      (type_ == lock_type::SHARED)    mutex_->unlock_shared();
            else if (type_ == lock_type::EXCLUSIVE) mutex_->unlock();
        }
        else
        {
            prev_mutex_ = mutex_;
            mutex_ = next_mutex;
        }

        mutex_ = next_mutex;
    }

private:
    lock_type type_;
    std::shared_mutex* mutex_;
    std::shared_mutex* prev_mutex_;
    bool do_not_unlock_;
};

template <typename K, typename V>
class hashmap
{
    using node_t = hashmap_node<K, V>;
    using node_ptr_t = std::shared_ptr<node_t>;

public:
    hashmap()
    {
        for (size_t i = 0; i < data_.size(); i++)
            data_[i] = std::make_shared<node_t>();
    }

    void debug_info() const
    {
        for (size_t i = 0; i < data_.size(); i++)
        {
            auto node = data_.at(i);
            int count = 0;
            while (node != nullptr)
            {
                node = node->get_next();
                count ++;
            }
            if (count >= 0)
                std::cout << "Bucket " << i << ": "
                          << count << " nodes" << std::endl;
        }
        exit(0);
    }

    std::optional<V> find_value_copy(const K& key) const
    {
        node_ptr_t node = data_.at(hash(key));
        forward_lock_guard<K, V> lock(lock_type::SHARED, node);

        // Skip sentinel node
        node = node->get_next();
        if (node) lock.forward(node);

        while (node != nullptr && node->get_key() != key)
        {
            node = node->get_next();
            if (node) lock.forward(node);
        }

        if (node == nullptr)
            return std::nullopt;

        return *(node->get_value());
    }

    node_ptr_t find_node_locked(const K& key)
    {
        unsigned long index = hash(key);
        node_ptr_t node = data_.at(index);
        forward_lock_guard<K, V> lock(lock_type::EXCLUSIVE, node, true);

        // Skip sentinel node
        node = node->get_next();
        if (node) lock.forward(node);

        while (node != nullptr && node->get_key() != key)
        {
            node = node->get_next();
            if (node) lock.forward(node);
        }

        if (node == nullptr)
            lock.set_do_not_unlock(false);

        return node;
    }

    node_ptr_t find_node_unlocked(const K& key)
    {
        unsigned long index = hash(key);
        node_ptr_t node = data_.at(index);
        forward_lock_guard<K, V> lock(lock_type::SHARED, node);

        // Skip sentinel node
        node = node->get_next();
        if (node) lock.forward(node);

        while (node != nullptr && node->get_key() != key)
        {
            node = node->get_next();
            if (node) lock.forward(node);
        }

        return node;
    }

    void remove(const K& key)
    {
        unsigned long index = hash(key);
        node_ptr_t node = data_.at(index);
        forward_lock_guard<K, V> lock(lock_type::EXCLUSIVE, node);

        // Skip sentinel node
        node_ptr_t prev_node = node;
        node = node->get_next();
        if (node) lock.forward(node);

        while (node != nullptr && node->get_key() != key)
        {
            prev_node = node;
            node = node->get_next();
            if (node) lock.forward_remove(node, key);
        }

        if (node == nullptr)
            return;

        // Remove node
        // At this stage prev_node and node are locked because
        // we called forward_remove
        prev_node->set_next(node->get_next());
    }

    template <typename T>
    void insert_value(const K& key, const T& value)
    {
        unsigned long index = hash(key);

        node_ptr_t node = data_.at(index);
        forward_lock_guard<K, V> lock(lock_type::EXCLUSIVE, node);

        // Skip sentinel node
        node_ptr_t prev_node = node;
        node = node->get_next();
        if (node) lock.forward(node);

        while (node != nullptr && node->get_key() != key)
        {
            prev_node = node;
            node = node->get_next();
            if (node) lock.forward(node);
        }

        if (node != nullptr)
        {
            node->get_value()->emplace_back(value);
            return;
        }

        // Create new node
        const auto new_node = std::make_shared<node_t>(key, std::make_shared<V>());
        new_node->get_value()->emplace_back(value);
        prev_node->set_next(new_node);
    }

    node_ptr_t create_node(const K& key)
    {
        unsigned long index = hash(key);

        node_ptr_t sentinel = data_.at(index);
        forward_lock_guard<K, V> lock_sentinel(lock_type::EXCLUSIVE, sentinel);

        // Create new node
        const auto new_node = std::make_shared<node_t>(key, std::make_shared<V>());
        forward_lock_guard<K, V> lock(lock_type::EXCLUSIVE, new_node, true);
        new_node->set_next(sentinel->get_next());
        sentinel->set_next(new_node);

        return new_node;
    }

    template <typename T>
    void remove_value(const K& key, const T& value)
    {
        unsigned long index = hash(key);

        node_ptr_t node = data_.at(index);
        forward_lock_guard<K, V> lock(lock_type::EXCLUSIVE, node);

        // Skip sentinel node
        node = node->get_next();
        if (node) lock.forward(node);

        while (node != nullptr && node->get_key() != key)
        {
            node = node->get_next();
            if (node) lock.forward(node);
        }

        if (node != nullptr)
        {
            auto v = node->get_value();
            v->erase(std::remove(v->begin(), v->end(), value), v->end());
        }
    }
private:
    inline unsigned long hash(const K& key) const
    {
        return std::hash<K>{}(key) % data_.size();
    }

    std::array<node_ptr_t, 8192> data_;
};