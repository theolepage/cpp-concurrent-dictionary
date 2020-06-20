#include "hashmap_dictionary.hpp"
#include <algorithm>
#include <iostream>

hashmap_dictionary::hashmap_dictionary(const dictionary_t& d)
{
    this->_init(d);
}

void hashmap_dictionary::init(const dictionary_t& d)
{
    this->_init(d);
}

void hashmap_dictionary::_init(const dictionary_t& d)
{
    for (auto&& [id, text] : d)
    {
        auto& document = m_dico[id];
        for (auto&& word : text)
        {
            document.insert(word);
            m_rev_dico[word].insert(id);
        }
    }
}

result_t hashmap_dictionary::search(const char* word) const
{
    std::lock_guard l(m);

    result_t r;

    const auto itemptr = m_rev_dico.find(word);
    if (itemptr == nullptr)
        return r;

    auto& item = itemptr->get_value();
    r.m_count = std::min(int(item.size()), MAX_RESULT_COUNT);
    std::copy_n(item.begin(), r.m_count, r.m_matched);
    return r;
}

void hashmap_dictionary::insert(int document_id, gsl::span<const char*> text)
{
    std::lock_guard l(m);

    auto& document = m_dico[document_id];
    for (auto&& word : text)
    {
        document.insert(word);
        m_rev_dico[word].insert(document_id);
    }
}

void hashmap_dictionary::remove(int document_id)
{
    std::lock_guard l(m);

    const auto entry = m_dico.find(document_id);
    if (entry == nullptr)
        return;

    for (const auto& w : entry->get_value())
        m_rev_dico[w].erase(document_id);
    m_dico.erase(document_id);
}