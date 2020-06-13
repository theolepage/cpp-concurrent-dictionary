#pragma once

#include <utility>
#include <vector>

#include "IDictionary.hpp"
#include "tbb/concurrent_hash_map.h"
#include "tbb/tbb.h"

class Node
{
public:
    using book_set =
        tbb::concurrent_hash_map<int, int, tbb::tbb_hash_compare<int>,
                                 tbb::tbb_allocator<std::pair<int, int>>>;

    Node(char letter)
        : letter_(letter)
        , children_(std::vector<Node>())
        , books_(book_set())
    {}

    void add_child(char letter)
    {
        children_.emplace_back(Node(letter));
    }

    void add_book(int book)
    {
        is_leaf = true;
        books_.insert(std::make_pair(book, book));
    }

    void remove_book(int book)
    {
        if (is_leaf)
            books_.erase(book);
    }

    bool operator==(char l) const
    {
        return letter_ == l;
    }

    std::vector<Node>& getChildren()
    {
        return children_;
    }

    const std::vector<Node>& getChildren() const
    {
        return children_;
    }

    const book_set* getBooks()
    {
        return &books_;
    }

    const book_set* getBooks() const
    {
        return &books_;
    }

    const bool& isLeaf() const
    {
        return is_leaf;
    }

private:
    char letter_;
    std::vector<Node> children_;
    book_set books_;
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

private:
    void _init(const dictionary_t& d);
    const Node::book_set* _search_word(const char* word) const;
    void _add_word(const char* word, int book);
    void _remove(int document_id, Node& node);

    Node root_;
};