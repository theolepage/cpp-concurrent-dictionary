#pragma once

#include <utility>
#include <vector>
#include <shared_mutex>
#include <memory>

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
        auto j = 0;
        for (auto i = books.begin(); i != books.end() && j < r.m_count; i++)
            r.m_matched[j++] = (*i);
    }

    mutable std::shared_mutex m;
    book_set books;
};

class Node
{
public:
    using book_set = std::unordered_set<int>;

    Node() = default;

    explicit Node(char letter)
        :leaf(nullptr)
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
            leaf = std::make_unique<Leaf>();
            is_leaf = true;
        }

        leaf->insert(book);
    }

    void remove_book(int book)
    {
        leaf->erase(book);
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
            leaf->read_books(r);
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

    void _init_leafs(std::vector<Node*>& leafs)
    {
        if (is_leaf)
            leafs.emplace_back(this);
        for (int i = 0; i < NB_LETTERS; ++i)
        {
            if (children_[i] != nullptr)
                children_[i]->_init_leafs(leafs);
        }
    }

private:

    std::unique_ptr<Leaf> leaf;
    char letter_;
    std::unique_ptr<Node> children_[NB_LETTERS];
    bool is_leaf = false;
};

class Tree_Dictionary : public IReversedDictionary
{
public:
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

private:
    void _init(const dictionary_t& d);
    void _search_word(const char* word, result_t& r) const;
    //void _add_word(const char* word, int book);
    void _remove(int document_id);

    Node root_;
    std::vector<Node*> leafs;
};