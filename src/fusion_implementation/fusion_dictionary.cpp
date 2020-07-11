#include "fusion_dictionary.hpp"

#include <algorithm>
#include <unordered_set>

#include "../IDictionary.hpp"

Fusion_Dictionary::Fusion_Dictionary() : root_(Node('\0'))
{}

Fusion_Dictionary::Fusion_Dictionary(const dictionary_t& d) : root_(Node('\0'))
{
    this->_init(d);
}

void Fusion_Dictionary::init(const dictionary_t& d)
{
    this->_init(d);
}

void Fusion_Dictionary::_init(const dictionary_t& d)
{
    for (const auto& [book, words] : d)
        for (const auto& word : words)
            _add_word(word, book);
    root_._init_Sub_nodes(book_Sub_nodes_own_);
}

void Fusion_Dictionary::_add_word(const char* word, int book,
                                std::shared_ptr<std::vector<std::shared_ptr<Sub_node>>> vect)
{
    Node* cur = &root_;
    while (*word != '\0')
    {
        if ((*cur)[*word] == nullptr)
        {
            // If 2 threads enters in the if at the same time
            // Can't use std::call_once cause this function maybe called
            // multiple time
            std::lock_guard(cur->m);
            if ((*cur)[*word] == nullptr)
                cur->add_child(*word);
        }
        cur = (*cur)[*word];
        ++word;
    }

    cur->add_book(book);
    vect->emplace_back(cur->get_Sub_node());
}

void Fusion_Dictionary::_add_word(const char* word, const int book)
{
    Node* cur = &root_;
    // Here data race on array but on the array itself, since we are accessing the cells in a thread-safe manner
    // There is a data race on the array but not on the cell, the programm is thus working

    while (*word != '\0')
    {
        if ((*cur)[*word] == nullptr)
        {
            // If 2 threads enters in the if at the same time, thread-safe
            // Can't use std::call_once cause this function maybe called
            // multiple times
            std::lock_guard(cur->m);
            if ((*cur)[*word] == nullptr)
                cur->add_child(*word);
        }
        cur = (*cur)[*word];
        ++word;
    }

    cur->add_book(book);
}

void Fusion_Dictionary::_search_word(const char* word, result_t& r) const
{
    const Node* cur = &root_;
    const int len = strlen(word);
    for (int i = 0; i < len; ++i)
    {
        cur = (*cur)[word[i]];
        if (cur == nullptr)
        {
            r.m_count = 0;
            return;
        }
    }

    cur->read_books(r);
}

result_t Fusion_Dictionary::search(const char* word) const
{
    result_t r;
    _search_word(word, r);
    return r;
}

void Fusion_Dictionary::insert(int document_id, gsl::span<const char*> text)
{
    if (book_Sub_nodes_own_.find_node_unlocked(document_id) != nullptr)
        return;

    auto s = std::unordered_set<const char*>();
        for (const char* word : text)
            s.emplace(word);

    auto node = book_Sub_nodes_own_.create_node(document_id);

    for (const char* word : s)
        _add_word(word, document_id, node->get_value());

    node->get_mutex().unlock();
}

void Fusion_Dictionary::_remove(int document_id)
{
    auto node = book_Sub_nodes_own_.find_node_locked(document_id);
    if (node == nullptr)
        return;

    auto value = node->get_value();
    for (size_t i = 0; i < value->size(); ++i)
        value->at(i)->erase(document_id);

    node->get_mutex().unlock();
    book_Sub_nodes_own_.remove(document_id);
}

void Fusion_Dictionary::remove(int document_id)
{
    _remove(document_id);
}
