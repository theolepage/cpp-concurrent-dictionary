#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <functional>

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
};

template <typename K, typename V>
class hashmap
{
public:
    hashmap(unsigned long size)
        : data_(size)
    {}

    hashmap()
        : hashmap(64)
    {}

    std::optional<V> find(const K& key) const
    {
        std::shared_ptr<hashmap_node<K, V>> node = data_.at(hash(key));
        while (node != nullptr)
        {
            if (node->get_key() == key)
                return node->get_value();
            node = node->get_next();
        }
        return std::nullopt;
    }

    V& operator[](const K& key)
    {
        std::shared_ptr<hashmap_node<K, V>> node = data_.at(hash(key));
        while (node != nullptr)
        {
            if (node->get_key() == key)
                return node->get_value();
            node = node->get_next();
        }
        return data_.at(0)->get_value();
    }

    void insert(const K& key, const V& value)
    {
        unsigned long index = hash(key);
        std::shared_ptr<hashmap_node<K, V>> prev_node = nullptr;
        std::shared_ptr<hashmap_node<K, V>> node = data_.at(index);

        // Find new node position
        while (node != nullptr && node->get_key() != key)
        {
            prev_node = node;
            node = node->get_next();
        }

        if (node != nullptr)
            return;

        // Create new node
        auto new_node = std::make_shared<hashmap_node<K, V>>(key, value);
        if (prev_node == nullptr)
            data_[index] = new_node;
        else
            prev_node->set_next(new_node);
    }

    void erase(const K& key)
    {
        unsigned long index = hash(key);
        std::shared_ptr<hashmap_node<K, V>> prev_node = nullptr;
        std::shared_ptr<hashmap_node<K, V>> node = data_.at(index);

        // Find node to remove position
        while (node != nullptr && node->get_key() != key)
        {
            prev_node = node;
            node = node->get_next();
        }

        if (node == nullptr)
            return;

        if (prev_node == nullptr)
            data_[index] = node->get_next();
        else
            prev_node->set_next(node->get_next());
    }
private:
    unsigned long hash(const K& key) const
    {
        return static_cast<unsigned long>(key) % data_.size();
    }

    std::vector<std::shared_ptr<hashmap_node<K, V>>> data_;
};