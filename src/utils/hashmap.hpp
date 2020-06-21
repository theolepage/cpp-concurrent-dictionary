#pragma once

#include <memory>
#include <shared_mutex>
#include <iostream>

template <typename K, typename V>
class hashmap_node
{
public:
    hashmap_node(const K& key, const V& value)
        : key_(key), value_(value)
    {}

    K get_key() const
    {
        return key_;
    }

    V& get_value()
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

    void set_key(const K& key)
    {
        key_ = key;
    }

    void set_value(const V& value)
    {
        value_ = value;
    }

    void set_next(std::shared_ptr<hashmap_node> next)
    {
        next_ = next;
    }

private:
    K key_;
    V value_;
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
    forward_lock_guard(lock_type type, node_ptr_t node)
        : type_(type), mutex_(&node->get_mutex())
    {
        // Lock mutex_
        if      (type_ == lock_type::SHARED)    mutex_->lock_shared();
        else if (type_ == lock_type::EXCLUSIVE) mutex_->lock();
    }

    ~forward_lock_guard()
    {
        if (mutex_ == nullptr)
            return;

        // Unlock mutex_
        if      (type_ == lock_type::SHARED)    mutex_->unlock_shared();
        else if (type_ == lock_type::EXCLUSIVE) mutex_->unlock();
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

        mutex_ = next_mutex;
    }

private:
    lock_type type_;
    std::shared_mutex* mutex_;
};

template <typename K, typename V>
class hashmap
{
    using node_t = hashmap_node<K, V>;
    using node_ptr_t = std::shared_ptr<node_t>;

public:
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

    node_ptr_t find(const K& key) const
    {
        // debug_info();

        node_ptr_t node = data_.at(hash(key));
        if (node == nullptr)
            return nullptr;

        forward_lock_guard<K, V> lock(lock_type::SHARED, node);

        while (node != nullptr && node->get_key() != key)
        {
            node = node->get_next();
            if (node)
                lock.forward(node);
        }
        return node;
    }

    V& operator[](const K& key)
    {
        unsigned long index = hash(key);
        node_ptr_t node = data_.at(index);

        if (node == nullptr)
        {
            const auto new_node = std::make_shared<node_t>(key, V());
            data_[index] = new_node;
            return new_node->get_value();
        }

        forward_lock_guard<K, V> lock(lock_type::EXCLUSIVE, node);
        node_ptr_t prev_node = nullptr;

        while (node != nullptr && node->get_key() != key)
        {
            prev_node = node;
            node = node->get_next();
            if (node)
                lock.forward(node);
        }

        if (node != nullptr)
            return node->get_value();

        // Create new node
        // prev_node is still locked
        const auto new_node = std::make_shared<node_t>(key, V());
        prev_node->set_next(new_node);
        return new_node->get_value();
    }

    void remove(const K& key)
    {
        unsigned long index = hash(key);
        node_ptr_t node = data_.at(index);

        if (node == nullptr)
            return;
        if (node->get_key() == key)
        {
            data_[index] = node->get_next();
            return;
        }

        forward_lock_guard<K, V> lock(lock_type::EXCLUSIVE, node);
        node_ptr_t prev_node = nullptr;

        while (node != nullptr && node->get_key() != key)
        {
            prev_node = node;
            node = node->get_next();
            if (node)
                lock.forward_remove(node, key);
        }

        if (node == nullptr)
            return;

        // Remove node
        // prev_node and node are locked
        prev_node->set_next(node->get_next());
        prev_node->get_mutex().unlock();
    }
private:
    inline unsigned long hash(const K& key) const
    {
        return std::hash<K>{}(key) % data_.size();
    }

    std::array<node_ptr_t, 4096> data_;
};