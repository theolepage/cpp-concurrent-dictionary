#pragma once

#include <vector>

#include "IDictionary.hpp"

class Node
{
public:
    Node(char letter)
        : letter_(letter)
        , children_(std::vector<Node>())
        , books_(std::vector<int>())
    {}

    void add_child(char letter)
    {
        children_.emplace_back(Node(letter));
    }

    void add_book(int book)
    {
        is_leaf = true;
        books_.emplace_back(book);
    }

    void remove_book(int book)
    {
        if (is_leaf)
            for (auto i = books_.begin(); i != books_.end(); i++)
                if (*i == book)
                    books_.erase(i);
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

    const std::vector<int>& getBooks()
    {
        return books_;
    }

    const std::vector<int>& getBooks() const
    {
        return books_;
    }

    const bool& isLeaf() const
    {
        return is_leaf;
    }

private:
    char letter_;
    std::vector<Node> children_;
    std::vector<int> books_;
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
    std::optional<std::vector<int>> _search_word(const char* word) const;
    std::optional<std::vector<int>> r_search_word(const char* word,
                                                  const Node& node) const;
    void _add_word(const char* word, int book);
    void r_add_word(const char* word, int book, Node& node);
    void _remove(int document_id, Node& node);

    Node root_;
};