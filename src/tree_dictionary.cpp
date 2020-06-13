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

void Tree_Dictionary::r_add_word(const char* word, int book, Node& node)
{
    if (*word == '\0')
        return node.add_book(book);
    for (auto& child : node.getChildren())
        if (child == *word)
            r_add_word(word + 1, book, child);
    node.add_child(*word);
    r_add_word(word + 1, book, node.getChildren().back());
}

void Tree_Dictionary::_add_word(const char* word, int book)
{
    r_add_word(word, book, root_);
}

std::optional<Node::book_set>
Tree_Dictionary::r_search_word(const char* word, const Node& node) const
{
    if (*word == '\0')
        return std::make_optional(node.getBooks());
    for (auto& child : node.getChildren())
        if (child == *word)
            return r_search_word(word + 1, child);
    return std::nullopt;
}

std::optional<Node::book_set>
Tree_Dictionary::_search_word(const char* word) const
{
    return r_search_word(word, root_);
}

result_t Tree_Dictionary::search(const char* word) const
{
    result_t r;
    auto res = _search_word(word);
    r.m_count = std::min(int(res.has_value() ? res.value().size() : 0), MAX_RESULT_COUNT);
    auto j = 0;
    for (auto i = res->begin(); i != res->end() && j < MAX_RESULT_COUNT; i++)
        r.m_matched[j++] = (*i).second;
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