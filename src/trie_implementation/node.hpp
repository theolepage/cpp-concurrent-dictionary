#pragma once

#include <memory>
#include <shared_mutex>
#include <tbb/concurrent_hash_map.h>
#include <unordered_set>
#include <utility>
#include <vector>

#include "sub_node.hpp"
#include "../hashmap_implementation/hashmap.hpp"

static constexpr auto NB_LETTERS = 26;

class Node
{
public:
    using delete_map =
        tbb::concurrent_hash_map<int, std::vector<std::shared_ptr<Sub_node>>>;
    using delete_map_own = hashmap<int, std::vector<std::shared_ptr<Sub_node>>>;

    Node() = default;

    explicit Node(char letter)
        : Sub_node_(nullptr)
        , letter_(letter)
    {
        for (int i = 0; i < NB_LETTERS; ++i)
            children_[i] = nullptr;
    }

    void add_child(char letter)
    {
        children_[letter - 'a'] = std::make_unique<Node>(letter);
    }

    void add_book(int book)
    {
        if (!is_Sub_node)
        {
            Sub_node_ = std::make_shared<Sub_node>();
            is_Sub_node = true;
        }

        Sub_node_->insert(book);
    }

    void remove_book(int book)
    {
        Sub_node_->erase(book);
    }

    bool empty(void) const
    {
        return letter_ == 0;
    }

    const Node* operator[](char c) const
    {
        return children_[c - 'a'].get();
    }

    Node* operator[](char c)
    {
        return children_[c - 'a'].get();
    }

    bool operator==(char l) const
    {
        return letter_ == l;
    }

    void read_books(result_t& r) const
    {
        if (is_Sub_node)
        {
            Sub_node_->read_books(r);
        } else
        {
            r.m_count = 0;
        }
    }

    bool isSub_node(void) const
    {
        return is_Sub_node;
    }

    void _init_Sub_nodes(delete_map_own& book_Sub_nodes)
    {
        if (is_Sub_node)
        {
            for (const int book : Sub_node_->books)
            {
                auto node = book_Sub_nodes.find_node_locked(book);
                if (node != nullptr)
                {
                    node->get_value()->emplace_back(Sub_node_);
                    node->get_mutex().unlock();
                }
                else
                {
                    auto new_node = book_Sub_nodes.create_node(book);
                    new_node->get_value()->emplace_back(Sub_node_);
                    new_node->get_mutex().unlock();
                }
            }
        }

        for (int i = 0; i < NB_LETTERS; ++i)
        {
            if (children_[i] != nullptr)
                children_[i]->_init_Sub_nodes(book_Sub_nodes);
        }
    }


    void _init_Sub_nodes(delete_map& book_Sub_nodes)
    {
        if (is_Sub_node)
        {
            for (const int book : Sub_node_->books)
            {
                delete_map::accessor a;
                if (book_Sub_nodes.find(a, book))
                    a->second.emplace_back(Sub_node_);
                else
                {
                    book_Sub_nodes.insert(std::make_pair(
                        book, std::vector<std::shared_ptr<Sub_node>>{Sub_node_}));
                }
            }
        }

        for (int i = 0; i < NB_LETTERS; ++i)
        {
            if (children_[i] != nullptr)
                children_[i]->_init_Sub_nodes(book_Sub_nodes);
        }
    }

    std::shared_ptr<Sub_node> get_Sub_node()
    {
        return Sub_node_;
    }

    mutable std::mutex m; // To lock when adding a new word
private:
    std::shared_ptr<Sub_node> Sub_node_; // Pointer to Sub_node if Sub_node
    char letter_; // Letter of node
    std::unique_ptr<Node> children_[NB_LETTERS]; // Array size 26 of pointer to child nodes
    bool is_Sub_node = false; // To know if Sub_node
};