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
}

void Tree_Dictionary::_add_word(const char* word, int book)
{
    Node* cur = &root_;
    while (*word != '\0')
    {
        auto flag = false;
        for (Node& child : cur->getChildren())
            if (child == *word)
            {
                cur = &child;
                word++;
                flag = true;
                break;
            }
        if (!flag)
        {
            cur->add_child(*word);
            cur = &(cur->getChildren().back());
            word++;
        }
    }

    cur->add_book(book);
}

const Node::book_set* Tree_Dictionary::_search_word(const char* word) const
{
    const Node* cur = &root_;
    while (*word != '\0')
    {
        auto flag = false;
        for (const Node& child : cur->getChildren())
            if (child == *word)
            {
                cur = &child;
                word++;
                flag = true;
                break;
            }
        if (!flag)
            return nullptr;
    }

    return cur->getBooks();
}

result_t Tree_Dictionary::search(const char* word) const
{
    result_t r;
    auto res = _search_word(word);
    if (!res)
        r.m_count = 0;
    else
    {
        r.m_count = std::min(int(res->size()), MAX_RESULT_COUNT);
        auto j = 0;
        for (auto i = res->begin(); i != res->end() && j < r.m_count; i++)
            r.m_matched[j++] = (*i).second;
    }
    return r;
}
void Tree_Dictionary::insert(int document_id, gsl::span<const char*> text)
{
    for (auto word : text)
        _add_word(word, document_id);
}

void Tree_Dictionary::_remove(int document_id, Node& node)
{
    node.remove_book(document_id);
    for (auto& child : node.getChildren())
        _remove(document_id, child);
}

void Tree_Dictionary::remove(int document_id)
{
    _remove(document_id, root_);
}