#pragma once

#include <utility>
#include <vector>
#include <shared_mutex>
#include <memory>
#include <tbb/concurrent_hash_map.h>

#include "IDictionary.hpp"
#include <unordered_set>

static constexpr auto NB_LETTERS = 26;

struct Leaf
{
    using book_set = std::unordered_set<int>;

    void insert(int book)
    {
        std::unique_lock l(m);
        books.insert(book);
    }

    void erase(int book)
    {
        std::unique_lock l(m);
        books.erase(book);
    }

    void read_books(result_t& r)
    {
        std::shared_lock l(m);
        r.m_count = std::min(int(books.size()), MAX_RESULT_COUNT);
        std::copy_n(books.begin(), r.m_count, r.m_matched);
    }

    mutable std::shared_mutex m;
    book_set books;
};

class Node
{
public:
    using book_set = std::unordered_set<int>;
    using delete_map = tbb::concurrent_hash_map<int, std::vector<Leaf::book_set*>>;

    Node() = default;

    explicit Node(char letter)
        : leaf_(nullptr)
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
        if (!is_leaf)
        {
            leaf_ = std::make_unique<Leaf>();
            is_leaf = true;
        }

        leaf_->insert(book);
    }

    void remove_book(int book)
    {
        leaf_->erase(book);
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
        if (is_leaf)
        {
            leaf_->read_books(r);
        }
        else
        {
            r.m_count = 0;
        }
    }

    bool isLeaf(void) const
    {
        return is_leaf;
    }

    void _init_leafs(delete_map& book_leafs)
    {
        if (is_leaf)
        {
            for (const int book : leaf_->books)
            {
                delete_map::accessor a;
                if (book_leafs.find(a, book))
                    a->second.emplace_back(&(leaf_->books));
                else
                {
                    book_leafs.insert(
                        std::make_pair(
                            book, std::vector<Leaf::book_set*>{&(leaf_->books)}));
                }
            }
        }

        for (int i = 0; i < NB_LETTERS; ++i)
        {
            if (children_[i] != nullptr)
                children_[i]->_init_leafs(book_leafs);
        }
    }

    Leaf* get_leaf()
    {
        return leaf_.get();
    }

    // TODO use a getter
    mutable std::mutex m;                           // To lock when adding a new word
private:

    std::unique_ptr<Leaf> leaf_;                    // Pointer to leaf if leaf
    char letter_;                                   // Letter of node
    std::unique_ptr<Node> children_[NB_LETTERS];    // Array size 26 of pointer to child nodes
    bool is_leaf = false;                           // To know if leaf
};

class Tree_Dictionary : public IReversedDictionary
{
public:
    using delete_map = tbb::concurrent_hash_map<int, std::vector<Leaf::book_set*>>;
    Tree_Dictionary();
    Tree_Dictionary(const dictionary_t& init);

    template <class Iterator>
    Tree_Dictionary(Iterator begin, Iterator end)
    {
        _init({begin, end});
    }

    virtual void init(const dictionary_t& d) final;
    virtual result_t search(const char* word) const final;
    virtual void insert(int document_id, gsl::span<const char*> text) final;
    virtual void remove(int document_id) final;
    void _add_word(const char* word, int book);
    void _add_word(const char* word, int book, std::vector<Leaf::book_set*>& vect);

    // TODO private
    Node root_;
    delete_map book_leafs_;

private:
    void _init(const dictionary_t& d);
    void _search_word(const char* word, result_t& r) const;
    //void _add_word(const char* word, int book);
    void _remove(int document_id);

};