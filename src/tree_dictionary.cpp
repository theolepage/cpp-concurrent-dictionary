#include "tree_dictionary.hpp"

#include <algorithm>

#include "IDictionary.hpp"

Tree_Dictionary::Tree_Dictionary()
    : root_(Node('\0'))
{}

Tree_Dictionary::Tree_Dictionary(const dictionary_t& d)
    : root_(Node('\0'))
{
    this->_init(d);
}

void Tree_Dictionary::init(const dictionary_t& d)
{
    this->_init(d);
}

void Tree_Dictionary::_init(const dictionary_t& d)
{
    for (const auto& [book, words] : d)
        for (const auto& word : words)
            _add_word(word, book);
    root_._init_leafs(leafs);
}

void Tree_Dictionary::_add_word(const char* word, int book)
{
    Node* cur = &root_;
    while (*word != '\0')
    {
        if ((*cur)[*word] == nullptr)
            cur->add_child(*word);
        cur = (*cur)[*word];
        ++word;
    }

    cur->add_book(book);
}

void Tree_Dictionary::_search_word(const char* word, result_t& r) const
{
    const Node* cur = &root_;
    while (*word != '\0')
    {
        cur = (*cur)[*word];
        ++word;
        if (cur == nullptr)
        {
            r.m_count = 0;
            return;
        }
    }

    cur->read_books(r);
}

result_t Tree_Dictionary::search(const char* word) const
{
    result_t r;
    _search_word(word, r);
    return r;
}

void Tree_Dictionary::insert(int document_id, gsl::span<const char*> text)
{
    for (auto word : text)
        _add_word(word, document_id);
}

void Tree_Dictionary::_remove(int document_id)
{
    for (Node* leaf : leafs)
        leaf->remove_book(document_id);
}

void Tree_Dictionary::remove(int document_id)
{
    _remove(document_id);
}