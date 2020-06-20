#pragma once

#include <memory>
#include "forward_shared_lock_guard.hpp"

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

template <typename K, typename V>
class hashmap
{
public:
    using node_ptr_t = std::shared_ptr<hashmap_node<K, V>>;

    node_ptr_t find(const K& key) const
    {
        return search(key);
    }

    V& operator[](const K& key)
    {
        node_ptr_t prev_node = nullptr;
        node_ptr_t node = search(key, prev_node);
        if (node != nullptr)
            return node->get_value();

        // Create new node
        const auto new_node = std::make_shared<hashmap_node<K, V>>(key, V());
        if (prev_node == nullptr)
            data_[hash(key)] = new_node;
        else
            prev_node->set_next(new_node);
        return new_node->get_value();
    }

    void insert(const K& key, const V& value)
    {
        node_ptr_t prev_node = nullptr;
        node_ptr_t node = search(key, prev_node);
        if (node != nullptr)
            return;

        // Create new node
        const auto new_node = std::make_shared<hashmap_node<K, V>>(key, value);
        if (prev_node == nullptr)
            data_[hash(key)] = new_node;
        else
            prev_node->set_next(new_node);
    }

    void erase(const K& key)
    {
        node_ptr_t prev_node = nullptr;
        node_ptr_t node = search(key, prev_node);
        if (node == nullptr)
            return;

        if (prev_node == nullptr)
            data_[hash(key)] = node->get_next();
        else
            prev_node->set_next(node->get_next());
    }
private:
    inline unsigned long hash(const K& key) const
    {
        return std::hash<K>{}(key) % data_.size();
    }

    node_ptr_t search(const K& key, node_ptr_t& prev_node) const
    {
        prev_node = nullptr;
        node_ptr_t node = data_.at(hash(key));
        if (node == nullptr)
            return nullptr;

        forward_shared_lock_guard lock(node->get_mutex());
        while (node != nullptr)
        {
            if (node->get_key() == key)
                return node;

            prev_node = node;
            node = node->get_next();

            if (node)
                lock.forward(node->get_mutex());
        }
        return nullptr;
    }

    node_ptr_t search(const K& key) const
    {
        node_ptr_t node = data_.at(hash(key));
        if (node == nullptr)
            return nullptr;

        forward_shared_lock_guard lock(node->get_mutex());
        while (node != nullptr)
        {
            if (node->get_key() == key)
                return node;

            node = node->get_next();

            if (node)
                lock.forward(node->get_mutex());
        }
        return nullptr;
    }

    std::array<node_ptr_t, 4096> data_;
};