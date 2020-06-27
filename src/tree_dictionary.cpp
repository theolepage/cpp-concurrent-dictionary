#include "tree_dictionary.hpp"

#include <algorithm>
#include <tbb/tbb.h>
#include <iostream>

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
    root_._init_leafs(book_leafs_);
}

void Tree_Dictionary::_add_word(const char* word, int book, std::vector<Leaf::book_set*>& vect)
{
    Node* cur = &root_;
    while (*word != '\0')
    {
        if ((*cur)[*word] == nullptr)
        {
            // If 2 threads enters in the if at the same time
            // Can't use std::call_once cause this function maybe called multiple time
            std::lock_guard(cur->m);
            if ((*cur)[*word] == nullptr)
                cur->add_child(*word);
        }
        cur = (*cur)[*word];
        ++word;
    }

    cur->add_book(book);
    vect.emplace_back(&(cur->get_leaf()->books));
}

void Tree_Dictionary::_add_word(const char* word, const int book)
{
    Node* cur = &root_;
    while (*word != '\0')
    {
        if ((*cur)[*word] == nullptr)
        {
            // If 2 threads enters in the if at the same time, thread-safe
            // Can't use std::call_once cause this function maybe called multiple times
            std::lock_guard(cur->m);
            if ((*cur)[*word] == nullptr)
                cur->add_child(*word);
        }
        cur = (*cur)[*word];
        ++word;
    }

    cur->add_book(book);
}

void Tree_Dictionary::_search_word(const char* word, result_t& r) const
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

result_t Tree_Dictionary::search(const char* word) const
{
    result_t r;
    _search_word(word, r);
    return r;
}
/*
class apply_add_word
{
private:
    const gsl::span<const char*>& list_;
    Tree_Dictionary* dico_;
    const int document_id_;
    Tree_Dictionary::delete_map::accessor& acc_;

public:
    void operator()(const tbb::blocked_range<size_t>& r) const
    {
        for (size_t i = r.begin(); i != r.end(); ++i)
        {
            dico_->_add_word(list_[i], document_id_, acc_->second);
        }
    }

    apply_add_word(const gsl::span<const char*>& list,
                    Tree_Dictionary* dico,
                    const int document_id,
                    Tree_Dictionary::delete_map::accessor& acc) :
                    list_(list), dico_(dico), document_id_(document_id), acc_(acc)
                    {}
};
*/
void Tree_Dictionary::insert(int document_id, gsl::span<const char*> text)
{
    // TODO change it to use their concurrent hashmap
    delete_map::accessor a;
    if (!book_leafs_.find(a, document_id))
    {
        book_leafs_.insert(
                    std::make_pair(
                        document_id, std::vector<Leaf::book_set*>{}));
        book_leafs_.find(a, document_id); // Fill a
    }

    for (const char* word : text)
    {
        _add_word(word, document_id, a->second);
    }

    //parallel_for(tbb::blocked_range<size_t>(0,text.size()),
    //    apply_add_word(text, this, document_id, a));
}


void Tree_Dictionary::_remove(int document_id)
{
    delete_map::accessor a;
    if (book_leafs_.find(a, document_id))
    {
        if (a->second[0] == nullptr)
            return;

        for (size_t i = 0; i < a->second.size(); ++i)
        {
            a->second[i]->erase(document_id);
            a->second[i] = nullptr;
        }
    }
}

void Tree_Dictionary::remove(int document_id)
{
    _remove(document_id);
}